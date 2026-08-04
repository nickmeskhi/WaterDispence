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




//To call function active clock

void active_clock(int second, int minute, int hour,int state,int time)
{
    circular_clock_marks();
    draw_hour_numerals();
    while (state == 1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
        second++;
        time++;
        ESP_LOGI(TAG, "This is the time: %d", time);
        second_hand(second-1, C_BLACK); // Erase previous second hand
        minute_hand(minute, C_BLUE);
        hour_hand(hour, C_GREEN);
        circular_clock_marks();
        second_hand(second, C_RED);
        if (second >= 60) {
            second = 0;
            minute++;
            minute_hand(minute-1, C_BLACK);
            if (minute >= 60) {
                minute = 0;
                hour++;
                hour_hand(hour-1, C_BLACK);
                if (hour >= 12) {
                    hour = 0;
                }
            }
        }


    } 
    








    /*
    {                     
    for(;hour < 12; hour++) 
        {
        hour_hand(hour-1, C_BLACK); // Erase previous hour hand
        vTaskDelay(pdMS_TO_TICKS(100)); // Short delay to ensure the previous hand is erased before drawing the new one
        hour_hand(hour, C_GREEN);
        for(;minute < 60; minute++) 
        {
            minute_hand(minute-1, C_BLACK); // Erase previous minute hand
            vTaskDelay(pdMS_TO_TICKS(100)); // Short delay to ensure the previous hand is erased before drawing the new one
            minute_hand(minute, C_BLUE);
            for(;second < 60; second++) 
            {
                second_hand(second-1, C_BLACK); // Erase previous second hand
                minute_hand(minute, C_BLUE);
                hour_hand(hour, C_GREEN);
                circular_clock_marks();
                vTaskDelay(pdMS_TO_TICKS(10)); // Short delay to ensure the previous hand is erased before drawing the new one
                second_hand(second, C_RED);
                vTaskDelay(pdMS_TO_TICKS(990)); // Wait for 1 second before updating the second hand
                time ++;
                if (time >= 60) {
                    time = 0;
                    minute++;
                    if (minute >= 60) {
                        minute = 0;
                        hour++;
                        if (hour >= 12) {
                            hour = 0;
                        }
                    }
                }
                ESP_LOGI(TAG, "This is the time: %d", time);
            }}}}
        */
        
        
        }
    


void select_clock(int minute, int hour, int state,bool up,bool down,bool ok)
{
    circular_clock_marks();
    draw_hour_numerals();
    minute_hand(0,C_BLUE);
    hour_hand(0,C_GREEN);
    while(state==0)
    {
        if(up==1)
        {
            hour++;
            
            if (hour>12)
            {
                hour=0;
            }
            hour_hand(hour-1,C_BLACK);
            hour_hand(hour,C_GREEN);
            
        }
        if (down==1)
        {
            hour--;
            if(hour<=0)
            {
                hour=12;
            }
            hour_hand(hour-1,C_BLACK);
            hour_hand(hour,C_GREEN);
        }
        if (ok==1)
        {
            
            if(up==1)
            {
                minute++;
            if (minute>60)
            {
                minute=0;
            }
            minute_hand(minute-1,C_BLACK);
            minute_hand(minute,C_BLUE);
            }
            if (down==1)
            {
                minute--;
                if(minute<0)
                {
                    minute=60;

                }
                minute_hand(minute-1,C_BLACK);
                minute_hand(minute,C_BLUE);
            }
            if (ok==1)
            {
                state=0;

            }
            else
            {
                vTaskDelay(pdMS_TO_TICKS(10));
               // time=minute*60+hour*3600;
            }
            
            
        }
        else
        {
           vTaskDelay(pdMS_TO_TICKS(10));
        }
        


    }



} 