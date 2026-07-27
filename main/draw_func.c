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
// New circular design for clock marks, with 60 ticks and 12 longer ticks for hours

void draw_glyph(char c, int x, int y, uint16_t col) {
    switch (c) {
        case 'I':
            draw_line(x + 2, y, x + 2, y + GLYPH_H - 1, col);
            break;
        case 'V':
            draw_line(x,     y + GLYPH_H -1 , x + 2, y , col);
            draw_line(x + 4, y  + GLYPH_H - 1, x + 2, y, col);
            break;
        case 'X':
            draw_line(x, y,              x + 4, y + GLYPH_H - 1, col);
            draw_line(x, y + GLYPH_H - 1, x + 4, y,              col);
            break;
    }
}



void draw_roman_string(const char *s, int cx, int cy, uint16_t col) {
    int len   = (int)strlen(s);
    int width = len * GLYPH_W + (len - 1) * GLYPH_GAP;
    int x = cx - width / 2;
    int y = cy - GLYPH_H / 2;
    for (int i = 0; s[i]; i++) {
        draw_glyph(s[i], x, y, col);
        x += GLYPH_W + GLYPH_GAP;
    }
}

/* ---- the 12 hour labels, upright, sitting inside the tick ring ---- */
void draw_hour_numerals(void) {
/*    static const char *ROMAN[12] = {
        "I", "II", "III", "IV", "V",  "VI",
        "VII", "VIII", "IX", "X", "XI", "XII"
    };
*/
    static const char *ROMAN[12] = {
        "XI", "X", "IX", "VIII", "VII",  "VI",
        "V", "IV", "III", "II", "I", "XII"
    };
    const int r_num = CR - 28;   // pull in from the ticks - tune to your CR

    for (int h = 1; h <= 12; h++) {
        float rad = (h * 30 + 90) * M_PI / 180.0f;   // -90 puts XII at the top
        int nx = CX + r_num * cosf(rad);
        int ny = CY + r_num * sinf(rad);
        draw_roman_string(ROMAN[h - 1], nx, ny, C_WHITE);
    }
}

void circular_clock_marks(void) {
    fill_screen(C_BLACK);
    draw_circle(CX, CY, CR - 2, C_WHITE);
    for (int deg = 0; deg < 360; deg += 6) {
        float rad   = deg * M_PI / 180.0f;
        int   r_out = CR - 3;
        int   r_in  = (deg % 30 == 0) ? CR - 16 : CR - 9;
        uint16_t col = (deg % 30 == 0) ? C_WHITE : C_GREY;
        int x0 = CX + r_out * cosf(rad), y0 = CY + r_out * sinf(rad);
        int x1 = CX + r_in  * cosf(rad), y1 = CY + r_in  * sinf(rad);
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
    fill_circle(CX, CY, 4, C_WHITE);
}