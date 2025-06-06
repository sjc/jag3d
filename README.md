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

Controls:

- D-pad up/down: Move forward/back
- D-pad left/right: Rotate
- 4/6 or left trigger/right trigger: move left/right
- 1/7: move up/down
- 3/9: look up/down (rotate camera around x axis)
- A: explosion

## Models

Each model should be places in its own directory under `/models`. The build process will try to build a model from the files inside each directory. To exclude a directory, prefix its name with `_`.

Model data will be compiled from `.3ds` (via `3dsconv`), `.s` or `.c` files. All `.tga` files will be converted to `.cry` (via `tga2cry`) and included in the binary.

Models converted from `.3ds` files will be exposed in the code with the symbol `<filename>data`. eg. to reference the model built from `ship1.3ds` use `extern N3DObjdata ship1data;`. Models built from `.s` or `.c` source will use the global symbols declared there.

A directory can contain multiple model files.

## Data Types

The interface to the library (described below) requires two pieces of information to represent the model to be rendered: the model data (`N3DObjdata`, described above) and a `Transform`.

The `Transform` contains the position (`xpos`, `ypos`, `zpos`) and rotation around each axis (`alpha` `beta` and `gamma` for x, y and z axis) of the object in world space. This struct was named `angles` in the original `demo.c` code. Rotations by default are represented by a value between 0 and 2047, so that each step represent 1/1024 of Pi and there are 2048 steps in a complete rotation. The `AddRotation()` and `SubRotation()` macros can be used to keep these values clamped, although this is not required by the renderer.

The camera's position and rotation are also represented by an instance of `Transform`.

The included demo code uses a `N3DObject` to encapsulate these data types. This is for historical reasons. You will probably find it easier to define your own data structure. Besides the model data and the transform, none of the members of `N3DObject` are used by the renderer.

## Lights

TODO

## Renderers

The type of renderer to use should be chosen by un-commenting the appropriate block of defines in the `jag3d/renderer/renderer.s` file. (TODO: Make this not suck.) All 6 example renderers from Atari's demo are available.

If the "Gouraud Shaded Textures" version is used, the `FixModelTextures()` function should be run on each of your models. (NOTE: This renderer doesn't run well at the moment, probably due to lack of buffer memory in the GPU. This should improve in the future.)

## Setup and Rendering

Load the renderer into GPU memory with `LoadAndInitRenderer()`. This takes no parameters and will load and initialise the default renderer, built from `renderer.s`. 

If you are not using the GPU for anything else, you can be load and initialise the rendering code once eg. before entering your main game loop. Once the initialisation code has been run it is no longer needed and the space it occupied is freed for use by the renderer.

At the start of each frame, after swapping the screen buffers and making any updates to the camera's position and orientation, call `SetupFrame()`. This sets up the camera matrix used by all subsequent calls to the object rendering function. It will also clear the buffer ready for drawing, if `INITCLEARBUFFER` is set in `renderer.s`. If not, the `ClearScreen()` function can be used.

Each object is rendered to the current buffer using `RenderObject()`. This takes as parameters:

- A pointer to the model data 'N3DObjdata`
- A pointer to the object `Transform`
- A pointer to the lights struct
- A pointer to scratch memory which the GPU will use for transformed points. It should have enough room for `sizeof(TPoint)` bytes for each point in your largest model. (See example code for pre-allocating a buffer.)

*Note:* Since the GPU will be reading data from (and in the case of the scratch memory, writing data to) each of the areas of memory these pointers point to, it is essential to have them aligned on longword (4 byte) boundaries. The best approach for this would be to allocate them in an assembly language file's `.bss` section with `.long` alignment. Next best is the approach shown in the demo code, where `malloc()` is used to request a buffer slightly larger than required, the address of which is then aligned.

(TODO: It should be possible, if using one of the smaller renderers and models with a small number of points, to have `tpoints` reference an area of GPU memory. This needs testing.)

## Multiple Renderers

Multiple renderers should be possible by duplicating the main `renderer.s` file, renaming the exposed symbols (`renderer_code`, `renderer_init`, `renderer_frameinit` and `renderer_objinit`) and using the `…Custom()` versions of the functions described above. This has not yet been tested. It will be documented once it has.

## 3D Sprites and Billboards

A 3D sprite is a very simple model, just a rectangle with a texture mapped to one face. They can be used to introduce simple images into the 3D scene, such as projectiles and explosions. The library provides a helper function to setup a 3D sprite, `Init3DSprite()`. This takes as parameters:

- The memory to hold the model data, which should be `SIZEOF_3DSPRITE` bytes and phrase aligned
- The width and height of the sprite in world units
- A pointer to the `Material` to use as the image

The `Material` should be phrase aligned. It in turn includes a pointer to a `Bitmap` object which defines the size of the image used and contains a pointer to the image data.

Image data can be imported via the /models directory. Any `.tga` file included in a sub-directory will be imported with the symbol name the same as the filename, preceded by an underscore (eg. `my_texture.tga` will be imported as `_my_texture`). These image files do not need to be associated with a model (see the /models/textures directory in the example project).

3D sprites can be drawn with transparency, allowing them to have cut-out sections and irregular edges. There are a couple of settings within `renderer.s` which control this behaviour.

`TRANSPARENTTEX` will turn texture transparency on and off for the "Unshaded Textures" renderer. The other two texture renderers get transparency by default as a side-effect of the multi-pass technique used to implement them.

`TRANSPARENTPIXEL` is used to choose which CrY colour is to be treated as transparent.

If `TRANSPARENTPIXEL = 1` then CrY colour 0xFFFF (a shade of yellow) is treated as transparent. A range of RGB colours map to this CrY value. Since this is the default used by the "Flat Shaded Textures" and "Gouraud Shaded Textures" renderers, it's possible to accidentally introduce transparency when it is not expected.

If `TRANSPARENTPIXEL = 0` then CrY colour 0xFFFF (black) is treated as transparent. Only RGB 0,0,0 maps to this CrY value, so it's probably the better choice for transparency, especially if using it for yellow-ish things like explosions. You can still use other almost-black colours in the same texture and they will not be treated as transparent.

A further complication when using either the "Flat Shaded Textures" or the "Gouraud Shaded Textures" is that the shading they apply will probably change the chosen transparent colour before it is removed, which happens in the last of their multiple steps. To avoid this, transparent 3D sprites should be rendered using a light model with 0 point lights. You can achieve this by defining a separate light model (see the demo code). You will probably want to increase the strength of the ambient light in this model to brighten the texture.

3D sprites can be made to behave like simple 'billboards' and always face towards the camera, rotating around their y axis, in one of two ways:

- set the sprite's transform's `beta` to the camera's `beta` + 1024. This will give a billboard which is always square-on to the camera.
- set the sprite's transform's `beta` property using `AngleAlongVector()`. This will give a billboard which faces towards the camera.

See the demo code for examples of both approaches.

## Particles

A simple point particle system can optionally be included. Set the `PARTICLES` flag to 1 in `renderer.s`. Please note that due to the extra space this requires in GPU RAM it can only be used with the un-textured Gouraud or wireframe renderers.

To use it, pass a pointer to an array of `Particle` structs to the `RenderPoints()` function, along with a count of the number of particles to render. Particles with `.ttl` set to 0 will not be drawn.

The `RunPoints()` function is used to move particles and manage their lifetime. It acts on all particles with non-0 `.ttl`, decreasing this value by 1, and returns the number of still-active particles.

## Helper Functions

`RGBtoCrY()` will convert a colour defined by red, green and blue components into the equivalent 16-bit CrY value. This uses code from the `rgb2cry` command line utility.

`CopyModelShallow()` will populate a `N3Dobjdata`-sized buffer with the properties from another `N3DObjdata`, optionally replacing the texture pointer.

`SizeOfModel()` will calculate the memory needed to hold a copy of a `N3DObjdata` and the arrays of `Point`, `Face` and `Material` it references.

`CopyModelDeep()` will copy a `N3DObjdata` and optionally its points, faces and material arrays. `SizeOfModel()` can be used to calculate the size needed for the destination buffer. `CopyModelDeep()` can also scale the object by an 8.8 fixed-point fraction while copying its points.

## Updating from Atari 3D Demo

- The `angles` struct has been renamed to `Transform`.

- It is no longer necessary to call `mkMatrix()` to prepare a `Matrix` from the `angles` (`Transform`) struct prior to rendering. This is now done on the GPU in `RenderObject()`. The `mkMatrix()` function has been removed.

- It is not necessary to call `N3Dclrbuf()` to clear the screen- and Z-buffer before rendering. This is now done in `SetupFrame()` if the `INITCLEARBUFFER` flag is set. The `N3Dclrbuf()` function has been renamed to `ClearScreen()` and is still available.

- The order of the `width` and `height` properties of the `Bitmap` struct (as used for the screen buffers) have been reversed.


## TODO

- Fix issue where wireframe renderer draws a line along screen edges where faces are clipped
- Optomise GPU code, particularly with respect to instruction order
- Investigate using GPU memory or CLUT as scratch space for low-point models
- Investigate whether texdraw.inc and texdraw2.inc need to wait for the blitter to finish before returning
- Find constants which can be pre-loaded into spare registers
