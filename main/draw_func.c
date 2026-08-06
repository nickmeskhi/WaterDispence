#include <math.h>
#include <stdbool.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_gc9a01.h"
#include "esp_log.h"
#include "esp_lcd_types.h"
#include "draw_func.h"

// Frame buffer (single row)
uint16_t line_buf[LCD_W];

// Panel handle — defined here and referenced from main
esp_lcd_panel_handle_t panel = NULL;



// Fill whole screen with one colour using line-by-line writes
void fill_screen(uint16_t colour) {
	for (int x = 0; x < LCD_W; x++) line_buf[x] = colour;
	for (int y = 0; y < LCD_H; y++)
		esp_lcd_panel_draw_bitmap(panel, 0, y, LCD_W, y + 1, line_buf);
}

// Draw a single pixel (slow — use sparingly)
void draw_pixel(int x, int y, uint16_t colour) {
	if (x < 0 || x >= LCD_W || y < 0 || y >= LCD_H) return;
	esp_lcd_panel_draw_bitmap(panel, x, y, x + 1, y + 1, &colour);
}

// Horizontal line into line_buf, then flush a region
void draw_hline_buf(int x0, int x1, int y, uint16_t col) {
	if (y < 0 || y >= LCD_H) return;
	if (x0 > x1) { int t = x0; x0 = x1; x1 = t; }
	x0 = x0 < 0 ? 0 : x0;
	x1 = x1 >= LCD_W ? LCD_W - 1 : x1;
	for (int x = x0; x <= x1; x++) line_buf[x] = col;
	esp_lcd_panel_draw_bitmap(panel, x0, y, x1 + 1, y + 1, line_buf + x0);
}


void draw_line(int x0, int y0, int x1, int y1, uint16_t col) {
	// Bresenham line algorithm
	int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int err = dx + dy;
	while (1) {
		draw_pixel(x0, y0, col);
		if (x0 == x1 && y0 == y1) break;
		int e2 = 2 * err;
		if (e2 >= dy) { err += dy; x0 += sx; }
		if (e2 <= dx) { err += dx; y0 += sy; }
	}
}




// Filled circle via scanlines
void fill_circle(int cx, int cy, int r, uint16_t col) {
	for (int dy = -r; dy <= r; dy++) {
		int y = cy + dy;
		if (y < 0 || y >= LCD_H) continue;
		int dx = (int)sqrtf((float)(r * r - dy * dy));
		draw_hline_buf(cx - dx, cx + dx, y, col);
	}
}

// Draw circle outline (Bresenham)
void draw_circle(int cx, int cy, int r, uint16_t col) {
	int x = 0, y = r, d = 1 - r;
	while (x <= y) {
		draw_pixel(cx + x, cy + y, col); draw_pixel(cx - x, cy + y, col);
		draw_pixel(cx + x, cy - y, col); draw_pixel(cx - x, cy - y, col);
		draw_pixel(cx + y, cy + x, col); draw_pixel(cx - y, cy + x, col);
		draw_pixel(cx + y, cy - x, col); draw_pixel(cx - y, cy - x, col);
		if (d < 0) d += 2 * x + 3;
		else       { d += 2 * (x - y) + 5; y--; }
		x++;
	}
}

// Draw rectangle 
void draw_rect(int x0, int y0, int x1, int y1, uint16_t col) {
    if (x0 > x1) { int t = x0; x0 = x1; x1 = t; }
    if (y0 > y1) { int t = y0; y0 = y1; y1 = t; }
    draw_hline_buf(x0, x1, y0, col);
    draw_hline_buf(x0, x1, y1, col);
    for (int y = y0 + 1; y < y1; y++) {
        draw_pixel(x0, y, col);
        draw_pixel(x1, y, col);
    }
}

//fill rectangle
void fill_rect(int x0, int y0, int x1, int y1, uint16_t col) {
    if (x0 > x1) { int t = x0; x0 = x1; x1 = t; }
    if (y0 > y1) { int t = y0; y0 = y1; y1 = t; }
    for (int y = y0; y <= y1; y++) {
        draw_hline_buf(x0, x1, y, col);
    }
}






static const uint8_t font5x7[27][5] = {
    {0x7C,0x12,0x11,0x12,0x7C}, // A
    {0x7F,0x49,0x49,0x49,0x36}, // B
    {0x3E,0x41,0x41,0x41,0x22}, // C
    {0x7F,0x41,0x41,0x22,0x1C}, // D
    {0x7F,0x49,0x49,0x49,0x41}, // E
    {0x7F,0x09,0x09,0x09,0x01}, // F
    {0x3E,0x41,0x49,0x49,0x7A}, // G
    {0x7F,0x08,0x08,0x08,0x7F}, // H
    {0x00,0x41,0x7F,0x41,0x00}, // I
    {0x20,0x40,0x41,0x3F,0x01}, // J
    {0x7F,0x08,0x14,0x22,0x41}, // K
    {0x7F,0x40,0x40,0x40,0x40}, // L
    {0x7F,0x02,0x0C,0x02,0x7F}, // M
    {0x7F,0x04,0x08,0x10,0x7F}, // N
    {0x3E,0x41,0x41,0x41,0x3E}, // O
    {0x7F,0x09,0x09,0x09,0x06}, // P
    {0x3E,0x41,0x51,0x21,0x5E}, // Q
    {0x7F,0x09,0x19,0x29,0x46}, // R
    {0x46,0x49,0x49,0x49,0x31}, // S
    {0x01,0x01,0x7F,0x01,0x01}, // T
    {0x3F,0x40,0x40,0x40,0x3F}, // U
    {0x1F,0x20,0x40,0x20,0x1F}, // V
    {0x3F,0x40,0x38,0x40,0x3F}, // W
    {0x63,0x14,0x08,0x14,0x63}, // X
    {0x07,0x08,0x70,0x08,0x07}, // Y
    {0x61,0x51,0x49,0x45,0x43}, // Z
    {0x02,0x01,0x59,0x09,0x06}  // ?
};

static void draw_char(int x, int y, char c, uint16_t col) {
    if (c == ' ') return;
    if (c == '?') {
        const uint8_t *glyph = font5x7[26];
        for (int ix = 0; ix < 5; ix++) {
            uint8_t bits = glyph[ix];
            for (int iy = 0; iy < 7; iy++) {
                if (bits & (1 << iy)) draw_pixel(x + ix, y + (6 - iy), col);
            }
        }
        return;
    }
    if (c >= 'a' && c <= 'z') c -= 'a' - 'A';
    if (c < 'A' || c > 'Z') return;
    const uint8_t *glyph = font5x7[c - 'A'];
    for (int ix = 0; ix < 5; ix++) {
        uint8_t bits = glyph[ix];
        for (int iy = 0; iy < 7; iy++) {
            if (bits & (1 << iy)) draw_pixel(x + ix, y + (6 - iy), col);
        }
    }
}

static void draw_char_scaled(int x, int y, char c, uint16_t col, int scale) {
    if (c == ' ') return;
    if (c == '?') {
        const uint8_t *glyph = font5x7[26];
        for (int ix = 0; ix < 5; ix++) {
            uint8_t bits = glyph[ix];
            for (int iy = 0; iy < 7; iy++) {
                if (bits & (1 << iy)) {
                    for (int sx = 0; sx < scale; sx++) {
                        for (int sy = 0; sy < scale; sy++) {
                            draw_pixel(x + ix * scale + sx, y + (6 - iy) * scale + sy, col);
                        }
                    }
                }
            }
        }
        return;
    }
    if (c >= 'a' && c <= 'z') c -= 'a' - 'A';
    if (c < 'A' || c > 'Z') return;
    const uint8_t *glyph = font5x7[c - 'A'];
    for (int ix = 0; ix < 5; ix++) {
        uint8_t bits = glyph[ix];
        for (int iy = 0; iy < 7; iy++) {
            if (bits & (1 << iy)) {
                for (int sx = 0; sx < scale; sx++) {
                    for (int sy = 0; sy < scale; sy++) {
                        draw_pixel(x + ix * scale + sx, y + (6 - iy) * scale + sy, col);
                    }
                }
            }
        }
    }
}

void draw_text_size(const char *s, int x, int y, uint16_t col, int size) {
    if (size < 1) size = 1;
    int cx = x;
    for (const char *p = s; *p; p++) {
        if (*p == ' ') {
            cx += 4 * size;
            continue;
        }
        draw_char_scaled(cx, y, *p, col, size);
        cx += 6 * size;
    }
}

void draw_text(const char *s, int x, int y, uint16_t col) {
    draw_text_size(s, x, y, col, 2);
}


// Draw Clock (Old version, replaced by circular_clock_marks() and draw_hour_numerals())
/*
void draw_colon(uint16_t colour)
{
    fill_rect(117,101,122,110,colour);
    fill_rect(117,127,122,137,colour);
}

void blink_colon(uint16_t colour)
{
    draw_colon(colour);
    vTaskDelay(pdMS_TO_TICKS(500));
    draw_colon(C_BLACK);
    vTaskDelay(pdMS_TO_TICKS(500));
}


void draw_segment(int x, int y,int A, int B, int C, int D, int E, int F, int G,uint16_t colour)
{

    if(A!=0)
    {   //     draw_line(x,y+102,x+46,y+102,colour);
        fill_rect(x+2,y+98,x+42,y+100,colour);
        
    }
    if(B!=0)
    {    //    draw_line(x+46,y+51,x+46,y+102,colour);
        fill_rect(x+40,y+100,x+42,y+50,colour);
    }
    if(C!=0)
    {    //    draw_line(x+46,y,x+46,y+51,colour);
        fill_rect(x+40,y+2,x+42,y+50,colour);
    }
    if(D!=0)
    {   //     draw_line(x,y,x+46,y,colour);
        fill_rect(x+2,y+2,x+42,y+4,colour);
    }
    if(E!=0)
    {     
         //   draw_line(x,y,x,y+51,colour);
            fill_rect(x+2,y+2,x+4,y+50,colour);
    }
    if(F!=0)
    {     //   draw_line(x,y+51,x,y+102,colour);
        fill_rect(x+2,y+50,x+4,y+100,colour);
    }
    if(G!=0)
    {    //    draw_line(x,y+51,x+46,y+51,colour);
        fill_rect(x+2,y+50,x+42,y+52,colour);
    }
}

void draw_digit(int digit, int x, int y,uint16_t colour)
{
    switch(digit)
    {
        case 0:
            draw_segment(x,y,1,1,1,1,1,1,0,colour);
            break;
        case 1:
            draw_segment(x,y,0,1,1,0,0,0,0,colour);
            break;
        case 2:
            draw_segment(x,y,1,1,0,1,1,0,1,colour);
            break;
        case 3:
            draw_segment(x,y,1,1,1,1,0,0,1,colour);
            break;
        case 4:
            draw_segment(x,y,0,1,1,0,0,1,1,colour);
            break;
        case 5:
            draw_segment(x,y,1 ,0 ,1 ,1 ,0 ,1 ,1 ,colour);
            break;
        case 6:
            draw_segment(x,y ,1 ,0 ,1 ,1 ,1 ,1 ,1 ,colour);
            break;
        case 7:
            draw_segment(x,y ,1 ,1 ,1 ,0 ,0 ,0 ,0 ,colour);
            break;
        case 8:
            draw_segment(x,y ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,colour);
            break;
        case 9:
            draw_segment(x,y ,1 ,1 ,1 ,0 ,0 ,1 ,1 ,colour);
            break;
    }
}


void select_digit_1(int digit,int x,int y,uint16_t colour)
{
    
    
        
            fill_rect(15,69,61,171,C_BLACK);
            draw_digit(digit,x,y,colour);
            vTaskDelay(pdMS_TO_TICKS(500));
            fill_rect(15,69,61,171,colour);
            draw_digit(digit,x,y,C_BLACK);
            vTaskDelay(pdMS_TO_TICKS(500));

        

   
}
*/