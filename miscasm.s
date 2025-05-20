;
; miscellaneous assembly language stuff
; 
	.include	'jaguar.inc'

	.globl	_DISPBUF0
	.globl	_DISPBUF1
	.globl	DISPBUF0
	.globl	DISPBUF1

	.bss
DISPBUF0:
_DISPBUF0:
	.ds.w	4
DISPBUF1:
_DISPBUF1:
	.ds.w	3*320*240

	.phrase
_jaguar_logo_material_copy:: 	ds.l 	2

	.data

;
; checkerboard pattern
;
	.phrase
_chkbrd_data:
	dc.w	$7820,$78c0,$7820,$78c0,$7820,$78c0,$7820,$78c0
	dc.w	$78c0,$7820,$78c0,$7820,$78c0,$7820,$78c0,$7820
	dc.w	$7820,$78c0,$7820,$78c0,$7820,$78c0,$7820,$78c0
	dc.w	$78c0,$7820,$78c0,$7820,$78c0,$7820,$78c0,$7820
	dc.w	$7820,$78c0,$7820,$78c0,$7820,$78c0,$7820,$78c0
	dc.w	$78c0,$7820,$78c0,$7820,$78c0,$7820,$78c0,$7820
	dc.w	$7820,$78c0,$7820,$78c0,$7820,$78c0,$7820,$78c0
	dc.w	$78c0,$7820,$78c0,$7820,$78c0,$7820,$78c0,$7820
_chkbrd::
	.dc.w	8,8
	.dc.l	WID8|PIXEL16
	.dc.l 	_chkbrd_data

;
; alternating colored squares
;
	.phrase
; 8x8 checkered flag pattern
_squares_data:
	dc.w	$f080,$7f80,$f080,$7f80,$f080,$7f80,$f080,$7f80
	dc.w	$7f80,$f080,$7f80,$f080,$7f80,$f080,$7f80,$f080
	dc.w	$f080,$7f80,$f080,$7f80,$f080,$7f80,$f080,$7f80
	dc.w	$7f80,$f080,$7f80,$f080,$7f80,$f080,$7f80,$f080
	dc.w	$f080,$7f80,$f080,$7f80,$f080,$7f80,$f080,$7f80
	dc.w	$7f80,$f080,$7f80,$f080,$7f80,$f080,$7f80,$f080
	dc.w	$f080,$7f80,$f080,$7f80,$f080,$7f80,$f080,$7f80
	dc.w	$7f80,$f080,$7f80,$f080,$7f80,$f080,$7f80,$f080
_squares::
	.dc.w	8,8
	.dc.l	WID8|PIXEL16
	.dc.l 	_squares_data

;
; basic indexed checkerboard texture
;
	.phrase
_indexed_chkbrd_data:
	dc.w 	$0102,$0102,$0102,$0102
	dc.w 	$0201,$0201,$0201,$0201
	dc.w 	$0102,$0102,$0102,$0102
	dc.w 	$0201,$0201,$0201,$0201
	dc.w 	$0102,$0102,$0102,$0102
	dc.w 	$0201,$0201,$0201,$0201
	dc.w 	$0102,$0102,$0102,$0102
	dc.w 	$0201,$0201,$0201,$0201
_indexed_chkbrd::
	.dc.w 	8,8
	.dc.l 	WID8|PIXEL8
	.dc.l  	_indexed_chkbrd_data
