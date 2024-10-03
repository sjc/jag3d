;
; objinit.inc
; Per-object rendering setup
; Params:
;   params[0] = pointer to object data
;   params[1] = pointer to object angles
;   params[2] = pointer to lighting model
;   params[3] = pointer to work array to use for transformed points
;

    .extern _params     ; access the parameters array in DRAM
    .extern calc_matrix ; matrix calculation GPU subroutine

    .gpu

;
; rename registers
;

objdata     .equr   orclips             ; pointer to object data
objmat      .equr   subr                ; pointer to object matrix
srclmodel   .equr   andclips            ; pointer to lighting model

tpoints     .equr   r5,1
ptcount     .equr   r6,1
points      .equr   r7,1
matlist     .equr   r8,1                ; pointer to material list

;
; load up our parameters
;
    movei   #_params,temp0
    load    (temp0),objdata         ; get pointer to object data
    addqt   #4,temp0            ; 
    load    (temp0),objmat          ; get pointer to object angles
    addqt   #4,temp0
    load    (temp0),srclmodel       ; get pointer to source lighting model
    addqt   #4,temp0
    load    (temp0),tpoints         ; get pointer to "tpoints" array

;
; load object data information
;
    ; TODO can we speed this up by assuming they're aligned to long words and loading 2 at once?

    loadw   (objdata),tricount      ; get number of triangles
    addqt   #2,objdata
    loadw   (objdata),ptcount       ; get number of points
    addqt   #6,objdata          ; skip # of points, # of materials, and reserved word
    load    (objdata),tri           ; get pointer to face array
    addqt   #4,objdata
    load    (objdata),points        ; get pointer to "points" array
    addqt   #4,objdata
    load    (objdata),matlist       ; get texture boxes list
    addqt   #4,objdata

;
; save points, tpoints, and materials
;
    movei   #gnumpoints,temp0       ; store numpoints, points and tpoints
    store   ptcount,(temp0)
    addqt   #4,temp0
    store   points,(temp0)
    addqt   #4,temp0
    store   tpoints,(temp0)
    addqt   #4,temp0
    store   matlist,(temp0)         ; store texture list table

    .equrundef  tpoints
    .equrundef  ptcount
    .equrundef  points
    .equrundef  matlist

;
; clear the point transformation array
; (actually, set it all to 1's)
; there are "numpoints" elements in the array,
; each 16 bytes long
;

    ; slightly changed to only clear the first 4 bytes, in the hope it might be a
    ;   little faster, although it seems unlikely to help
    ; not using phase mode may even be slower
    ; TODO: look into just clearing the first phase

    ; TODO: see whether we could do this with a tight code loop

    moveq   #0,temp0
    movefa  a1_ptr,temp2
    store   temp0,(temp2)       ; A1_PIXEL = 0
    subqt   #(A1_PIXEL-A1_BASE),temp2
    movei   #gtpoints,temp1
    load    (temp1),temp1
    movei   #(PIXEL8|WID16|PITCH1|XADDPIX),temp0        ; flags for the blit 
        ; XADDPHR
    store   temp1,(temp2)       ; A1_BASE = tpoints
    addqt   #4,temp2
    store   temp0,(temp2)       ; A1_FLAGS = temp0
    movei   #A1_STEP,temp2
    ;movei   #$0001FFF0,temp0 ; draw 4, step = -4
    movei   #$0001FFFC,temp0
    store   temp0,(temp2)       ; A1_STEP = -16 in X, 1 in Y
    movei   #gnumpoints,temp1
    load    (temp1),temp1
    shlq    #16,temp1       ; number of lines is = to the number of points
    ;bset    #4,temp1        ; number of "pixels" per line is 16
    bset    #2,temp1
    movei   #B_COUNT,temp2
    store   temp1,(temp2)
    subqt   #(B_COUNT-B_CMD),temp2
    movei   #LFU_ONE|UPDA1,temp1
    store   temp1,(temp2)

;*
; set up the various transformation matrices
;
; we can use r4-r21 here with impunity
;*
;
; calculate the object space -> camera space transform:
;   M' = cameraM * objectM

destmat     .equr   r4,1        ; pointers to matrices
srcmat      .equr   r5,1

srcyx       .equr   r6,1        ; source data for matrix multiply 
src1z       .equr   r7,1        ; Y component in high word, X in low word

xrite       .equr   r8,1        ; result registers for matrix multiply
xdown       .equr   r9,1
xhead       .equr   r10,1
xposn       .equr   r11,1

yrite       .equr   r12,1
ydown       .equr   r13,1
yhead       .equr   r14,1
yposn       .equr   r15,1

zrite       .equr   r16,1
zdown       .equr   r17,1
zhead       .equr   r18,1
zposn       .equr   r19,1

; r20 is used for the output bitmap
; r21 is free

col1a       .equr   r0,0        ; storage for other matrix
col1b       .equr   r1,0
col2a       .equr   r2,0
col2b       .equr   r3,0
col3a       .equr   r4,0
col3b       .equr   r5,0
col4a       .equr   r6,0
col4b       .equr   r7,0

;
; load the transpose of the object matrix into the alternate register bank
; Also put it into the xrite, yrite, etc. registers for lighting calculations
;
;* Format note: the matrices are already stored transposed relative
;* to the format that we expect internally, i.e. in
;* external memory the matrix looks like:
;*  xrite yrite zrite
;*  xhead yhead zhead
;*  etc...
;*
;* whereas internally we want matrices stored as
;*  xrite xhead xdown xposn
;*  yrite yhead ydown yposn
;*  etc.

;
; Call calc_matrix() to load the object angles from objmat and calculate
;   the matrix into the named registers
;

    move    objmat,temp0            ; pointer to the angles struct for this object
    movei   #calc_matrix,temp1
    move    PC,return
    jump    (temp1)
    addqt   #6,return

    ; required to fix-up some overflowing values, which happens because
    ;   we're now calculating these on the GPU to 32-bits, rather than
    ;   loading 16-bit values from memory
    ; TODO: move this into the matrix calculation if we have to fix them
    ;   everywhere
    
    movei   #$0000FFFF,temp0

; column 1
    move    xdown,srcyx
    and     temp0,xrite         ; fix potential overflow
    shlq    #16,srcyx
    and     temp0,xhead         ; fix potential overflow
    or      xrite,srcyx

    moveta  xhead,col1b
    moveta  srcyx,col1a

; column 2
    move    ydown,srcyx
    and     temp0,yrite         ; fix potential overflow
    shlq    #16,srcyx
    and     temp0,yhead         ; fix potential overflow
    or      yrite,srcyx
    
    moveta  yhead,col2b
    moveta  srcyx,col2a

; column 3
    move    zdown,srcyx
    and     temp0,zrite         ; fix potential overflow
    shlq    #16,srcyx
    and     temp0,zhead         ; fix potential overflow
    or      zrite,srcyx

    moveta  zhead,col3b
    moveta  srcyx,col3a

; column 4

    ; TODO: calc_matrix() could read x and y in as a single long
    ;   and pass it back pre-packed

    move    yposn,srcyx
    bset    #30,zposn         ; last row was 0 0 0 1, so last column gets a 1 (=$4000)            
    shlq    #16,srcyx
    moveta  zposn,col4b
    or      xposn,srcyx
    moveta  srcyx,col4a

;*
; Transform lighting vectors from world space to object space.
; For this, we need the inverse of the object->world space transform.
; We already loaded the transpose of obj->M into the rite, down, and head
; registers; now we need the posn registers to be recalculated correctly
;*

    ; calculate new object position from the object matrix

    imultn  xrite,xposn
    imacn   xdown,yposn
    imacn   xhead,zposn
    resmac  temp0           ; will be new value for xposn

    imultn  yrite,xposn
    imacn   ydown,yposn
    imacn   yhead,zposn
    resmac  temp1           ; will be new value for yposn

    imultn  zrite,xposn
    imacn   zdown,yposn
    imacn   zhead,zposn
    resmac  temp2           ; will be new value for zposn

    neg temp0
    neg temp1
    neg temp2
    sharq   #14,temp0
    sharq   #14,temp1
    sharq   #14,temp2
    move    temp0,xposn
    move    temp1,yposn
    move    temp2,zposn

;*
; Transform lights to object space
; srclmodel points to the source lighting model
; destmat will point to the destination matrix
;
;* the Hoverstrike lighting model is slightly
;* different from our internal one: it doesn't have
;* an explicit count of lights, but rather marks
;* the end of list with a 0
;
;*
    movei   #_GPUTLights,destmat
    loadw   (srclmodel),temp1       ; get ambient light
    addqt   #2,srclmodel
    loadw   (srclmodel),temp2       ; get number of lights
    addqt   #2,srclmodel
    shlq    #16,temp1           ; put ambient light in high word of temp1

    cmpq    #MAX_LIGHTS,temp2       ; are there too many lights?
    jr  MI,.lightsok            ; if not, proceed
    nop
        moveq   #MAX_LIGHTS,temp2   ; otherwise, take only the first MAX_LIGHTS
.lightsok:
    or  temp2,temp1
    store   temp1,(destmat)
    addqt   #4,destmat

    subq    #1,temp2        ; set up for dbra loop

;
; register aliases for the next section
;

lx      .equr   srcmat
ly      .equr   srcyx
lz      .equr   src1z
bright  .equr   r20,1
one     .equr   r21,1

;
; while (numlights > 0)
; NOTE: this has been transformed into a do-while loop,
; under the assumption that numlights is initially > 0
; (a good assumption, usually). So we always transform
; at least one light source; this may be bogus, but
; the cost is pretty low, and it optimizes the more
; common case of numlights > 0 initially.
;
.lightloop:
    ; load next light
    loadw   (srclmodel),lx
    addqt   #2,srclmodel
    loadw   (srclmodel),ly
    addqt   #2,srclmodel
    loadw   (srclmodel),lz
    addqt   #2,srclmodel
    loadw   (srclmodel),bright
    addqt   #2,srclmodel
;
; "one" is a misleading register name; it is 0 for sunlight vectors
; (which transform like vectors, i.e. don't get translated), and
; 1 (=$4000 in 0.14 format) for in-scene lights (which transform like
; points)
;
    btst    #15,bright      ; sunlight vectors have bit 15 clear
    moveq   #0,one
    jr      EQ,.sunlight
    nop
        bset    #14,one
.sunlight:
    imultn  xrite,lx
    imacn   xdown,ly
    imacn   xhead,lz
    imacn   xposn,one
    resmac  temp0

    imultn  yrite,lx
    imacn   ydown,ly
    imacn   yhead,lz
    imacn   yposn,one
    resmac  temp1

    sharq   #14,temp0       ; X value for light
    shlq    #2,temp1        ; Y value for light is in high word
    shlq    #16,temp0
    shrq    #16,temp1
    or  temp0,temp1
    store   temp1,(destmat)
    addqt   #4,destmat

    imultn  zrite,lx
    imacn   zdown,ly
    imacn   zhead,lz
    imacn   zposn,one
    resmac  temp0           ; Z value for light

    sharq   #14,temp0
    shlq    #16,temp0
    or  bright,temp0        ; put brightness in low word
    store   temp0,(destmat)
    addqt   #4,destmat

    subq    #1,temp2        ; decrement # of lights
    movei   #.lightloop,temp0
    jump    PL,(temp0)      ; if so, we're done with the lighting
    nop
.donelight:

    .equrundef  lx
    .equrundef  ly
    .equrundef  lz
    .equrundef  bright
    .equrundef  one


;*
; copy inverse of the camera matrix (destmat) to _GPUM (srcmat)
; the camera matrix is assumed to give the camera->world space
; transform (i.e. we can use an object matrix as a camera matrix)
; so we actually want to copy the INVERSE of this matrix to _GPUM
;
;  note that we have to transfrom it from external format (3x4) to
;  internal format (4x3) by doing a transpose
;

    ; Not required, since we calculated the camera matrix and stored
    ;   it in GPU memory during frameinit().

;*
; Multiply camera matrix and object matrix to get the object->screen transformation
;*

    ; `cammatrix` is the in-memory camera matrix stored during frameinit()

    movei   #cammatrix,srcmat
    movei   #G_MTXA,temp2
    store   srcmat,(temp2)

    movefa  col1a,srcyx
    movefa  col1b,src1z
    nop
    mmult  srcyx,xrite
    sharq   #14,xrite

    nop
    mmult  srcyx,yrite
    sharq   #14,yrite

    nop
    mmult  srcyx,zrite
    sharq   #14,zrite

    store   srcmat,(temp2)      ; reset for next column

    movefa  col2a,srcyx
    movefa  col2b,src1z

    mmult   srcyx,xdown
    sharq   #14,xdown
    nop
    mmult  srcyx,ydown
    sharq   #14,ydown
    nop
    mmult  srcyx,zdown
    sharq   #14,zdown

    store   srcmat,(temp2)      ; reset for next column
    movefa  col3a,srcyx
    movefa  col3b,src1z

    nop
    mmult  srcyx,xhead
    sharq   #14,xhead

    nop
    mmult  srcyx,yhead
    sharq   #14,yhead

    nop
    mmult  srcyx,zhead
    sharq   #14,zhead

    store   srcmat,(temp2)      ; reset for next column
    movefa  col4a,srcyx
    movefa  col4b,src1z

    nop
    mmult  srcyx,xposn
    sharq   #14,xposn
    nop
    mmult  srcyx,yposn
    sharq   #14,yposn
    nop
    mmult  srcyx,zposn
    sharq   #14,zposn

;*
; Calculate the camera's position in object space.
; This is found from the inverse of the transformation matrix M we just calculated.
;*
    imultn  xrite,xposn
    imacn   yrite,yposn
    imacn   zrite,zposn
    resmac  temp0         ; camera x position in object space

    imultn  xdown,xposn
    imacn   ydown,yposn
    imacn   zdown,zposn
    resmac  temp1         ; camera y position in object space

    sharq   #14,temp0
    sharq   #14,temp1
    neg temp0
    neg temp1

    imultn  xhead,xposn
    imacn   yhead,yposn
    imacn   zhead,zposn
    resmac  temp2         ; camera z position in object space
    sharq   #14,temp2
    neg temp2

    movei   #_GPUcampos,destmat
    store   temp0,(destmat)
    addqt   #4,destmat
    store   temp1,(destmat)
    addqt   #4,destmat
    store   temp2,(destmat)

    ; the 4th value is always $4000, so we pre-set the _GPUcampos storage to this

    ; Restore the `aspectratio` value we calculated in frameinit()
    ; can we either guarentee that cam -> aspectratio, or store aspectration in a reg?

    movei   #aspectratio,cam
    load    (cam),temp2
    addqt   #4,cam          ; points back to camscale, as following code expects

    imult   temp2,xrite     ; adjust matrix by aspect ratio + scaling factor (if any)
    imult   temp2,xhead
    sharq   #14,xrite
    sharq   #14,xhead
    imult   temp2,xdown
    imult   temp2,xposn
    sharq   #14,xdown
    sharq   #14,xposn

;
; store matrix into _GPUM
;

    movei   #_GPUM,srcmat

    store   xrite,(srcmat)
    addqt   #4,srcmat
    store   xdown,(srcmat)
    addqt   #4,srcmat
    store   xhead,(srcmat)
    addqt   #4,srcmat
    store   xposn,(srcmat)
    addqt   #4,srcmat

    store   yrite,(srcmat)
    addqt   #4,srcmat
    store   ydown,(srcmat)
    addqt   #4,srcmat
    store   yhead,(srcmat)
    addqt   #4,srcmat
    store   yposn,(srcmat)
    addqt   #4,srcmat

    store   zrite,(srcmat)
    addqt   #4,srcmat
    store   zdown,(srcmat)
    addqt   #4,srcmat
    store   zhead,(srcmat)
    addqt   #4,srcmat
    store   zposn,(srcmat)

;
; This file will be included inline with the renderer, so here we fall through to the rest of the code
;

    .equrundef destmat
    .equrundef srcmat
    .equrundef srcyx
    .equrundef src1z
    .equrundef xrite
    .equrundef xdown
    .equrundef xhead
    .equrundef xposn
    .equrundef yrite
    .equrundef ydown
    .equrundef yhead
    .equrundef yposn
    .equrundef zrite
    .equrundef zdown
    .equrundef zhead
    .equrundef zposn
    .equrundef col1a
    .equrundef col1b
    .equrundef col2a
    .equrundef col2b
    .equrundef col3a
    .equrundef col3b
    .equrundef col4a
    .equrundef col4b

