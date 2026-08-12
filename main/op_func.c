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

void set_function(int *state, int *set_time, int *set_duration, int *set_day_count) {
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
            if (set_hour_2 > 4) {
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
    draw_text("Day count (1=today)", 10, 50, C_WHITE);
    draw_text("2=tomorrow", 50, 30, C_WHITE);

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

    *set_day_count = day_count_1 * 10 + day_count_2;
    if (*set_day_count < 1) {
        *set_day_count = 1;
    }
    ESP_LOGI(TAG, "Set day count: %d", *set_day_count);

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
    circular_clock_marks();
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
            circular_clock_marks();
            hour_hand(*hour, C_GREEN);
            minute_hand(*minute, C_BLUE);
            second_hand(*second, C_RED);
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

static void draw_hour_marker(int hour_pos, uint16_t colour) {
    int r_inner = CR - 12;
    int r_outer = CR - 6;
    float angle = hour_pos * 30.0f - 90.0f;
    float rad = angle * M_PI / 180.0f;
    int x0 = CX + r_inner * cosf(rad);
    int y0 = CY + r_inner * sinf(-rad);
    int x1 = CX + r_outer * cosf(rad);
    int y1 = CY + r_outer * sinf(-rad);
    draw_line(x0, y0, x1, y1, colour);
}

static int calculate_next_trigger(int current_seconds, int set_time_of_day, int day_count) {
    int current_day_seconds = current_seconds % 86400;
    if (current_day_seconds < 0) {
        current_day_seconds += 86400;
    }
    int target_seconds = set_time_of_day % 86400;
    if (target_seconds < 0) {
        target_seconds += 86400;
    }

    int day_offset = day_count - 1;
    if (day_offset < 0) {
        day_offset = 0;
    }

    int base_day = current_seconds - current_day_seconds;
    int trigger = base_day + day_offset * 86400 + target_seconds;
    if (current_day_seconds >= target_seconds) {
        trigger += 86400;
    }
    return trigger;
}

static void draw_remaining_hours(int current_seconds, int end_seconds) {
    int remaining_seconds = end_seconds - current_seconds;
    if (remaining_seconds <= 0) {
        return;
    }

    int current_hour = (current_seconds / 3600) % 12;
    if (current_hour < 0) {
        current_hour += 12;
    }

    int hours_left = (remaining_seconds + 3599) / 3600;
    if (hours_left > 24) {
        hours_left = 24;
    }

    draw_circle(CX, CY, CR - 10, C_YELLOW);
    for (int i = 1; i <= hours_left; i++) {
        int marker_hour = (current_hour + i) % 12;
        draw_hour_marker(marker_hour, C_GREEN);
    }
}

void active_function_clock(int *second, int *minute, int *hour, int *state, int *time, int *set_time, int *set_duration, int *set_day_count) {
    fill_screen(C_BLACK);
    circular_clock_marks();
    draw_hour_numerals();

    int next_trigger = calculate_next_trigger(*time, *set_time, *set_day_count);
    *second = (*time) % 60;
    *minute = ((*time) / 60) % 60;
    *hour = ((*time) / 3600) % 12;

    draw_remaining_hours(*time, next_trigger);
    hour_hand(*hour, C_GREEN);
    minute_hand(*minute, C_BLUE);
    second_hand(*second, C_RED);

    int64_t next_tick_us = esp_timer_get_time() + 1000000;
    while (*state == 11) {
        int elapsed = 0;
        while (esp_timer_get_time() >= next_tick_us) {
            next_tick_us += 1000000;
            elapsed++;
        }

        if (elapsed > 0) {
            int prev_second = *second;
            int prev_minute = *minute;
            int prev_hour = *hour;

            *time += elapsed;
            *second = *time % 60;
            *minute = (*time / 60) % 60;
            *hour = (*time / 3600) % 12;

            int next_trigger = calculate_next_trigger(*time, *set_time, *set_day_count);
            int remaining_seconds = next_trigger - *time;

            second_hand(prev_second, C_BLACK);
            if (*minute != prev_minute) {
                minute_hand(prev_minute, C_BLACK);
            }
            if (*hour != prev_hour) {
                hour_hand(prev_hour, C_BLACK);
            }
            circular_clock_marks();
            draw_hour_numerals();
            draw_remaining_hours(*time, next_trigger);
            hour_hand(*hour, C_GREEN);
            minute_hand(*minute, C_BLUE);
            second_hand(*second, C_RED);

            if (remaining_seconds <= 0) {
                *state = 12;
                return;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void select_clock(int *minute, int *hour, int *state, int *time) {
    circular_clock_marks();
    while (*state == 0) {
        hour_hand(*hour, C_GREEN);
        bool up = button_pressed(UP, &last_up);
        bool down = button_pressed(DOWN, &last_down);
        bool ok = button_pressed(OK, &last_ok);

        if (up) {
            ESP_LOGI(TAG, "UP button pressed");
            (*hour)++;
            hour_hand(*hour - 1, C_BLACK);
            hour_hand(*hour, C_GREEN);
            if (*hour > 12) {
                *hour = 0;
            }
        }
        if (down) {
            ESP_LOGI(TAG, "DOWN button pressed");
            (*hour)--;
            hour_hand(*hour + 1, C_BLACK);
            hour_hand(*hour, C_GREEN);
            if (*hour == 0) {
                *hour = 12;
            }
        }
        if (ok) {
            ESP_LOGI(TAG, "OK button pressed");
            *state = 1;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    while (*state == 1) {
        minute_hand(*minute, C_BLUE);
        bool up = button_pressed(UP, &last_up);
        bool down = button_pressed(DOWN, &last_down);
        bool ok = button_pressed(OK, &last_ok);

        if (up) {
            ESP_LOGI(TAG, "UP button pressed");
            (*minute)++;
            hour_hand(*hour, C_GREEN);
            minute_hand(*minute - 1, C_BLACK);
            minute_hand(*minute, C_BLUE);
            if (*minute > 59) {
                *minute = 0;
            }
        }
        if (down) {
            ESP_LOGI(TAG, "DOWN button pressed");
            (*minute)--;
            hour_hand(*hour, C_GREEN);
            minute_hand(*minute + 1, C_BLACK);
            minute_hand(*minute, C_BLUE);
            
            if (*minute == 0) {
                *minute = 59;
            }
        }
        if (ok) {
            ESP_LOGI(TAG, "OK button pressed");
            *state = 2;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    int ampm = 0;
    draw_text_size("PM", 50, 120, C_WHITE,2);
    while (*state == 2) 
    {
    bool up = button_pressed(UP, &last_up);
    bool down = button_pressed(DOWN, &last_down);
    bool ok = button_pressed(OK, &last_ok);
    
    if (up)
        {
            ampm ++;
            if(ampm >1)
            {
                ampm = 0;
                fill_rect(50, 120, 80, 140, C_BLACK);
                draw_text_size("PM", 50, 120, C_WHITE,2);
            }
            else
            {
                fill_rect(50, 120, 80, 140, C_BLACK);
                draw_text_size("AM", 50, 120, C_WHITE,2);
            }
        }
    if (down) 
    {
            ampm --;
            if(ampm <0)
            {
                ampm = 1;
                fill_rect(50, 120, 80, 140, C_BLACK);
                draw_text_size("AM", 50, 120, C_WHITE,2);
            }
            else
            {
                fill_rect(50, 120, 80, 140, C_BLACK);
                draw_text_size("PM", 50, 120, C_WHITE,2);
            }
        }
        if (ok) {
            ESP_LOGI(TAG, "OK button pressed");
            *state = 3;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    if(ampm == 0)
    {
        *hour += 12;
    }

    *time = *minute * 60 + *hour * 3600;
    if (*time < 0) {
        *time = -*time;
    }
    ESP_LOGI(TAG, "This is the time: %d", *time);
}