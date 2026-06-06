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
extern long renderer_render_points[];

/* from lib/gpulib.s */
extern void GPUload(void *);    /* loads a package into the GPU */
extern void GPUrun(void *);     /* runs a GPU program */

/* parameter space in GPU RAM -- currently only 4 longs long */
extern long params[];

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

//
// Points / Particles
//

void 
ClearPoints(Particle *particles, short max_count)
{
    while (max_count--) {
        particles->ttl = 0;
        particles++;
    }
}

// this assumes that there will be at least one free element in the list
Particle *FindFreePoint(Particle *particles)
{
    while (particles->ttl) particles++;
    return particles;
}

void
RenderPointsCustom(void *gpufunc, Particle *particles, short count)
{    
    if (count == 0) return;

    params[0] = (long)count;
    params[1] = (long)particles;

    GPUrun(gpufunc);
}

void
RenderPoints(Particle *particles, short count) {
    RenderPointsCustom(renderer_render_points, particles, count);
}

/****************************************************************
 *  N3Dclrbuf(buf)                      *
 * Clears the bitmap pointed to by "buf", filling its data with *
 * a solid color, and its Z buffer with a null value        *
 ****************************************************************/

// NOTE: This is now done on the GPU as part of SetupFrame()

void
ClearScreenAndZBuffer(Bitmap *buf) 
{
    long zvalue = 0xffffffff;       /* Z value (16.16 fraction) */

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
    B_COUNT = ((long)buf->height << 16) | (buf->width);
    B_CMD = UPDA1|DSTWRZ|PATDSEL;
}

void
ClearScreenBuffer(Bitmap *buf)
{
    B_PATD[0] = buf->clearcolor;
    B_PATD[1] = buf->clearcolor;
    A1_BASE = (long)buf->data;
    A1_STEP = 0x00010000L | ((-buf->width) & 0x0000ffff);
    A1_FLAGS = buf->blitflags|XADDPHR;
    A1_PIXEL = 0;
    A1_CLIP = 0;
    B_COUNT = ((long)buf->height << 16) | (buf->width);
    B_CMD = UPDA1|PATDSEL;
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

/*
 * CrY colours
 */

extern unsigned short do_cry(int, int, int);

unsigned short RGBtoCrY(short r, short g, short b) {
    return do_cry(r, g, b);
}

// unsigned short RGB16toCrY(short rgb16) {

// }

/*
 * Model Helpers
 */

void CopyModelShallow(N3DObjdata *original, N3DObjdata *copy, Material *materials) {
    copy->numpolys      = original->numpolys;
    copy->numpoints     = original->numpoints;
    copy->nummaterials  = original->nummaterials;
    copy->reserved      = original->reserved;
    copy->faces         = original->faces;
    copy->points        = original->points;
    copy->materials     = materials ?: original->materials;
}

short SizeOfModel(N3DObjdata *model, short copy_faces, short copy_points, short copy_materials) {

    // assume packing: model, points, faces, materials without additional padding

    short size = sizeof(N3DObjdata);

    if (copy_faces) {
        // check for faces which contain more than 3 faces
        void *ptr = (void *)model->faces;
        short count = model->numpolys;

        while (count--) {
            short points = ((Face *)ptr)->npts;
            short extra = (points - 3) * 4; // 4 bytes for every extra point
            size += sizeof(Face) + extra;
            ptr += sizeof(Face) + extra;
        }
    }

    if (copy_points) size += (sizeof(Point) * model->numpoints);
    if (copy_materials) size += (sizeof(Material) * model->nummaterials);

    return size;
}
