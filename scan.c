#include "scan.h"

#include "avr/io.h"
#include "avr/interrupt.h"
#include "avr/sleep.h"

#include "matrix.h"

// threshold of a 10k - 220k voltage divider
// pullups should be at least 20k, so this gives some noise margin
static const uint16_t adc_thresh = 1023UL * 220UL / 230UL;

volatile uint8_t framebuf[5];
static uint8_t int_framebuf[5];

volatile uint8_t btnmask;
static volatile uint8_t int_btnmask;

volatile bool wants_wake;

static inline void move_framebuf() {
    for(int i = 0; i < 5; i++) {
        int_framebuf[i] = framebuf[i];
    }
    btnmask = int_btnmask;
}

void start_scan() {
    move_framebuf();
    // 8MHz
    TCCR1 = (1 << CTC1) | 0b0101; // divide by 16
    OCR1C = 208; // reset at 208
    OCR1A = 1; // C can't interrupt in CTC mode so we have it immediately cause A
    TIMSK = (1 << OCIE1A); // interrupt on overflow

    // ADC
    ADCSRA = (1 << ADIE); // enable ADC interrupts
}

static uint8_t current_tick = 0;
ISR(TIMER1_COMPA_vect) {
    if(current_tick < 30) { // 0 - 29: show LED
        uint8_t row = current_tick / 6;
        uint8_t col = current_tick % 6;
        if(int_framebuf[row] & (1 << col)) {
            PORTB = matrix[current_tick].high;
            DDRB = matrix[current_tick].high | matrix[current_tick].low;
        } else {
            PORTB = 0;
            DDRB = 0;
        }
    } else if(current_tick == 30) {
        // Precharge pull-up resistors
        DDRB = 0;
        PORTB = BTNA | BTNB | BTNC | BTND;
    } else if(current_tick == 31) {
        // Start ADC conversion
        int_btnmask = 0;
        ADMUX = 0;
        ADCSRA |= (1 << ADEN) | (1 << ADSC);
    }

    current_tick ++;
    if(current_tick == 40) {
        move_framebuf();
        current_tick = 0;
        wants_wake = true;
    }
}

ISR(ADC_vect) {
    uint8_t adc_line = ADMUX;

    // make sure the ADC registers are read in the correct order
    uint8_t adcl = ADCL;
    uint8_t adch = ADCH;
    // hopefully gcc recognizes this idiom as not actually requiring a shift
    uint16_t adcval = ((uint16_t) adch) << 8 | adcl;
    if(adcval < adc_thresh) int_btnmask |= (1 << adc_line);

    if(adc_line < 3) {
        ADMUX++;
        ADCSRA |= (1 << ADSC);
    } else {
        ADCSRA &= ~(1 << ADEN);
    }
}