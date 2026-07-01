#ifndef DRAW_FUNC_H
#define DRAW_FUNC_H

#include <stdint.h>
#include "esp_lcd_types.h"

// Display geometry
#define LCD_W       240
#define LCD_H       240
#define CX          120
#define CY          120
#define CR          120

// Colour helper
static inline uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
	uint16_t c = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
	return (c >> 8) | (c << 8);
}

#define C_BLACK    rgb565(0,   0,   0  )
#define C_WHITE    rgb565(255, 255, 255)
#define C_RED      rgb565(255, 0,   0  )
#define C_GREEN    rgb565(0,   255, 0  )
#define C_BLUE     rgb565(0,   0,   255)
#define C_YELLOW   rgb565(255, 255, 0  )
#define C_CYAN     rgb565(0,   255, 255)
#define C_MAGENTA  rgb565(255, 0,   255)
#define C_ORANGE   rgb565(255, 165, 0  )
#define C_GREY     rgb565(80,  80,  80 )

// Shared buffer and panel handle
extern uint16_t line_buf[LCD_W];
extern esp_lcd_panel_handle_t panel;

// Draw helper prototypes
void fill_screen(uint16_t colour);
void draw_pixel(int x, int y, uint16_t colour);
void draw_hline_buf(int x0, int x1, int y, uint16_t col);
void draw_line(int x0, int y0, int x1, int y1, uint16_t col);
void fill_circle(int cx, int cy, int r, uint16_t col);
void draw_circle(int cx, int cy, int r, uint16_t col);
void draw_rect(int x0, int y0, int x1, int y1, uint16_t col);
void fill_rect(int x0, int y0, int x1, int y1, uint16_t col);
void draw_colon(uint16_t colour);
void draw_segment(int x, int y,int A, int B, int C, int D, int E, int F, int G,uint16_t colour);
void draw_digit(int digit, int x, int y, uint16_t colour);

#endif // DRAW_FUNC_H
