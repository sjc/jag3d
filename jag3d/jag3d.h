//
// jag3d.h
//

#ifndef JAG3D_H
#define JAG3D_H

#include "n3d.h"
#include "n3dintern.h"

/*
 * The object renderer
 */

//
// functions using default renderer built from jag3d/renderer/renderer.s
//
void LoadAndInitRenderer(void);
void SetupFrame(Bitmap *window, Transform *camtrans);
void RenderObject(N3DObjdata *data, Transform *trans, Lightmodel *lmodel, TPoint *tpoints);

//
// functions to support multiple custom renderers
//
void LoadAndInitRendererCustom(long *gpucode, void *gpufunc);
void SetupFrameCustom(void *gpufunc, Bitmap *window, Transform *camtrans);
void RenderObjectCustom(void *gpufunc, N3DObjdata *data, Transform *trans, Lightmodel *lmodel, TPoint *tpoints);

//
// helpers
//

// clears a screen buffer and its associated z-buffer
void ClearBuffer(Bitmap *buf);

// Return an 'angle' in the range 0-2047 representing the rotation defined by the
//  vector. The parameter names `x` and `y` are just placeholders, the angle can
//  be defined in any 2D plane.
short AngleAlongVector(short x, short y);

/*
 * Transforms
 */

// assuming 2048 step rotation, these can be used to clamp values
#define ROTMASK 0x7FF
#define AddRotation(a,b)    ((a+b)&ROTMASK)
#define SubRotation(a,b)    ((a-b)&ROTMASK)

/*
 * Models
 * Use if using the "Gouraud Shaded Textures" renderer
 */

void FixTexture(Bitmap *texture);
void FixModelTextures(N3DObjdata *model);

/*
 * 3D Sprites
 */

// == sizeof(N3DObjdata)
//  + 4                     padding so that the facelist is also phrase aligned
//  + sizeof(Face)
//  + 4                     the sprite is a rect therefore the Face has an extra point
//  + 4 * sizeof(Point)
#define SIZEOF_3DSPRITE     (24+28+48)  // == 

void Init3DSprite(N3DObjdata *sprite, Material *texture, short width, short height);

/*
 * Particles
 */

typedef struct {
    short ttl;
    short z,y,x; // order designed to optomise loading in GPU
    short color;
    short dx,dy,dz;
} Particle;

//
// renderers
//
void RenderPointsCustom(void *gpufunc, Particle *particles, short count);
void RenderPoints(Particle *particles, short count);

//
// helpers
//
void ClearPoints(Particle *particles, short max_count);

// this call assumes that there is at least one free (.ttl == 0) particle
Particle *FindFreePoint(Particle *particles);

// returns the updated count, max_count - expired particles
short RunPoints(Particle *particles, short count);


#endif // JAG3D_H
