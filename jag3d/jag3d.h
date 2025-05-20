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
 * Texture Helpers
 * Use if using the "Gouraud Shaded Textures" renderer
 */

void FixTexture(Bitmap *texture);
void FixModelTextures(N3DObjdata *model);

/*
 * Model Helpers
 */

//
// Create a shallow copy of a model, copying just the top-level properties
//  from `original` into `copy`, which should be pre-allocated.
// If `materials` is provided then the `.materials` property is replaced; if
//  this parameter is 0 then the pointer from `original` will be copied.
//
void CopyModelShallow(N3DObjdata *original, N3DObjdata *copy, Material *materials);

//
// Calculate the size required to hold a deep copy of a model, as created
//  by CopyModelDeep().
//
short SizeOfModel(N3DObjdata *model, short copy_faces, short copy_points, short copy_materials);

//
// Create a deep copy of a model.
// The parameters `copy_faces`, `copy_points` and `copy_materials` control
//  whether the respective arrays are copied into memory after the header
//  object. Passing 0 for all three is equivalent of calling CopyModelShallow().
// The `scale` parameter is an 8.8 fixed point fraction used to scale the
//  position of the vertexes and thus scale the model. It is only effective if
//  `copy_points` is 1. Pass `NO_SCALE` to maintain the original scale.
//
void *CopyModelDeep(N3DObjdata *original, void *buffer, short copy_faces, short copy_points, short copy_materials, short scale);

#define NO_SCALE 0

/*
 * 3D Sprites
 */

// == sizeof(N3DObjdata)
//  + sizeof(Face)
//  + 4                     the sprite is a rect therefore the Face has an extra point
//  + 4 * sizeof(Point)
#define SIZEOF_3DSPRITE     (20+28+48)  // == 96

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

/*
 * CrY colour helpers
 */
unsigned short RGBtoCrY(short red, short green, short blue);

#endif // JAG3D_H
