;
; utils.s
;

    .bss

;
; space for testing returning values from the GPU
;

    .long
_test_returns::
    .ds.l   8 ; enough for 16 words


; ram buffer for Gouraud shaded texture mapping;
; use this if the CLUT is unavailable

    .phrase
_rambuf::
    .ds.w   324


;
; array for holding profiling information
; not currently used
;
    .long
_proftime::
    .ds.l   32

    .text

;
; Constants
; These must be kept in sync with n3d.h
;

SIZEOF_N3DOBJDATA   = 24 ;20    ; == sizeof(N3DObjData)
SIZEOF_FACE         = 28 ;24    ; == sizeof(Face)
SIZEOF_POINT        = 12    ; == sizeof(Point)
SIZEOF_MATERIAL     = 8     ; == sizeof(Material)

;
; CopyModelDeep()
;

_CopyModelDeep::

    ; void *CopyModelDeep(
    ;   N3DObjdata *original, 
    ;   void *buffer, 
    ;   short copy_faces, 
    ;   short copy_points, 
    ;   short scale, 
    ;   short copy_materials
    ; );

; params:
ORIGINAL        = 4
BUFFER          = 8
COPY_FACES      = 12
COPY_POINTS     = 14
COPY_MATERIALS  = 16
SCALE           = 18

; scale values
NO_SCALE    = 0
SCALE_1_0   = $0100 ; 1.0 in 8.8 fixed point format

    ; a0 == source (original)
    ; a1 == destination (buffer)

    move.l  ORIGINAL(sp),a0
    move.l  BUFFER(sp),a1

    movem.l     d2-d3/a2-a3,-(sp)

    move.l  (a0)+,d0        ; copy numpolys|numpoints
    move.l  (a0)+,d1        ; copy nummaterials|reserved
    
    move.l  d0,(a1)+     
    move.l  d1,(a1)+

    lea     12(a1),a3       ; buffer after the object header, for other data
                            ; doesn't require padding for alignment
    ;
    ; faces
    ;

    move.l  (a0)+,a2        ; pointer to original faces

    tst.w   (COPY_FACES+16)(sp)     ; check copy_faces param
    bne     .copy_faces

    move.l  a2,(a1)+
    bra     .copy_faces_done

.copy_faces:

    move.l  a3,(a1)+        ; pointer to copy faces

    swap    d0          ; numpolys|numpoints -> numpoints|numpolys
    subq.w  #1,d0       ; numpolys counter

.faces_loop:

    move.l  (a2)+,(a3)+     ; copy fx|fy
    move.l  (a2)+,(a3)+     ; copy fz|fd
    move.l  (a2)+,d2        ; copy npts|material
    move.l  d2,(a3)+

        swap    d2      ; npts
        subq.w  #1,d2

.face_points_loop:

        move.l  (a2)+,(a3)+     ; point index|u,v
        dbra    d2,.face_points_loop

    dbra    d0,.faces_loop

    swap    d0 ; swap back for numpoint counter

.copy_faces_done:

    ;
    ; points
    ;

    move.l  (a0)+,a2        ; pointer to original points

    tst.w   (COPY_POINTS+16)(sp)    ; check copy_points param
    bne     .copy_points

    move.l  a2,(a1)+
    bra     .copy_points_done

.copy_points:

    move.l  a3,(a1)+        ; pointer to copy points

    subq.w  #1,d0           ; numpoint counter

    move.w  (SCALE+16)(sp),d2  ; should we scale the points?
    beq     .no_scale_loop
    cmp.w   #SCALE_1_0,d2
    beq     .no_scale_loop

    ;
    ; scaled points
    ;

.scaled_loop:

    move.w  (a2)+,d3        ; x
    muls    d2,d3
    asr.l   #8,d3
    move.w  d3,(a3)+

    move.w  (a2)+,d3        ; y
    muls    d2,d3
    asr.l   #8,d3
    move.w  d3,(a3)+

    move.w  (a2)+,d3        ; z
    muls    d2,d3
    asr.l   #8,d3
    move.w  d3,(a3)+

    move.w  (a2)+,(a3)+     ; vx
    move.l  (a2)+,(a3)+     ; vy,vz

    dbra    d0,.scaled_loop

    bra     .copy_points_done

    ;
    ; un-scaled points
    ;

.no_scale_loop:

    move.l  (a2)+,(a3)+     ; copy x|y
    move.l  (a2)+,(a3)+     ; copy z|vx
    move.l  (a2)+,(a3)+     ; copy vy,vz

    dbra    d0,.no_scale_loop

.copy_points_done:

    ;
    ; materials
    ;

    move.l  (a0)+,a2        ; pointer to original materials

    tst.w   (COPY_MATERIALS+16)(sp)
    bne     .copy_materials

    move.l  a2,(a1)         ; pointer to provided material
    bra     .done

.copy_materials:

    move.l  a3,(a1)     ; pointer to copied materials

    swap    d1          ; nummaterials counter
    subq.w  #1,d1

.materials_loop:

    move.l  (a2)+,(a3)+     ; copy color|flags
    move.l  (a2)+,(a3)+     ; copy of tmap

    dbra    d1,.materials_loop

.done:

    move.l  a3,d0           ; return value, pointer to end of copied structure

    movem.l     (sp)+,d2-d3/a2-a3
    rts


;
; Init3DSprite()
;

_Init3DSprite::

    ; void Init3DSprite(N3DObjdata *sprite, Material *texture, short width, short height);

SPRITE      = 4
MATERIAL    = 8
WIDTH       = 12
HEIGHT      = 14

    .extern _3d_sprite_template

    ;
    ; call CopyModelDeep() to clone 3D sprite template
    ;

    move.l  SPRITE(sp),d0   ; sprite param == destination

    clr.l   -(sp)       ; NO_SCALE, do not copy materials
    moveq   #1,d1
    move.w  d1,-(sp)    ; copy points
    move.w  d1,-(sp)    ; copy faces

    move.l  d0,-(sp)    ; destination buffer

    move.l  #_3d_sprite_template,-(sp)    ; source model

    bsr     _CopyModelDeep
    adda.l  #16,sp

    ;
    ; fix-up cloned sprite
    ;

    move.l  SPRITE(sp),a0   ; sprite param == destination

    move.l  MATERIAL(sp),d0
    move.l  d0,16(a0)       ; sprite->material == material

    move.l  12(a0),a0       ; pointer to points array

    move.w  WIDTH(sp),d0
    lsr.w   #1,d0

    move.w  HEIGHT(sp),d1
    lsr.w   #1,d1

    neg.w   d0
    neg.w   d1

    move.w  d0,(a0)+        ; x coord
    move.w  d1,(a0)         ; y coord
    lea     10(a0),a0

    neg.w   d0

    move.w  d0,(a0)+        ; x coord
    move.w  d1,(a0)         ; y coord
    lea     10(a0),a0

    neg.w   d0
    neg.w   d1

    move.w  d0,(a0)+        ; x coord
    move.w  d1,(a0)         ; y coord
    lea     10(a0),a0

    neg.w   d0

    move.w  d0,(a0)+        ; x coord
    move.w  d1,(a0)         ; y coord

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


;
; short RunPoints(Particle *particles, short count)
;

; params
PARTICLES   = 4
COUNT       = 8

; particle struct
Z   = 2
Y   = 4
X   = 6
DX  = 10
DY  = 12
DZ  = 14

_RunPoints::

    movea.l     PARTICLES(sp),a0
    move.w      COUNT(sp),d1        ; == loop counter

    subq.w      #1,d1
    bmi         ._RunPoints_done    ; count was 0

    move.l      d2,-(sp)
    moveq       #0,d0
    
    bra         ._RunPoints_start   ; skip first increment

._RunPoints_loop:

    add     #16,a0

._RunPoints_start:

    move.w      (a0),d2
    beq         ._RunPoints_loop    ; unused, find next one
    
    subq.w      #1,d2
    move.w      d2,(a0)
    beq         ._RunPoints_next    ; ttl expired, move on

    addq.w      #1,d0       ; count of live particles

    move.w      X(a0),d2
    add.w       DX(a0),d2
    move.w      d2,X(a0)

    move.w      Y(a0),d2
    add.w       DY(a0),d2
    move.w      d2,Y(a0)

    move.w      Z(a0),d2
    add.w       DZ(a0),d2
    move.w      d2,Z(a0)

._RunPoints_next:

    dbra    d1,._RunPoints_loop

    move.l  (sp)+,d2

._RunPoints_done:

    rts
