# Simple Game Engine Framework

The demo code in this repo is organised in such a way that it could form the template of your own projects. This document describes the parts of this template. You are welcome to pick and choose which parts you reuse, or to ignore it completely and just use the renderer.

## engine.h

Use `engine.h` to import headers for everything discussed below.

## const.inc

The file `const.inc` contains values used by parts of the engine, such as maximum sizes of arrays used to store components. It can also be used to store your own shared constants. Like `jag3dcfg.inc`, this file is converted into a companion `.h` file containing the same defined values, for use from the C code in your game.

## Game Loop

The main game loop is the infinite loop found in `main.c`. It handles a number of common functions while calling out to code the developer provides for game logic.

### Fixed Game Speed

The main loop attempts to run at 30fps on NTSC consoles and 25fps on PAL consoles. This is done on the assumption that two frames (33ms under NTSC, 40ms under PAL) is a reasonable target to render each screen. The main loop does this by keeping a count of frames processed and comparing this to the count of VBlank interrupts seen. Game logic is run every odd frame. If the main loop has fallen behind the VBlank count it will run the game logic multiple times in a row until it catches up. The screen rendering code will but run at most once per loop.

### Callbacks

The game loop invokes a number of functions during at specific points, if those function pointers are set. You can define a custom function and assign it to the pointer to have your game logic called.

`process_input_cb` is called after reading the pad inputs and is passed bit masks indicating which buttons on pad one are down and which of these have only just been pressed. You would typically use this callback to handle input based on your current game mode (eg. move the player). The example implementation in `scenes/demo.c` called `demo_process_input()` includes an example of one way to implement game pausing.

`process_frame_cb` is called every tick and can be used to run your game logic (move objects, check for collision, update the camera position, and so on).

`render_frame_cb` is called after the per-frame setup has been performed (the call to `SetupFrame()` with the camera transform and the frame buffer to be drawn into). This is where you would make the calls to render your game objects. After calling this function the buffers will be swapped.

`clear_buffer_cb` is called once we've finished drawing the next frame, while we're waiting for it to be made visible. The buffer which will be used to draw the next frame after that is passed as parameter. Most of the time you will want to call `ClearScreenAndZBuffer()` (or `ClearScreenBuffer()` if you're using indexed mode), but if your game world has a background which can be drawn that can be done here.

#### Scenes

The callbacks listed above can be used to group your game code into 'scenes'. The demo project contains an example of this, with the `scenes/` directory containing the implementation files. Each contains example callback functions, along with a `_setup()` function which is used to install the callback handlers. You can swap between scenes with `OPTION` + `1` or `3`.

### Reset

The game loop includes handling of soft resets, triggered by the player pressing `*` and `#` together. This code calls a function named `reset_state()`, which the developer should provide. In the case of the demo, this is defined in `main.c`.

### Triple Buffers

The example game loop uses three buffers. Generally, the first is being displayed, while the second is either being drawn into, or has been drawn into and is waiting to be displayed. The third buffer will be drawn into during the next pass through the game loop. In the meantime, we can clear it (or otherwise set it up, see the `clear_buffer_cb` above) while we wait for the first and second buffers to swap and the next game loop to begin.

### Text Overlay

The original version of the "New 3D" example code included functions to write text directly into the same buffer as we draw the 3D scene to. Doing it this way has a few disadvantages, mainly that it needs to be done every time we draw the scene, and can slow things down. If you use the overlay to display score and lives, it's unlikely that you will need to update these every frame.

The example code now includes overlay indexed bitmap objects, along with simple routines for writing text into them. These need only be updated when the information they are displaying has changed, rather than every frame.

The old example text-drawing code is still in the repo if you wish to use it.

## Counters

`counters.h` and its accompanying implementation file include some useful values which will allow your game to run at the same pace between NTSC and PAL consoles. It introduces the 'tock', a lowest common denominator time signal which occurs every 1/5th of a second under both timings. This is driven by the `tick_counter()` function which is called during the game loop. It also exports values for rotation steps and movement vectors which converge to the same values across a 'tock'. These are described in the header file.

## Tweens

TODO
