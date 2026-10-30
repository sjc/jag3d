/*
 * Test program for new 3D renderer.
 * Copyright 1995 Atari Corporation.
 * All Rights Reserved.
 */

#include "font.h"
#include "joypad.h"
#include "string.h"
#include "stdlib.h"

#include "jag3d/jag3d.h"

#include "screen.h" 	// setting up object lists and draw buffers

#include "engine.h"
#include "scenes/demo.h"

#include "models.h" // TEMP

/****************************************************************
 *  Defines                         									 *
 ****************************************************************/

/* 1/100th of the clock speed */
// #define MHZ 265900L



/****************************************************************
 *	External functions					*
 ****************************************************************/

extern int sprintf(char *, const char *, ...);	/* you know what this does */

/* NOTE: clock() is useful only for debugging; it works on
 * current developer consoles, but may fail on production
 * units and/or future Jaguar consoles!
 */
// extern unsigned long clock(void);		/* timer function using the PIT */

/****************************************************************
 *	External variables					*
 ****************************************************************/

/* library count of number of 300ths of a second elapsed */
extern long _timestamp;

/* timing information */
// extern long proftime[];

/* the font we'll use for printing stuff on screen */
extern FNThead usefnt[];





/****************************************************************
 *	main()																		 *
 * The main demo loop.														 *
 ****************************************************************/

int
main()
{
	Bitmap *shown_buffer; 	// the buffer associated with the current displayed object list
	Bitmap *draw_buffer; 	// the buffer into which we should draw, which will be shown next
	Bitmap *clear_buffer; 	// the buffer we should clear ready for drawing into next frame

	long *shown_olist; 		// the object list which is currently being shown
	long *draw_olist;  		// the object list which will be shown next
	long *clear_olist;  		// the object list associated with the buffer to clear

	long buts, shotbuts;		// joypad input

	long curframe;				/* current frame counter */
	long framespersecond;	/* frames per second counter */
	// long time;					/* elapsed time */
	
	char buf[256];				/* scratch buffer for sprintf */



	// build object lists from the definitions in screen.h
	setup_object_lists();

	// assign our tripple buffers -- use these from now on
	shown_buffer 	= &scrn1;
	draw_buffer 	= &scrn2;
	clear_buffer 	= &scrn3;

	shown_olist 	= packed_olist1;
	draw_olist 		= packed_olist2;
	clear_olist 	= packed_olist3;

	ClearScreenBuffer(shown_buffer);
	// TODO: do we need to wait for blitter?
	ClearScreenAndZBuffer(draw_buffer);

	// set the initial object list
	OLPset(shown_olist);

	/* wait for video sync (paranoid code) */
	VIDsync();

	// reset value used by the 'engine'
	setup_engine();

	// one time setup of materials for the logo on the 3D sprite
	setup_models();


	//camangles = (void *)((long)malloc(sizeof(Transform) + 3) & 0xFFFFFFFC);

	/* set up the viewer's position */
	// camangles->alpha = camangles->gamma = 0;
	// camangles->beta = 0;
	// camangles->xpos = camangles->ypos = camangles->zpos = 0;


	// This is only required for the "Gouraud Shaded Textures" rendered (TEXSHADE = 2)
	// for (i = 0; i < EXPLOSION_COUNT; i++) {
	// 	FixTexture(explosion_material[i]->tmap);
	// }

	/* initialize timing information */
	curframe = _timestamp;			/* timestamp is updated every vblank, and is elapsed time in 300ths of a second */
	framespersecond = 1;

	/* load the renderer into GPU memory */
	LoadAndInitRenderer();

	// setup the default scene
	setup_demo_scene();

	/* loop forever */
	for(;;) {

		/* wait for vblank */
		VIDsync();

		/* calculate frames per second, etc. */
		framespersecond = 300/(_timestamp - curframe);
		curframe = _timestamp;

		/* setup the shared rendering state for this frame:
			 - calculates the camera matrix
			 - clears the image- and z-buffer (if INITCLEARBUFFER is set)
		*/
		SetupFrame(draw_buffer, camtrans);

		/* now draw the objects, timing how long it takes */
		/* NOTE: the clock() function uses unsupported hardware
		 * mechanisms; it happens to work on current developer
		 * machines, but will fail on some Jaguars. If this
		 * were production code, we would have to use a
		 * different timing mechanism!
		 */
		// time = clock();

		/* buts will contain all buttons currently pressed */
		/* shotbuts will contain the ones that are pressed now, but weren't
		   pressed last time JOYget() was called
		 */
		buts = JOYget(JOY1);
		shotbuts = JOYedge(JOY1);

		if (process_input_cb) process_input_cb(buts, shotbuts);

		if (process_frame_cb) process_frame_cb();

		if (render_frame_cb) render_frame_cb();

		// time = clock() - time;

#ifndef INDEXEDMODE
		/* debug status display */
		/* frames per second */
		sprintf(buf, "%d fps", (int)framespersecond);
		FNTstr(20, 0, buf, draw_buffer->data, draw_buffer->blitflags, usefnt, 0x27ff, 0 );

		/* use this one for whatever you like */
		sprintf(buf, "L/R: rotate, U/D: forward/back, 4/6: left/right");
		FNTstr(20, 12, buf, draw_buffer->data, draw_buffer->blitflags, usefnt, 0xf0ff, 0 );
#endif

		// display the object list containing the buffer we just drew into
		OLPset(draw_olist);

		// begin clearing the next buffer
		if (clear_buffer_cb) clear_buffer_cb(clear_buffer);

		// now swap all the buffers and list pointers around
		Bitmap *temp_buffer = shown_buffer;
		shown_buffer = draw_buffer;
		draw_buffer = clear_buffer;
		clear_buffer = temp_buffer;

		long *temp_olist = shown_olist;
		shown_olist = draw_olist;
		draw_olist = clear_olist;
		clear_olist = temp_olist;
	}

	return 0;
}
