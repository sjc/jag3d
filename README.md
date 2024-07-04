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
        - One entry point to init shared data once per frame
        - Second entry point to draw each object, to allow multiple calls with minimum re-setup
    - Moved object matrix calculation code to the GPU (replacing 68k `mkMatrix()`)
    - Consolidated all example renderers into a single file
        - Renderer type chosen at compile time
        - The assumption is that a project will choose one type of renderer to use
    - Drawing code is unchanged, because I don't understand that stuff

## Building

The [jaguar-sdk](https://github.com/cubanismo/jaguar-sdk) is the supported method of building this code.

## Running

The 3D demo will not run on [Virtual Jaguar](https://icculus.org/virtualjaguar/). This is a known issue with all version of this code.

The demo will run with [the BigPEmu](https://www.richwhitehouse.com/jaguar/) emulator. This can be used along with the [Neosis](https://richwhitehouse.com/index.php?content=inc_projects.php&showproject=91) debugger.

I have not yet tested this code on a genuine Jaguar.

## Models

Each model should be places in its own directory under `/models`. The build process will try to build a model from the files inside each directory. To exclude a directory, prefix its name with `_`.

Model data will be compiled from `.3ds` (via `3dsconv`), `.s` or `.c` files. All `.tga` files will be converted to `.cry` (via `tga2cry`) and included in the binary.

Models converted from `.3ds` files will be exposed in the code with the symbol `<filename>data`. eg. to reference the model built from `ship1.3ds` use `extern N3DObjdata ship1data;`. Models built from `.s` or `.c` source will use the global symbols declared there.

A directory can contain multiple model files.

## Objects

Instances of models are represented by `N3DObject` structures. The `.data` member should point to a model data struct (`N3DObjdata`). The `angles` member contains the position (`xpos`, `ypos`, `zpos`) and rotation around each axis (`alpha` `beta` and `gamma` for x, y and z axis) of the object, and are used for transforming the object within the scene.

The camera's position and rotation are represented by an instance of the `Angles` struct.

The `Angles` struct also contains the sine and cosine values for each rotation angle. If a rotation angle has changed, before passing the camera or an object to the renderer, these values should be updated using the `UpdateAngles()` function. This is not necessary if only the position has updated.

## Lights

TODO

## Rendering

The type of renderer to use should be chosen by un-commenting the appropriate block of defines in the `jag3d/renderer/renderer.s` file. (TODO: Make this not suck.) All 6 example renderers from Atari's demo are available.

If the "Gouraud Shaded Textures" version is used, the `FixModelTextures()` function should be run on each of your models. (NOTE: This renderer doesn't run well at the moment, probably due to lack of buffer memory in the GPU. This should improve in the future.)

Load the renderer into GPU memory with `LoadAndInitRenderer()`. For the default renderer build, pass `renderer_code` as the first parameter and `renderer_init` as the second. If you are not using the GPU for anything else, this can be done once eg. before entering your main game loop. Once the init code has been run it is no longer needed and the space it uses if freed-up for use by the renderer.

At the start of each frame, after swapping the screen buffers and making any updates to the camera's position and orientation, call `SetupFrame()`. This sets up values used by all subsequent objects. For the default renderer, pass `renderer_frameinit` as parameter.

Each object is rendered to the current buffer using `RenderObject()`. For the default renderer, pass `renderer_objinit` as parameter. `tpoints` should point to scratch memory which the GPU will use for transformed points. It should have enough room for `sizeof(TPoint)` bytes for each point in your largest model. (See example code for pre-allocating a buffer.)

(TODO: It should be possible, if using one of the smaller renderers and models with a small number of points, to have `tpoints` reference an area of GPU memory. This needs testing.)

## TODO

- Fix issue where wireframe renderer draws a line across top and bottom of the screen
- Optomise GPU code, particularly with respect to instruction order
- Investigate using GPU memory as scratch space for low-point models

