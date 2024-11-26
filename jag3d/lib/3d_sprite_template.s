;
; 3D Library Data File
; This file is used as a template for creating 3D sprites via
;	the Init3DSprite() 68k function
;

	.include	'jaguar.inc'

	.data

_3d_sprite_template::
	dc.w	1			; Number of faces
	dc.w	4			; Number of points
	dc.w	1			; Number of materials
	dc.w	0			; reserved word
	dc.l	.facelist
	dc.l	.vertlist
	dc.l	0 			; .matlist -- pointer to a Material
						; This will be replaced with the parameter to Init3DSprite()
						; See eg. _jaguar_logo_material in storage.s in the demo
						;	project for how to set these up to reference imported
						;	images

	; Init3DSprite() will insert 4 bytes padding here

.facelist:
;* Face 0
	dc.w	$0,$0,$4000,$0	; face normal
	dc.w	4		; number of points
	dc.w	0		; index of material in .matlist
	dc.w	0, $ff00	; Point index, texture coordinates
	dc.w	2, $ffff	; Point index, texture coordinates
	dc.w	3, $00ff	; Point index, texture coordinates
	dc.w	1, $0000	; Point index, texture coordinates

	; Init3DSprite() will write the vertext list out directly without copying the
	; 	data here, which is left for reference

	.long
.vertlist:
;* Vertex 0
	dc.w	-16,-16,0	; coordinates
	dc.w	$0000,$0000,$4000	; vertex normal

;* Vertex 1
	dc.w	16,-16,0	; coordinates
	dc.w	$0000,$0000,$4000	; vertex normal

;* Vertex 2
	dc.w	-16,16,0	; coordinates
	dc.w	$0000,$0000,$4000	; vertex normal

;* Vertex 3
	dc.w	16,16,0	; coordinates
	dc.w	$0000,$0000,$4000	; vertex normal
