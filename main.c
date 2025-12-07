#include "avr/io.h"

#include "misc.h"

int main() {
    DDRB |= 1<<4;
    while(1) {
        PORTB |= 1<<4 | 1<<3;
        for(uint32_t i = 10; i > 0; i--) NOP;
        PORTB &= ~(1<<4);
        for(uint32_t i = 90; i > 0; i--) NOP;
        PORTB &= ~(1<<3);
        for(uint32_t i = 500; i > 0; i--) NOP;
    }
}