;
; utils.s
;


;
; Init3DSprite()
;

SIZEOF_N3DOBJDATA   = 24    ; == sizeof(N3DObjData) + 4 bytes padding to maintain alignment
SIZEOF_FACE         = 28    ; == sizeof(Face) + 4 bytes

_Init3DSprite::

    ; void Init3DSprite(N3DObjdata *sprite, Material *texture, short width, short height);

SPRITE      = 4
MATERIAL    = 8
WIDTH       = 12
HEIGHT      = 14

    ;a0 == sprite (destination)
    ;a1 == _3d_sprite_template (source)

    .extern _3d_sprite_template

    move.l  SPRITE(sp),d0   ; sprite param == destination
    move.l  d0,a0

    lea     _3d_sprite_template,a1

    move.l  (a1)+,(a0)+     ; num faces, num points
    move.l  (a1)+,(a0)+     ; num materials, reserved

    ; the facelist and vertlist will follow this in memory

    add.l   #SIZEOF_N3DOBJDATA,d0
    move.l  d0,(a0)+        ; pointer to facelist

    add.l   #SIZEOF_FACE,d0
    move.l  d0,(a0)+        ; pointer to vertlist

    move.l  MATERIAL(sp),(a0)+  ; pointer to matlist (with 1 material)

    clr.l   (a0)+           ; padding so that facelist is phrase aligned

    adda.l  #12,a1          ; copy facelist
    move.l  (a1)+,(a0)+
    move.l  (a1)+,(a0)+     ; face normal
    move.l  (a1)+,(a0)+     ; num points, material index
    move.l  (a1)+,(a0)+     ; point index, texture coords
    move.l  (a1)+,(a0)+
    move.l  (a1)+,(a0)+
    move.l  (a1)+,(a0)+

    move.w  WIDTH(sp),d0
    lsr.w   #1,d0

    move.w  HEIGHT(sp),d1
    lsr.w   #1,d1

    lea     $4000,a1        ; z vertex normal

    neg.w   d0
    neg.w   d1

    move.w  d0,(a0)+        ; x coord
    move.w  d1,(a0)+        ; y coord
    clr.l   (a0)+           ; z coord, x vertex normal
    move.l  a1,(a0)+        ; y vertex normal, z vertex normal

    neg.w   d0

    move.w  d0,(a0)+        ; x coord
    move.w  d1,(a0)+        ; y coord
    clr.l   (a0)+           ; z coord, x vertex normal
    move.l  a1,(a0)+        ; y vertex normal, z vertex normal

    neg.w   d0
    neg.w   d1

    move.w  d0,(a0)+        ; x coord
    move.w  d1,(a0)+        ; y coord
    clr.l   (a0)+           ; z coord, x vertex normal
    move.l  a1,(a0)+        ; y vertex normal, z vertex normal

    neg.w   d0

    move.w  d0,(a0)+        ; x coord
    move.w  d1,(a0)+        ; y coord
    clr.l   (a0)+           ; z coord, x vertex normal
    move.l  a1,(a0)+        ; y vertex normal, z vertex normal

    rts


;
; AngleAlongVector()
;

_AngleAlongVector::

; via https://www.freesteel.co.uk/wpblog/2009/06/05/encoding-2d-angles-without-trigonometry/
; short diamond_angle(short dx, short dy) {
;   long x = (long)dx << 16;
;   long y = (long)dy << 16;
;   long result;
;
;   if (y >= 0) {
;       if (x >= 0) {
;           result = y/(x+y);
;       } else {
;           result = 0x10000-x/(-x+y);
;      } 
;   } else {
;       if (x < 0) {
;           result = 0x20000-y/(-x-y);
;       } else {
;           result = 0x30000+x/(x-y);
;      }
;   }
;   return result >> 16;
; }

;
; 68k 'diamond angles' function
; returns an angle 0-2047
; quadrants:
;   4|1
;   3|2
;
; dest.x - src.x
; dest.z - src.z (which we call y below)
;

SH  .equ 9      ; shift used to correct divisions 

    move.l  d2,-(sp)

    ; values 1-4 are pre-shifted by this amount
    ; we use this for shifting the result of the calculation of the
    ;   dividend before dividing (because lsl takes 8 max as immediate)
    move.w  #SH,d2 

    move.w  8(sp),d0    ; call it x
    move.w  10(sp),d1   ; call it y
    bmi     .da_y_neg

.da_y_pos:

    tst.w   d0
    bmi     .da_y_pos_x_neg

.da_y_pos_x_pos:

    ; = 1 - y/(x+y)

    add.w   d1,d0       ; = x+y
    ;beq.s  .return_256 ; x+y == 0 cannot happen

    ext.l   d1
    lsl.l   d2,d1   ; = y
    
    divs    d0,d1   ; = d1/d0

    move.w  #(1<<SH),d0 ; = 512 - result
    sub.w   d1,d0

    move.l  (sp)+,d2
    rts

;.return_256:
;   move.w  #256,d0
;   movem.l (sp)+,d1-d2
;   rts

.da_y_pos_x_neg:

    ; = 4 + x/(-x+y)

    sub.w   d0,d1   ; = y-x == -x+y
    ; TODO: check d1 is not 0
    ; return 2304 or 256 if it is

    ext.l   d0
    lsl.l   d2,d0   ; = x

    divs    d1,d0   ; = d0/d1

    ; The -1 is a fix for a bug which occurs when the results of 
    ;   the division is 0, causing the overall result to be 2048.
    ;   This causes the code to index past the desired vector
    ;   table and into the one following it. On PAL this meant
    ;   a normal vector_120 fetch to return the 1st vector_1600
    ;   entry, and cause the enemy ship to jump up the screen.
    add.w   #(4<<SH)-1,d0   ; = 2048 + result

    move.l  (sp)+,d2
    rts

.da_y_neg:

    tst.w   d0
    bmi     .da_y_neg_x_neg

.da_y_neg_x_pos:

    ; = 2 - x/(x-y)

    neg.w   d1      ; = -y
    add.w   d0,d1   ; = -y+x = x-y  
    ; TODO: check d1 is not 0
    ; return 1280 if it is

    ext.l   d0
    lsl.l   d2,d0   ; = x
    
    divs    d1,d0   ; = d0/d1

    neg.w   d0
    add.w   #(2<<SH),d0     ; = 1024 - result

    move.l  (sp)+,d2
    rts

.da_y_neg_x_neg:

    ; = 3 + y/(-x-y)

    neg.w   d0      ; = -x
    sub.w   d1,d0   ; = -x-y
    ; TODO: check d0 is not 0
    ; return 1792 if it is

    ext.l   d1
    lsl.l   d2,d1   ; = y

    
    divs    d0,d1   ; = d1/d0

    add.w   #(3<<SH),d1     ; = 1536 + result
    move.w  d1,d0

    move.l  (sp)+,d2
    rts

