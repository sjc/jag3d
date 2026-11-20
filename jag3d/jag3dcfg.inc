;
; Renderers
; Configuration Options
;
; INDEXED
; renderers use indexed (8 bit-per-pixel and below) modes
;
; WIREFRAME
; render only wireframes
;
; PHRASEMODE
; chooses the phrase-mode Gouraud shader
;
; TEXTURES
; can we draw textures (0 if no, 1 if yes)
;
; TEXSHADE
; if we do textures, how do we shade them?
; (0 = no shading, 1 = flat shading, 2 = gouraud shading)
;
; POLYSIDES
; Maximum number of polygon sides supported
;

;
; Un-comment exactly one of these blocks to build that renderer
;

; "Wire Frames"
;INDEXED    =   0
;WIREFRAME  =   1
;PHRASEMODE =   0
;TEXTURES   =   0
;TEXSHADE   =   0
;POLYSIDES  =   3

; "Gouraud Only"
;INDEXED    =   0
;WIREFRAME  =   0
;PHRASEMODE =   0
;TEXTURES   =   0
;TEXSHADE   =   0
;POLYSIDES  =   4

; "Phrase Mode Gouraud"
;INDEXED    =   0
;WIREFRAME  =   0
;PHRASEMODE =   1
;TEXTURES   =   0
;TEXSHADE   =   0
;POLYSIDES  =   4

; "Unshaded Textures"
INDEXED     =   0
WIREFRAME   =   0
PHRASEMODE  =   0
TEXTURES    =   1
TEXSHADE    =   0
POLYSIDES   =   4

; "Flat Shaded Textures"
;INDEXED    =   0
;WIREFRAME  =   0
;PHRASEMODE =   0
;TEXTURES   =   1
;TEXSHADE   =   1
;POLYSIDES  =   4

; "Gouraud Shaded Textures"
; NOTE: You must call FixModelTextures() on each textured model in this mode
;INDEXED    =   0
;WIREFRAME  =   0
;PHRASEMODE =   0
;TEXTURES   =   1
;TEXSHADE   =   2
;POLYSIDES  =   4

; "Indexed Mode Wire Frames"
;INDEXED    =   1
;WIREFRAME  =   1
;PHRASEMODE =   0
;TEXTURES   =   0
;TEXSHADE   =   0
;POLYSIDES  =   3

; "Indexed Mode Untextured"
;INDEXED    =   1
;WIREFRAME  =   0
;PHRASEMODE =   0
;TEXTURES   =   0
;TEXSHADE   =   0
;POLYSIDES  =   4

; "Indexed Mode Textured"
;INDEXED    =   1
;WIREFRAME  =   0
;PHRASEMODE =   0
;TEXTURES   =   1
;TEXSHADE   =   0
;POLYSIDES  =   4


;
; MAX_LIGHTS
; Maximum number of lights in a scene
;
MAX_LIGHTS  =   4

;
; PERSP_SHIFT
; adjustment for perspective effect; this is a power of 2
; "2" is a good default; "0" produces very exaggerated
; perspective; bigger numbers produce smaller perspective
; effects
;
PERSP_SHIFT =   2

;
; MINZ
; Minimum Z value not clipped (must be > 0)
;
MINZ        =   8

;
; TEXSHADE2BUF
; Temporary buffer used by the TEXSHADE = 2 rendered (texdraw2.inc)
; It's up to you to ensure that this is a valid free address in DRAM
; If this value is set to 0, the CLUT is used instead
;
;TEXSHADE2BUF = $1ef000
TEXSHADE2BUF = 0

;
; TRANSPARENTTEX
; For "Unshaded Textures" only, treat the CrY pixel colour chosen by
; TRANSPARENTPIXEL as transparent when drawing a texture. The other
; texture-mapping modes do this implicitly as part of the multi-buffer
; approach
;   For "Indexed Mode Textured", pixels with value 0 are treated as
; transparent and not copied into the screen buffer.
;
TRANSPARENTTEX = 1

;
; TRANSPARENTPIXEL
; Choose which CrY colour is trated as transparent when drawing textures.
; This only affects the "Unshaded Textures" renderer if TRANSPARENTTEX is
; set to 1. For the other two texture-mapping renderers, it modifies the
; pre-existing behaviour
;
; Set to 1 to select CrY $1111 (yellow) -- which is the default -- or to 
; 0 to select CrY $0000 (black)
;
; This is not used for "Indexed Mode Textured".
;
TRANSPARENTPIXEL = 0

;
; Whether to clear the buffers passed to frameinit. If set to 0 then you
; are responsible for clearing the buffers in some other way eg. with the
; ClearScreenBufferAndZBuffer() or ClearScreenBuffer() functions
;
INITCLEARBUFFER = 0

;
; Enable the basic particle system. This requires extra GPU memory and so
;   will only work with the un-textured Gouraud or wireframe renderers
;
PARTICLES = 0

;
; Number of discrete steps in a full rotation
; This is used as the angle value in the Transform struct
; If you need to change this:
;  - The value should be a power of 2
;  - You will need to generate new tables to replace lib/sincos.s
;  - You will need to update renderer/gpumatrix.inc to use this new table
;  - You will also either need to replace the vector tables used in game logic, or
;      take into acount the difference in ROTSTEP when indexing into them
;  - You should take into account that AngleAlongVector() will still return a value
;      in the range 0-2047
; The `tables.py` tool can be used to generate the sine/cosine and vector tables.
;
ROTSTEPS = 2048
ROTMASK = (ROTSTEPS-1)

; structs in n3d.h
SIZEOF_N3DOBJECT = 28
SIZEOF_TRANSFORM = 12
SIZEOF_LIGHT = 8
SIZEOF_LIGHTMODEL = (4 + (SIZEOF_LIGHT * MAX_LIGHTS))

; == sizeof(N3DObjdata)
;  + sizeof(Face)
;  + 4                     the sprite is a rect therefore the Face has an extra point
;  + 4 * sizeof(Point)
SIZEOF_3DSPRITE = (20+28+48)  ; == 96
