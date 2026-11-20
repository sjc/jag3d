#include "engine.h"

void setup_engine() {
    // ensure the callbacks are clear
    process_input_cb = 0;
    process_moved_cb = 0;
    process_frame_cb = 0;
    render_frame_cb = 0;
    clear_buffer_cb = 0;

    // clear out the camera transform
    camtrans->xpos = 0;
    camtrans->ypos = 0;
    camtrans->zpos = 0;
    camtrans->alpha = 0;
    camtrans->beta = 0;
    camtrans->gamma = 0;

    // setup the default lights
    lightm.ambient = 0x4000;
    lightm.numlights = 1;
    lightm.lights[0].x = -0x24f3;
    lightm.lights[0].y = -0x24f3;
    lightm.lights[0].z = -0x24f3;
    lightm.lights[0].bright =  0x4000;

    nolightm.ambient = 0x8000;
    nolightm.numlights = 0;
}
