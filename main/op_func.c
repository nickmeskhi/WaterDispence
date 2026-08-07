#include <math.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
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

    while (*state == 3) {
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
                *state = (v == 0) ? 4 : 2;
                break;
            }
            vTaskDelay(pdMS_TO_TICKS(10));
        }

        if (*state != 3) {
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

void active_clock(int second, int minute, int hour, int *state, int time) {
    fill_screen(C_BLACK);
    circular_clock_marks();
    draw_hour_numerals();
    while (*state == 2) {
        for (int tick = 0; tick < 100; tick++) {
            if (button_pressed(OK, &last_ok)) {
               // ESP_LOGI(TAG, "OK button pressed");
                *state = 3;
                return;
            }
            vTaskDelay(pdMS_TO_TICKS(10));
        }
        second++;
        time++;
       // ESP_LOGI(TAG, "This is the time: %d", time);
        second_hand(second - 1, C_BLACK); // Erase previous second hand
        minute_hand(minute, C_BLUE);
        hour_hand(hour, C_GREEN);
        circular_clock_marks();
        second_hand(second, C_RED);
        if (second >= 60) {
            second = 0;
            minute++;
            minute_hand(minute - 1, C_BLACK);
            if (minute >= 60) {
                minute = 0;
                hour++;
                hour_hand(hour - 1, C_BLACK);
                if (hour >= 12) {
                    hour = 0;
                }
            }
        }
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
            hour_hand(*hour, C_GREEN);
            minute_hand(*minute + 1, C_BLACK);
            minute_hand(*minute, C_BLUE);
            (*minute)--;
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

    *time = *minute * 60 + *hour * 3600;
    if (*time < 0) {
        *time = -*time;
    }
}
