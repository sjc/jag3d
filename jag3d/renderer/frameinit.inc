;
; frameinit.inc
; Setup camera matrix, destination bitmap, and anything else
;   which is common to all objects rendered in the same frame
;

    .extern _params     ; access the parameters array in DRAM

    .extern calc_matrix ; matrix calculation code

;
; params[0] = pointer to bitmap to use for output
; params[1] = pointer to camera angles
;

    .gpu

;
; make sure we're in bank 1
;
    movei   #G_FLAGS,r0
    load    (r0),r1
    bset    #14,r1
    store   r1,(r0)
    nop
    nop
;   .REGBANK1

;
; we want 16.16 divides
;
   movei   #G_DIVCTRL,temp0
   moveq   #1,temp1
   store   temp1,(temp0)

;
; set up for 4xn matrices
;
   subqt   #(G_DIVCTRL-G_MTXC),temp0       ; move to G_MTXC
   moveq   #4,temp1
   store   temp1,(temp0)

;
; rename registers
;  TODO: work out which ones we need
;

;objdata     .equr   orclips             ; pointer to object data
;objmat      .equr   subr                ; pointer to object matrix
;srclmodel   .equr   andclips            ; pointer to lighting model
cammat      .equr   pgon                ; pointer to camera matrix

destplanes  .equr   r4,1                ; pointer to GPU RAM clipping planes
tpoints     .equr   r5,1
ptcount     .equr   r6,1
points      .equr   r7,1
matlist     .equr   r8,1                ; pointer to material list

;
; load up our parameters
;
    movei   #_params,temp0

    ;load    (temp0),objdata         ; get pointer to object data
    ;addqt   #4,temp0            ; 
    ;load    (temp0),objmat          ; get pointer to object matrix
    ;addqt   #4,temp0

    load    (temp0),cam         ; get output bitmap pointer
    addqt   #4,temp0
    load    (temp0),cammat          ; get camera matrix pointer

    ;addqt   #4,temp0
    ;load    (temp0),srclmodel       ; get pointer to source lighting model
    ;addqt   #4,temp0
    ;load    (temp0),tpoints         ; get pointer to "tpoints" array
        ; PROBABLY DON'T NEED THE SCRATCH ARRAY

    .equrundef  destplanes
    .equrundef  tpoints
    .equrundef  ptcount
    .equrundef  points
    .equrundef  matlist

;
;
; set up constant registers for the trapezoid renderer
;
   movei   #A1_PIXEL,temp1
   movei   #A2_PIXEL,temp2
   moveta  temp1,a1_ptr
   moveta  temp2,a2_ptr
   movei   #B_Z3,temp1
   movei   #B_CMD,temp2
   moveta  temp1,bz3_ptr
   moveta  temp2,bcmd_ptr

;
; make sure the blitter is free
;
.bwait:
   load     (temp2),temp0
   btst     #0,temp0
   jr       EQ,.bwait
   nop

;
; zero out the clipping rectangle
;
   moveq   #0,temp0
   movei   #A1_CLIP,temp1
   store   temp0,(temp1)

;*
; set up the various transformation matrices
;
; we can use r4-r21 here with impunity
;*
;
; calculate the object space -> camera space transform:
;   M' = cameraM * objectM

;destmat     .equr   r4,1        ; pointers to matrices
srcmat      .equr   r5,1

;srcyx       .equr   r6,1        ; source data for matrix multiply (Y component in high word, X in low word)
;src1z       .equr   r7,1

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

;col1a       .equr   r0,0        ; storage for other matrix
;col1b       .equr   r1,0
;col2a       .equr   r2,0
;col2b       .equr   r3,0
;col3a       .equr   r4,0
;col3b       .equr   r5,0
;col4a       .equr   r6,0
;col4b       .equr   r7,0

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
; TODO: Instead of reading from objmat, this should call the code to
;   calculate the matrix from the object angles struct (which eg. we
;   can pass in params and read into objmat)
;


;
; SETUP AND STORE CAMERA MATRIX
;

    ; call our matrix calculator function
    ; this returns with the matrix in named registers


    move    cammat,temp0    ; will be a pointer to camera _angles_
    movei   #calc_matrix,temp1
    move    PC,return
    jump    (temp1)
    addqt   #6,return

    ; store it into local GPU memory
    ; TODO: do 16-bit stores into GPU memory work?

    ;movei   #cameramatrix,temp0
    ;storew  xrite,(temp0)
    ;addqt   #2,temp0
    ;storew  xdown,(temp0) 
    ;addqt   #2,temp0
    ;storew  xhead,(temp0) 
    ;addqt   #2,temp0
    ;storew  yrite,(temp0) 
    ;addqt   #2,temp0
    ;storew  ydown,(temp0) 
    ;addqt   #2,temp0
    ;storew  yhead,(temp0) 
    ;addqt   #2,temp0
    ;storew  zrite,(temp0) 
    ;addqt   #2,temp0    
    ;storew  zdown,(temp0)
    ;addqt   #2,temp0
    ;storew  zhead,(temp0) 
    ;addqt   #2,temp0
    ;storew  xposn,(temp0) 
    ;addqt   #2,temp0
    ;storew  yposn,(temp0) 
    ;addqt   #2,temp0
    ;storew  zposn,(temp0) 


    ; do some maths with the matrix and save it to local memory

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

    ; now store the camera matrix for use by objsetup

    movei   #cammatrix,srcmat   ; load pointer now we're back from calculating the matrix

    store   xrite,(srcmat)      ; store first word
    addqt   #4,srcmat
    store   xdown,(srcmat)
    addqt   #4,srcmat
    store   xhead,(srcmat)
    addqt   #4,srcmat
    store   xposn,(srcmat)
    addqt   #4,srcmat

    store   yrite,(srcmat)      ; store next word
    addqt   #4,srcmat
    store   ydown,(srcmat)
    addqt   #4,srcmat
    store   yhead,(srcmat)
    addqt   #4,srcmat
    store   yposn,(srcmat)
    addqt   #4,srcmat

    store   zrite,(srcmat)      ; store next word
    addqt   #4,srcmat
    store   zdown,(srcmat)
    addqt   #4,srcmat
    store   zhead,(srcmat)
    addqt   #4,srcmat
    store   zposn,(srcmat)

;
; get camera info, and adjust the transformation matrix so that
; it transforms the viewing pyramid into "canonical" form
; (-z < x,y < z)
;
    
    ; this could probably go in the one-time init...
    ; although the dest address will change each frame...

    loadw   (cam),temp0     ; get width
    addqt   #2,cam
    loadw   (cam),temp1     ; get height
    addqt   #2,cam
    load    (cam),temp2     ; get the blitter flags
    addqt   #4,cam
    moveta  temp2,destflags
    load    (cam),temp2     ; get the bitmap address

    moveta  temp2,destaddr
.if (^^defined WIREFRAME)
    subqt   #1,temp0        ; the wireframe renderer draws all the way to the
    subqt   #1,temp1        ; right and bottom edges (polygon renderers don't)
.endif
    shlq    #15,temp0       ; convert to 16.16, and divide by 2
    shlq    #15,temp1
    move    temp1,temp2
    div     temp0,temp2     ; make temp2 = aspect ratio = height/width

    movei   #camscale+12,cam    ; or aspectratio+16

    bset    #15,temp1       ; add 1/2 for rounding
    bset    #15,temp0
    store   temp1,(cam)     ; store ycenter (=height/2)
    subqt   #4,cam
    store   temp0,(cam)     ; store xcenter (=width/2)
    subqt   #4,cam
    sharq   #16,temp1       ; convert back to 0.16 integers
    sharq   #16,temp0
    store   temp1,(cam)     ; store yscale
    subqt   #4,cam
    store   temp0,(cam)     ; store xscale

    shrq    #2,temp2        ; convert aspect ratio to a 0.14 fixed point number
    subqt   #4,cam
    store   temp2,(cam)

;
; all done, kill the GPU
;

    movei   #G_CTRL,r0
    moveq   #2,r1
.frame_die:
    store   r1,(r0)
    nop
    nop
    ;nop
    ;nop
    ;nop
    ;nop
    ;nop
    ;nop
    jr  .frame_die
    nop

    .equrundef  cammat

    .equrundef  srcmat
    ;.equrundef  srcyx
    ;.equrundef  src1z
    .equrundef  xrite
    .equrundef  xdown
    .equrundef  xhead
    .equrundef  xposn
    .equrundef  yrite
    .equrundef  ydown
    .equrundef  yhead
    .equrundef  yposn
    .equrundef  zrite
    .equrundef  zdown
    .equrundef  zhead
    .equrundef  zposn
