//
// counter.c
//

#include "counters.h"

//
// configure counters and shared values for PAL / NTSC
//

short tick_current;

//
// One-time timing setup
//

extern short VIDpal; // 1 == PAL, 0 == NTSC. set in video.s

extern short vectors_100[];
extern short vectors_120[];

#define TOCKMCOUNT  32  // number of entries in the tock_multiples array

void setup_counters() {

    if (VIDpal) {
        // PAL
        tock_size = 5;
        tock_size_x2 = 10;
        one_second = 25;

        move_step = 120;
        move_steps = vectors_120;
        rotation_step = 24; // works for camera. may want half this for animations.

    } else {
        // NTSC
        tock_size = 6;
        tock_size_x2 = 12;
        one_second = 30;

        move_step = 100;
        move_steps = vectors_100;
        rotation_step = 20;
    }

    // set up look-up table of multiples
    short i = 0;
    short multiple = 0;
    while (i < TOCKMCOUNT) {
        tock_multiples[i++] = multiple;
        multiple += tock_size;
    }

    // setup counter variables
    reset_tick_counter();
}


//
// Run counters
// Call this once per frame
//

void tick_counter() {
    tick_current--;
    if (tick_current) is_tock_frame = 0;
    else {
        tick_current = tock_size;
        is_tock_frame = 1;
    }
}

void reset_tick_counter() {
    tick_current = tock_size;
    is_tock_frame = 0;
}
