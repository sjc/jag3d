#include "screen.h"

//
// build packed versions of the object lists based on current video mode
//
void setup_object_lists() {
    if (VIDpal) {
        OLbldto(buf1_olist_pal, packed_olist1, OLIST_LEN);
        OLbldto(buf2_olist_pal, packed_olist2, OLIST_LEN);
        OLbldto(buf3_olist_pal, packed_olist3, OLIST_LEN);
        drawbuffer_x = SCREENX_PAL;
        drawbuffer_y = SCREENY_PAL;
    } else {
        OLbldto(buf1_olist_ntsc, packed_olist1, OLIST_LEN);
        OLbldto(buf2_olist_ntsc, packed_olist2, OLIST_LEN);
        OLbldto(buf3_olist_ntsc, packed_olist3, OLIST_LEN);
        drawbuffer_x = SCREENX_NTSC;
        drawbuffer_y = SCREENY_NTSC;
    }
}

void clear_hud_buffer() {
    short *ptr = HUDBUFFER;
    short count = (HUDWIDTH/4)*HUDHEIGHT;
    while (count--) *ptr++ = 0;
}

void set_drawbuffers_offset(short ox, short oy) {
    position_drawbuffers(drawbuffer_x + ox, drawbuffer_y + oy);
}
