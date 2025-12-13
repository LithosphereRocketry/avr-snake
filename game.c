#include "game.h"

#include "scan.h"

void setup() {
    framebuf[0] = 0b000010;
    framebuf[1] = 0b000100;
    framebuf[2] = 0b001000;
    framebuf[3] = 0b010000;
    framebuf[4] = 0b100000;
}


static int fcount = 0;

void loop() {
    fcount ++;
    if(fcount == 60) {
        for(int i = 0; i < 5; i++) {
            framebuf[i] <<= 1;
            if(framebuf[i] >= 0b111111) framebuf[i] = 1;
        }
        fcount = 0;
    }
}