#include <math.h>
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




// Draw Clock

void draw_colon(uint16_t colour)
{
    fill_rect(117,101,122,110,colour);
    fill_rect(117,127,122,137,colour);
}


void draw_segment(int x, int y,int A, int B, int C, int D, int E, int F, int G,uint16_t colour)
{

    if(A!=0)
    {        draw_line(x,y+102,x+46,y+102,colour);
    }
    if(B!=0)
    {        draw_line(x+46,y+51,x+46,y+102,colour);
    }
    if(C!=0)
    {        draw_line(x+46,y,x+46,y+51,colour);
    }
    if(D!=0)
    {        draw_line(x,y,x+46,y,colour);
    }
    if(E!=0)
    {     
            draw_line(x,y,x,y+51,colour);
    }
    if(F!=0)
    {        draw_line(x,y+51,x,y+102,colour);
    }
    if(G!=0)
    {        draw_line(x,y+51,x+46,y+51,colour);
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