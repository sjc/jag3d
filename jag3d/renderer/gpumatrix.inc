;
; gpumatrix.inc
; Calculate a transform matrix on the GPU
; Based on the mkmat.s 68k code
; Parameters:
;   temp0 - pointer to `angles` struct
;   return - return address
;

    ; TODO: remove this once we're happy it's all working
    .extern _test_returns

    .gpu

    .globl  calc_matrix
calc_matrix:

;   
; This code will calculate the elements of the matrix and assign them
; to the same registers as used by the original code (which unpacked
; them from the structure calculated by the 68k).
;

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

; rename temps

alpha_sin   .equr   r4,1
alpha_cos   .equr   r5,1
beta_sin    .equr   r20,1
beta_cos    .equr   r21,1
gamma_sin   .equr   r6,1
gamma_cos   .equr   r7,1

    ; load from angles struct

    loadw   (temp0),alpha_sin
    addqt   #2,temp0
    loadw   (temp0),alpha_cos
    addqt   #2,temp0
    loadw   (temp0),beta_sin
    addqt   #2,temp0
    loadw   (temp0),beta_cos
    addqt   #2,temp0
    loadw   (temp0),gamma_sin
    addqt   #2,temp0
    loadw   (temp0),gamma_cos
    addqt   #2,temp0
    loadw   (temp0),xposn
    addqt   #2,temp0
    loadw   (temp0),yposn
    addqt   #2,temp0
    loadw   (temp0),zposn

    ;d3 = alpha sin
    ;d4 = beta sin
    ;d5 = gamma sin
    ;d6 = alpha cos
    ;d7 = beta cos
    ;a5 = gamma cos

    ;
    ; xrite = cos(gamma) * cos(beta) + sin(gamma) * sin(beta) * sin(alpha);
    ; 

        ;muls        d7, d0 ; d0 = a5 = gamma cos
    move   gamma_cos,xrite  ; xrite = d0, destination of maths
    imult  beta_cos,xrite   ; 32-bit

        ;move.w  d5, d1
        ;muls    d4, d1
        ;moveq   #14, d2
        ;asr.l   d2, d1
    move   gamma_sin,temp1
    imult  beta_sin,temp1   ; sin(gamma) * sin(beta)
    sharq  #14,temp1

        ;muls    d3, d1
        ;add.l   d1, d0
        ;moveq   #14, d2
        ;asr.l   d2, d0
        ;move.w  d0, 0(a1)
    imult  alpha_sin,temp1  ; sin(alpha) * (sin(gamma) * sin(beta))
    add    temp1,xrite
    sharq  #14,xrite        ; completed

    ;
    ; xdown = sin(gamma) * cos(alpha);
    ; 

        ;move.w  d5, d0
        ;muls        d6, d0
        ;moveq   #14, d2
        ;asr.l   d2, d0
        ;move.w  d0, 2(a1)
    move   gamma_sin,xdown  ; xdown = d0, destination of math
    imult  alpha_cos,xdown  ; 32-bit
    sharq  #14,xdown

    ;
    ; xhead = sin(gamma) * sin(alpha) * cos(beta) - cos(gamma) * sin(beta);
    ; 

        ;move.w  d5, d0
        ;muls        d3, d0
        ;moveq   #14, d2
        ;asr.l   d2, d0
    move    gamma_sin,xhead
    imult   alpha_sin,xhead
    sharq   #14,xhead

        ;muls        d7, d0
    imult   beta_cos,xhead  ; 32-bit

        ;move.w  a5, d1
        ;muls        d4, d1
    move    gamma_cos,temp1
    imult   beta_sin,temp1  ; 32-bit
    
        ;sub.l   d1, d0
        ;moveq   #14, d2
        ;asr.l   d2, d0
        ;move.w  d0, 4(a1)
    sub     temp1,xhead
    sharq   #14,xhead       ; completed

    ;
    ; yrite = cos(gamma) * sin(alpha) * sin(beta) - sin(gamma) * cos(beta);
    ; 

        ;move.w  a5, d0
        ;muls        d3, d0
        ;moveq   #14, d2
        ;asr.l   d2, d0
    move    gamma_cos,yrite
    imult   alpha_sin,yrite
    sharq   #14,yrite

        ;muls        d4, d0
    imult   beta_sin,yrite  ; 32-bit

        ;move.w  d5, d1
        ;muls        d7, d1
    move    gamma_sin,temp1
    imult   beta_cos,temp1  ; 32-bit

        ;sub.l   d1, d0
        ;moveq   #14, d2
        ;asr.l   d2, d0
        ;move.w  d0, 6(a1)
    sub     temp1,yrite
    sharq   #14,yrite       ; complete

    ;
    ; ydown = cos(gamma) * cos(alpha);
    ; 

        ;move.w  a5, d0
        ;muls        d6, d0
        ;moveq   #14, d2
        ;asr.l   d2, d0
        ;move.w  d0, 8(a1)
    move    gamma_cos,ydown
    imult   alpha_cos,ydown
    sharq   #14,ydown       ; complete

    ;
    ; yhead = cos(gamma) * sin(alpha) * cos(beta) + sin(gamma) * sin(beta);
    ; 

        ;move.w  a5, d0
        ;muls        d3, d0
        ;moveq   #14, d2
        ;asr.l   d2, d0
    move    gamma_cos,yhead
    imult   alpha_sin,yhead
    sharq   #14,yhead

        ;muls        d7, d0
    imult   beta_cos,yhead  ; 32-bit

        ;move.w  d5, d1
        ;muls        d4, d1
    move    gamma_sin,temp1
    imult   beta_sin,temp1  ; 32-bit

        ;add.l   d1, d0
        ;moveq   #14, d2
        ;asr.l   d2, d0
        ;move.w  d0, 10(a1)
    add     temp1,yhead
    sharq   #14,yhead       ; complete

    ;
    ; zrite = sin(beta) * cos(alpha);
    ; 

        ;move.w  d4, d0
        ;muls        d6, d0
        ;moveq   #14, d2
        ;asr.l   d2, d0
        ;move.w  d0, 12(a1)
    move    beta_sin,zrite
    imult   alpha_cos,zrite
    sharq   #14,zrite       ; complete

    ;
    ; zdown = -sin(alpha);
    ; 

        ;move.w  d3, d0
        ;neg.w   d0
        ;move.w  d0, 14(a1)
    move    alpha_sin,zdown
    neg     zdown           ; complete

        ; does this neg work correctly on a 16-bit value in a 32-bit register?

        ; NOTE: this is the last time alpha_sin is used, so we could load
        ;   the value directly into the zdown register to save a move

    ;
    ; zhead = cos(beta) * cos(alpha);
    ; 

        ;move.w  d7, d0
        ;muls        d6, d0
        ;moveq   #14, d2
        ;asr.l   d2, d0
        ;move.w  d0, 16(a1)
    move    beta_cos,zhead
    imult   alpha_cos,zhead
    sharq   #14,zhead

        ; NOTE: we could load either beta_cos or alpha_cos into zhead

    ;
    ; TEST: Put the values we've calculated into test_returns in
    ; the order: xrite xdown xhead yrite ydown yhead zrite zdown zhead
    ; so we can compare them against those calculated by the 68k
    ;

    ;movei   #_test_returns,temp0
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

    ;
    ; We return with the 12 named matrix registers setup as if
    ; the matrix has just been read from external memory
    ;

    jump    (return)
    nop

;
; now kill the GPU
;
;    movei   #G_CTRL,r0
;    moveq   #2,r1
;.die:
;    store   r1,(r0)
;    nop
;    nop
;    jr  .die
;    nop
