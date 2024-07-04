//
// jag3d.h
//

#include "n3d.h"
#include "n3dintern.h"

/*
 * The object renderer
 */

// start and entry points for the renderer
extern long renderer_code[];
extern long renderer_init[];
extern long renderer_frameinit[];
extern long renderer_objinit[];

void LoadAndInitRenderer(long *gpucode, void *gpufunc);
void SetupFrame(void *gpufunc, Bitmap *window, Angles *camangles);
void RenderObject(void *gpufunc, N3DObject *obj, Lightmodel *lmodel, TPoint *tpoints);

void ClearBuffer(Bitmap *buf);

/*
 * Models
 */

/*
 * Copy sin() and cos() of each angle (alpha, beta, gamma) into
 * the Angles struct, ready for passing to the GPU.
 *
 * Defined in mkamt.s
 */
extern void UpdateAngles(Angles *angles);

void FixTexture(Bitmap *texture);
void FixModelTextures(N3DObjdata *model);
