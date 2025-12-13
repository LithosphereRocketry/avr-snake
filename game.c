#include "game.h"

#include "scan.h"
#include "matrix.h"

void setup() {
    // framebuf[0] = 0b000010;
    // framebuf[1] = 0b000100;
    // framebuf[2] = 0b001000;
    // framebuf[3] = 0b010000;
    // framebuf[4] = 0b100000;
}


uint8_t fruitx, fruity;

static int fcount = 0;

void loop() {
    framebuf[4] = 0;
    if(btnmask & BTNA) framebuf[4] |= 1<<0;
    if(btnmask & BTNB) framebuf[4] |= 1<<2;
    if(btnmask & BTNC) framebuf[4] |= 1<<3;
    if(btnmask & BTND) framebuf[4] |= 1<<5;
}