#ifndef SCAN_H
#define SCAN_H

#include "stdint.h"
#include "stdbool.h"

extern volatile uint8_t framebuf[5];
extern volatile uint8_t btnmask;
extern volatile bool wants_wake;

void start_scan();

#endif