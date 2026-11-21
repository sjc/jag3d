#include "align.h"
#include "demo.h"
#include "../engine.h"
#include "../include/joypad.h"
#include "../include/string.h"
#include "../screen.h"
#include "../models.h"

extern int sprintf(char *, const char *, ...);

extern short font_ac[];
extern Transform jaguar_logo_trans[];

short offset_x;
short offset_y;

char buf[8]; // assign here because of sprintf() weirdness

void write_instructions() {
    draw_string_to_buffer("D-Pad: Adjust drawbuffer position", PEN1, HUDBUFFER+(HUDSTRIDE*4), HUDSTRIDE, font_ac);
}

void write_offsets() {
    sprintf(buf, "X=%d ", (int)(drawbuffer_x + offset_x));
    draw_string_to_buffer(buf, PEN1, HUDBUFFER+(HUDSTRIDE*8)+1, HUDSTRIDE, font_ac);
    sprintf(buf, "Y=%d ", (int)(drawbuffer_y + offset_y));
    draw_string_to_buffer(buf, PEN1, HUDBUFFER+(HUDSTRIDE*12)+1, HUDSTRIDE, font_ac);
}

void align_process_input(unsigned long down, unsigned long pressed) {
    short dx = 0;
    short dy = 0;

    // scene switching
    if (down & OPTION) {
        if ((down & KEY_1) /*&& ((pressed & OPTION) || (pressed & KEY_1))*/) {
            setup_demo_scene();
        }
        return; // skip further processing if OPTION is held
    }

    if (down & JOY_UP) dy = -2;
    else if (down & JOY_DOWN) dy = 2;
    else if (down & JOY_LEFT) dx = -1;
    else if (down & JOY_RIGHT) dx = 1;

    if (dx || dy) {
        short x = drawbuffer_x + offset_x + dx;
        if (x < 0) { x = 0; } else { offset_x += dx; };
        short y =  drawbuffer_y + offset_y + dy;        
        if (y < 0) { y = 0; } else { offset_y += dy; };
        set_drawbuffers_offset(offset_x, offset_y);
        write_offsets();
    }
}

void align_process_frame() {
    jaguar_logo_trans->beta = AddRotation(jaguar_logo_trans->beta, rotation_step);
}

void align_render_frame() {
    RenderObject(&jaguar_logo_3d_sprite, jaguar_logo_trans, &nolightm, tpoints);
}

void align_clear_buffer(Bitmap *buffer) {
#if INDEXED == 1
        ClearScreenBuffer(buffer);
#else
        ClearScreenAndZBuffer(buffer);
#endif
}

void reset_align_scene() {
    offset_x = 0;
    offset_y = 0;
    set_drawbuffers_offset(offset_x, offset_y);
}

void setup_align_scene() {
    // set engine callbacks, which are invoked by the main loop
    process_input_cb = align_process_input;
    process_frame_cb = align_process_frame;
    render_frame_cb = align_render_frame;
    clear_buffer_cb = align_clear_buffer;

    // (re)set the camera position
    camtrans->xpos = 0;
    camtrans->ypos = 0;
    camtrans->zpos = 0;
    camtrans->alpha = 0;
    camtrans->beta = 0;
    camtrans->gamma = 0;

    // we'll re-use the jag logo sprite
    Init3DSprite(&jaguar_logo_3d_sprite, &jaguar_logo_material_copy, JAGUAR_LOGO_WIDTH, JAGUAR_LOGO_HEIGHT);
    jaguar_logo_trans->xpos = 0;
    jaguar_logo_trans->ypos = 0;
    jaguar_logo_trans->zpos = 1000;
    jaguar_logo_trans->alpha = 0;
    jaguar_logo_trans->beta = 1024;
    jaguar_logo_trans->gamma = 0;

#if TEXSHADE == 2
    FixTexture(jaguar_logo_material.tmap);
#endif


    // setup the text overlay
    clear_hud_buffer();
    write_instructions();
    write_offsets();
}
