/*
 * Test program for new 3D renderer.
 * Copyright 1995 Atari Corporation.
 * All Rights Reserved.
 */

#include "olist.h"
#include "font.h"
#include "blit.h"
#include "joypad.h"
#include "string.h"
#include "stdlib.h"

#include "jag3d/jag3d.h"

/****************************************************************
 *	Defines							*
 ****************************************************************/

/* camera width and height */
#define CAMWIDTH 320
#define CAMHEIGHT 200

/* the width and height of the screen object */
#define OBJWIDTH 320
#define OBJHEIGHT 200
#define WIDFLAG WID320		// blitter flag corresponding to OBJWIDTH

#define INDEXEDMODE

#ifdef INDEXEDMODE
// setup for indexed pixels
#define PIXELFLAG PIXEL8 	// blitter flag corresponding to pixel depth
#define PIXELSPERPHRASE 8  // how many pixels per 8-byte phrase
#define DEPTH 3  				// 4 == 16-bit, 3 == 8-bit
#define S1ZOFFS ZOFFS0   	// offset from screen 1 to z buffer
#define S2ZOFFS ZOFFS0   	// offset from screen 1 to z buffer
#define CLEAR_COLOR 0

#else
// setup for 16-bit CrY
#define PIXELFLAG PIXEL16 	// blitter flag corresponding to pixel depth
#define PIXELSPERPHRASE 4  // how many pixels per 8-byte phrase
#define DEPTH 4  				// 4 == 16-bit, 3 == 8-bit
#define S1ZOFFS ZOFFS2   	// offset from screen 1 to z buffer
#define S2ZOFFS ZOFFS1   	// offset from screen 1 to z buffer
#define CLEAR_COLOR 0x27a027a0

#endif //INDEXEDMODE

/* length in bytes of a screen line (2 data buffers+1 Z buffer ->
   3*2 = 6 bytes/pixel */
#define LINELEN (OBJWIDTH*6L)

/* 1/100th of the clock speed */
#define MHZ 265900L

/****************************************************************
 *	Type definitions					*
 ****************************************************************/

/*
 * a model consists of:
 * (1) a pointer for the data for the model
 * (2) starting X,Y,Z coordinates for the model
 */
typedef struct model {
	N3DObjdata *data;
	short initx, inity, initz;
	short reserved;		/* pads structure to a longword boundary */
} MODEL;

/****************************************************************
 *	External functions					*
 ****************************************************************/

extern void VIDsync(void);			/* waits for a vertical blank interrupt */
extern int sprintf(char *, const char *, ...);	/* you know what this does */

/* NOTE: clock() is useful only for debugging; it works on
 * current developer consoles, but may fail on production
 * units and/or future Jaguar consoles!
 */
extern unsigned long clock(void);		/* timer function using the PIT */

/****************************************************************
 *	External variables					*
 ****************************************************************/

/* library count of number of 300ths of a second elapsed */
extern long _timestamp;

/* timing information */
extern long proftime[];

/* the 2 screen buffers */
extern short DISPBUF0[], DISPBUF1[];
#define DATA1 ((char *)DISPBUF0)
#define DATA2 ((char *)DISPBUF1)

/* the font we'll use for printing stuff on screen */
extern FNThead usefnt[];

/* for 3D sprite examples */
extern N3DObjdata jaguar_logo_3d_sprite;
extern Material jaguar_logo_material;

#define JAGUAR_LOGO_WIDTH  512
#define JAGUAR_LOGO_HEIGHT 128

extern N3DObjdata explosion_3d_sprite;
extern Material explosion_0_material, explosion_1_material, explosion_2_material, explosion_3_material;

#define EXPLOSION_COUNT 	4

Material *explosion_material[EXPLOSION_COUNT] = {
	&explosion_0_material,
	&explosion_1_material,
	&explosion_2_material,
	&explosion_3_material,
};

#define EXPLOSION_WIDTH 	64
#define EXPLOSION_HEIGHT 	64

/* 3D object data */
extern N3DObjdata ship1data, globedata, radardata, torusdata, knightdata, robotdata, castledata, cubedata;

#define MOD_SHIP 		0
#define MOD_GLOBE		1
#define MOD_RADAR		2
#define MOD_TORUS		3
#define MOD_KNIGHT	4
#define MOD_ROBOT		5
#define MOD_CASTLE	6
#define MOD_CUBE		7

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

#define OLIST_LEN 2

/* object list for first screen */
union olist buf1_olist[OLIST_LEN] =
{
	{{OL_BITMAP,	/* type */
	 14+(320-OBJWIDTH)/2, 20+(240-OBJHEIGHT),		/* x, y */
	 0L,		/* link */
	 DATA1,		/* data */
	 OBJHEIGHT, OBJWIDTH*3/PIXELSPERPHRASE, OBJWIDTH/PIXELSPERPHRASE,		/* height, dwidth, iwidth */
	 DEPTH, 3, 0, 0, 0,	/* depth, pitch, index, flags, firstpix */
	 0,0,0}},		/* scaling stuff */

	{{OL_STOP}}
};

/* object list for second screen */
union olist buf2_olist[OLIST_LEN] =
{
	{{OL_BITMAP,	/* type */
	 14+(320-OBJWIDTH)/2, 20+(240-OBJHEIGHT),		/* x, y */
	 0L,		/* link */
	 DATA2,		/* data */
	 OBJHEIGHT, OBJWIDTH*3/PIXELSPERPHRASE, OBJWIDTH/PIXELSPERPHRASE,		/* height, dwidth, iwidth */
	 DEPTH, 3, 0, 0, 0,	/* depth, pitch, index, flags, firstpix */
	 0,0,0}},		/* scaling stuff */

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

Lightmodel lightm = {
	0x4000,
	1,
	{
	  { -0x24f3, -0x24f3, -0x24f3, 0x4000 },
	  { 0, 0, 0, 0xC000 },
	  { 0x4000, 0, 0, 0x4000 },
	  { 0, 0x4000, 0, 0x4000 },
	  { 0, 0, 0x4000, 0x4000 },
	}
};

Lightmodel nolightm = {
	0x8000,
	0,
	{
	  { -0x24f3, -0x24f3, -0x24f3, 0x4000 },
	  { 0, 0, 0, 0xC000 },
	  { 0x4000, 0, 0, 0x4000 },
	  { 0, 0x4000, 0, 0x4000 },
	  { 0, 0, 0x4000, 0x4000 },
	}
};

extern short vectors_100[];

/****************************************************************
 *	Global variables					*
 ****************************************************************/

/* storage for packed object lists */
int packed_olist1[160];
int packed_olist2[160];


/****************************************************************
 *	main()							*
 * The main demo loop.						*
 ****************************************************************/

#define OBJCOUNT 4

extern Material jaguar_logo_material_copy;
extern Bitmap indexed_chkbrd;
extern Bitmap chkbrd;
extern Bitmap squares;

int
main()
{
	int i, temp;
	int drawbuf;				/* flag: 0 means first buffer, 1 means second */
	Bitmap *curwindow;		/* pointer to output bitmap */
	Transform *camangles; 	/* camera position and rotation */
	long buts, shotbuts;		/* joystick buttons pushed */
	long curframe;				/* current frame counter */
	long framespersecond;	/* frames per second counter */
	long time;					/* elapsed time */
	
	char buf[256];				/* scratch buffer for sprintf */

	N3DObject *objects; 		/* objects which will be drawn each frame */
	TPoint *tpoints; 			/* scratch space for GPU point transforms */
	Transform *jaguar_logo_trans;
	Transform *explosion_trans;
	short explosion_frame, explosion_frame_count;

#ifdef INDEXEDMODE
	// setup some basic colour indexes for testing indexed drawing
	// because including <jaguar.h> breaking joypad reading...
	unsigned short *palette = (unsigned short *)0xF00400;
	palette[0] = RGBtoCrY(255, 0, 0);
	palette[1] = RGBtoCrY(0, 255, 0);
	palette[2] = RGBtoCrY(0, 0, 255);

	jaguar_logo_material_copy.color = 0x01; // this is a colour index
	jaguar_logo_material_copy.flags = 0;
	jaguar_logo_material_copy.tmap = &indexed_chkbrd;
#else
	jaguar_logo_material_copy.color = RGBtoCrY(0,255,0); // a CrY colour
	jaguar_logo_material_copy.flags = 0;
	jaguar_logo_material_copy.tmap = jaguar_logo_material.tmap;
#endif

	/* build packed versions of the two object lists */
	/* (output is double buffered)			 */
	OLbldto(buf1_olist, packed_olist1, OLIST_LEN);
	OLbldto(buf2_olist, packed_olist2, OLIST_LEN);

	/* initialize the video */
	OLPset(packed_olist2);

	/* wait for video sync (paranoid code) */
	VIDsync();

	drawbuf = 0;			/* draw on buffer 1, while displaying buffer 2 */

	objects = (void *)((long)malloc(OBJCOUNT * sizeof(N3DObject) + 3) & 0xFFFFFFFC);

	/* initialize the test objects */
	for (i = 0; i < OBJCOUNT; i++) {
		memset(&objects[i], 0, sizeof(N3DObject));
		objects[i].data = models[i].data;
		objects[i].transform.xpos = models[i].initx;
		objects[i].transform.ypos = models[i].inity;
		objects[i].transform.zpos = models[i].initz;

		// This is only required for the "Gouraud Shaded Textures" rendered (TEXSHADE = 2)
		// FixModelTextures(models[i].data);
	}

	camangles = (void *)((long)malloc(sizeof(Transform) + 3) & 0xFFFFFFFC);

	/* set up the viewer's position */
	camangles->alpha = camangles->gamma = 0;
	camangles->beta = 0;
	camangles->xpos = camangles->ypos = camangles->zpos = 0;

	/* setup the logo 3d sprite */
	Init3DSprite(&jaguar_logo_3d_sprite, &jaguar_logo_material_copy, JAGUAR_LOGO_WIDTH, JAGUAR_LOGO_HEIGHT);
	jaguar_logo_trans = (void *)((long)malloc(sizeof(Transform) + 3) & 0xFFFFFFFC);
	jaguar_logo_trans->xpos = 0;
	jaguar_logo_trans->ypos = 0;
	jaguar_logo_trans->zpos = 2000;
	jaguar_logo_trans->alpha = 0;
	jaguar_logo_trans->beta = 1024;
	jaguar_logo_trans->gamma = 0;

	// This is only required for the "Gouraud Shaded Textures" rendered (TEXSHADE = 2)
	// FixTexture(jaguar_logo_material.tmap);

	/* setup the explosion 3D sprite -- not shown yet */
	Init3DSprite(&explosion_3d_sprite, &explosion_0_material, EXPLOSION_WIDTH, EXPLOSION_HEIGHT);
	explosion_trans = (void *)((long)malloc(sizeof(Transform) + 3) & 0xFFFFFFFC);
	jaguar_logo_trans->ypos = 0;
	jaguar_logo_trans->alpha = 0;
	jaguar_logo_trans->gamma = 0;
	explosion_frame = -1;
	explosion_frame_count = 0;

	// This is only required for the "Gouraud Shaded Textures" rendered (TEXSHADE = 2)
	// for (i = 0; i < EXPLOSION_COUNT; i++) {
	// 	FixTexture(explosion_material[i]->tmap);
	// }

	/* initialize timing information */
	curframe = _timestamp;			/* timestamp is updated every vblank, and is elapsed time in 300ths of a second */
	framespersecond = 1;

	/* find the maximum number of points in the models */
	temp = 0;
	for (i = 0; i < MODEL_COUNT; i++) {
		// Check point count
		if (models[i].data->numpoints > temp) {
			temp = models[i].data->numpoints;
		}
	}

	/* allocate temporary storage for the GPU to transform points */
   tpoints = (void *)((long)malloc(temp * sizeof(TPoint) + 3) & 0xFFFFFFFC);

	/* load the renderer into GPU memory */
	LoadAndInitRenderer();

	/* loop forever */
	for(;;) {
		/* select bitmap for drawing */
		curwindow = (drawbuf) ? &scrn2 : &scrn1;

		/* setup the shared rendering state for this frame:
			 - calculates the camera matrix
			 - clears the image- and z-buffer
		*/
		SetupFrame(curwindow, camangles);

		/* now draw the objects, timing how long it takes */
		/* NOTE: the clock() function uses unsupported hardware
		 * mechanisms; it happens to work on current developer
		 * machines, but will fail on some Jaguars. If this
		 * were production code, we would have to use a
		 * different timing mechanism!
		 */
		time = clock();
		
		for (i = 0; i < OBJCOUNT; i++) {
			RenderObject(objects[i].data, &objects[i].transform, &lightm, tpoints);
		}

		// 'billboard' the 3D sprite so it's always square-on to the camera
		jaguar_logo_trans->beta = AddRotation(camangles->beta, 1024);

		RenderObject(&jaguar_logo_3d_sprite, jaguar_logo_trans, &nolightm, tpoints);

		if (explosion_frame != -1) {
			// rotate the sprite to face towards the camera
			explosion_trans->beta = AngleAlongVector(
				camangles->xpos - explosion_trans->xpos,
				camangles->zpos - explosion_trans->zpos
			);

			RenderObject(&explosion_3d_sprite, explosion_trans, &nolightm, tpoints);
			
			// animate the explosion by changing the material applied to the sprite
			if (++explosion_frame_count == 2) {
				if (++explosion_frame == EXPLOSION_COUNT) {
					explosion_frame = -1; // stop showing
				} else {
					explosion_3d_sprite.materials = explosion_material[explosion_frame];
				}
				explosion_frame_count = 0;
			}
		}

		time = clock() - time;

#ifndef INDEXEDMODE
		/* debug status display */
		/* frames per second */
		sprintf(buf, "%d fps", (int)framespersecond);
		FNTstr(20, 0, buf, curwindow->data, curwindow->blitflags, usefnt, 0x27ff, 0 );

		/* use this one for whatever you like */
		sprintf(buf, "L/R: rotate, U/D: forward/back, 4/6: left/right");
		FNTstr(20, 12, buf, curwindow->data, curwindow->blitflags, usefnt, 0xf0ff, 0 );
#endif

		/* buts will contain all buttons currently pressed */
		/* shotbuts will contain the ones that are pressed now, but weren't
		   pressed last time JOYget() was called
		 */
		buts = JOYget(JOY1);
		shotbuts = JOYedge(JOY1);

		/* now interpret the user's keypresses */
		/* each roation step is 1/1024 PI, meaning 2048 steps in a complete rotation */
#define ROTINC 0x10

		// left/right rotates the camera around the y axis
		if (buts & JOY_LEFT) {
			camangles->beta = SubRotation(camangles->beta, 8);
		} else if (buts & JOY_RIGHT) {
			camangles->beta = AddRotation(camangles->beta, 8);
		}

		// up/down moves forward and back
		if (buts & JOY_UP) {
			short index = camangles->beta << 1;
			camangles->xpos += vectors_100[index];
			camangles->zpos += vectors_100[index+1];
		} else if (buts & JOY_DOWN) {
			short index = camangles->beta << 1;
			camangles->xpos -= vectors_100[index];
			camangles->zpos -= vectors_100[index+1];
		}

		// 4/6 LT/RT sidesteps
		if (buts & KEY_4) {
			short index = SubRotation(camangles->beta, 512) << 1;
			camangles->xpos += vectors_100[index];
			camangles->zpos += vectors_100[index+1];
		} else if (buts & KEY_6) {
			short index = AddRotation(camangles->beta, 512) << 1;
			camangles->xpos += vectors_100[index];
			camangles->zpos += vectors_100[index+1];
		}

		// 1/7 move the camera up/down
		if (buts & KEY_1) {
			camangles->ypos -= 100;
		} else if (buts & KEY_7) {
			camangles->ypos += 100;
		}

		// 3/9 angle the camera up/down
		if (buts & KEY_3) {
			camangles->alpha = SubRotation(camangles->alpha, 8);
		} else if (buts & KEY_9) {
			camangles->alpha = AddRotation(camangles->alpha, 8);
		}

		if (buts & KEY_5) {
			camangles->xpos = camangles->ypos = camangles->zpos = 0;
			camangles->alpha = camangles->beta = camangles->gamma = 0;
		}

		// button A triggers an explosion
		if (buts & FIRE_A && explosion_frame == -1) {
			// position the explosion in front of the camera at a distance
			short index = camangles->beta << 1;
			explosion_trans->xpos = camangles->xpos + (vectors_100[index] << 2);
			explosion_trans->zpos = camangles->zpos + (vectors_100[index+1] << 2);
			explosion_frame = 0;
			explosion_frame_count = 0;
			explosion_3d_sprite.materials = explosion_material[0];
		}


		/* display the buffer we just drew */
		OLPset(drawbuf ? packed_olist2 : packed_olist1);

		/* wait for vblank */
		VIDsync();

		/* calculate frames per second, etc. */
		framespersecond = 300/(_timestamp - curframe);
		curframe = _timestamp;

		/* switch drawing buffers */
		drawbuf = !drawbuf;
	}

	return 0;
}
