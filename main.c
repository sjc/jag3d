/*
 * Test program for new 3D renderer.
 * Copyright 1995 Atari Corporation.
 * All Rights Reserved.
 */

#include "joypad.h"
#include "timer.h"
#include "string.h"
#include "stdlib.h"
#include "text.h"

#include "jag3d/jag3d.h"

#include "screen.h" 	// setting up object lists and draw buffers

#include "engine.h"
#include "scenes/demo.h"
#include "counters.h"

#include "models.h" // TEMP

/****************************************************************
 *	External functions					*
 ****************************************************************/

extern int sprintf(char *, const char *, ...);	/* you know what this does */

/****************************************************************
 *	External variables					*
 ****************************************************************/

/* library count of number of 300ths of a second elapsed */
extern long _timestamp;

extern short font_ac[]; // font used for framerate

//
// Reset the game to a known state (eg. the title screen, if you've got that far)
//	 in response to the user pressing * and #
//
void reset_state() {
	// init the main demo scene
	setup_demo_scene();
}

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

	short delta, updated; 	// for the movement and rendering code

	long buts, shotbuts;		// joypad input

	long curframe;				/* current frame counter */
	long framespersecond;	/* frames per second counter */
	short logic_time, render_time, idle_time;

	char buf[256];				/* scratch buffer for sprintf */

	setup_timer();

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

	// clear the buffer used to overlay text
	clear_hud_buffer();

	// setup colours used in the HUD overlay
	*(unsigned short int *)0xf00402 = 0x88ff; /* Color reg 1 = white */

	/* wait for video sync (paranoid code) */
	VIDsync();

	// reset value used by the 'engine'
	setup_engine();

	// setup counters and values used to sync between NTSC and PAL
	setup_counters();

	// one time setup of materials for the logo on the 3D sprite
	setup_models();

	/* initialize timing information */
	curframe = _timestamp;			/* timestamp is updated every vblank, and is elapsed time in 300ths of a second */
	framespersecond = 1;
	logic_time = render_time = idle_time = 0;

	/* load the renderer into GPU memory */
	LoadAndInitRenderer();

	// setup the default scene
	reset_state();

	curvbtick = vb_ticks;

	/* loop forever */
	while(1) {

		/* wait for the next vblank, if we haven't already missed it */
		if (curvbtick == vb_ticks) VIDsync();

		// 60hz = 16.66ms per frame
		// 50hz = 20ms per frame

		idle_time = read_timer();
		idle_time = idle_time - (render_time <= idle_time ? render_time : logic_time);
		reset_timer();

		/* calculate frames per second, etc. */
		framespersecond = 300/(_timestamp - curframe);
		curframe = _timestamp;

		/* buts will contain all buttons currently pressed */
		/* shotbuts will contain the ones that are pressed now, but weren't
		   pressed last time JOYget() was called
		 */
		buts = JOYget(JOY1);
		shotbuts = JOYedge(JOY1);

		// check for * and # pressed to trigger a reset
		// specifically, check that both are down and at least one of them has just
		//	 transitioned to down, to avoid triggering on repeats
		if ((buts & KEY_S) && (buts & KEY_H) && ((shotbuts & KEY_S) || (shotbuts & KEY_H))) {
			reset_state();
		}

		if (process_input_cb) process_input_cb(buts, shotbuts);

		/* calculate how many frames we have to process */
		delta = (short)(vb_ticks - curvbtick);
		updated = 0;

		while (delta--) {

			// increase the frame counter towards vb_ticks
			curvbtick++;

			if (curvbtick & 1) {
				//
				// Run the game logic for every odd frame, at 30fps (25fps PAL)
				// The general idea is that we assume it will take over one frame to
				//  render each screen. If it has taken more than 2 frames, we run
				//	 the game logic without re-rendering to allow it to catch up.
				//
				tick_counter();

				if (process_frame_cb) process_frame_cb();

				updated = 1; // we have something to render
			}
		}

		logic_time = read_timer();

		if (updated) {
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

			if (render_frame_cb) render_frame_cb();

			/* debug status display */
			/* frames per second, duration for logic, render and idle */
			sprintf(buf, "%d fps (%dms, %dms, %dms)  ", (int)framespersecond, (int)logic_time, (int)render_time, (int)idle_time);
			draw_string_to_buffer(buf, PEN1, HUDBUFFER, HUDSTRIDE, font_ac);

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

			render_time = read_timer() - logic_time;
		}
	}

	return 0;
}
