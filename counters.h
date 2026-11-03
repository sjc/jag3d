//
// counters.h
//

#ifndef COUNTERS_H
#define COUNTERS_H

//
// variables adjusted for PAL or NTSC
//  - there are 50 or 60 frames per second
//  - to try to maintain a reasonable frame rate, movement code
//      is executed every off-frame, so 25 or 30 times per second
//  - every off-frame is a tick
//  - a tock is 1/5 of a second and provides a higher-level
//      common timing value which is PAL/NTSC agnostic
//  - other movement and rotation step sizes are adjusted so
//      that they sum to the same values over a tock
//

//
// REMEMBER: Since these hold the whole value, you need to DEC and
//  CHECK in the same frame, else all the timings will end up off
//  by one
//

//
// number of ticks (off-frames) to count per tock (5 PAL, 6 NTSC)
//
extern short tock_size;
extern short tock_size_x2;

//
// array where each entry == index * tock_size
//
extern short tock_multiples[];

//
// number of frames per second (5 tocks)
//
extern short one_second;

#define ONE_SECOND 5 // one second in tocks

//
// basic step size, which will sum to the same distance
//  over 1 tock under PAL and NTSC
//
extern short move_step;

//
// array of ROTSTEPS 2-element vectors, each move_step long
//
extern short *move_steps;

//
// number of ROTSTEPS steps to rotate by each frame
//
extern short rotation_step;

//
// whether this is currently a 1/5 second 'tock' frame
// set when tick_counter() is called
//
extern short is_tock_frame;

//
// setup counters and strides based on PAL/NTSC
//
void setup_counters(void);

//
// add 1 tick to the counter
// sets is_tock_frame to 1 if we've reached a 1/5 second tock
//
void tick_counter(void);

//
// reset tick counters so that the next is_tock_frame is
//  guarenteed to be in 1/5 second time
//
void reset_tick_counter(void);


#endif // COUNTERS_H
