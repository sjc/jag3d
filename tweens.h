//
// tweens.h
//

#ifndef TWEENS_H
#define TWEENS_H

#include "../jag3d/jag3d.h"

//
// Tweens update GameObj properties over time in order to
//  automate movement and other animation.
// The basic unit of tween time is the tock, meaning that
//  tweens will run for the same duration on NTSC and PAL
//

// Caution should be taken when using tweens with automatic
//  object movement.
// - objects used for tweens should be treated in the same
//      way as other objects re. setting the flags to include
//      GOF_USED, so that the object is rendered
// (do we need a GOF_INV flag???)
// - automatic animation (GOF_MOV, GOF_ROT) can be used alongside
//      tweens. tweens should not use an object's .d to store
//      variables -- use the storage in the Tween struct
// - do not try to change the same value with both tweens and
//      automatic animation at the same time.

// TODO: hide some of this stuff in tweens_internal.h

//
// Tween structure
// You should not need to interact with these directly
//

typedef struct tween {
    union {
        void *target;
        // GameObj *object;
        Transform *transform;
    };
    long func;          // TweenFunc
    short start;        // first active tock, in tween time space
    short duration;     // number of whole tocks this tween will be active for
    short v0,v1,v2,v3;  // var storage for tweens
    long data;
} Tween;

#define TWEEN_NO_OBJ ((void *)1) // placeholder if we aren't working on a Transform or Object

//
// TWEENSMAX (in const.inc/.h)
// number of tween slots to allocate
// dev should work out max that they
// tween_count isn't exposed, by can be got at as an extern
//

//
// Callback function which each tween type should provide
//  tick == 0 on first tick of tock
// return either TWEEN_CONTINUE or TWEEN_DONE to exit early

typedef short (*TweenFunc)(Tween *tween, void *target, short tock, short tick);

#define TWEEN_CONTINUE  0
#define TWEEN_DONE      1


//
// timing variables
// these represent the time of the NEXT iteration of run_tweens()
//
short tween_ticks, tween_tocks;

//
// Call once to set everything up at code load time
//
void tweens_onetime_init(void);

//
// Reset the current tween time to 0 and remove all current tweens
//
void reset_tweens(void);

//
// Run all current tweens by one tick
//
void run_tweens(void);

//
// Find a free tween, set it up with the provided required values, and
//  return a pointer to it to be passed into other tween configuation
//  functions
// Return NULL (0) if there are no more free tweens
// Tweens run for a number of whole tocks, starting with start
// `target` should be of a type which coresponds to the tween function
//  you want to use. For most of the functions below that means a pointer
//  to a Transform.
// Pass TWEEN_NO_OBJ for cases where this value isn't used.
//
Tween *init_tween(void *target, short start, short duration);

//
// helpers
//

// number of ticks the tween will run for -- depends on NTSC/PAL
short tween_duration_ticks(Tween *tween);


//
// Basic Tweens
//

// General rules:
// - When setting up a tween, the transform will be set to its starting
//      position. So setup tweens from last to first.
// - Tweens will only update the values of the properties they will
//      be changing (eg. an xz tween will not touch .ypos)
// - Movement is assumed to be some ^2 multiple of move_step per tick 

typedef enum coords {
    X,Y,Z
} Coords;

typedef enum planes {
    XY, XZ, YZ
} Planes;

typedef enum axis {
    Alpha, Beta, Gamma
} Axis;

//
// Run an arbitary function, which must match TweenFunc
// The same can be achieved be assigning straight to the .func property
// Run fewer times by returning TWEEN_DONE
//
void tween_func(Tween *tween, TweenFunc func);

//
// remove all objs_count game_objs
// TODO: this doesn't belong here
//
// void tween_remove_game_objs(Tween *tween);

//
// update a single positional property
// Target: Transform
//
void tween_move_1D(Tween *tween, Coords property, short delta);

//
// move the object in a 2D plane to the given position, setting the object's
//  position to the start position needed to perform the transform
// Target: Transform
//
void tween_move_to_2D(Tween *tween, short dest0, short dest1, Planes plane, short angle, short speed_scale);

//
// move the object in a 2D plane along a vector, starting from its current
//  position
// Target: Transform
//
void tween_move_from_2D(Tween *tween, short pos0, short pos1, Planes plane, short angle, short speed_scale);

//
// move the object in a 2D plane from its current position to a new position
//  over the duration of the tween
// Target: Transform
//
void tween_move_from_current_to_2D(Tween *tween, short pos0, short pos1, Planes plane);

//
// rotate the object a fixed amount
//  - angles are 0-2047 indexes, each pi/1024 radians
//  - if from_angle == to_angle then it's assumed a complete rotation is required
//  - to rotate more than once, provide values greater than 2048 eg.
//    - start at 3 o'clock: from_angle = 512
//    - rotate 1.25 time, to end up at 6 o'clock: to_angle = 2048 + 512 = 2560
//  - rotations clockwise are +ve, rotations counter-clockwise are -ve
// Target: Transform
//

#define ROT_CW  1
#define ROT_CCW 0

void tween_rotate(Tween *tween, short from_angle, short to_angle, Axis axis, short clockwise);

//
// calculate tween in a circle around a point calculated from the
//  object's current position
//

//
// Animate object position around a circle with radius supplied by the given
//  list of vectors, to (pos0,pos1) in the chosen plane.
// The object's position will be updated to the start position, from_angle 
//  around the circle upon return from the function
// Target: Transform
//
void tween_circle_to_2D(Tween *tween, short pos0, short pos1, Planes plane, short from_angle, short to_angle, short clockwise, short *vectors);

#endif // TWEENS_H
