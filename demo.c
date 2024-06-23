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
#define WIDFLAG WID320		/* blitter flags corresponding to OBJWIDTH */

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

extern void VIDon(int);				/* turns video on */
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

/* object list for first screen */
union olist buf1_olist[2] =
{
	{{OL_BITMAP,	/* type */
	 14+(320-OBJWIDTH)/2, 20+(240-OBJHEIGHT),		/* x, y */
	 0L,		/* link */
	 DATA1,		/* data */
	 OBJHEIGHT, OBJWIDTH*3/4, OBJWIDTH/4,		/* height, dwidth, iwidth */
	 4, 3, 0, 0, 0,	/* depth, pitch, index, flags, firstpix */
	 0,0,0}},		/* scaling stuff */

	{{OL_STOP}}
};

/* object list for second screen */
union olist buf2_olist[2] =
{
	{{OL_BITMAP,	/* type */
	 14+(320-OBJWIDTH)/2, 20+(240-OBJHEIGHT),		/* x, y */
	 0L,		/* link */
	 DATA2,		/* data */
	 OBJHEIGHT, OBJWIDTH*3/4, OBJWIDTH/4,		/* height, dwidth, iwidth */
	 4, 3, 0, 0, 0,	/* depth, pitch, index, flags, firstpix */
	 0,0,0}},		/* scaling stuff */

	{{OL_STOP}}
};

#define CLEAR_COLOR 	0x27a027a0

/* Bitmaps for the two screens */
Bitmap scrn1 = {
	CAMWIDTH, CAMHEIGHT,
	PIXEL16|PITCH3|ZOFFS2|WIDFLAG,
	(void *)(DATA1 + ((OBJWIDTH-CAMWIDTH)*3L) + (((OBJHEIGHT-CAMHEIGHT)/2)*LINELEN) ),
	CLEAR_COLOR
};

/* initial data for camera corresponding to second screen buffer */
Bitmap scrn2 = {
	CAMWIDTH, CAMHEIGHT,
	PIXEL16|PITCH3|ZOFFS1|WIDFLAG,
	(void *)(DATA2 + ((OBJWIDTH-CAMWIDTH)*3) + (((OBJHEIGHT-CAMHEIGHT)/2)*LINELEN) ),
	CLEAR_COLOR
};

Lightmodel lightm = {
	0x0000,
	1,
	{
	  { -0x24f3, -0x24f3, -0x24f3, 0x4000 },
	  { 0, 0, 0, 0xC000 },
	  { 0x4000, 0, 0, 0x4000 },
	  { 0, 0x4000, 0, 0x4000 },
	  { 0, 0, 0x4000, 0x4000 },
	}
};


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

int
main()
{
	int i, temp;
	int drawbuf;				/* flag: 0 means first buffer, 1 means second */
	Bitmap *curwindow;		/* pointer to output bitmap */
	Angles camangles; 		/* camera position and rotation */
	long buts, shotbuts;		/* joystick buttons pushed */
	long curframe;				/* current frame counter */
	long framespersecond;	/* frames per second counter */
	long time;					/* elapsed time */
	
	char buf[256];			/* scratch buffer for sprintf */

	N3DObject objects[OBJCOUNT]; 	/* objects which will be drawn each frame */
	TPoint *tpoints; 					/* scratch space for GPU point transforms */

	/* build packed versions of the two object lists */
	/* (output is double buffered)			 */
	OLbldto(buf1_olist, packed_olist1);
	OLbldto(buf2_olist, packed_olist2);

	/* initialize the video */
	OLPset(packed_olist2);
	VIDon(0x6c1);			/* 0x6c1 = CRY; 0x6c7 = RGB */

	/* wait for video sync (paranoid code) */
	VIDsync();

	/* clear the drawing area to black */
	//memset(DATA1, 0x00, OBJWIDTH*(long)OBJHEIGHT*2L*3);	/* clear screen to black */

	drawbuf = 0;			/* draw on buffer 1, while displaying buffer 2 */

	/* initialize the test objects */
	for (i = 0; i < OBJCOUNT; i++) {
		memset(&objects[i], 0, sizeof(N3DObject));
		objects[i].data = models[i].data;
		objects[i].angles.xpos = models[i].initx;
		objects[i].angles.ypos = models[i].inity;
		objects[i].angles.zpos = models[i].initz;
	}

	/* set up the viewer's position */
	camangles.alpha = camangles.gamma = 0;
	camangles.beta = 256;
	camangles.xpos = camangles.ypos = camangles.zpos = 0;

	/* initialize timing information */
	curframe = _timestamp;			/* timestamp is updated every vblank, and is elapsed time in 300ths of a second */
	framespersecond = 1;

	/* fix all model textures, and find maximum number of points */
	temp = 0;
	for (i = 0; i < MODEL_COUNT; i++) {
		// This is only required for the Gouraud Shaded Textures rendered (TEXSHADE = 2)
		//FixModelTextures(models[i].data);
		
		// Check point count
		if (models[i].data->numpoints > temp) {
			temp = models[i].data->numpoints;
		}
	}

	/* allocate temporary storage for the GPU to transform points */
   tpoints = malloc(temp * sizeof(TPoint));

	/* load the renderer into GPU memory */
	LoadRenderer(renderer_code);

	/* loop forever */
	for(;;) {
		/* select bitmap for drawing */
		curwindow = (drawbuf) ? &scrn2 : &scrn1;

		/* update objects, looking-up sin and cos for current rotations */
		for (i = 0; i < OBJCOUNT; i++) {
			UpdateAngles(&objects[i].angles);
		}

		/* update camera, looking-up sin and cos for current rotations */
		UpdateAngles(&camangles);

		/* clear the current draw buffer */
		/* TODO: this could be moved to the GPU, as part of frameinit() */
		ClearBuffer(curwindow);

		/* setup the shared rendering state for this frame */
		SetupFrame(renderer_frameinit, curwindow, &camangles);

		/* now draw the objects, timing how long it takes */
		/* NOTE: the clock() function uses unsupported hardware
		 * mechanisms; it happens to work on current developer
		 * machines, but will fail on some Jaguars. If this
		 * were production code, we would have to use a
		 * different timing mechanism!
		 */
		time = clock();
		
		for (i = 0; i < OBJCOUNT; i++) {
			RenderObject(renderer_objinit, &objects[i], &lightm, tpoints);
		}

		time = clock() - time;

		/* debug status display */
		/* frames per second */
		sprintf(buf, "%d fps", (int)framespersecond);
		FNTstr(20, 0, buf, curwindow->data, curwindow->blitflags, usefnt, 0x27ff, 0 );

		/* use this one for whatever you like */
		sprintf(buf, "Left/right to rotate, up/down to raise/lower");
		FNTstr(20, 12, buf, curwindow->data, curwindow->blitflags, usefnt, 0xf0ff, 0 );

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
			camangles.beta -= 4;
		} else if (buts & JOY_RIGHT) {
			camangles.beta += 4;
		}

		// up/down raises or lowers the camera
		if (buts & JOY_UP) {
			camangles.ypos -= 4;
			camangles.alpha -= 2;
		} else if (buts & JOY_DOWN) {
			camangles.ypos += 4;
			camangles.alpha += 2;
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
