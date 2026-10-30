;
; storage.s
;

    .include    'jaguar.inc'

    .bss

; the screen buffers -- 3 image buffers and a Z buffer
    .phrase
_DISPBUF0::
    .ds.w   4
_DISPBUF1::
    .ds.w   4
_DISPBUF2::
    .ds.w   4*320*200

; variables used by engine.c
    .long
_camtrans::     .ds.w   6   ; sizeof(Transform)

; variables used by counters.c
_tock_size::        .ds.w   1
_tock_size_x2::     .ds.w   1
_one_second::       .ds.w   1
_move_step::        .ds.w   1
_move_steps::       .ds.l   1
_rotation_step::    .ds.w   1
_is_tock_frame::    .ds.w   1

_tock_multiples::   .ds.w   32 

; variables used by tweens.c
TWEENSMAX = 40
TWEENSIZE = 24
_tween_count::      .ds.w    1
_tween_list::       .ds.l    TWEENSMAX*(TWEENSIZE/4)
_tween_pointers::   .ds.l    TWEENSMAX

SIZEOF_3DSPRITE = 100

;
; Storage for the Jaguar logo 3D sprite, which we create with Init3DSprite()
;

    .phrase
_jaguar_logo_3d_sprite::
    .ds.l    (SIZEOF_3DSPRITE/4)

;
; The Material made from the imported Jaguar image, which is passed as the
;   Material parameter to Init3DSprite()
;

    .data
    .phrase
_jaguar_logo_material::
    .dc.w    $0000, 0                ; colour for shading, reserved
    .dc.l    ._jaguar_logo_bitmap    ; texture

    .extern     _jaguar_logo         ; created from models/textures/jaguar_logo.tga

._jaguar_logo_bitmap:
    .dc.w   512, 128
    .dc.l   PITCH1|PIXEL16|WID512
    .dc.l   _jaguar_logo


;
; Storage for the explosion 3D sprite which is produced by the A button
;

    .bss
    .phrase
_explosion_3d_sprite::
    .ds.l   (SIZEOF_3DSPRITE/4)

;
; The materials made from the imported explosion frames
; image from https://opengameart.org/content/explosion CC0 Public Domain by Cuzco
;

    .extern _explosion_0
    .extern _explosion_1
    .extern _explosion_2
    .extern _explosion_3

    .data
    .phrase
_explosion_0_material::
    .dc.w    $0000, 0
    .dc.l    ._explosion_0_bitmap

._explosion_0_bitmap:
    .dc.w   64, 64
    .dc.l   PITCH1|PIXEL16|WID64
    .dc.l   _explosion_0

    .phrase
_explosion_1_material::
    .dc.w    $0000, 0
    .dc.l    ._explosion_1_bitmap

._explosion_1_bitmap:
    .dc.w   64, 64
    .dc.l   PITCH1|PIXEL16|WID64
    .dc.l   _explosion_1

    .phrase
_explosion_2_material::
    .dc.w    $0000, 0
    .dc.l    ._explosion_2_bitmap

._explosion_2_bitmap:
    .dc.w   64, 64
    .dc.l   PITCH1|PIXEL16|WID64
    .dc.l   _explosion_2

    .phrase
_explosion_3_material::
    .dc.w    $0000, 0
    .dc.l    ._explosion_3_bitmap

._explosion_3_bitmap:
    .dc.w   64, 64
    .dc.l   PITCH1|PIXEL16|WID64
    .dc.l   _explosion_3
