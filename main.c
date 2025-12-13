#include "avr/io.h"
#include "avr/interrupt.h"
#include "avr/sleep.h"

#include "scan.h"
#include "game.h"

int main() {
    set_sleep_mode(SLEEP_MODE_IDLE);
    start_scan();
    sei();
    setup();

    while(1) {
        do { sleep_mode(); } while(!wants_wake);
        wants_wake = false;
        loop();
    }
}