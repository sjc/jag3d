;*****************************************************************
; 3D POLYGON RENDERER
;
; Copyright 1995 Atari Corporation. All Rights Reserved.
;
; The renderer is basically divided up into 4 modules:
;	init	-- initializes global variables
;	load	-- loads points from RAM, transforms them (if
;		   necessary), and stores them in our internal
;		   format
;	clip	-- does a perspective transform, and clips polys
;		   to the screen. The perspective transform is
;		   done here rather than in 'load' because
;		   clipping to the front view plane must be done
;		   in world coordinates, not screen coordinates.
;	draw	-- draws the polygons
;*****************************************************************

;
; Configuration Options
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

; "Wire Frames" (wfrend.s)
;WIREFRAME	=	1
;PHRASEMODE	=	0
;TEXTURES	=	0
;TEXSHADE	=	0
;POLYSIDES	=	3

; "Gouraud Only" (gourrend.s)
;WIREFRAME	=	0
;PHRASEMODE	=	0
;TEXTURES	=	0
;TEXSHADE	=	0
;POLYSIDES	=	4

; "Phrase Mode Gouraud" (gourphr.s)
;WIREFRAME	=	0
;PHRASEMODE	=	1
;TEXTURES	=	0
;TEXSHADE	=	0
;POLYSIDES	=	4

; "Unshaded Textures" (texrend.s)
WIREFRAME	=	0
PHRASEMODE	=	0
TEXTURES	=	1
TEXSHADE	=	0
POLYSIDES	=	4

; "Flat Shaded Textures" (flattex.s)
;WIREFRAME	=	0
;PHRASEMODE	=	0
;TEXTURES	=	1
;TEXSHADE	=	1
;POLYSIDES	=	4

; "Gouraud Shaded Textures" (gstex.s)
; NOTE: You must call FixModelTextures() on each textured model in this mode
;WIREFRAME	=	0
;PHRASEMODE	=	0
;TEXTURES	=	1
;TEXSHADE	=	2
;POLYSIDES	=	4


;
; MAX_LIGHTS
; Maximum number of lights in a scene
;
MAX_LIGHTS	=	6

;
; PERSP_SHIFT
; adjustment for perspective effect; this is a power of 2
; "2" is a good default; "0" produces very exaggerated
; perspective; bigger numbers produce smaller perspective
; effects
;
PERSP_SHIFT	=	2

;
; MINZ
; Minimum Z value not clipped (must be > 0)
;
MINZ		=	8

;
; TEXSHADE2BUF
; Temporary buffer used by the TEXSHADE = 2 rendered (texdraw2.inc)
; It's up to you to ensure that this is a valid free address in DRAM
; If this value is set to 0, the CLUT is used instead
;
;TEXSHADE2BUF = $1ef000
TEXSHADE2BUF = 0

	.include 	'jaguar.inc'
;
; GPU code for doing polygon rendering
;
SIZEOF_POLYGON	equ	(1+(6*(POLYSIDES+5)))		; polygon size in longs: 1 long for num. points, + 6 longs per point
							; make sure to allow 1 point per clipping plane for clipping
							; output relults!
SIZEOF_XPOINT	equ	24		; Xpoint size in bytes

	.extern	_params

	.globl	_renderer_code
	.data
_renderer_code:
	.dc.l	startblit, endblit-startblit ; location, length

	.gpu

	.include 	"globlreg.inc"
	.include	"polyregs.inc"

	.org	G_RAM

startblit:

	.globl	_renderer_objinit
_renderer_objinit:

;
; GPU triangle renderer
;
; Parameters:
;   params[0] = pointer to object data
;   params[1] = pointer to object transform
;   params[2] = pointer to lighting model
;   params[3] = pointer to work array to use for transformed points
;
; Global variables initialized in init.inc
;
;	GPUcampos:	camera position vector (object space)
;	GPUM:		transformation matrix (longword entries)
;	GPUscplanes:	clipping planes for screen coordinates
;	GPUTLights:	lighting model (transformed to object space)
;

	.include "objinit.inc"

;
; wireframe-specific code for setting-up blitter
;
.if WIREFRAME
	.PRINT "Wireframe setup code"
	movefa	bcmd_ptr,temp0
.bwait0:
	load	(temp0),temp1
	btst	#0,temp1
	jr	EQ,.bwait0
	nop
;
; set up register window A1 to point at the screen
;
	movefa	destaddr,temp0
	movei	#A1_BASE,temp2
	movefa	destflags,temp1
	store	temp0,(temp2)

	moveq 	#3,temp0 	;movei	#XADDINC,temp0
	shlq 	#16,temp0 	; stalls, buts saves a word

	addqt	#4,temp2
	or		temp0,temp1
		; stall
	store	temp1,(temp2)
.endif

;***********************************************************************
; main loop goes here
; it is assumed that "return" points to "triloop"
;***********************************************************************

triloop:
	movei 	#skipface,return
	moveta	return,altskip

	.globl	skipface
skipface:

;***********************************************************************
; Polygon loading code goes here
;***********************************************************************
	.include	"load.inc"

;***********************************************************************
; Clipping and perspective transformation code goes here
;***********************************************************************
	.include	"clip.inc"

;******************************************************************
; here's where we render the polygon
;******************************************************************
	
	movei	#curmaterial,altpgon

.if WIREFRAME = 1
	.include "wfdraw.inc"
.else
	.include "drawpoly.inc"
.endif

	movei	#triloop,return		; main loop expects its address in "return"
	jump	(return)		; branch to the main loop
	nop
;
; bottom of the triangle loop
;

endtriloop:
gpudone:

;
; done drawing, so kill the GPU
;
	movei	#G_CTRL,r0
	moveq	#2,r1
.die:
	store	r1,(r0)
	nop
	nop
	jr	.die
	nop


	.include	"clipsubs.inc"

.if TEXTURES
.if TEXSHADE = 2
	.include	"texdraw2.inc"
.else
.if TEXSHADE = 1
	.include	"texdraw1.inc"
.else
	.include	"texdraw.inc"
.endif
.endif
.endif

.if WIREFRAME = 0
.if PHRASEMODE
	.include 	"phrdraw.inc"
.else
	.include	"gourdraw.inc"
.endif
.endif

	.equrundef	thisplane

	.long
	.globl	_GPUcampos
	.globl	_GPUM
	.globl	_GPUscplanes
	.globl	_GPUTLights
;
; the code expects the next 3 arrays to always remain in order
	; 4x1 vector: camera's position in object space
	; 4th element is always $4000, so we can pre-fill it here
_GPUcampos:
	.dcb.l	4,$4000
	; 4x3 matrix, 1 longword per entry
_GPUM:
	.dcb.l	12,0

	; screen coordinate clipping planes; z, x, x, y, y
_GPUscplanes:
	.dc.l	 0,  0, 1, -MINZ
	.dc.l	 1,  0, 1, 0
	.dc.l	-1,  0, 1, 0
	.dc.l	 0,  1, 1, 0
	.dc.l	 0, -1, 1, 0

	; lighting model: 4 bytes + 8 bytes per light
_GPUTLights:
	.dcb.l	1+(2*MAX_LIGHTS),0		; room for 8 lights
;
; variables
;
	.globl	gpoints,gtpoints
gnumpoints:
	.dc.l	0		; local copy of "numpoints" variable
gpoints:
	.dc.l	0		; local copy of "points" variable
gtpoints:
	.dc.l	0		; local copy of "tpoints" variable
materialtable:
	.dc.l	0		; local copy of materials table

aspectratio:
	.dc.l 	0 		; copy of camera aspect ratio
						; here just because the camscale code iterates backwards
camscale:
	.dc.l	0		; copy of camera x scale
	.dc.l	0		; copy of camera y scale
	.dc.l	0		; copy of camera x center
	.dc.l	0		; copy of camera y center
curmaterial:
	.dcb.l	2,0		; local copy of polygon material
cammatrix:
	.dcb.l  12,0 	; copy of the camera matrix, persisted between objects

;
; Function to setup the camera and other variables for this frame
; Params:
; 	params[0] = pointer to bitmap to use for output
; 	params[1] = pointer to camera angles
;

	.globl	_renderer_frameinit
_renderer_frameinit:

frameinit:

	.include 	"frameinit.inc"

	; will stop the GPU once done

;
; Subroutine to calculate a matrix from an angles struct
;

	.include 	"gpumatrix.inc"

	; will jump (return) when done

;***********************************************************************
; Initialization code, and 324 word buffer for texture mapping
;
; The initialization code (which only needs to run once) can be
; shared with the GPU temporary buffer (which we only need
; after initialization is finished). The temporary buffer is used
; to accelerate texture mapping, and to allow shading, when Z-buffering.
; We do the initial blit from the texture source into GPU RAM, and
; then can do a phrase mode, Z-buffered blit from GPU RAM to the screen.
; This roughly halves the per-pixel cost (!) but also doubles the
; per scan-line cost.
;
; Note that the polygon storage areas (for polygon conversion) can also
; overlap the initialization code, since they aren't needed there.
;***********************************************************************
;
	.phrase

	; 1 screen wide draw buffer for texture mapped blits, plus 4 words for alignment
initcode:

	.globl	_GPUP1,_GPUP2
	.globl	_gpubuf

_GPUP1	=	initcode
_GPUP2	=	_GPUP1 + (4*SIZEOF_POLYGON)
_gpubuf = 	_GPUP2 + (4*SIZEOF_POLYGON)
; _gpubuf must be phrase aligned!
; _gpubuf is only used by texdraw1 (if TEXSHADE	= 1)

;
; One-time initialisation code
;

	.globl	_renderer_init
_renderer_init:

	.include 	"init.inc"

	; will stop the GPU once done

	.long
endblit:

	.PRINT  "renderer.s:"
	.PRINT	"GPU RAM USE: ",/u/w (endblit-startblit), " Bytes used"
	.PRINT  "   ",/u/w 4096-(endblit-startblit), " Bytes free"
	.PRINT  "   ",/u/w 4096-(_gpubuf-startblit), " Bytes available for buffer"

