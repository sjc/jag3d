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

/*
 * Transforms
 */

// assuming 2048 step rotation, these can be used to clamp values
#define ROTMASK 0x7FF
#define AddRotation(a,b)    ((a+b)&ROTMASK)
#define SubRotation(a,b)    ((a-b)&ROTMASK)

/*
 * Models
 */

void FixTexture(Bitmap *texture);
void FixModelTextures(N3DObjdata *model);

#endif // JAG3D_H
