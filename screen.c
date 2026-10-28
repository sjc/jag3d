#include "screen.h"

//
// build packed versions of the object lists
//
void setup_object_lists() {
    OLbldto(buf1_olist, packed_olist1, OLIST_LEN);
    OLbldto(buf2_olist, packed_olist2, OLIST_LEN);
}
