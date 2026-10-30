#include "demo.h"
#include "../engine.h"
#include "../models.h"

#include "../jag3d/jag3d.h"
#include "../include/joypad.h"

#include "../include/stdlib.h"
#include "../include/string.h"

#define OBJCOUNT 4

N3DObject *objects;         /* objects which will be drawn each frame */
Transform *jaguar_logo_trans;
Transform *explosion_trans;
short explosion_frame, explosion_frame_count;




/* each roation step is 1/1024 PI, meaning 2048 steps in a complete rotation */
#define ROTINC 0x10

extern short vectors_100[];

    // process_input_cb = 0;
void demo_process_input(unsigned long down, unsigned long pressed) {

    // left/right rotates the camera around the y axis
    if (down & JOY_LEFT) {
        camtrans->beta = SubRotation(camtrans->beta, 8);
    } else if (down & JOY_RIGHT) {
        camtrans->beta = AddRotation(camtrans->beta, 8);
    }

    // up/down moves forward and back
    if (down & JOY_UP) {
        short index = camtrans->beta << 1;
        camtrans->xpos += vectors_100[index];
        camtrans->zpos += vectors_100[index+1];
    } else if (down & JOY_DOWN) {
        short index = camtrans->beta << 1;
        camtrans->xpos -= vectors_100[index];
        camtrans->zpos -= vectors_100[index+1];
    }

    // 4/6 LT/RT sidesteps
    if (down & KEY_4) {
        short index = SubRotation(camtrans->beta, ROT90DEG) << 1;
        camtrans->xpos += vectors_100[index];
        camtrans->zpos += vectors_100[index+1];
    } else if (down & KEY_6) {
        short index = AddRotation(camtrans->beta, ROT90DEG) << 1;
        camtrans->xpos += vectors_100[index];
        camtrans->zpos += vectors_100[index+1];
    }

    // 1/7 move the camera up/down
    if (down & KEY_1) {
        camtrans->ypos -= 100;
    } else if (down & KEY_7) {
        camtrans->ypos += 100;
    }

    // 3/9 angle the camera up/down
    if (down & KEY_3) {
        camtrans->alpha = SubRotation(camtrans->alpha, 8);
    } else if (down & KEY_9) {
        camtrans->alpha = AddRotation(camtrans->alpha, 8);
    }

    if (down & KEY_5) {
        camtrans->xpos = camtrans->ypos = camtrans->zpos = 0;
        camtrans->alpha = camtrans->beta = camtrans->gamma = 0;
    }

    // button A triggers an explosion
    // if (pressed & FIRE_A && explosion_frame == -1) {
    //     // position the explosion in front of the camera at a distance
    //     short index = camtrans->beta * 2;
    //     explosion_trans->xpos = camtrans->xpos + (vectors_100[index] << 2);
    //     explosion_trans->zpos = camtrans->zpos + (vectors_100[index+1] << 2);
    //     explosion_frame = 0;
    //     explosion_frame_count = 0;
    //     explosion_3d_sprite.materials = explosion_material[0];
    // }
}

    // process_moved_cb = 0;

void demo_process_frame() {
    // 'billboard' the 3D sprite so it's always square-on to the camera
    jaguar_logo_trans->beta = AddRotation(camtrans->beta, ROT180DEG);
}

void demo_render_frame() {
    short i;

    for (i = 0; i < OBJCOUNT; i++) {
        RenderObject(objects[i].data, &objects[i].transform, &lightm, tpoints);
    }

    RenderObject(&jaguar_logo_3d_sprite, jaguar_logo_trans, &nolightm, tpoints);

    if (explosion_frame != -1) {
        // rotate the sprite to face towards the camera
        explosion_trans->beta = AngleAlongVector(
            camtrans->xpos - explosion_trans->xpos,
            camtrans->zpos - explosion_trans->zpos
        );

        RenderObject(&explosion_3d_sprite, explosion_trans, &nolightm, tpoints);
            
        // animate the explosion by changing the material applied to the sprite
        if (++explosion_frame_count == 2) {
            if (++explosion_frame == EXPLOSION_COUNT) {
                explosion_frame = -1; // stop showing
            } else {
                explosion_3d_sprite.materials = explosion_material[explosion_frame];
            }
            explosion_frame_count = 0;
        }
    }
}
    




void demo_clear_buffer(Bitmap *buffer) {
    // TODO: this needs to have access to INDEXEDMODE eg. from a shared config
#ifdef INDEXEDMODE
        ClearScreenBuffer(buffer);
#else
        ClearScreenAndZBuffer(buffer);
#endif
}


void setup_demo_scene() {
    short i;

    // set engine callbacks, which are invoked by the main loop
    process_input_cb = demo_process_input;
    // process_moved_cb = 0;
    process_frame_cb = demo_process_frame;
    render_frame_cb = demo_render_frame;

    // we could also just assign ClearScreenAndZBuffer
    clear_buffer_cb = demo_clear_buffer;

    // (re)set the camera position
    camtrans->xpos = 0;
    camtrans->ypos = 0;
    camtrans->zpos = 0;
    camtrans->alpha = 0;
    camtrans->beta = 0;
    camtrans->gamma = 0;

    // TODO: replace this with a static array
    objects = (void *)((long)malloc(OBJCOUNT * sizeof(N3DObject) + 3) & 0xFFFFFFFC);

    /* initialize the test objects */
    for (i = 0; i < OBJCOUNT; i++) {
        memset(&objects[i], 0, sizeof(N3DObject));
        objects[i].data = models[i].data;
        objects[i].transform.xpos = models[i].initx;
        objects[i].transform.ypos = models[i].inity;
        objects[i].transform.zpos = models[i].initz;

        // This is only required for the "Gouraud Shaded Textures" rendered (TEXSHADE = 2)
        // FixModelTextures(models[i].data);
    }

    /* setup the logo 3d sprite */
    Init3DSprite(&jaguar_logo_3d_sprite, &jaguar_logo_material_copy, JAGUAR_LOGO_WIDTH, JAGUAR_LOGO_HEIGHT);
    jaguar_logo_trans = (void *)((long)malloc(sizeof(Transform) + 3) & 0xFFFFFFFC);
    jaguar_logo_trans->xpos = 0;
    jaguar_logo_trans->ypos = 0;
    jaguar_logo_trans->zpos = 2000;
    jaguar_logo_trans->alpha = 0;
    jaguar_logo_trans->beta = 1024;
    jaguar_logo_trans->gamma = 0;

    // This is only required for the "Gouraud Shaded Textures" rendered (TEXSHADE = 2)
    // FixTexture(jaguar_logo_material.tmap);

    /* setup the explosion 3D sprite -- not shown yet */
    Init3DSprite(&explosion_3d_sprite, &explosion_0_material, EXPLOSION_WIDTH, EXPLOSION_HEIGHT);
    explosion_trans = (void *)((long)malloc(sizeof(Transform) + 3) & 0xFFFFFFFC);
    explosion_trans->ypos = 0;
    explosion_trans->alpha = 0;
    explosion_trans->gamma = 0;
    explosion_frame = -1;
    explosion_frame_count = 0;

}
