#include "scan.h"

#include "avr/io.h"
#include "avr/interrupt.h"
#include "avr/sleep.h"

#include "matrix.h"

// determined by basically trial and error
static const uint16_t adc_thresh = 1023UL * 11/12; // about 500mV drop on normal pins, set a threshold at half that = 250mV at 3V VCC
static const uint16_t rst_thresh = 1023UL * 7/8; // about 750mV drop on reset pin

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
    OCR1C = 253; // reset at 253
    // should be about 33*60Hz
    OCR1A = 1; // C can't interrupt in CTC mode so we have it immediately cause
    TIMSK = (1 << OCIE1A); // an interrupt on compare A

    // ADC
    ADCSRA = (1 << ADIE); // enable ADC interrupts
}

static uint8_t current_tick = 0;
static uint8_t adc_line = 0;

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
        PORTB = ADC_PINS;
    } else if(current_tick == 31) {
        // Start ADC conversion
        int_btnmask = 0;
        adc_line = 0;
        ADMUX = adc_line;
        ADCSRA |= (1 << ADEN) | (1 << ADSC);
    }

    current_tick ++;
    if(current_tick == 33) {
        move_framebuf();
        current_tick = 0;
        wants_wake = true;
    }
}

ISR(ADC_vect) {
    // make sure the ADC registers are read in the correct order
    uint8_t adcl = ADCL;
    uint8_t adch = ADCH;
    // hopefully gcc recognizes this idiom as not actually requiring a shift
    uint16_t adcval = ((uint16_t) adch) << 8 | adcl;
    if(adcval < (adc_line == 0 ? rst_thresh : adc_thresh)) int_btnmask |= (1 << adc_line);

    if(adc_line < 3) {
        adc_line++;
        ADMUX = adc_line;
        ADCSRA |= (1 << ADSC);
    } else {
        ADCSRA &= ~(1 << ADEN);
        PORTB = 0;
    }
}