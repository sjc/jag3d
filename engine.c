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
}
