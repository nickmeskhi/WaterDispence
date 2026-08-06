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

static const char *TAG = "clock";

#define OK  32
#define RIGHT  33
#define LEFT  25
#define UP  26
#define DOWN  27

static int last_up = 1;
static int last_down = 1;
static int last_ok = 1;

static bool button_pressed(int gpio_num, int *last_state) {
    int current_state = gpio_get_level(gpio_num);
    if (current_state == 0 && *last_state == 1) {
        *last_state = 0;
        return true;
    } else if (current_state == 1 && *last_state == 0) {
        *last_state = 1;
    }
    return false;
}



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
    static const char *ROMAN[12] = {
        "I", "II", "III", "IV", "V",  "VI",
        "VII", "VIII", "IX", "X", "XI", "XII"
    };


    const int r_num = CR - 28;   // pull in from the ticks - tune to your CR

    for (int h = 1; h <= 12; h++) {
        float rad = (h * 30 - 90) * M_PI / 180.0f;   // -90 puts XII at the top
        int nx = CX + r_num * cosf(rad);
        int ny = CY + r_num * sinf(-rad);
        draw_roman_string(ROMAN[h - 1], nx, ny, C_WHITE);
    }
}

void circular_clock_marks(void) {

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


//dynamic clock


void second_hand (int second , uint16_t colour)
{
    int second_angle = (second % 60) * 6; // Each second represents 6 degrees
    float rad = (second_angle - 90) * M_PI / 180.0f; // Adjust for 12 o'clock position
    int second_length = CR; // Length of the second hand
    int x1 = CX + second_length * cosf(rad);
    int y1 = CY + second_length * sinf(-rad);
    draw_line(CX, CY, x1, y1, colour);
    draw_hour_numerals();
    fill_circle(CX, CY, 4, C_WHITE); 
}


void minute_hand (int minute , uint16_t colour)
{
    int minute_angle = (minute % 60) * 6; // Each minute represents 6 degrees
    float rad = (minute_angle - 90) * M_PI / 180.0f; // Adjust for 12 o'clock position
    int minute_length = CR - 40; // Length of the minute hand
    int x1 = CX + minute_length * cosf(rad);
    int y1 = CY + minute_length * sinf(-rad);
    draw_line(CX, CY, x1, y1, colour);
    draw_hour_numerals();
    fill_circle(CX, CY, 4, C_WHITE); 
}

void hour_hand (int hour , uint16_t colour)
{
    int hour_angle = ((hour % 12) * 30); // Each hour represents 30 degrees, and each minute adds 0.5 degrees
    float rad = (hour_angle - 90) * M_PI / 180.0f; // Adjust for 12 o'clock position
    int hour_length = CR - 80; // Length of the hour hand
    int x1 = CX + hour_length * cosf(rad);
    int y1 = CY + hour_length * sinf(-rad);
    draw_line(CX, CY, x1, y1, colour);
    draw_hour_numerals();
    fill_circle(CX, CY, 4, C_WHITE); 
}




