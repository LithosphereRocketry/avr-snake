#include "game.h"

#include "scan.h"
#include "matrix.h"

#include <stdlib.h>

void setup() {
}


enum {
    STATE_TITLE,
    STATE_GAME,
    STATE_DEAD
} state = STATE_TITLE;

enum {
    UP,
    DOWN,
    LEFT,
    RIGHT
} dir, next_dir;

uint32_t title[5] = {
    0b010101011101010010010010111,
    0b010101000101010101010110001,
    0b010101001100110111011010111,
    0b000000000101010101010010100,
    0b010101011101010101010010111
};

static const uint8_t score_tens[3][5] = {
    {
        0, 0, 0, 0, 0
    },
    {
        0b10,
        0b11,
        0b10,
        0b10,
        0b10
    },
    {
        0b11,
        0b10,
        0b11,
        0b01,
        0b11
    },
    {
        0b11,
        0b10,
        0b11,
        0b10,
        0b11
    }
};

static const uint8_t score_ones[10][5] = {
    {
        0b111000,
        0b101000,
        0b101000,
        0b101000,
        0b111000
    },
    {
        0b010000,
        0b011000,
        0b010000,
        0b010000,
        0b010000
    },
    {
        0b111000,
        0b100000,
        0b111000,
        0b001000,
        0b111000
    },
    {
        0b111000,
        0b100000,
        0b111000,
        0b100000,
        0b111000
    },
    {
        0b101000,
        0b101000,
        0b111000,
        0b100000,
        0b100000
    },
    {
        0b111000,
        0b001000,
        0b111000,
        0b100000,
        0b111000
    },
    {
        0b111000,
        0b001000,
        0b111000,
        0b101000,
        0b111000
    },
    {
        0b111000,
        0b100000,
        0b100000,
        0b100000,
        0b100000
    },
    {
        0b111000,
        0b101000,
        0b111000,
        0b101000,
        0b111000
    },
    {
        0b111000,
        0b101000,
        0b111000,
        0b100000,
        0b100000
    },
};

static const uint8_t diff_curve[31] = {
    60,
    60,
    60,
    50,
    45,

    40,
    40,
    35,
    35,
    30,

    30,
    30,
    30,
    25,
    25,

    25,
    25,
    25,
    25,
    25,

    20,
    20,
    20,
    20,
    20,

    20
};

uint8_t scroll = 0;
uint8_t framecount = 0;

uint8_t fruitpos;
uint8_t playerpos;

uint8_t tailbuf[32];
uint8_t head_pt, tail_pt;
uint8_t score;

static inline uint8_t adv(uint8_t pt) {
    return (pt+1) & 0x1F;
}

static inline char fruit_safe() {
    if(fruitpos == playerpos) return 0;
    for(uint8_t i = tail_pt; i != head_pt; i = adv(i)) {
        if(fruitpos != tailbuf[i]) return 0;
    }
    return 1;
}

static void roll_fruit() {
    uint16_t rn = rand();
    rn %= 30;
    uint8_t row = rn / 6;
    uint8_t col = rn % 6;
    fruitpos = row << 4 | col;
}

uint8_t diffcount = 0;

uint8_t last_btnmask;

static inline void start_game() {
    score = 0;
    playerpos = 0x21;
    dir = RIGHT;
    next_dir = RIGHT;
    roll_fruit();
    head_pt = 0;
    tail_pt = 0;
    state = STATE_GAME;
}

void loop() {
    switch(state) {
        case STATE_TITLE:
            framecount++;
            if(framecount == 20) {
                for(char i = 0; i < 5; i++) {
                    uint8_t wrap_bit = framebuf[i] & 1;
                    framebuf[i] >>= 1;
                    uint8_t in_bit = title[i] & 1;
                    if(in_bit) framebuf[i] |= 0x80;
                    title[i] >>= 1;
                    if(wrap_bit) title[i] |= 0x80000000;
                }
                framecount = 0;
            }
            if(last_btnmask && !btnmask) {
                start_game();
            } else {
                // advance PRNG to randomize based on when buttons are pressed to start
                volatile int dummy = rand();
            }
            last_btnmask = btnmask;
            break;
        case STATE_GAME:
            for(char i = 0; i < 5; i++) {
                framebuf[i] = 0;
            }
            if(framecount & 0x8) {
                framebuf[fruitpos >> 4] |= 1 << (fruitpos & 0xF);
            }
            framebuf[playerpos >> 4] |= 1 << (playerpos & 0xF);
            for(uint8_t i = tail_pt; i != head_pt; i = adv(i)) {
                framebuf[tailbuf[i] >> 4] |= 1 << (tailbuf[i] & 0xF);
            }
            framecount++;
            diffcount++;
            if(diffcount >= diff_curve[score]) {
                tailbuf[head_pt] = playerpos;
                head_pt = adv(head_pt);
                dir = next_dir;
                diffcount = 0;
                uint8_t xpos = playerpos & 0xF;
                uint8_t ypos = playerpos >> 4;
                switch(dir) {
                    case UP:
                        ypos --;
                        if(ypos == 0xFF) ypos = 4;
                        break;
                    case DOWN:
                        ypos ++;
                        if(ypos == 5) ypos = 0;
                        break;
                    case LEFT:
                        xpos --;
                        if(xpos > 6) xpos = 5;
                        break;
                    case RIGHT:
                        xpos ++;
                        if(xpos == 6) xpos = 0;
                        break;
                }
                playerpos = ypos << 4 | xpos;

                if(playerpos == fruitpos) {
                    score++;
                    roll_fruit();
                } else {
                    tail_pt = adv(tail_pt);
                }
                for(char i = tail_pt; i != head_pt; i = adv(i)) {
                    if(tailbuf[i] == playerpos) {
                        last_btnmask = 0;
                        framecount = 0;
                        diffcount = 0;
                        state = STATE_DEAD;
                        break;
                    }
                }
            }

            switch(dir) {
                case UP:
                case DOWN:
                    if(btnmask & BTNA && !(btnmask & BTND)) next_dir = LEFT;
                    if(btnmask & BTND && !(btnmask & BTNA)) next_dir = RIGHT;
                    break;
                case LEFT:
                case RIGHT:
                    if(btnmask & BTNB && !(btnmask & BTNC)) next_dir = UP;
                    if(btnmask & BTNC && !(btnmask & BTNB)) next_dir = DOWN;
                    break;
            }
            break;
        case STATE_DEAD:
            if(diffcount < 6) {
                for(char i = 0; i < 5; i++) {
                    framebuf[i] = 0;
                }
                uint8_t xpos = playerpos & 0xF;
                uint8_t ypos = playerpos >> 4;
                uint8_t on_left = xpos >= diffcount;
                uint8_t on_right = xpos + diffcount < 6;
                uint8_t on_top = ypos >= diffcount;
                uint8_t on_bot = ypos + diffcount < 5;

                if(on_bot && on_left) framebuf[ypos + diffcount] |= 1 << (xpos - diffcount);
                if(on_bot) framebuf[ypos + diffcount] |= 1 << (xpos);
                if(on_bot && on_right) framebuf[ypos + diffcount] |= 1 << (xpos + diffcount);
                if(on_top && on_left) framebuf[ypos - diffcount] |= 1 << (xpos - diffcount);
                if(on_top) framebuf[ypos - diffcount] |= 1 << (xpos);
                if(on_top && on_right) framebuf[ypos - diffcount] |= 1 << (xpos + diffcount);
                if(on_left) framebuf[ypos] |= 1 << (xpos - diffcount);
                if(on_right) framebuf[ypos] |= 1 << (xpos + diffcount);

                if(framecount == 20) {
                    framecount = 0;
                    diffcount ++;
                }
            } else if(diffcount == 6) {
                uint8_t ones = score % 10;
                uint8_t tens = score / 10;
                for(char i = 0; i < 5; i++) {
                    framebuf[i] = score_tens[tens][i] | score_ones[ones][i];
                }
                diffcount ++;
            } else {
                if(last_btnmask && !btnmask) {
                    start_game();
                } else {
                    // advance PRNG to randomize based on when buttons are pressed to start
                    volatile int dummy = rand();
                }
            }
            last_btnmask = btnmask;
            framecount++;
    }
}