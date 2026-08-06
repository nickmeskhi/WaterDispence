#ifndef CLOCK_H
#define CLOCK_H

#include "draw_func.h"


void draw_glyph(char c, int x, int y, uint16_t col);
void draw_roman_string(const char *s, int cx, int cy, uint16_t col);
void draw_hour_numerals(void);
void circular_clock_marks(void);
void second_hand(int second, uint16_t colour);
void minute_hand(int minute, uint16_t colour);
void hour_hand(int hour, uint16_t colour);





#endif 
