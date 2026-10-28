
#ifndef SCREEN_H
#define SCREEN_H

#include "jag3d/jag3d.h"
#include "blit.h"

//
// In addition to defines used below, this includes OPSet(), which
//  sets which object list will be copied into place during the
//  next vertical blank
//
#include "olist.h"

//
// Function to build the object lists based on the definitions below
//
void setup_object_lists(void);

/****************************************************************
 *  Defines                         *
 ****************************************************************/

/* camera width and height */
#define CAMWIDTH 320
#define CAMHEIGHT 200

/* the width and height of the screen object */
#define OBJWIDTH 320
#define OBJHEIGHT 200
#define WIDFLAG WID320      // blitter flag corresponding to OBJWIDTH

//
// Un-comment this define to test indexed mode, along with selecting
// one of the INDEXED = 1 renderers from renderer.s
//
// #define INDEXEDMODE

#ifdef INDEXEDMODE
// setup for indexed pixels
#define PIXELFLAG PIXEL8    // blitter flag corresponding to pixel depth
#define PIXELSPERPHRASE 8  // how many pixels per 8-byte phrase
#define DEPTH 3                 // 4 == 16-bit, 3 == 8-bit
#define S1ZOFFS ZOFFS0      // offset from screen 1 to z buffer
#define S2ZOFFS ZOFFS0      // offset from screen 1 to z buffer
#define CLEAR_COLOR 0       // colour index to clear to

#else
// setup for 16-bit CrY
#define PIXELFLAG PIXEL16   // blitter flag corresponding to pixel depth
#define PIXELSPERPHRASE 4  // how many pixels per 8-byte phrase
#define DEPTH 4                 // 4 == 16-bit, 3 == 8-bit
#define S1ZOFFS ZOFFS2      // offset from screen 1 to z buffer
#define S2ZOFFS ZOFFS1      // offset from screen 1 to z buffer
#define CLEAR_COLOR 0x27a027a0

#endif //INDEXEDMODE

/* the 2 screen buffers */
extern short DISPBUF0[], DISPBUF1[];
#define DATA1 ((char *)DISPBUF0)
#define DATA2 ((char *)DISPBUF1)

//
// the number of objects in the object list, not counting the initial STOP objects
//
#define OLIST_LEN 2

/* length in bytes of a screen line (2 data buffers+1 Z buffer ->
   3*2 = 6 bytes/pixel */
#define LINELEN (OBJWIDTH*6L)

/* object list for first screen */
union olist buf1_olist[OLIST_LEN] =
{
    {{OL_BITMAP,    /* type */
     14+(320-OBJWIDTH)/2, 20+(240-OBJHEIGHT),       /* x, y */
     0L,        /* link */
     DATA1,     /* data */
     OBJHEIGHT, OBJWIDTH*3/PIXELSPERPHRASE, OBJWIDTH/PIXELSPERPHRASE,       /* height, dwidth, iwidth */
     DEPTH, 3, 0, 0, 0, /* depth, pitch, index, flags, firstpix */
     0,0,0}},       /* scaling stuff */

    {{OL_STOP}}
};

/* object list for second screen */
union olist buf2_olist[OLIST_LEN] =
{
    {{OL_BITMAP,    /* type */
     14+(320-OBJWIDTH)/2, 20+(240-OBJHEIGHT),       /* x, y */
     0L,        /* link */
     DATA2,     /* data */
     OBJHEIGHT, OBJWIDTH*3/PIXELSPERPHRASE, OBJWIDTH/PIXELSPERPHRASE,       /* height, dwidth, iwidth */
     DEPTH, 3, 0, 0, 0, /* depth, pitch, index, flags, firstpix */
     0,0,0}},       /* scaling stuff */

    {{OL_STOP}}
};

/* Bitmaps for the two screens */
Bitmap scrn1 = {
    CAMHEIGHT, CAMWIDTH,
    PIXELFLAG|PITCH3|S1ZOFFS|WIDFLAG,
    (void *)(DATA1 + ((OBJWIDTH-CAMWIDTH)*3L) + (((OBJHEIGHT-CAMHEIGHT)/2)*LINELEN) ),
    CLEAR_COLOR
};

/* initial data for camera corresponding to second screen buffer */
Bitmap scrn2 = {
    CAMHEIGHT, CAMWIDTH,
    PIXELFLAG|PITCH3|S2ZOFFS|WIDFLAG,
    (void *)(DATA2 + ((OBJWIDTH-CAMWIDTH)*3L) + (((OBJHEIGHT-CAMHEIGHT)/2)*LINELEN) ),
    CLEAR_COLOR
};

/* storage for packed object lists */
long packed_olist1[160];
long packed_olist2[160];

/****************************************************************
 *  External functions                  *
 ****************************************************************/

extern void VIDsync(void);          /* waits for a vertical blank interrupt */

#endif // SCREEN_H
