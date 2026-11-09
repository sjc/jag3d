//
// Simple text writing functions
//

//
// Use these constants to change text colour mid-string. They map 
//  to indexes 1 to 15 of whatever colour palette is selected in 
//  the BITMAP
// 
#define cPEN1    "\xFF"
#define cPEN2    "\xFE"
#define cPEN3    "\xFD"
#define cPEN4    "\xFC"
#define cPEN5    "\xFB"
#define cPEN6    "\xFA"
#define cPEN7    "\xF9"
#define cPEN8    "\xF8"
#define cPEN9    "\xF7"
#define cPEN10   "\xF6"
#define cPEN11   "\xF5"
#define cPEN12   "\xF4"
#define cPEN13   "\xF3"
#define cPEN14   "\xF2"
#define cPEN15   "\xF1"

//
// Use these constants as the pen param to the functions below.
//  They map to indexes 1 to 15 of whatever colour palette is 
//  selected in the BITMAP
// 
#define PEN1    (-1)
#define PEN2    (-2)
#define PEN3    (-3)
#define PEN4    (-4)
#define PEN5    (-5)
#define PEN6    (-6)
#define PEN7    (-7)
#define PEN8    (-8)
#define PEN9    (-9)
#define PEN10   (-10)
#define PEN11   (-11)
#define PEN12   (-12)
#define PEN13   (-13)
#define PEN14   (-14)
#define PEN15   (-15)

//
// Draw 8x8 characters
// stride is the width of the buffer being drawn into
//
extern void draw_string_to_buffer(char *string, short pen, void *buffer, short stride, void *font);

//
// Draw 8x16 characters
// stride is the width of the buffer being drawn into
//
extern void draw_string_to_buffer_16(char *string, short pen, void *buffer, short stride, void *font);
