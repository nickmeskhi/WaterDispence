#include <math.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "draw_func.h"
#include "clock.h"
#include "op_func.h"

static const char *TAG = "op_func";

#define OK  32
#define RIGHT 33
#define LEFT  25
#define UP  26
#define DOWN 27

static int last_up = 1;
static int last_down = 1;
static int last_ok = 1;
static int last_left = 1;
static int last_right = 1;

static const int SEGMENT_1_X = 15;
static const int SEGMENT_2_X = 69;
static const int SEGMENT_3_X = 123;
static const int SEGMENT_4_X = 177;
static const int SEGMENT_Y = 69;

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

void prompt_func(int *state) {
    fill_screen(C_BLACK);
    draw_text("Do you want to", 30, 120, C_WHITE);
    draw_text("set the function?", 40, 100, C_WHITE);

    int v = 0;
    bool blink = false;

    while (*state == 4) {
        for (int tick = 0; tick < 50; tick++) {
            if (button_pressed(LEFT, &last_left)) {
                v = 1 - v;
                ESP_LOGI(TAG, "LEFT button pressed");
            }
            if (button_pressed(RIGHT, &last_right)) {
                v = 1 - v;
                ESP_LOGI(TAG, "RIGHT button pressed");
            }
            if (button_pressed(OK, &last_ok)) {
                ESP_LOGI(TAG, "OK button pressed");
                *state = (v == 0) ? 5 : 3;
                break;
            }
            vTaskDelay(pdMS_TO_TICKS(10));
        }

        if (*state != 4) {
            break;
        }

        blink = !blink;
        if (v == 0) {
            if (blink) {
                fill_rect(70, 60, 100, 80, C_WHITE);
                draw_text("YES", 70, 60, C_BLACK);
            } else {
                fill_rect(70, 60, 100, 80, C_BLACK);
                draw_text("YES", 70, 60, C_WHITE);
            }
            fill_rect(135, 60, 165, 80, C_BLACK);
            draw_text("NO", 135, 60, C_WHITE);
        } else {
            fill_rect(70, 60, 100, 80, C_BLACK);
            draw_text("YES", 70, 60, C_WHITE);
            if (blink) {
                fill_rect(135, 60, 165, 80, C_WHITE);
                draw_text("NO", 135, 60, C_BLACK);
            } else {
                fill_rect(135, 60, 165, 80, C_BLACK);
                draw_text("NO", 135, 60, C_WHITE);
            }
        }
    }
}

void set_function(int *state, int *set_time, int *set_duration, int *set_day_delay) {
    fill_screen(C_BLACK);
    int set_hour_1 = 0;
    int set_hour_2 = 0;
    int set_minute_1 = 0;
    int set_minute_2 = 0;
    draw_colon(C_WHITE);
    draw_text("Start time", 30, 50, C_WHITE);

    draw_digit(0, SEGMENT_1_X, SEGMENT_Y, C_BLUE);
    while (*state == 5) {
        bool up = button_pressed(UP, &last_up);
        bool down = button_pressed(DOWN, &last_down);
        bool ok = button_pressed(OK, &last_ok);

        if (up) {
            ESP_LOGI(TAG, "UP button pressed");
            set_hour_1++;
            if (set_hour_1 > 2) {
                set_hour_1 = 0;
            }
            draw_digit(8, SEGMENT_1_X, SEGMENT_Y, C_BLACK);
            draw_digit(set_hour_1, SEGMENT_1_X, SEGMENT_Y, C_YELLOW);
        }
        if (down) {
            ESP_LOGI(TAG, "DOWN button pressed");
            set_hour_1--;
            if (set_hour_1 < 0) {
                set_hour_1 = 2;
            }
            draw_digit(8, SEGMENT_1_X, SEGMENT_Y, C_BLACK);
            draw_digit(set_hour_1, SEGMENT_1_X, SEGMENT_Y, C_BLUE);
        }
        if (ok) {
            ESP_LOGI(TAG, "OK button pressed");
            *state = 6;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    draw_digit(0, SEGMENT_2_X, SEGMENT_Y, C_YELLOW);
    while (*state == 6) {
        bool up = button_pressed(UP, &last_up);
        bool down = button_pressed(DOWN, &last_down);
        bool ok = button_pressed(OK, &last_ok);

        if (up) {
            ESP_LOGI(TAG, "UP button pressed");
            set_hour_2++;
            if (set_hour_2 > 4 && set_hour_1 == 2) {
                set_hour_2 = 0;
            }
            draw_digit(8, SEGMENT_2_X, SEGMENT_Y, C_BLACK);
            draw_digit(set_hour_2, SEGMENT_2_X, SEGMENT_Y, C_YELLOW);
        }
        if (down) {
            ESP_LOGI(TAG, "DOWN button pressed");
            set_hour_2--;
            if (set_hour_2 < 0) {
                set_hour_2 = 4;
            }
            draw_digit(8, SEGMENT_2_X, SEGMENT_Y, C_BLACK);
            draw_digit(set_hour_2, SEGMENT_2_X, SEGMENT_Y, C_BLUE);
        }
        if (ok) {
            ESP_LOGI(TAG, "OK button pressed");
            *state = 7;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    draw_digit(0, SEGMENT_3_X, SEGMENT_Y, C_RED);
    while (*state == 7) {
        bool up = button_pressed(UP, &last_up);
        bool down = button_pressed(DOWN, &last_down);
        bool ok = button_pressed(OK, &last_ok);

        if (up) {
            ESP_LOGI(TAG, "UP button pressed");
            set_minute_1++;
            if (set_minute_1 > 5) {
                set_minute_1 = 0;
            }
            draw_digit(8, SEGMENT_3_X, SEGMENT_Y, C_BLACK);
            draw_digit(set_minute_1, SEGMENT_3_X, SEGMENT_Y, C_YELLOW);
        }
        if (down) {
            ESP_LOGI(TAG, "DOWN button pressed");
            set_minute_1--;
            if (set_minute_1 < 0) {
                set_minute_1 = 5;
            }
            draw_digit(8, SEGMENT_3_X, SEGMENT_Y, C_BLACK);
            draw_digit(set_minute_1, SEGMENT_3_X, SEGMENT_Y, C_BLUE);
        }
        if (ok) {
            ESP_LOGI(TAG, "OK button pressed");
            *state = 8;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    draw_digit(0, SEGMENT_4_X, SEGMENT_Y, C_GREEN);
    while (*state == 8) {
        bool up = button_pressed(UP, &last_up);
        bool down = button_pressed(DOWN, &last_down);
        bool ok = button_pressed(OK, &last_ok);

        if (up) {
            ESP_LOGI(TAG, "UP button pressed");
            set_minute_2++;
            if (set_minute_2 > 9) {
                set_minute_2 = 0;
            }
            draw_digit(8, SEGMENT_4_X, SEGMENT_Y, C_BLACK);
            draw_digit(set_minute_2, SEGMENT_4_X, SEGMENT_Y, C_YELLOW);
        }
        if (down) {
            ESP_LOGI(TAG, "DOWN button pressed");
            set_minute_2--;
            if (set_minute_2 < 0) {
                set_minute_2 = 9;
            }
            draw_digit(8, SEGMENT_4_X, SEGMENT_Y, C_BLACK);
            draw_digit(set_minute_2, SEGMENT_4_X, SEGMENT_Y, C_BLUE);
        }
        if (ok) {
            ESP_LOGI(TAG, "OK button pressed");
            *state = 9;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    *set_time = (set_hour_1 * 10 + set_hour_2) * 3600 + (set_minute_1 * 10 + set_minute_2) * 60;

    fill_screen(C_BLACK);
    draw_text("Day set", 30, 50, C_WHITE);

    draw_digit(0, SEGMENT_2_X, SEGMENT_Y, C_YELLOW);
    draw_digit(1, SEGMENT_3_X, SEGMENT_Y, C_YELLOW);

    int day_count_1 = 0;
    int day_count_2 = 1;

    while (*state == 9) {
        bool up = button_pressed(UP, &last_up);
        bool down = button_pressed(DOWN, &last_down);
        bool ok = button_pressed(OK, &last_ok);

        if (up) {
            ESP_LOGI(TAG, "UP button pressed");
            day_count_2++;
            if (day_count_2 > 9) {
                day_count_2 = 0;
                day_count_1++;
                if (day_count_1 > 9) {
                    day_count_1 = 0;

                }
            }
            
            draw_digit(8, SEGMENT_2_X, SEGMENT_Y, C_BLACK);
            draw_digit(8, SEGMENT_3_X, SEGMENT_Y, C_BLACK);
            draw_digit(day_count_1, SEGMENT_2_X, SEGMENT_Y, C_YELLOW);
            draw_digit(day_count_2, SEGMENT_3_X, SEGMENT_Y, C_YELLOW);
        }
        if (down) {
            ESP_LOGI(TAG, "DOWN button pressed");
            day_count_2--;
            if (day_count_2 < 0) {
                day_count_2 = 9;
                day_count_1--;
                if (day_count_1 < 0) {
                    day_count_1 = 9;
                }
            }
            if(day_count_1 == 0 && day_count_2 == 0) {
                day_count_1 = 0;
                day_count_2 = 1;
            }
            draw_digit(8, SEGMENT_2_X, SEGMENT_Y, C_BLACK);
            draw_digit(8, SEGMENT_3_X, SEGMENT_Y, C_BLACK);
            draw_digit(day_count_1, SEGMENT_2_X, SEGMENT_Y, C_YELLOW);
            draw_digit(day_count_2, SEGMENT_3_X, SEGMENT_Y, C_YELLOW);
        }
        if (ok) {
            ESP_LOGI(TAG, "OK button pressed");
            *state = 10;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    *set_day_delay = (day_count_1 * 10 + day_count_2);

    fill_screen(C_BLACK);
    draw_text("Duration IN MIN", 30, 50, C_WHITE);

    draw_digit(0, SEGMENT_1_X, SEGMENT_Y, C_YELLOW);
    draw_digit(1, SEGMENT_2_X, SEGMENT_Y, C_YELLOW);
    draw_digit(0, SEGMENT_3_X, SEGMENT_Y, C_YELLOW);
    draw_digit(0, SEGMENT_4_X, SEGMENT_Y, C_YELLOW);
    draw_colon(C_WHITE);

    int duration_minute_1 = 0;
    int duration_minute_2 = 1;
    while (*state == 10) {
        bool up = button_pressed(UP, &last_up);
        bool down = button_pressed(DOWN, &last_down);
        bool ok = button_pressed(OK, &last_ok);

        if (up) {
            ESP_LOGI(TAG, "UP button pressed");
            duration_minute_2++;
            if (duration_minute_2 > 9) {
                duration_minute_2 = 0;
                duration_minute_1++;
                if (duration_minute_1 > 5) {
                    duration_minute_1 = 0;
                }
            }
            draw_digit(8, SEGMENT_1_X, SEGMENT_Y, C_BLACK);
            draw_digit(8, SEGMENT_2_X, SEGMENT_Y, C_BLACK);
            draw_digit(duration_minute_1, SEGMENT_1_X, SEGMENT_Y, C_YELLOW);
            draw_digit(duration_minute_2, SEGMENT_2_X, SEGMENT_Y, C_YELLOW);
        }
        if (down) {
            ESP_LOGI(TAG, "DOWN button pressed");
            duration_minute_2--;
            if (duration_minute_2 < 0) {
                duration_minute_2 = 9;
                duration_minute_1--;
                if (duration_minute_1 < 0) {
                    duration_minute_1 = 5;
                }
            }
            draw_digit(8, SEGMENT_1_X, SEGMENT_Y, C_BLACK);
            draw_digit(8, SEGMENT_2_X, SEGMENT_Y, C_BLACK);
            draw_digit(duration_minute_1, SEGMENT_1_X, SEGMENT_Y, C_YELLOW);
            draw_digit(duration_minute_2, SEGMENT_2_X, SEGMENT_Y, C_YELLOW);
        }
        if (ok) {
            ESP_LOGI(TAG, "OK button pressed");
            *state = 11;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    *set_duration = (duration_minute_1 * 10 + duration_minute_2) * 60;
    ESP_LOGI(TAG, "Set duration: %d seconds", *set_duration);
}

void active_clock(int *second, int *minute, int *hour, int *state, int *time) {
    fill_screen(C_BLACK);
    circular_clock_marks(C_WHITE);
    draw_hour_numerals();
    hour_hand(*hour, C_GREEN);
    minute_hand(*minute, C_BLUE);
    second_hand(*second, C_RED);

    // Free-running 1 Hz time base. Counting vTaskDelay() iterations drifts,
    // because the redraw below costs far more than the delay itself.
    int64_t next_tick_us = esp_timer_get_time() + 1000000;

    while (*state == 3) {
        if (button_pressed(OK, &last_ok)) {
            // ESP_LOGI(TAG, "OK button pressed");
            *state = 4;
            return;
        }

        // Advance by however many whole seconds actually elapsed, so a slow
        // redraw makes the hands jump rather than making the clock run late.
        int elapsed = 0;
        while (esp_timer_get_time() >= next_tick_us) {
            next_tick_us += 1000000;
            elapsed++;
        }

        if (elapsed > 0) {
            int prev_second = *second;
            int prev_minute = *minute;
            int prev_hour = *hour;

            for (int i = 0; i < elapsed; i++) {
                (*time)++;
                (*second)++;
                if (*second >= 60) {
                    *second = 0;
                    (*minute)++;
                    if (*minute >= 60) {
                        *minute = 0;
                        (*hour)++;
                        if (*hour >= 12) {
                            *hour = 0;
                        }
                    }
                }
            }
            // ESP_LOGI(TAG, "This is the time: %d", *time);

            second_hand(prev_second, C_BLACK); // Erase previous second hand
            if (*minute != prev_minute) {
                minute_hand(prev_minute, C_BLACK);
            }
            if (*hour != prev_hour) {
                hour_hand(prev_hour, C_BLACK);
            }
            circular_clock_marks(C_WHITE);
            hour_hand(*hour, C_GREEN);
            minute_hand(*minute, C_BLUE);
            second_hand(*second, C_RED);
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void draw_arc_hour_marker(int hour_1,int hour_2, uint16_t colour) 
{
    
    
    for(int j=0; j <=10; j++) 
    {
        draw_arc(hour_1 * 30-90, hour_2 * 30-90, CR - 6-j, colour);
            
    }
}

void arc_trigger (int *time, int *set_time, int *set_day_delay, int *elapsed_day)
{
    int remaining_days = *set_day_delay - *elapsed_day;

    /* Compute total remaining seconds until the next trigger, taking
       remaining_days into account. Ensure the value is > 0 so the arc
       always represents a forward interval. */
    long total_remaining = (long)(*set_time) - (long)(*time) + (long)remaining_days * 86400L;
    while (total_remaining <= 0) total_remaining += 86400L;

    /* Start angle based on current time (fractional hours). End angle
       is start + span corresponding to total_remaining hours. Angles
       are in degrees; one hour == 30 degrees on the 12-hour face. */
    double start_hours = ((double)(*time)) / 3600.0;
    double start_angle = start_hours * 30.0 - 90.0; /* -90 to make 12 o'clock at top */
    double span_hours = ((double)total_remaining) / 3600.0;
    double end_angle = start_angle + span_hours * 30.0;

    /* Clear existing ring area first (draw full ring black). */
    for (int j = 0; j <= 10; j++) {
        draw_arc(-90, -90 + 360, CR - 6 - j, C_BLACK);
    }

    if (remaining_days == 0) {
        /* If the remaining span is 12 hours or more, draw a full circle
           (so 1:00 -> 13:00 appears as a full ring). Otherwise draw the
           arc from start to end. */
        if (span_hours >= 12.0 - 1e-6) {
            for (int j = 0; j <= 10; j++) {
                draw_arc((int)round(start_angle), (int)round(start_angle + 360.0), CR - 6 - j, C_YELLOW);
            }
            ESP_LOGI(TAG, "Full ring: remaining_hours=%.2f", span_hours);
        } else {
            for (int j = 0; j <= 10; j++) {
                draw_arc((int)round(start_angle), (int)round(end_angle), CR - 6 - j, C_YELLOW);
            }
            ESP_LOGI(TAG, "Start angle: %.1f, End angle: %.1f, remaining_hours=%.2f", start_angle, end_angle, span_hours);
        }
    } else if (remaining_days > 0) {
        /* If trigger is several days away, indicate with filled ring. */
        for (int j = 0; j <= 10; j++) {
            draw_circle(120, 120, CR - 6 - j, C_YELLOW);
        }
        ESP_LOGI(TAG, "Trigger in %d days (visual filled ring)", remaining_days);
    }
}






void func_clock(int *second, int *minute, int *hour, int *state, int *time, int *set_time, int *set_duration, int *set_day_delay)
{
    int elapsed_day = 1;
    fill_screen(C_BLACK);
    circular_clock_marks(C_WHITE);
    arc_trigger(time, set_time, set_day_delay, &elapsed_day);
    draw_hour_numerals();
    hour_hand(*hour, C_GREEN);
    minute_hand(*minute, C_BLUE);
    second_hand(*second, C_RED);

    // Free-running 1 Hz time base. Counting vTaskDelay() iterations drifts,
    // because the redraw below costs far more than the delay itself.
    int64_t next_tick_us = esp_timer_get_time() + 1000000;

    while (*state == 11) {
        if (button_pressed(OK, &last_ok)) {
            // ESP_LOGI(TAG, "OK button pressed");
            *state = 12;
            return;
        }

        // Advance by however many whole seconds actually elapsed, so a slow
        // redraw makes the hands jump rather than making the clock run late.
        int elapsed = 0;
        while (esp_timer_get_time() >= next_tick_us) {
            next_tick_us += 1000000;
            elapsed++;
        }

        if (elapsed > 0) {
            int prev_second = *second;
            int prev_minute = *minute;
            int prev_hour = *hour;

            for (int i = 0; i < elapsed; i++) {
                (*time)++;
                (*second)++;
                if (*second >= 60) {
                    *second = 0;
                    (*minute)++;
                    if (*minute >= 60) {
                        *minute = 0;
                        (*hour)++;
                        arc_trigger(time, set_time, set_day_delay, &elapsed_day);
                        if (*hour >= 12) {
                            *hour = 0;
                        }
                    }
                }
                if (*time >= 86400) {
                    *time -= 86400;
                    elapsed_day++;
                }
            }
            // ESP_LOGI(TAG, "This is the time: %d", *time);

            second_hand(prev_second, C_BLACK); // Erase previous second hand
            if (*minute != prev_minute) {
                minute_hand(prev_minute, C_BLACK);
            }
            if (*hour != prev_hour) {
                hour_hand(prev_hour, C_BLACK);
            }
            circular_clock_marks(C_WHITE);
            
            hour_hand(*hour, C_GREEN);
            minute_hand(*minute, C_BLUE);
            second_hand(*second, C_RED);

        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }

    
}




void select_clock(int *minute, int *hour, int *state, int *time) {
    circular_clock_marks(C_WHITE);

    // Work with a 12-hour display value for selection. Convert the incoming
    // `*hour` (which may be 0..23 or 0..11) into 1..12 for the UI.
    int disp_hour = (*hour) % 12;
    if (disp_hour == 0) disp_hour = 12;

    // Hour selection state
    while (*state == 0) {
        hour_hand(disp_hour % 12, C_GREEN);
        bool up = button_pressed(UP, &last_up);
        bool down = button_pressed(DOWN, &last_down);
        bool ok = button_pressed(OK, &last_ok);

        if (up) {
            ESP_LOGI(TAG, "UP button pressed");
            disp_hour++;
            if (disp_hour > 12) disp_hour = 1;
            hour_hand((disp_hour - 1) % 12, C_BLACK);
            hour_hand(disp_hour % 12, C_GREEN);
            ESP_LOGI(TAG, "Selected hour: %d", disp_hour);
        }
        if (down) {
            ESP_LOGI(TAG, "DOWN button pressed");
            disp_hour--;
            if (disp_hour < 1) disp_hour = 12;
            hour_hand((disp_hour + 1) % 12, C_BLACK);
            hour_hand(disp_hour % 12, C_GREEN);
            ESP_LOGI(TAG, "Selected hour: %d", disp_hour);
        }
        if (ok) {
            ESP_LOGI(TAG, "OK button pressed");
            *state = 1;
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    // Minute selection
    while (*state == 1) {
        minute_hand(*minute, C_BLUE);
        bool up = button_pressed(UP, &last_up);
        bool down = button_pressed(DOWN, &last_down);
        bool ok = button_pressed(OK, &last_ok);

        if (up) {
            ESP_LOGI(TAG, "UP button pressed");
            (*minute)++;
            if (*minute > 59) *minute = 0;
            minute_hand((*minute - 1 + 60) % 60, C_BLACK);
            minute_hand(*minute, C_BLUE);
        }
        if (down) {
            ESP_LOGI(TAG, "DOWN button pressed");
            (*minute)--;
            if (*minute < 0) *minute = 59;
            minute_hand((*minute + 1) % 60, C_BLACK);
            minute_hand(*minute, C_BLUE);
        }
        if (ok) {
            ESP_LOGI(TAG, "OK button pressed");
            *state = 2;
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    // AM/PM selection: 0 = AM, 1 = PM. Start with PM shown for backward
    // compatibility with previous UI (which displayed PM initially).
    int ampm = 1; // default PM
    draw_text_size(ampm ? "PM" : "AM", 50, 120, C_WHITE, 2);
    while (*state == 2) {
        bool up = button_pressed(UP, &last_up);
        bool down = button_pressed(DOWN, &last_down);
        bool ok = button_pressed(OK, &last_ok);
        if (up || down) {
            ampm = 1 - ampm;
            fill_rect(50, 120, 80, 140, C_BLACK);
            draw_text_size(ampm ? "PM" : "AM", 50, 120, C_WHITE, 2);
        }
        if (ok) {
            ESP_LOGI(TAG, "OK button pressed");
            *state = 3;
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    // Convert selected 12-hour value + AM/PM into 24-hour `*hour`.
    int final_hour24;
    if (ampm == 0) { // AM
        final_hour24 = (disp_hour == 12) ? 0 : disp_hour;
    } else { // PM
        final_hour24 = (disp_hour == 12) ? 12 : (disp_hour + 12);
    }
    *hour = final_hour24;

    // Compose `*time` from hour/minute
    *time = (*minute) * 60 + (*hour) * 3600;
    ESP_LOGI(TAG, "This is the time: %d (hour=%d minute=%d ampm=%d)", *time, *hour, *minute, ampm);
}