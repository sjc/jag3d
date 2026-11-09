;
; text.s
;

    .include 'jaguar.inc'

SPACE       .equ    32  ; first ASCII character
CHARSHIFT   .equ    5   ; each 8px char is 32 bytes
CHARSHIFT16 .equ    6   ; each 16px char is 64 bytes

;
; void draw_string_to_buffer(char *string, short pen, void *buffer, short stride, void *font);
;
_draw_string_to_buffer::

    movem.l d2/a2-a4,-(sp)

    move.l  16+4(sp),a0        ; string
    move.b  16+9(sp),d2        ; initial pen colour -- should be a PENn constant
    move.l  16+10(sp),a1       ; buffer
    move.w  16+14(sp),d0       ; stride
    move.l  16+16(sp),a4       ; font

    ; drop through to set the initial colour mask

.write_string_set_colour:

    neg.b   d2
    subq.w  #1,d2
    lsl.w   #2,d2
    lea     colour_masks,a3
    move.l  (a3,d2.w),d1
    bra.s   .write_string_to_buffer

; 
; a0: 0-terminated string
; a1: buffer
; a4: font
; d0.w: stride
; d1.l: colour mask
; uses: d2,a2,a3,a4
;

.write_string_to_buffer_loop:

    lsl.w   #CHARSHIFT,d2           ; calculate address of char
    move.l  a4,a2
    adda.w  d2,a2

    move.l  a1,a3                   ; pointer to destination in buffer
    addq.l  #4,a1

    ; unwound 8-row drawing

    move.l  (a2)+,d2                ; read char row
    and.l   d1,d2                   ; apply colour mask
    move.l  d2,(a3)                 ; write row
    adda.w  d0,a3                   ; move down to next row

    move.l  (a2)+,d2
    and.l   d1,d2
    move.l  d2,(a3)
    adda.w  d0,a3

    move.l  (a2)+,d2
    and.l   d1,d2
    move.l  d2,(a3)
    adda.w  d0,a3

    move.l  (a2)+,d2
    and.l   d1,d2
    move.l  d2,(a3)
    adda.w  d0,a3

    move.l  (a2)+,d2
    and.l   d1,d2
    move.l  d2,(a3)
    adda.w  d0,a3

    move.l  (a2)+,d2
    and.l   d1,d2
    move.l  d2,(a3)
    adda.w  d0,a3

    move.l  (a2)+,d2
    and.l   d1,d2
    move.l  d2,(a3)
    adda.w  d0,a3

    move.l  (a2)+,d2
    and.l   d1,d2
    move.l  d2,(a3)
    adda.w  d0,a3

.write_string_to_buffer:

    clr.w   d2
    move.b  (a0)+,d2
    bmi.s   .write_string_set_colour
    sub.b   #SPACE,d2
    bpl.b   .write_string_to_buffer_loop        ; we'll quit on anything non-printable

    movem.l (sp)+,d2/a2-a4
    rts


;
; void draw_string_to_buffer_16(char *string, short pen, void *buffer, short stride, void *font);
;
; as above, but using the 16px tall font
;
_draw_string_to_buffer_16::

    movem.l d2/a2-a4,-(sp)

    move.l  16+4(sp),a0        ; string
    move.b  16+9(sp),d2        ; initial pen colour -- should be a PENn constant
    move.l  16+10(sp),a1       ; buffer
    move.w  16+14(sp),d0       ; stride
    move.l  16+16(sp),a4       ; font

    ; drop through to set the initial colour mask

.write_string_set_colour_16:

    neg.b   d2
    subq.w  #1,d2
    lsl.w   #2,d2
    lea     colour_masks,a3
    move.l  (a3,d2.w),d1
    bra     .write_string_to_buffer_16

.write_string_to_buffer_16_loop:

    lsl.w   #CHARSHIFT16,d2           ; calculate address of char
    move.l  a4,a2
    adda.w  d2,a2

    move.l  a1,a3                   ; pointer to destination in buffer
    addq.l  #4,a1

    ; unwrapped 8-row drawing

    move.l  (a2)+,d2                ; read char row
    and.l   d1,d2                   ; apply colour mask
    move.l  d2,(a3)                 ; write row
    adda.l  d0,a3                   ; move down to next row

    move.l  (a2)+,d2
    and.l   d1,d2
    move.l  d2,(a3)
    adda.l  d0,a3

    move.l  (a2)+,d2
    and.l   d1,d2
    move.l  d2,(a3)
    adda.l  d0,a3

    move.l  (a2)+,d2
    and.l   d1,d2
    move.l  d2,(a3)
    adda.l  d0,a3

    move.l  (a2)+,d2
    and.l   d1,d2
    move.l  d2,(a3)
    adda.l  d0,a3

    move.l  (a2)+,d2
    and.l   d1,d2
    move.l  d2,(a3)
    adda.l  d0,a3

    move.l  (a2)+,d2
    and.l   d1,d2
    move.l  d2,(a3)
    adda.l  d0,a3

    move.l  (a2)+,d2
    and.l   d1,d2
    move.l  d2,(a3)
    adda.l  d0,a3

    move.l  (a2)+,d2
    and.l   d1,d2
    move.l  d2,(a3)
    adda.l  d0,a3

    move.l  (a2)+,d2
    and.l   d1,d2
    move.l  d2,(a3)
    adda.l  d0,a3

    move.l  (a2)+,d2
    and.l   d1,d2
    move.l  d2,(a3)
    adda.l  d0,a3

    move.l  (a2)+,d2
    and.l   d1,d2
    move.l  d2,(a3)
    adda.l  d0,a3

    move.l  (a2)+,d2
    and.l   d1,d2
    move.l  d2,(a3)
    adda.l  d0,a3

    move.l  (a2)+,d2
    and.l   d1,d2
    move.l  d2,(a3)
    adda.l  d0,a3

    move.l  (a2)+,d2
    and.l   d1,d2
    move.l  d2,(a3)
    adda.l  d0,a3

    move.l  (a2)+,d2
    and.l   d1,d2
    move.l  d2,(a3)
    adda.l  d0,a3

.write_string_to_buffer_16:

    clr.w   d2
    move.b  (a0)+,d2
    bmi     .write_string_set_colour_16
    sub.b   #SPACE,d2
    bpl     .write_string_to_buffer_16_loop     ; we'll quit on anything non-printable

    movem.l (sp)+,d2/a2-a4
    rts


    .data
    .even
colour_masks:   .dc.l   $11111111,$22222222,$33333333,$44444444
                .dc.l   $55555555,$66666666,$77777777,$88888888
                .dc.l   $99999999,$AAAAAAAA,$BBBBBBBB,$CCCCCCCC
                .dc.l   $DDDDDDDD,$EEEEEEEE,$FFFFFFFF
