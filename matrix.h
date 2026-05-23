#ifndef MATRIX_H
#define MATRIX_H

#include "stdint.h"

#define BTNA (1 << 2)
#define BTNC (1 << 3)
#define BTND (1 << 0)
#define BTNB (1 << 1)

#define ADC_PINS (0b00111100)

#define PINK (1 << 2) // adc1
#define BLUE (1 << 1)
#define TEAL (1 << 5) // reset, adc0
#define YELLOW (1 << 0)
#define ORANGE (1 << 3) // adc3
#define RED (1 << 4) // adc2

#define N_CELLS 30

struct matrix_cell {
    uint8_t low;
    uint8_t high;
};

extern const struct matrix_cell matrix[N_CELLS];

#endif