;
; pointrenderer.inc
; render points into world space
;

    .extern _params

;
; params[0] = number of points to draw
; params[1] = pointer to points array
;

    .gpu

; these are still defined when we get here. might be worth finding
;   where they're leaking from and fixing it

pcount  .equr   r3,1    ; count of points
pptr    .equr   r4,1    ; pointer to points

aratio  .equr   r5,1    ; aspect ratio

x       .equr   r6,1
y       .equr   r7,1
xy      .equr   r8,1
z       .equr   r9,1

xcenter     .equr   r10,1
ycenter     .equr   r11,1
xscale      .equr   r12,1
yscale      .equr   r13,1
zfrac       .equr   r14,1
znorm       .equr   r15,1

screenwidth     .equr   r16,1
screenheight    .equr   r17,1

colour      .equr   r18,1

pointloop   .equr   r19,1
pointskip   .equr   r20,1

blitcmd     .equr   r21,1
b_count     .equr   r22,1

    ; load count of points and pointer

    movei   #_params,temp0
        
        ; start of the point checking loop
        movei   #.nextpoint,pointloop

    load    (temp0),pcount  ; we are assuming this will never be called for 0 points
    addqt   #4,temp0
    
        ; end of the point loop, where the counter is decreased
        movei   #.pointdone,pointskip

    load    (temp0),pptr

    ; load camera parameters from GPU memory
    movei   #aspectratio,zfrac

        ; store the blit command
        movei   #PATDSEL|ZMODEGT|DSTENZ|DSTWRZ|DSTEN|BKGWREN|UPDA1|CLIP_A1,blitcmd

    load    (zfrac),aratio      ; load aspect ratio
    load    (zfrac+1),xscale     ; load scale for camera
    load    (zfrac+2),yscale
    load    (zfrac+3),xcenter    ; load center for camera
    load    (zfrac+4),ycenter

    ; xcenter and ycenter are 16.16 values
    ; we can rebuild the screen size from them

    move    xcenter,screenwidth
    move    ycenter,screenheight
    shrq    #16,screenwidth
    shrq    #16,screenheight
    shlq    #1,screenwidth
    shlq    #1,screenheight


    ;
    ; wait for blitter
    ; would having a single version of this somewhere be any quicker, considering
    ;   the amount of seyup needed to call a subroutine?

    ; if we stored this in a main register...
    ;
    ;   move    PC,return
    ;   jmp     (wait4blitter)
    ;   addqt   #6,return
    ;
    ; returns with temp0 -> bcmd_ptr
    ;

;    movefa  bcmd_ptr,temp0
;.bwait:
;    load    (temp0),temp1
;    btst    #0,temp1
;    jr      EQ,.bwait
;    nop
    
;
; shared blitter setup
;

    movefa  a1_ptr,zfrac    ; == A1_PIXEL
    movefa  destaddr,temp1
    subqt   #12,zfrac       ; == A1_BASE
    store   temp1,(zfrac)         ; to where we are blitting

    movefa  destflags,temp1
    movei   #XADDPIX,temp2
    or      temp2,temp1
    store   temp1,(zfrac+((A1_FLAGS-A1_BASE)/4))

    ; B_COUNT setup moved into main loop, because duh

    movei   #$0001FFFE,temp1
    store   temp1,(zfrac+((A1_STEP-A1_BASE)/4))

    ; TODO: could interleave these with the above to prevent stalls
    move    screenheight,temp1
    shlq    #16,temp1
    or      screenwidth,temp1
    store   temp1,(zfrac+((A1_CLIP-A1_BASE)/4))

    ; TODO: check if we're drawing 2x2 or 3x3
    movei   #$00020002,b_count

;
; point drawing loop
;

    movei   #cammatrix,temp0        ; temp0 will hold cammatrix during this loop

.nextpoint:

    load    (pptr),z        ; = ttl|z
    addqt   #4,pptr

        move    z,temp1

    load    (pptr),xy       ; = y|x
    addqt   #4,pptr

        shrq    #16,temp1       ; isolate ttl to see if this record is a valid point
                                ; sets flags for below
                                ; value in temp1 isn't actually needed
    loadw   (pptr),colour

    JR      EQ,.nextpoint   ; if temp1 == 0 then ttl == 0, so skip this point
    addqt   #8,pptr         ; point to next point, whether we branch or not

    shlq    #16,z   ; clear high word of register
    shrq    #16,z
    bset    #30,z           ; load $4000 into the high word, so z = 1|z

    ; TODO: interleave loading with the z stuff above?

    movefa  altgmtxa,temp2      ; == G_MTXA
    store   temp0,(temp2)       ; temp0 == cammatrix

    ; work out the location of the point in camera space
    ; TODO: we could save some space by using mmult against the camera matrix
    ;   we would have to update the x column with the aspect ratio though...
    ;   and would only be able to call this once...

    nop     ; "WARNING: we need at least 1 instruction between any store and mmult instruction; if any
    nop     ;   other processor (e.g. the blitter) is accessing GPU RAM, we need 2 instructions"

    mmult   xy,x            ; point x position in camera space

    nop     ; all these nops match those from the original code this copies from loadxpt.inc
    nop

    mmult   xy,y            ; point y position in camera space

    nop
    nop

    mmult   xy,z            ; point z position in camera space

    nop
    nop

    sharq   #14,x           ; adjust for aspect ratio (1)
    imult   aratio,x        ; adjust for aspect ratio (2)
                            ; do this after we've finished mmult-ing
    ;
    ; perspective transform from clip.inc
    ;

    ; TODO: turn this into a common subroutine. check what changes we've
    ;   made from the original version

    ; from loadxpt.inc, after transdoring points with _GPUM

    shlq    #2,z           ; convert Z from 14.14 to 14.16
    shlq    #2,y           ; convert Y from 14.14 to 14.16
    shlq    #2,x           ; convert X from 14.14 to 14.16

    ; perspective transform code from clip.inc
    ; this expects x,y,z to be 16.16 values

    moveq   #1,zfrac

.if PERSP_SHIFT > 0
    sharq   #PERSP_SHIFT,z      ; adjust for perspective shift effect on clipping
                                ; X and Y will be shifted left by PERSP_SHIFT before plotting
.endif

    shlq    #30,zfrac
    div     z,zfrac     ; we may assume Z > 0, since we've clipped already

    move    x,temp1     ; normalize X and Y
    move    y,temp2
    abs     temp1       ; temp1 = normi(X)
    abs     temp2       ; temp2 = normi(Y)
    normi   temp1,temp1
    normi   temp2,temp2
    addqt   #9,temp1
    addqt   #9,temp2
        normi   zfrac,znorm
        sha     temp1,x
        addqt   #9,znorm
        sha     temp2,y
    sha     znorm,zfrac

    add     znorm,temp1
    add     znorm,temp2
    neg     temp1
    neg     temp2
    addqt   #((30-15)),temp1   ; we'll pre-shift the result by 15 in order to multiply by cam->xscale
    addqt   #((30-15)),temp2   ; and cam->yscale

    imult   zfrac,x
    imult   zfrac,y

    sharq   #15,x
    sharq   #15,y
    imult   xscale,x        ; scale for this camera
    imult   yscale,y
    sha     temp1,x         ; re-normalize the values
    sha     temp2,y
    
    add     xcenter,x       ; test that the x coord is on screen
    jump    MI,(pointskip)
    cmp     x,screenwidth
    jump    PL,(pointskip)

    add     ycenter,y       ; test that the y coord is on screen
    jump    MI,(pointskip)
    cmp     y,screenheight
    jump    PL,(pointskip)

.if PERSP_SHIFT > 0
    shlq    #PERSP_SHIFT,z
.endif

    ;
    ; back to the drawing code
    ;

    ; by this point, the values should be correct

    shrq    #16,y
    shrq    #16,x
    subqt   #1,y
    subqt   #1,x
    shlq    #16,y
    or      x,y

    ;
    ; zfrac == r14 == a reg we can use for indexed access
    ;
    movefa  a1_ptr,zfrac  ; == A1_PIXEL

    ; you have to reset B_COUNT each pass, stupid
    store   b_count,(zfrac+((B_COUNT-A1_PIXEL)/4))

    ; packed y|x
    store   y,(zfrac)   ; == A1_PIXEL

    ; i don't understand how the below is working
    ; the registers we're storing have only data in the low word
    ; storew-ing doesn't work
    ; but this does?

    store   colour,(zfrac+((B_PATD-A1_PIXEL)/4))

    ; setup Z values
    ; we can get away with setting only the first two values, since we're drawing 2 pixels per line

    store   z,(zfrac+((B_SRCZ2-A1_PIXEL)/4))
    rorq    #16,z               ; switch to integer part
    store   z,(zfrac+((B_SRCZ1-A1_PIXEL)/4))

    ; issue blitter CMD
    store   blitcmd,(zfrac+((B_CMD-A1_PIXEL)/4))     ; go!

.pointdone:
    subq    #1,pcount       ; need this here and eat the stall
    jump    NE,(pointloop)
    nop

;
; all done, kill the GPU
;
    movei   #gpudone,r0
    jump    (r0)
    nop


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

