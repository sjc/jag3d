//
// jag3d.c
// Wrapper functions for working with the combined renderer
//

#include "jag3d.h"
#include "blit.h"

/* start and entry points for the default renderer */
extern long renderer_code[];
extern long renderer_init[];
extern long renderer_frameinit[];
extern long renderer_objinit[];

/* from lib/gpulib.s */
extern void GPUload(void *);    /* loads a package into the GPU */
extern void GPUrun(void *);     /* runs a GPU program */

extern void *malloc(long);
extern void free(void *);

/* long-aligned parameter block for the GPU, defined in miscasm.s */
extern long params[];

/* for testing returning values from the GPU */
//extern short test_returns[];

/* pointer to temporary storage for transformed points */
;

void
LoadAndInitRendererCustom(long *gpucode, void *gpufunc)
{
    GPUload(gpucode);

    // TODO: calculate the space left in GPU memory, so we could
    //  potentially use it as the triangle scratch space?

    GPUrun(gpufunc);
}

void
LoadAndInitRenderer() {
    LoadAndInitRendererCustom(renderer_code, renderer_init);
}

void
SetupFrameCustom(void *gpufunc, Bitmap *window, Transform *camtrans)
{
    params[0] = (long)window;
    params[1] = (long)camtrans;

    GPUrun(gpufunc);
}

void
SetupFrame(Bitmap *window, Transform *camtrans) {
    SetupFrameCustom(renderer_frameinit, window, camtrans);
}

void
RenderObjectCustom(void *gpufunc, N3DObjdata *data, Transform *trans, Lightmodel *lmodel, TPoint *tpoints)
{
    params[0] = (long)data;
    params[1] = (long)trans;
    params[2] = (long)lmodel;
    params[3] = (long)tpoints;

    GPUrun(gpufunc);
}

void
RenderObject(N3DObjdata *data, Transform *trans, Lightmodel *lmodel, TPoint *tpoints) {
    RenderObjectCustom(renderer_objinit, data, trans, lmodel, tpoints);
}

/****************************************************************
 *  N3Dclrbuf(buf)                      *
 * Clears the bitmap pointed to by "buf", filling its data with *
 * a solid color, and its Z buffer with a null value        *
 ****************************************************************/

// TODO: Move this to the GPU as part of the frame setup

void
ClearBuffer(Bitmap *buf)
{
    // long bgcolor = 0x27a027a0;       /* fill color, duplicated */
    long zvalue = 0xffffffff;       /* Z value (16.16 fraction) */

    // TODO: we could move this into the renderer, as part of frame_init()

    B_PATD[0] = buf->clearcolor;
    B_PATD[1] = buf->clearcolor;
    B_Z3 = zvalue;
    B_Z2 = zvalue;
    B_Z1 = zvalue;
    B_Z0 = zvalue;
    A1_BASE = (long)buf->data;
    A1_STEP = 0x00010000L | ((-buf->width) & 0x0000ffff);
    A1_FLAGS = buf->blitflags|XADDPHR;
    A1_PIXEL = 0;
    A1_CLIP = 0;
    // this could be optomised by storing heigh,width
    B_COUNT = ((long)buf->height << 16) | (buf->width);
    B_CMD = UPDA1|DSTWRZ|PATDSEL;
}

/****************************************************************
 *  FixTexture( texture )                   *
 * Adjust all intensities in a texture so that they are     *
 * relative to 0x80. This is done for renderers that do     *
 * shading on textures; because of the way the shading is   *
 * done, the source data must with intensities as signed    *
 * offsets to a base intensity (namely 0x80). So before using   *
 * a renderer that does shading on textures, this function  *
 * must be called on all textures in the model to be rendered.  *
 *                              *
 * Note that because of the implementation, calling this    *
 * function twice yields the original texture back. This is *
 * handy because it means that we can switch from unshaded  *
 * textures to shaded ones and then back again, calling this    *
 * function each time we switch.                *
 ****************************************************************/

// This is only required for the Gouraud Shaded Textures rendered (TEXSHADE = 2)

void
FixTexture(Bitmap *texture)
{
    long *lsrc;
    long numpixs;
    long i;

    numpixs = ((long)texture->width * (long)texture->height)/4;
    lsrc = (long *)texture->data;

    for (i = 0; i < numpixs; i++) {
        *lsrc++ ^= 0x00800080L;
        *lsrc++ ^= 0x00800080L;
    }
}

/*
 * Fix up all textures in a models.
 */

void
FixModelTextures(N3DObjdata *model)
{
    int j;
    Bitmap *map;

    for (j = 0; j < model->nummaterials; j++) {
        map = model->materials[j].tmap;
        if (map) {
            FixTexture(map);
        }
    }
}
