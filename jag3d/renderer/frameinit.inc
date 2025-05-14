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
; rename registers
;

cammat      .equr   pgon                ; pointer to camera matrix
bptr        .equr   r14,1               ; indexed pointer which is free outside matrix use

;
; load up our parameters
;

    movei   #_params,temp0
.if INITCLEARBUFFER
    movei   #A1_BASE,bptr
.endif
    load    (temp0),cam         ; get output bitmap pointer
    addqt   #4,temp0
    load    (temp0),cammat          ; get camera matrix pointer


;
; get camera info, and adjust the transformation matrix so that
; it transforms the viewing pyramid into "canonical" form
; (-z < x,y < z)
;

    load    (cam),temp0     ; == height|width
    addqt   #4,cam

    load    (cam),temp1     ; get the blitter flags
    addqt   #4,cam
        ; TODO: if we're only doing gouraud, we can set XADDPIX on destflags
        ;   after we've used it to start the blitter
    moveta  temp1,destflags

    load    (cam),temp2     ; get the bitmap address
    addqt   #4,cam
    moveta  temp2,destaddr

.if INITCLEARBUFFER
;
; use the blitter to clear the screen and Z buffer
;

    store   temp2,(bptr)    ; A1_BASE = buffer base address
        
    store   temp1,(bptr+((A1_FLAGS-A1_BASE)/4))    ; XADDPHR == 0, so nothing to do
        
    moveq   #0,temp2

    move    temp0,temp1     ; we want: 0x00010000L | ((-buf->width) & 0x0000ffff)
        
    shlq    #16,temp1    
    neg     temp1
    shrq    #16,temp1
    bset    #16,temp1

    store   temp1,(bptr+((A1_STEP-A1_BASE)/4))

    store   temp2,(bptr+((A1_CLIP-A1_BASE)/4))      ; temp2 == 0
    store   temp2,(bptr+((A1_PIXEL-A1_BASE)/4))
        
    subqt   #1,temp2    ; == -1 == $FFFFFFFF

    store   temp0,(bptr+((B_COUNT-A1_BASE)/4))

    load    (cam),temp1                             ; == clear colour
    store   temp1,(bptr+((B_PATD-A1_BASE)/4))
    store   temp1,(bptr+((B_PATD-A1_BASE)/4)+1)

    movefa  bcmd_ptr,bptr       ; bptr -> B_CMD

    store   temp2,(bptr+((B_Z0-B_CMD)/4))           ; temp2 == $FFFFFFFF
    store   temp2,(bptr+((B_Z1-B_CMD)/4))
    store   temp2,(bptr+((B_Z2-B_CMD)/4))
    store   temp2,(bptr+((B_Z3-B_CMD)/4))

    movei   #(UPDA1|DSTWRZ|PATDSEL),temp1
    store   temp1,(bptr)    ; start the blitter
.else
    moveq   #0,temp2
    subqt   #1,temp2    ; == $FFFFFFFF
.endif

    .equrundef  bptr    ; finished with this

    ; temp0 == height|width
    ; temp2 == $FFFFFFFF
    ; we want: temp0 == width, temp1 == height

    shrq    #16,temp2       ; == $0000FFFF
    move    temp0,temp1
    and     temp2,temp0     ; == width
    shrq    #16,temp1       ; == height

    shlq    #15,temp0       ; convert to 16.16, and divide by 2
    shlq    #15,temp1
    move    temp1,temp2
    div     temp0,temp2     ; make temp2 = aspect ratio = height/width

    movei   #camscale+12,cam    ; or aspectratio+16

.if WIREFRAME = 0
    bset    #15,temp1       ; add 1/2 for rounding
    bset    #15,temp0
.endif
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

    ; cam -> aspectratio


;
; calculate the camera matrix, which will be used to calculate the
; object space -> camera space transform:
;

srcmat      .equr   r5,1    ; TODO: if we can rearrange things to use r14 or r15 for
                            ; this, we could save a lot of instructions in loading
                            ; and saving values at offsets

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
; SETUP AND STORE CAMERA MATRIX
;

    ; call our matrix calculator function
    ; this returns with the matrix in the named registers

    move    cammat,temp0    ; will be a pointer to camera transform
    movei   #calc_matrix,temp1
    move    PC,return
    jump    (temp1)
    addqt   #6,return

    ; transform the camera position using its direction matrix 
    ; and save it to local memory for use with each object

    ; i think this takes the inverse of the camera matrix

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

    neg     temp0
    neg     temp1
    neg     temp2
    sharq   #14,temp0   ; == xposn
    sharq   #14,temp1   ; == yposn
    sharq   #14,temp2   ; == zposn

    ; store the camera matrix for use by objsetup

iptr        .equr   r15,1   ; == yposn, which we haven't used

    movei   #cammatrix,iptr   ; load pointer now we're back from calculating the matrix

    store   xrite,(iptr)      ; store first word
    store   xdown,(iptr+1)
    store   xhead,(iptr+2)
    store   temp0,(iptr+3)      ; temp0 == xposn

    store   yrite,(iptr+4)      ; store next word
    store   ydown,(iptr+5)
    store   yhead,(iptr+6)
    store   temp1,(iptr+7)      ; temp1 == yposn

    store   zrite,(iptr+8)      ; store next word
    store   zdown,(iptr+9)
    store   zhead,(iptr+10)
    store   temp2,(iptr+11)      ; temp2 == zposn

    .equrundef  iptr

;
; all done for now, kill the GPU
;

    movei   #gpudone,r0
    jump    (r0)
    nop

    .equrundef  cammat

    .equrundef  srcmat
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
