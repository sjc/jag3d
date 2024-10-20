# Atari Jaguar 3D Demo

This is a modified version of Atari's 3D demo for the Jaguar. Specifically, it is modified from the version included in the [jaguar-sdk](https://github.com/cubanismo/jaguar-sdk).

## Changes

- Rationalised project file structure
    - Renderer code and supporting files moved into `/jag3d` directory
    - Models moved into their own directories under `/models`, with a Makefile stage to build each
        - Prefix a directory name with `_` to exclude it
    - Other misc files moved into directories, leaving the project root for your code
    - TODO: Build into a `/build` directory and stop littering `.o` files everywhere
- Refactored the renderer
    - Interface changed to better suit use in a game
        - Load into GPU memory and init once
        - One entry point to init shared data (image buffer, camera matrix) once per frame
        - Second entry point to draw each object, to allow multiple calls with minimum re-setup
    - Moved object matrix calculation code to the GPU (replacing 68k `mkMatrix()`)
    - Consolidated all example renderers into a single file
        - Renderer type chosen at compile time
        - The assumption is that a project will choose one type of renderer to use
            - Support for multiple renders within one project is possible
    - Drawing code is unchanged, because I don't understand that stuff

## Building

The [jaguar-sdk](https://github.com/cubanismo/jaguar-sdk) is the supported method of building this code.

## Running

The 3D demo will not run on [Virtual Jaguar](https://icculus.org/virtualjaguar/). This is a known issue with all version of this code.

The demo will run with [the BigPEmu](https://www.richwhitehouse.com/jaguar/) emulator. This can be used along with the [Neosis](https://richwhitehouse.com/index.php?content=inc_projects.php&showproject=91) debugger.

This code has been tested on genuine Jaguar hardware and found to work just fine.

## Models

Each model should be places in its own directory under `/models`. The build process will try to build a model from the files inside each directory. To exclude a directory, prefix its name with `_`.

Model data will be compiled from `.3ds` (via `3dsconv`), `.s` or `.c` files. All `.tga` files will be converted to `.cry` (via `tga2cry`) and included in the binary.

Models converted from `.3ds` files will be exposed in the code with the symbol `<filename>data`. eg. to reference the model built from `ship1.3ds` use `extern N3DObjdata ship1data;`. Models built from `.s` or `.c` source will use the global symbols declared there.

A directory can contain multiple model files.

## Data Types

The interface to the library (described below) requires two pieces of information to represent the model to be rendered: the model data (`N3DObjdata`, described above) and a `Transform`.

The `Transform` contains the position (`xpos`, `ypos`, `zpos`) and rotation around each axis (`alpha` `beta` and `gamma` for x, y and z axis) of the object in world space. Rotations by default are represented by a value between 0 and 2047, so that each step represent 1/1024 of Pi and there are 2048 steps in a complete rotation. The `AddRotation()` and `SubRotation()` macros can be used to keep these values clamped, although this is not required by the renderer.

The camera's position and rotation are also represented by an instance of `Transform`.

The included demo code uses a `N3DObject` to encapsulate these data types. This is for historical reasons. You will probably find it easier to define your own data structure. Besides the model data and the transform, none of the members of `N3DObject` are used by the renderer.

## Lights

TODO

## Renderers

The type of renderer to use should be chosen by un-commenting the appropriate block of defines in the `jag3d/renderer/renderer.s` file. (TODO: Make this not suck.) All 6 example renderers from Atari's demo are available.

If the "Gouraud Shaded Textures" version is used, the `FixModelTextures()` function should be run on each of your models. (NOTE: This renderer doesn't run well at the moment, probably due to lack of buffer memory in the GPU. This should improve in the future.)

## Setup and Rendering

Load the renderer into GPU memory with `LoadAndInitRenderer()`. This takes no parameters and will load and initialise the default renderer, built from `renderer.s`. 

If you are not using the GPU for anything else, you can be load and initialise the rendering code once eg. before entering your main game loop. Once the init code has been run it is no longer needed and the space it occupied if freed-up for use by the renderer.

At the start of each frame, after swapping the screen buffers and making any updates to the camera's position and orientation, call `SetupFrame()`. This sets up the camera matrix used by all subsequent calls to the object rendering function.

Each object is rendered to the current buffer using `RenderObject()`. This takes as parameters:

    * A pointer to the model data 'N3DObjdata`
    * A pointer to the object `Transform`
    * A pointer to the lights struct
    * A pointer to scratch memory which the GPU will use for transformed points. It should have enough room for `sizeof(TPoint)` bytes for each point in your largest model. (See example code for pre-allocating a buffer.)

*Note:* Since the GPU will be reading data from (and in the case of the scratch memory, writing data to) each of the areas of memory these pointers point to, it is essential to have them aligned on longword (4 byte) boundaries. The best approach for this would be to allocate them in an assembly language file's `.bss` section with `.long` alignment. Next best is the approach shown in the demo code, where `malloc()` is used to request a buffer slightly larger than required, the address of which is then aligned.

(TODO: It should be possible, if using one of the smaller renderers and models with a small number of points, to have `tpoints` reference an area of GPU memory. This needs testing.)

## Multiple Renderers

Multiple renderers should be possible by duplicating the main `renderer.s` file, renaming the exposed symbols (`renderer_code`, `renderer_init`, `renderer_frameinit` and `renderer_objinit`) and using the `…Custom()` versions of the functions described above. This has not yet been tested. It will be documented once it has.

## TODO

- Fix issue where wireframe renderer draws a line across top and bottom of the screen
- Optomise GPU code, particularly with respect to instruction order
- Investigate using GPU memory or CLUT as scratch space for low-point models
- Move bitmap clearing blitter code into `frameinit`
- Move sin and cos lookup code onto the GPU
