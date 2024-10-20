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
    .extern _sincos_2048
    .gpu

calc_matrix::

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

;alpha_sin   .equr   r4,1   -- will use zdown
alpha_cos   .equr   r5,1
;beta_sin    .equr   r20,1  -- will use zrite
;beta_cos    .equr   r21,1  -- will use zhead
gamma_sin   .equr   r6,1
;gamma_cos   .equr   r7,1   -- will use yhead

    ; load from angles struct

    ; this version requires that the struct is long world aligned

    ; mask for valid rotations -- 2047 << 2
    movei   #$00001FFC,temp1

    ; load sin/cos table into base register for indexed loads
    movei   #_sincos_2048,yhead

    ;
    ; we could save 3 instructions, 3 stalls, and 6k, if here we just
    ;   clamp the angle instead of shifting + clamping and effectively
    ;   round all angles to 4 1/1024 PI
    ; i don't think the loss of precision will be noticable...
    ;

    load    (temp0),temp2       ; = xpos|alpha
    addqt   #4,temp0
    move    temp2,xposn
    shlq    #2,temp2            ; = alpha << 2
    and     temp1,temp2         ; clamp 0-2047 (<< 2)
    shrq    #16,xposn
    load    (yhead+temp2),alpha_cos

    load    (temp0),temp2       ; = ypos|beta
    addqt   #4,temp0
    move    temp2,yposn
    shlq    #2,temp2            ; = beta << 2
    and     temp1,temp2         ; clamp 0-2047 (<< 2)
    shrq    #16,yposn
    load    (yhead+temp2),zhead ; = beta_cos

    load    (temp0),temp2       ; = zpos|gamma
        ; no add, stall
    move    temp2,zposn
    shlq    #2,temp2            ; = gamma << 2
    and     temp1,temp2         ; clamp 0-2047 (<< 2)
    shrq    #16,zposn
    load    (yhead+temp2),yhead ; = gamma_cos

    ;
    ; end of loading test data
    ;

    movei   #$0000FFFF,temp1 ; mask -- keep this in a reg somewhere?

    move    alpha_cos,zdown     ; = alpha_sin
    and     temp1,alpha_cos     ; keep lower part
    shrq    #16,zdown           ; keep upper part

    move    zhead,zrite         ; = beta_sin
    and     temp1,zhead         ; keep lower part
    shrq    #16,zrite           ; keep upper part

    move    yhead,gamma_sin
    and     temp1,yhead         ; keep lower part
    shrq    #16,gamma_sin       ; keep upper part

    ;
    ; xrite = cos(gamma) * cos(beta) + sin(gamma) * sin(beta) * sin(alpha);
    ; 

    ; yhead = gamma_cos
    move   yhead,xrite
    move   gamma_sin,temp1

    ; cos(beta) * cos(gamma)
    ; zhead = beta_cos
    imult  zhead,xrite

    ; sin(gamma) * sin(beta)
    ; zrite = beta_sin
    imult  zrite,temp1   
                                        ;
                                        ; xdown = sin(gamma) * cos(alpha);
                                        ; 

                                        ; xdown = sin(gamma)
                                        move   gamma_sin,xdown  
    ; adjust back to 16-bit
    sharq  #14,temp1
                                        ; cos(alpha) * sin(gamma)
                                        imult  alpha_cos,xdown  

    ; sin(alpha) * (sin(gamma) * sin(beta))
    ; zdown = sin(alpha)
    imult  zdown,temp1  
                                        ; adjust result back to 16-bit
                                        ; complete
                                        sharq  #14,xdown

    ; (c(g)*c(b)) + (s(g)*s(b)*s(a))
    add    temp1,xrite

    ; from xhead below
    move    gamma_sin,xhead

    ; adjust result back to 16-bit
    ; completed
    sharq  #14,xrite        

    ;
    ; xhead = sin(gamma) * sin(alpha) * cos(beta) - cos(gamma) * sin(beta);
    ; 

    ;move    gamma_sin,xhead  -- moved above
    ; yhead = gamma_cos
    move    yhead,temp1

    ; sin(gamma) * sin(alpha)
    ; zdown = sin(alpha)
    imult   zdown,xhead

    ; sin(beta) * cos(gamma)
    ; zrite = beta_sin
    imult   zrite,temp1

    sharq   #14,xhead

                                        ;
                                        ; yrite = cos(gamma) * sin(alpha) * sin(beta) - sin(gamma) * cos(beta)
                                        ; 

                                        ; yhead = gamma_cos
                                        move    yhead,yrite

    ; cos(beta) * (sin(beta) * cos(gamma))
    ; zhead = beta_cos
    imult   zhead,xhead
                                        ; cos(gamma) * sin(alpha)
                                        ; zdown = sin(alpha)
                                        imult   zdown,yrite

    ; (s(g)*s(a) - c(b)*c(g)*s(b))
    sub     temp1,xhead
                                        sharq   #14,yrite
    ; completed
    sharq   #14,xhead
                                        ; sin(beta) * (cos(gamma) * sin(alpha))
                                        ; zrite = beta_sin
                                        imult   zrite,yrite
                                        move    gamma_sin,temp1

    ;
    ; ydown = cos(gamma) * cos(alpha)
    ; 

    ; yhead = gamma_cos
    move    yhead,ydown
                                        ; cos(beta) * sin(gamma)
                                        ; zhead = beta_cos
                                        imult   zhead,temp1
    ; cos(alpha) * cos(gamma)
    imult   alpha_cos,ydown
                                        ; (c(g)*s(a)*s(b)) - (s(g)*c(b))
                                        sub     temp1,yrite
    ; complete
    sharq   #14,ydown
                                        ; complete
                                        sharq   #14,yrite

    ;
    ; yhead = cos(gamma) * sin(alpha) * cos(beta) + sin(gamma) * sin(beta);
    ; 

    ; yhead = gamma_cos
    ;move    gamma_cos,yhead
    move    gamma_sin,temp1

    ; cos(gamma) * sin(alpha)
    ; zdown = sin(alpha)
    imult   zdown,yhead
    
    ; sin(beta) * sin(gamma)
    ; zrite = beta_sin
    imult   zrite,temp1

    sharq   #14,yhead
                                        ;
                                        ; zrite = sin(beta) * cos(alpha)
                                        ;

                                        ; zrite = beta_sin
                                        ;move    beta_sin,zrite

                                        ; sin(beta) * cos(alpha)
                                        imult   alpha_cos,zrite
    
    ; cos(beta) * (cos(gamma) * sin(alpha))
    ; zhead = beta_cos
    imult   zhead,yhead
                                        ; complete
                                        sharq   #14,zrite       

    ; (s(b) * s(g)) + (c(b) * c(g) * s(a))
    add     temp1,yhead

                                        ;
                                        ; zhead = cos(beta) * cos(alpha);
                                        ;

                                        ;move    beta_cos,zhead
                                        ; cos(alpha) * cos(beta)
                                        imult   alpha_cos,zhead
    ; complete
    sharq   #14,yhead
                                        ; complete
                                        sharq   #14,zhead
    ;
    ; zdown = -sin(alpha);
    ; 

    ; complete
    neg     zdown           

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
