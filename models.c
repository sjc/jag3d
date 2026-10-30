#include "models.h"
#include "engine.h"
#include "stdlib.h"

#ifndef CLUT
#define CLUT        (short *)(0xF00400)
#endif

extern short stacked_palette[];
extern short indexed_jaguar_logo_palette[];

void load_palette(short *palette, short first_index, short count) {
    short *dest = CLUT + first_index;
    while (count--) {
        *dest++ = *palette++;
    }
}

extern Bitmap indexed_jaguar_logo;

void setup_models() {
    short i, temp;

#ifdef INDEXEDMODE
    // setup some basic colour indexes for testing indexed drawing
    unsigned short *palette = CLUT;
    palette[0] = 0x27a0;
    palette[1] = RGBtoCrY(0, 255, 0);
    palette[2] = RGBtoCrY(0, 0, 255);

    load_palette(stacked_palette, 16, 16); // palette for explosions
    load_palette(indexed_jaguar_logo_palette, 32, 16);

    jaguar_logo_material_copy.color = 0x01; // this is a colour index
    jaguar_logo_material_copy.flags = 0;
    jaguar_logo_material_copy.tmap = &indexed_jaguar_logo;
#else
    jaguar_logo_material_copy.color = RGBtoCrY(0,255,0); // a CrY colour
    jaguar_logo_material_copy.flags = 0;
    jaguar_logo_material_copy.tmap = jaguar_logo_material.tmap;
#endif

    /* find the maximum number of points in the models... */
    temp = 0;
    for (i = 0; i < MODEL_COUNT; i++) {
        // Check point count
        if (models[i].data->numpoints > temp) {
            temp = models[i].data->numpoints;
        }
    }

    /* ...and allocate temporary storage for the GPU to transform points */
    tpoints = (void *)((long)malloc(temp * sizeof(TPoint) + 3) & 0xFFFFFFFC);
}
