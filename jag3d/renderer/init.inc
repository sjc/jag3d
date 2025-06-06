;
; init.inc
; One-time setup code
;

    .gpu

;
; make sure we're in bank 1
;
    ;.REGBANK0
    movei   #G_FLAGS,atemp0
    load    (atemp0),atemp1
    bset    #14,atemp1
    store   atemp1,(atemp0)
    nop
    nop
    ;.REGBANK1

    movefa  atemp0,gflags       ; store this in bank 1 for later

;
; we want 16.16 divides
;
    movei   #G_DIVCTRL,temp0
    moveq   #1,temp1
    store   temp1,(temp0)

;
; set up for 4xn matrices
;
    subqt   #(G_DIVCTRL-G_MTXC),temp0       ; move to G_MTXC
    moveq   #4,temp1
    store   temp1,(temp0)

;
; set up constant registers for the trapezoid renderer
;
   movei   #A1_PIXEL,temp1
   movei   #A2_PIXEL,temp2
   moveta  temp1,a1_ptr
   moveta  temp2,a2_ptr
   movei   #B_Z3,temp1
   movei   #B_CMD,temp2
   moveta  temp1,bz3_ptr
   moveta  temp2,bcmd_ptr




;
; all done, kill the GPU
;

    movei   #gpudone,r0
    jump    (r0)
    nop
