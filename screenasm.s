;
; screenasm.s
; Screen helper functions
;

;
; set xpos and ypos while preserving other values
; d0.w: xpos
; d1.w: ypos -- assume it's already doubled and shifted
; a0: pointer to bitmap
; uses: d2
;

_set_olbitmap_xy:

    move.w  6(a0),d2    ; ypos in 1st phrase
    andi.w  #$C007,d2   ; preserve height and type
    or.w    d1,d2
    move.w  d2,6(a0)

    move.w  14(a0),d2   ; xpos in 2nd phrase
    andi.w  #$F000,d2   ; preserve other stuff
    or.w    d0,d2
    move.w  d2,14(a0)

    rts

;
; change position of the main buffers to the given absolute coordinates
; void position_drawbuffers(short x, short y)
;

    ; these are defined in screen.h and setup in screen.c
    .extern _packed_olist1
    .extern _packed_olist2
    .extern _packed_olist3

_position_drawbuffers::

    move.l  d2,-(sp)

    move.w  4+4(sp),d0
    move.w  4+6(sp),d1
    lsl.w   #3,d1 ; shift y into place

    lea     _packed_olist1,a0
    jsr     _set_olbitmap_xy

    lea     _packed_olist2,a0
    jsr     _set_olbitmap_xy

    lea     _packed_olist3,a0
    jsr     _set_olbitmap_xy

    move.l  (sp)+,d2
    rts
