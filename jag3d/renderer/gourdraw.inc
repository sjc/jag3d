;
; gour: draw a Gouraud shaded triangle
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
; SET-UP ROUTINE FOR GOURAUD SHADED POLYGONS
; called in regbank 1
;
;	.REGBANK1

	.globl	gourpoly
gourpoly:

	; the caller will have ensured that the blitter is not in use
	; temp0 == bcmd_ptr

	addqt	#4,temp0		; now temp0 == B_COUNT
	moveta	temp0,bcount_ptr

	addqt 	#32,temp0 		; == B_I3 == B_COUNT + 64
	addqt 	#32,temp0

	movei	#PATDSEL|GOURD|ZBUFF|ZMODEGT|DSTENZ|DSTWRZ|DSTEN|BKGWREN,temp1

	moveta	temp0,bi3_ptr
	moveta	temp1,bcmdval

	movefa	b_color,temp1
	subqt 	#(B_I3-B_PATD),temp0 	; == B_PATD

	store	temp1,(temp0)
	addqt	#4,temp0

		movefa	a1_ptr,temp2

	store	temp1,(temp0)
	
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

	; do these need to be set every time through?
	move 	bi3_ptr,bzinc_ptr
	move 	bi3_ptr,biinc_ptr
	subqt	#(B_I3-B_ZINC),bzinc_ptr
	subqt	#(B_I3-B_IINC),biinc_ptr

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
	movei	#.zerocount,atemp1
	jump	MI,(atemp1)		; if the line is empty, don't actually do a blit
	nop

.if (^^defined PROFILING)
	ADDTIME	SCANLINES
.endif

;
; calculate 1/bcount
;
	move	bcount,atemp0
	moveq	#1,atemp1
	shlq	#16,atemp0
	shlq	#14,atemp1
	div		atemp0,atemp1		; atemp1 will be a 14 bit signed fraction
		; is there a way to replace this div?
	bset	#16,bcount 			; ready for blitter: lines|rows = 1|bcount

;
; set up b_zinc and b_iinc
;
	move	righti,b_iinc
	move	rightz,b_zinc
	sub		lefti,b_iinc
	sub		leftz,b_zinc
	sharq	#9,b_iinc
	sharq	#14,b_zinc
	imult	atemp1,b_iinc
	imult	atemp1,b_zinc
;
; normally, we would adjust b_iinc (which was shifted right by 9 above) by doing
;		sharq	#(14-9),b_iinc
; but since we want to strip the upper 8 bits of b_iinc anyways, instead we can
; do the next 2 instructions: (14-9) = 5, and 8-5 = 3
	shlq	#3,b_iinc
	shrq	#8,b_iinc

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
	store	b_iinc, (biinc_ptr)
	store	b_zinc, (bzinc_ptr)
	store	lefti, (bi3_ptr)
	store	leftz, (bz3_ptr)
	store	ay,(a1_ptr)			; set A1_PIXEL
	store	bcount,(bcount_ptr)
	store	bcmdval,(bcmd_ptr)	; starts the blit

.zerocount:
	add		rightxstep,rightx
	add		leftxstep,leftx
	movefa	leftistep,atemp1
	movefa	leftzstep,atemp0
	add		atemp1,lefti
	add		atemp0,leftz
	sat24	lefti
	movefa	rightistep,atemp1
	movefa	rightzstep,atemp0
	add		atemp1,righti
	add		atemp0,rightz
	sat24	righti

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
