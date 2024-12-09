;***********************************************************
; Texture mapping module: very simple version
; no shading
;***********************************************************
;
;

.if TRANSPARENTTEX
FIRSTBCMD	=	(SRCEN|LFU_S|DSTEN|DSTENZ|DSTWRZ|ZMODEGT|ZBUFF|DSTA2|BKGWREN|DCOMPEN)
.else
FIRSTBCMD	=	(SRCEN|LFU_S|DSTEN|DSTENZ|DSTWRZ|ZMODEGT|ZBUFF|DSTA2|BKGWREN)
.endif

srcpix		.equr	righti			; next value for A1_PIXEL
srcfpix		.equr	b_color			; next value for A1_FPIXEL

;
; SET-UP ROUTINE FOR TEXTURE MAPPED POLYGONS
; called in regbank 1
;
;	.REGBANK1

	.globl	POLYinit
POLYinit:
texpoly:

	; the calling code will have ensured the blitter has finished

	movei	#FIRSTBCMD,temp0
	moveta 	temp0,bcmdval

.if TRANSPARENTTEX
.if TRANSPARENTPIXEL
	moveq 	#1,temp0
	neg 	temp0
.else
	moveq 	#0,temp0
.endif
	movei	#B_PATD,temp1 		; == B_PATD
	store 	temp0,(temp1)
	addqt 	#4,temp1
	store 	temp0,(temp1)
.endif

	; moved out of the loop below (because otherwise we're doing nothing in here)
	;	...and it doesn't work. find out why...
	;movei	#FIRSTBCMD,bcmdval

	jump	(return)
	nop




;**********************************************
; Render a texture mapped trapezoid
;**********************************************

	.globl	POLYdraw
POLYdraw:
textrap:

	;.REGBANK0				; we are entered in register bank 0

;
; set up the destination registers
;

	move 	a2_ptr,atemp2
	subqt 	#(A2_PIXEL-A2_BASE),atemp2
	store	destaddr,(atemp2)		; set A2_BASE

	move 	destflags,atemp1
	addqt	#(A2_FLAGS-A2_BASE),atemp2
	bset	#16,atemp1 				; == #XADDPIX
		; TODO: can we make this change when we setup destflags? does it hold
		;	under all conditions apart from phrase-mode?
	store	atemp1,(atemp2)			; set A2_FLAGS

;
; set up the source registers (or as many as we can)
;
	move 	a1_ptr,atemp2
	subqt 	#(A1_PIXEL-A1_BASE),atemp2
	store	srcaddr,(atemp2)

	moveq	#3,atemp1 		; XADDINC == 0x00030000
	shlq 	#16,atemp1
	or		srcflags,atemp1	
	addqt	#4,atemp2
	store	atemp1,(atemp2)

		; replace with AND if we can spare a reg
	shrq	#16,ay						; strip fractional part of ay
	shlq	#16,ay

	movei	#texdone,atemp0
	subq	#1,anumlines
	jump	MI,(atemp0) 	; finished
	nop

	.globl	texline1
texline1:

	move	rightx,bcount
	move	leftx,atemp0
	shrq	#16,bcount
	shrq	#16,atemp0
	sub		atemp0,bcount
	or		atemp0,ay		; now ay has Y in high word, X in low word

	cmpq	#1,bcount
	movei	#texzerocount,atemp0
	jump	MI,(atemp0)		; if the line is empty, don't actually do a blit
	nop
;
; calculate 1/bcount
;
	move	bcount,atemp0
	moveq	#1,atemp1
	shlq	#16,atemp0
	shlq	#14,atemp1
	div		atemp0,atemp1		; atemp1 will be a 14 bit signed fraction

	bset	#16,bcount
;
; set up srcpix and srcfpix while the
; divide is happening
;
	move	leftv,atemp0
	move	leftu,srcpix			; srcpix is integer parts of pixel values
	shrq	#16,atemp0
	shrq	#16,srcpix			; low word of srcpix has integer part of leftu in it
	shlq	#16,atemp0			; high word of atemp0 has integer part of leftv in it
	move	leftu,srcfpix
	or		atemp0,srcpix
	shlq	#16,srcfpix			; now srcfpix has fractional part of leftu in high word
	move	leftv,atemp0
	shrq	#16,srcfpix
	shlq	#16,atemp0
	or		atemp0,srcfpix

;
; calculate b_uinc and b_vinc
;
	move	rightv,b_vinc
	move	rightu,b_uinc
	sub		leftv,b_vinc		; find delta V
	sub		leftu,b_uinc		; find delta U
	sharq	#10,b_vinc		; convert to 16 bit (assumes U,V are less than 512)
	sharq	#10,b_uinc
	imult	atemp1,b_vinc		; multiply by 1/linelen
	imult	atemp1,b_uinc
	sharq	#(14-10),b_vinc
	sharq	#(14-10),b_uinc
;
; calculate b_zinc
;
	move	rightz,b_zinc		; find delta Z
	sub		leftz,b_zinc
	move	b_zinc,atemp0		; atemp0 will hold the fractional part of delta Z
	sharq	#16,b_zinc		; put integer part of delta Z in low word
	imult	atemp1,atemp0		; multiply fractional part by reciprocal of line length
								;	(a 14 bit number)
	imult	atemp1,b_zinc		; multiply integer part by reciprocal of line length
								;	(a 14 bit number)
	sharq	#14,atemp0		; normalize fractional product
	shlq	#2,b_zinc		; normalize integer product
	add		atemp0,b_zinc
;
; now set up b_uinc so it is the integer part of A1_INC, b_vinc so it is the fractional part
;
	move	b_vinc,atemp0
	move	b_uinc,atemp1
	shrq	#16,atemp0			; strip fractional part of VINC
	shrq	#16,b_uinc			; get integer part of UINC into low word
	shlq	#16,atemp0
		; replace with AND $FFFF0000,atemp0
	or		atemp0,b_uinc		; integer part of VINC goes into high word of b_uinc

	shlq	#16,atemp1			; atemp2 has UINC; strip high word (integer part)
	shlq	#16,b_vinc
	shrq	#16,atemp1
		; replace with AND $FFFF0000,atemp1
		; stall
	or		atemp1,b_vinc

.bwait:
	load	(bcmd_ptr),atemp0
	btst	#0,atemp0
	jr	EQ,.bwait
	nop

	store	srcpix,(a1_ptr)					; set A1_PIXEL
	addqt	#(A1_FPIXEL-A1_PIXEL),a1_ptr	; == A1_FPIXEL

	store	ay,(a2_ptr)						; set A2_PIXEL to destination pixel
	addqt 	#(B_COUNT-A2_PIXEL),a2_ptr 		; == B_COUNT

	store	srcfpix,(a1_ptr)
	addqt 	#(A1_INC-A1_FPIXEL),a1_ptr 		; == A1_INC

	store 	bcount,(a2_ptr) 				; set B_COUNT
	subqt 	#(B_COUNT-A2_PIXEL),a2_ptr  	; reset to A2_PIXEL

	store	b_uinc,(a1_ptr) 				; set A1_INC
	addqt	#(A1_FINC-A1_INC),a1_ptr

	store	leftz,(bz3_ptr)					; set up the Z value

	store	b_vinc,(a1_ptr) 				; set A1_FINC
	subqt	#(A1_FINC-A1_PIXEL),a1_ptr 	;	 reset to A1_PIXEL

	move 	bz3_ptr,atemp2
	subqt	#(B_Z3-B_ZINC),atemp2 		; == B_ZINC
	store	b_zinc,(atemp2)

	store	bcmdval,(bcmd_ptr)			; start the first blit


;
; come here if a zero length line was encountered
;
texzerocount:
	movefa	leftistep,atemp1
	movefa	leftzstep,atemp0
	add		atemp1,lefti
	add		atemp0,leftz
	add		rightxstep,rightx
	add		leftxstep,leftx
	movefa	leftvstep,atemp0
	movefa	leftustep,atemp1
	add		atemp0,leftv
	add		atemp1,leftu
	movefa	rightvstep,atemp0
	movefa	rightustep,atemp1
	add		atemp0,rightv
	add		atemp1,rightu
	movefa	rightzstep,atemp0
	add		atemp0,rightz

	movei	#texline1,atemp1
	shrq	#16,ay				; isolate just the Y value
	subq	#1,anumlines
	addqt	#1,ay				; increment it
	jump	PL,(atemp1)
	shlq	#16,ay				; put Y back in high word

texdone:

;
; we have to wait here for the blitter to finish, because
; mmult loses if there's a blit going on out of
; GPU RAM
;
; TODO: Are we blitting out of GPU RAM here?
;
.bwait:
	load	(bcmd_ptr),atemp0
	btst	#0,atemp0
	jr	EQ,.bwait
	nop

	movefa	return,atemp2
	jump	(atemp2)
	movefa 	gflags,atemp1 		; setup for the code we're returning to


	.equrundef	srcpix
	.equrundef	srcfpix
