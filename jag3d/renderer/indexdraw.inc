;
; indexdraw: draw a flat-shaded triangle
;
; we can re-use some registers, namely
; leftu,leftustep,leftv,leftvstep,srcaddr,srcflags
; b_zinc

bcount_ptr	.equr	srcaddr			; address of B_COUNT
bi3_ptr		.equr	srcflags		; address of B_I3
traploop	.equr	leftu
biinc_ptr	.equr	rightu
bzinc_ptr	.equr	leftv

;
; SET-UP ROUTINE FOR FLAT-SHADED POLYGONS
; still named `gourpoly` since that's what the calling code expects
; called in regbank 1
;
;	.REGBANK1

	.globl	gourpoly
gourpoly:

	; the caller will have ensured that the blitter is not in use
	; temp0 == bcmd_ptr

	addqt	#4,temp0		; now temp0 == B_COUNT
	moveta	temp0,bcount_ptr

	addqt 	#32,temp0 		; == B_PATD == B_COUNT + 44
		moveq 	#0,temp1
	addqt 	#12,temp0
		bset 	#16,temp1   ; PATDSEL is bit 16 set

	moveta	temp1,bcmdval ; TODO: can we setup bcmdval once somewhere?

	movefa	b_color,temp1

	store	temp1,(temp0) ; just store b_color in the first long of B_PATD ???

	movefa	a1_ptr,temp2
	movefa	destaddr,temp1
	subqt	#(A1_PIXEL-A1_BASE),temp2
	store	temp1,(temp2)		; set A1_BASE
	
	movefa	destflags,temp1
	addqt	#4,temp2 			; == A1_FLAGS

	bset 	#16,temp1 			; XADDPIX == 0x00010000

	jump	(return)
	store	temp1,(temp2)		; set A1_FLAGS




;
; per-trapezoid routine for gouraud shading
; called in regbank 0
;
	.globl	gourtrap
gourtrap:
	;.REGBANK0

.if (^^defined PROFILING)
	ADDTIME	POLYSETUP
.endif

								; $FFFF0000
	shrq	#16,ay				; strip fractional part of ay
	move 	PC,traploop 		; adjusted below
	shlq	#16,ay

	subq	#1,anumlines
	movei	#gourdone,atemp0
	jump	MI,(atemp0)
	addqt 	#16,traploop 		; == gourline:

gourline: 
	move	leftx,atemp0			; branch optimization
	move	rightx,bcount
	shrq	#16,atemp0
	shrq	#16,bcount
	or		atemp0,ay			; ay = destination pixel (Y|X)
	sub		atemp0,bcount		; bcount = # of pixels to write

	cmpq	#1,bcount
	movei	#.zerocount,atemp1 ; TODO: load this into a register
	jump	MI,(atemp1)		; if the line is empty, don't actually do a blit
	nop

.if (^^defined PROFILING)
	ADDTIME	SCANLINES
.endif

	bset	#16,bcount 			; ready for blitter: lines|rows = 1|bcount

.bwait:
	load	(bcmd_ptr),atemp0
	btst	#0,atemp0
	jr	EQ,.bwait
	nop

.if (^^defined PROFILING)
	ADDTIME	BLITWAIT
.endif

;
; set Y to Y|X (i.e. A1_PIXEL value)
; and set bcount to the (16 bit) count value
;
	store	ay,(a1_ptr)			; set A1_PIXEL
	store	bcount,(bcount_ptr)
	store	bcmdval,(bcmd_ptr)	; starts the blit

.zerocount:
	add		rightxstep,rightx
	add		leftxstep,leftx

	shrq	#16,ay
	subq	#1,anumlines
	addqt	#1,ay
	jump	PL,(traploop) ; .gourline
	shlq	#16,ay


gourdone:

.if (^^defined PROFILING)
	ADDTIME	SCANLINES
.endif

	movefa	return,atemp2
	jump	(atemp2)
	movefa 	gflags,atemp1 		; setup for the code we're returning to


	.equrundef	bi3_ptr
	.equrundef	bcount_ptr
	.equrundef	traploop
	.equrundef	biinc_ptr
	.equrundef	bzinc_ptr
