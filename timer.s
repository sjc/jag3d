;
; Timer stuff
;

    .include 'jaguar.inc'

_setup_timer::
   move.l  #26590<<16|$ffff,PIT0    ; 1/1000s-Timer
   rts

_reset_timer::
   move.w  #$ffff,PIT1
   rts

_read_timer::
   move.w  PIT1,d0
   not.w   d0
   rts
