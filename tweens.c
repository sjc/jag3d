//
// tweens.c
//

#include "tweens.h"
#include "const.h"
#include "counters.h"

//
// tweens_asm.s
//

extern short tween_count;
extern Tween tween_list[];
extern Tween *tween_pointers[];

//
// one-time init
//

void tweens_onetime_init() {

    reset_tweens();

    // and maybe do something else?
}

//
// reset
//

void reset_tweens() {

    // ensure everything has been reset to a known value

    tween_count = 0;
    tween_ticks = tween_tocks = 0;

    Tween *ptr = tween_list;
    short i = TWEENSMAX;
    
    while (i--) {
        ptr->target = 0; // ...for certain values of 'everything'
        ptr++;
    }
}

//
// run tweens
//

void run_tweens() {

    // this will always be valid
    // tween_ticks (when passed into a tween function) ranges from 1 (first
    //  tick of the tween) to tock_size (last tick of the tween)

    tween_ticks++;

    // we will need to gather up a list of tweens to run because there's
    //  a chance that tweens will be adding or removing other tweens from
    //  the list as we run them
    Tween **tweens = tween_pointers;
    short active = 0;

    Tween *ptr = tween_list;
    short i = tween_count;

    while (i--) {
        while (ptr->target == 0) ptr++;

        // calculate number of tocks which have happened since this tween
        //  was scheduled to start
        // this value may be -ve if it hasn't started yet
        short tocks_since_start = tween_tocks - ptr->start;

        if (tocks_since_start >= ptr->duration) {
            // this tween has completed
            ptr->target = 0;
            tween_count--;
        
        } else if (ptr->func && tocks_since_start >= 0) {
            // this tween can be run
            *tweens++ = ptr;
            active++;
        }

        ptr++;
    }

    while (active--) {
        // TODO: maybe remove the object from the parameters list, as the function
        //  will have access to it via the Tween
        Tween *tween = *--tweens;
        TweenFunc func = (TweenFunc)tween->func;
        if (func(tween, tween->target, tween_tocks, tween_ticks) == TWEEN_DONE) {
            tween->target = 0;
            tween_count--;
        }
    }

    if (tween_ticks == tock_size) {
        tween_ticks = 0;
        tween_tocks++;
    }
}

//
// find a free tween and set initial values on it
//

Tween *init_tween(void *target, short start, short duration) {

    if (tween_count == TWEENSMAX) return 0; // no free tweens!
    tween_count++;

    Tween *ptr = tween_list;
    while (ptr->target != 0) ptr++;

    ptr->target = target;
    ptr->func = 0;
    ptr->start = start;
    ptr->duration = duration;

    // TODO: reset other values as we get to them

    return ptr;
}

//
// helpers
//

short tween_duration_ticks(Tween *tween) {
    return tween->duration * tock_size;
}

// because we'll generally be scaling-down:
//  -ve scales INCREASE the step size
//  +ve scales DECREASE the step size
// TODO: candidate to re-write in assembly
short scaled_move_step(short index, short scale) {
    if (scale < 0) return move_steps[index] << -scale;
    return move_steps[index] >> scale;
}

//
// example tweens
// move to another file
//

void tween_func(Tween *tween, TweenFunc func) {
    tween->func = (long)func;
}

// remove game objs

// short tween_remove_game_objs_func(Tween *tween, void *na, short tock, short tick) {
//     if (objs_count) {
//         free_used_objs(game_objs, objs_count);
//         objs_count = 0;
//     }
//     return TWEEN_DONE;
// }

// void tween_remove_game_objs(Tween *tween) {
//     tween->func = (long)tween_remove_game_objs_func;
// }

//
// update a single positional property
//

short tween_move_1D_func(Tween *tween, Transform *t, short tock, short tick) {
    t->xpos += tween->v0;
    t->ypos += tween->v1;
    t->zpos += tween->v2;
    return TWEEN_CONTINUE;
}

void tween_move_1D(Tween *tween, Coords property, short delta) {

    tween->v0 = tween->v1 = tween->v2 = 0;
    tween->func = (long)tween_move_1D_func;

    switch (property) {
    case X:
        tween->v0 = delta;
        break;
    case Y:
        tween->v1 = delta;
        break;
    case Z:
        tween->v2 = delta;
        break;
    }
}

//
// move to a point in the xz plane
// the starting location will be worked out from the final position,
//  the angle of travel and the duration of the tween
//

short tween_move_xy_func(Tween *tween, Transform *t, short tock, short tick) {
    t->xpos += tween->v0;
    t->ypos += tween->v1;
    return TWEEN_CONTINUE;
}

short tween_move_xz_func(Tween *tween, Transform *t, short tock, short tick) {
    t->xpos += tween->v0;
    t->zpos += tween->v1;
    return TWEEN_CONTINUE;
}

short tween_move_yz_func(Tween *tween, Transform *t, short tock, short tick) {
    t->ypos += tween->v0;
    t->zpos += tween->v1;
    return TWEEN_CONTINUE;
}

void config_tween_move_from_2D(Tween *tween, short pos0, short pos1, short d0, short d1, Planes plane) {

    Transform *t = tween->transform;

    switch (plane) {
    case XY:
        t->xpos = pos0;
        t->ypos = pos1;
        tween->func = (long)tween_move_xy_func;
        break;
    case XZ:
        t->xpos = pos0;
        t->zpos = pos1;
        tween->func = (long)tween_move_xz_func;
        break;
    case YZ:
        t->ypos = pos0;
        t->zpos = pos1;
        tween->func = (long)tween_move_yz_func;
        break;
    }

    // configure the tween
    tween->v0 = d0;
    tween->v1 = d1;
}

void tween_move_to_2D(Tween *tween, short dest0, short dest1, Planes plane, short angle, short speed_scale) {

    // the vector along which the object will travel
    short index = (angle & ROTMASK) * 2;
    short d0 = scaled_move_step(index, speed_scale);
    short d1 = scaled_move_step(index+1, speed_scale);

    // work out the starting position
    // we subtract the movement vector to return to the starting location
    short steps = tween_duration_ticks(tween);
    short pos0 = dest0 - (steps * d0);
    short pos1 = dest1 - (steps * d1);

    config_tween_move_from_2D(tween, pos0, pos1, d0, d1, plane);
}

void tween_move_from_2D(Tween *tween, short pos0, short pos1, Planes plane, short angle, short speed_scale) {

    short index = (angle & ROTMASK) * 2;

    // the vector along which the object will travel
    short d0 = scaled_move_step(index, speed_scale);
    short d1 = scaled_move_step(index+1, speed_scale);

    config_tween_move_from_2D(tween, pos0, pos1, d0, d1, plane);
}

void tween_move_from_current_to_2D(Tween *tween, short pos0, short pos1, Planes plane) {

    short s0, s1, d0, d1;

    short ticks = tween_duration_ticks(tween);
    if (ticks == 0) return;

    Transform *t = tween->transform;

    switch (plane) {
    case XY:
        s0 = t->xpos;
        s1 = t->ypos;
        break;
    case XZ:
        s0 = t->xpos;
        s1 = t->zpos;
        break;
    case YZ:
        s0 = t->ypos;
        s1 = t->zpos;
        break;
    default:
        s0 = s1 = 0;
        break;
    }

    d0 = (pos0 - s0) / ticks;
    d1 = (pos1 - s1) / ticks;

    config_tween_move_from_2D(tween, s0, s1, d0, d1, plane);
}

//
// rotation
//

short tween_rotate_alpha_func(Tween *tween, Transform *t, short tock, short tick) {
    t->alpha = AddRotation(t->alpha, tween->v0);
    return TWEEN_CONTINUE;
}

short tween_rotate_beta_func(Tween *tween, Transform *t, short tock, short tick) {
    t->beta = AddRotation(t->beta, tween->v0);
    return TWEEN_CONTINUE;
}

short tween_rotate_gamma_func(Tween *tween, Transform *t, short tock, short tick) {
    t->gamma = AddRotation(t->gamma, tween->v0);
    return TWEEN_CONTINUE;
}

//
// helper
//
short tween_rotation_delta(short from_angle, short to_angle, short clockwise, short steps) {
    short delta;
    if (clockwise == ROT_CW) {
        // rotate by a +ve delta from from_angle to to_angle
        // confirm that the angles make sense
        while (to_angle <= from_angle) to_angle += 2048; 
        delta = (to_angle - from_angle) / steps;
    } else { // counter-clockwise
        // rotate by a -ve delta from from_angle to to_angle
        while (to_angle >= from_angle) to_angle -= 2048;
        delta = (to_angle - from_angle) / steps;
    }
    return delta;
}

void tween_rotate(Tween *tween, short from_angle, short to_angle, Axis axis, short clockwise) {

    short steps = tween_duration_ticks(tween);
    short delta = tween_rotation_delta(from_angle, to_angle, clockwise, steps);

    Transform *t = tween->transform;

    // fix up the start angle, since it probably won't be an even multiple
    //  of the delta, to ensure we end in the correct rotation
    from_angle = SubRotation(to_angle, (delta * steps));

    switch (axis) {
    case Alpha:
        tween->func = (long)tween_rotate_alpha_func;
        t->alpha = from_angle;
        break;
    case Beta:
        tween->func = (long)tween_rotate_beta_func;
        t->beta = from_angle;
        break;
    case Gamma:
        tween->func = (long)tween_rotate_gamma_func;
        t->gamma = from_angle;
        break;
    }

    tween->v0 = delta;
}

//
// circling
//

//
// move along the edge of a circle to the provided coords
// object will be set to the starting position, which will be calculated from
//  the other parameters
// TODO: this needs a parameter to describe the radius of the circle, but that
//  will have to come later. we'll hardcode it for now
// this will be based on the code i already have from when the player's replacement
//  ship flys in after death
//

short tween_circle_xy_func(Tween *tween, Transform *t, short tock, short tick) {

    // update angle: current_angle += delta_angle
    short angle = tween->v2 = AddRotation(tween->v2, tween->v3);

    short index = angle * 2; // index into vector table
    short *vectors = (short *)tween->data;

    // update position
    t->xpos = tween->v0 + vectors[index];
    t->ypos = tween->v1 + vectors[index+1];

    return TWEEN_CONTINUE;
}

short tween_circle_xz_func(Tween *tween, Transform *t, short tock, short tick) {

    // update angle: current_angle += delta_angle
    short angle = tween->v2 = AddRotation(tween->v2, tween->v3);

    short index = angle * 2; // index into vector table
    short *vectors = (short *)tween->data;

    // update position
    t->xpos = tween->v0 + vectors[index];
    t->zpos = tween->v1 + vectors[index+1];

    return TWEEN_CONTINUE;
}

short tween_circle_yz_func(Tween *tween, Transform *t, short tock, short tick) {

    // update angle: current_angle += delta_angle
    short angle = tween->v2 = AddRotation(tween->v2, tween->v3);

    short index = angle * 2; // index into vector table
    short *vectors = (short *)tween->data;

    // update position
    t->ypos = tween->v0 + vectors[index];
    t->zpos = tween->v1 + vectors[index+1];

    return TWEEN_CONTINUE;
}

//
// parameters:
//  - tween to use
//  - destination coords (on circle)
//  - plane to move in
//  - angle around circle to move from
//  - angle around circle to move to
//

void tween_circle_to_2D(Tween *tween, short pos0, short pos1, Planes plane, short from_angle, short to_angle, short clockwise, short *vectors) {

    // calculate the angle step size
    short steps = tween_duration_ticks(tween);
    short delta = tween_rotation_delta(from_angle, to_angle, clockwise, steps);

    // assuming that the destination is the important part, fix it up so we
    //  arrive exactly there, which may mean adjusting the starting angle
    from_angle = SubRotation(to_angle, delta * steps);

    // calculate the centre point around which we will rotate
    // we work this out from the to destination and the to_angle
    // here we're using a fixed vector length

    // take opposite of the to_angle
    short angle = AddRotation(to_angle, 1024);
    short index = angle * 2;

    short centre0 = pos0 + vectors[index];
    short centre1 = pos1 + vectors[index+1];

    // the starting position is from_angle away from the centre
    index = from_angle * 2;
    short start0 = centre0 + vectors[index];
    short start1 = centre1 + vectors[index+1];

    Transform *t = tween->transform;

    switch (plane) {
    case XY:
        tween->func = (long)tween_circle_xy_func;
        t->xpos = start0;
        t->ypos = start1;
        break;
    case XZ:
        tween->func = (long)tween_circle_xz_func;
        t->xpos = start0;
        t->zpos = start1;
        break;
    case YZ:
        tween->func = (long)tween_circle_yz_func;
        t->ypos = start0;
        t->zpos = start1;
        break;
    }

    tween->v0 = centre0;
    tween->v1 = centre1;
    tween->v2 = from_angle;
    tween->v3 = delta;

    tween->data = (long)vectors;
}

