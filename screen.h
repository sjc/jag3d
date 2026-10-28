
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
 *  Defines                                                     *
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

// TODO: Triple-buffering breaks something with indexed mode. Plus we really don't
//  need to allocate a Z buffer. Fix this. Make the config global.

#ifdef INDEXEDMODE
// setup for indexed pixels
#define PIXELFLAG PIXEL8    // blitter flag corresponding to pixel depth
#define PIXELSPERPHRASE 8  // how many pixels per 8-byte phrase
#define DEPTH 3                 // 4 == 16-bit, 3 == 8-bit
#define S1ZOFFS ZOFFS0      // offset from screen 1 to the (non-existant) z buffer
#define S2ZOFFS ZOFFS0      // offset from screen 2 to the (non-existant) z buffer
#define S3ZOFFS ZOFFS0      // offset from screen 3 to the (non-existant) z buffer
#define CLEAR_COLOR 0       // colour index to clear to

#else
// setup for 16-bit CrY
#define PIXELFLAG PIXEL16   // blitter flag corresponding to pixel depth
#define PIXELSPERPHRASE 4  // how many pixels per 8-byte phrase
#define DEPTH 4                 // 4 == 16-bit, 3 == 8-bit
#define S1ZOFFS ZOFFS3      // offset from screen 1 to the z buffer
#define S2ZOFFS ZOFFS2      // offset from screen 2 to the z buffer
#define S3ZOFFS ZOFFS1      // offset from screen 3 to the z buffer
#define CLEAR_COLOR 0x27a027a0

#endif //INDEXEDMODE

#define SCREENX     (14+(320-OBJWIDTH)/2)
#define SCREENY     (20+(240-OBJHEIGHT))

//
// The 3 screen buffers
//
extern short DISPBUF0[], DISPBUF1[], DISPBUF2[];
#define DATA1 ((char *)DISPBUF0)
#define DATA2 ((char *)DISPBUF1)
#define DATA3 ((char *)DISPBUF2)

//
// the number of objects in the object list, not counting the initial STOP objects
//
#define OLIST_LEN 2

#define OLIST(LABEL,X,Y,DATA) union olist LABEL[OLIST_LEN] = { \
    {{OL_BITMAP, X, Y, \
        0L, DATA, OBJHEIGHT, OBJWIDTH*4/PIXELSPERPHRASE, OBJWIDTH/PIXELSPERPHRASE, \
        DEPTH, 4, 0, 0, 0, \
        0,0,0}}, \
    {{OL_STOP}}, \
}; \

//
// Object list definitions for the three screens
// These will be 'compiled' to packed object lists by setup_object_lists()
//
OLIST(buf1_olist, SCREENX,SCREENY, DATA1)
OLIST(buf2_olist, SCREENX,SCREENY, DATA2)
OLIST(buf3_olist, SCREENX,SCREENY, DATA3)

//
// Bitmaps for the three screens
// Note that each has a different SxZOFFS value (defined above) because each
//  has a different offset from the Z buffer
//
Bitmap scrn1 = {
    CAMHEIGHT, CAMWIDTH,
    PIXELFLAG|PITCH4|S1ZOFFS|WIDFLAG,
    (void *)DATA1,
    CLEAR_COLOR
};

Bitmap scrn2 = {
    CAMHEIGHT, CAMWIDTH,
    PIXELFLAG|PITCH4|S2ZOFFS|WIDFLAG,
    (void *)DATA2,
    CLEAR_COLOR
};

Bitmap scrn3 = {
    CAMHEIGHT, CAMWIDTH,
    PIXELFLAG|PITCH4|S3ZOFFS|WIDFLAG,
    (void *)DATA3,
    CLEAR_COLOR
};

//
// Storage for the packed object lists
//
long packed_olist1[16];
long packed_olist2[16];
long packed_olist3[16];


/****************************************************************
 *  External functions                                          *
 ****************************************************************/

extern void VIDsync(void);          /* waits for a vertical blank interrupt */

#endif // SCREEN_H
