#ifndef MODELS_H
#define MODELS_H

#include "jag3d/jag3d.h"

//
// 3D object data
//
extern N3DObjdata ship1data, globedata, radardata, torusdata, knightdata, robotdata, castledata, cubedata;

#define MOD_SHIP        0
#define MOD_GLOBE       1
#define MOD_RADAR       2
#define MOD_TORUS       3
#define MOD_KNIGHT      4
#define MOD_ROBOT       5
#define MOD_CASTLE      6
#define MOD_CUBE        7

/*
 * a model consists of:
 * (1) a pointer for the data for the model
 * (2) starting X,Y,Z coordinates for the model
 */
typedef struct model {
    N3DObjdata *data;
    short initx, inity, initz;
    short reserved;     /* pads structure to a longword boundary */
} MODEL;

/* models we can draw */
MODEL models[] = {
    { &ship1data,  500, 0, 250, 0 },
    { &globedata, -250, 0, 250, 0 },
    { &radardata,  250, 0,-250, 0 },
    { &torusdata, -1000, 0,-1000, 0 },

    // these are problematic
    { &knightdata,1000, 0,-1000, 0 },
    { &robotdata, -1000, 0,-1000, 0 },  
    { &castledata,-25000, 0,-25000, 0 },
    { &cubedata,  -1000, 0, 1000, 0 },
};

#define MODEL_COUNT (sizeof(models)/sizeof(MODEL))


//
// for the 3D sprite examples
//
extern N3DObjdata jaguar_logo_3d_sprite;
extern Material jaguar_logo_material;
extern Material jaguar_logo_material_copy;

#define JAGUAR_LOGO_WIDTH  512
#define JAGUAR_LOGO_HEIGHT 128

extern N3DObjdata explosion_3d_sprite;
extern Material explosion_0_material, explosion_1_material, explosion_2_material, explosion_3_material;

#ifdef INDEXEDMODE
extern Bitmap stacked_0, stacked_1, stacked_2, stacked_3;
Material stacked_0_material = { 0, 0, &stacked_0 };
Material stacked_1_material = { 0, 0, &stacked_1 };
Material stacked_2_material = { 0, 0, &stacked_2 };
Material stacked_3_material = { 0, 0, &stacked_3 };
#endif

#define EXPLOSION_COUNT     4

Material *explosion_material[EXPLOSION_COUNT] = {
#ifdef INDEXEDMODE
    &stacked_0_material,
    &stacked_1_material,
    &stacked_2_material,
    &stacked_3_material,
#else
    &explosion_0_material,
    &explosion_1_material,
    &explosion_2_material,
    &explosion_3_material,
#endif
};

#define EXPLOSION_WIDTH     64
#define EXPLOSION_HEIGHT    64

void setup_models(void);

#endif // MODELS_H
