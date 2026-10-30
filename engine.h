//
// engine.h
//

#ifndef ENGINE_H
#define ENGINE_H

#include "jag3d/jag3d.h"
// #include "const.h"
// #include "screen.h"

#include "counters.h"
#include "tweens.h"

//
// low-level functions
//

extern void VIDsync(void);          // waits for a vertical blank interrupt
extern long vb_ticks;               // count of VBIs

//
// one-time init
//

void setup_engine(void);

//
// global vars
//

extern Transform camtrans[];  // camera position and rotation

// extern Lightmodel lightm;     // the scene lights
// extern Lightmodel nolightm;

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

TPoint *tpoints;            // pointer to scratch space for GPU point transforms

long curvbtick;             // current vertical blank counter

// extern short cheats_enabled;    // level select and option+powerup

// #ifdef DEBUG
// extern long test_returns[];
// short can_display_debug;   // set by a scene to indicate the HUD is capable of displaying debug
// long debug_value;          // might be useful for setting it from elsewhere
// #endif

//
// callbacks
//

typedef void (*InputCallback)(unsigned long down, unsigned long pressed);

//
// process non-player input
// read_pad_one() has been called before this is invoked
//
InputCallback process_input_cb;

typedef void (*EngineCallback)(void);

//
// called after all objects have been moved
// may be called multiple times before a frame is rendered
//
EngineCallback process_moved_cb;

//
// called when a frame is about to be rendered
//
EngineCallback process_frame_cb;

//
// called to render a frame after SetupFrame() has been called
//
EngineCallback render_frame_cb;

typedef void (*BufferCallback)(Bitmap *);

//
// called to clear the next buffer ready for rendering
//
BufferCallback clear_buffer_cb;

//
// restart from the menu screen
//
void reset_state(void);

#endif // ENGINE_H
