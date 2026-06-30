#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_log.h"
#include "test_patterns.h"

static const char *TAG = "gc9a01_test";

void test_solid_colours(void) {
    ESP_LOGI(TAG, "Test 1: solid colours");
    uint16_t cols[] = { C_RED, C_GREEN, C_BLUE, C_WHITE,
                        C_YELLOW, C_CYAN, C_MAGENTA };
    for (int i = 0; i < 7; i++) {
        fill_screen(cols[i]);
        vTaskDelay(pdMS_TO_TICKS(600));
    }
    fill_screen(C_BLACK);
}

void test_concentric_rings(void) {
    ESP_LOGI(TAG, "Test 2: concentric rings");
    fill_screen(C_BLACK);
    uint16_t cols[] = { C_RED, C_ORANGE, C_YELLOW, C_GREEN,
                        C_CYAN, C_BLUE, C_MAGENTA, C_WHITE };
    int n = 8;
    for (int i = 0; i < n; i++) {
        int r = CR - i * (CR / n);
        fill_circle(CX, CY, r, cols[i]);
    }
    vTaskDelay(pdMS_TO_TICKS(2000));
    fill_screen(C_BLACK);
}

void test_colour_wedges(void) {
    ESP_LOGI(TAG, "Test 3: colour wedges");
    fill_screen(C_BLACK);
    uint16_t cols[] = { C_RED, C_YELLOW, C_GREEN, C_CYAN, C_BLUE, C_MAGENTA };
    int n = 6;
    for (int y = 0; y < LCD_H; y++) {
        for (int x = 0; x < LCD_W; x++) {
            int dx = x - CX, dy = y - CY;
            float dist = sqrtf(dx * dx + dy * dy);
            if (dist > CR) { line_buf[x] = C_BLACK; continue; }
            float angle = atan2f(dy, dx);
            if (angle < 0) angle += 2.0f * M_PI;
            int seg = (int)(angle / (2.0f * M_PI) * n) % n;
            line_buf[x] = cols[seg];
        }
        esp_lcd_panel_draw_bitmap(panel, 0, y, LCD_W, y + 1, line_buf);
    }
    vTaskDelay(pdMS_TO_TICKS(2000));
    fill_screen(C_BLACK);
}

void test_grid(void) {
    ESP_LOGI(TAG, "Test 4: grid");
    fill_screen(C_BLACK);
    for (int y = 0; y < LCD_H; y += 20)
        draw_hline_buf(0, LCD_W - 1, y, C_GREY);
    for (int x = 0; x < LCD_W; x += 20) {
        for (int y = 0; y < LCD_H; y++)
            draw_pixel(x, y, C_GREY);
    }
    draw_hline_buf(0, LCD_W - 1, CY, C_WHITE);
    for (int y = 0; y < LCD_H; y++) draw_pixel(CX, y, C_WHITE);
    draw_circle(CX, CY, CR - 1, C_WHITE);
    vTaskDelay(pdMS_TO_TICKS(2500));
    fill_screen(C_BLACK);
}

void test_clock_marks(void) {
    ESP_LOGI(TAG, "Test 5: clock marks");
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
    vTaskDelay(pdMS_TO_TICKS(2500));
    fill_screen(C_BLACK);
}

void test_checkerboard(void) {
    ESP_LOGI(TAG, "Test 6: checkerboard");
    int sz = 10;
    for (int y = 0; y < LCD_H; y++) {
        for (int x = 0; x < LCD_W; x++) {
            int dx = x - CX, dy = y - CY;
            if (dx * dx + dy * dy > CR * CR)
                line_buf[x] = C_BLACK;
            else
                line_buf[x] = ((x / sz + y / sz) % 2) ? C_WHITE : C_BLACK;
        }
        esp_lcd_panel_draw_bitmap(panel, 0, y, LCD_W, y + 1, line_buf);
    }
    vTaskDelay(pdMS_TO_TICKS(2500));
    fill_screen(C_BLACK);
}

void test_greyscale_ramp(void) {
    ESP_LOGI(TAG, "Test 7: greyscale ramp");
    for (int y = 0; y < LCD_H; y++) {
        for (int x = 0; x < LCD_W; x++) {
            uint8_t v = (uint8_t)(x * 255 / (LCD_W - 1));
            line_buf[x] = rgb565(v, v, v);
        }
        esp_lcd_panel_draw_bitmap(panel, 0, y, LCD_W, y + 1, line_buf);
    }
    draw_circle(CX, CY, CR - 1, C_RED);
    vTaskDelay(pdMS_TO_TICKS(2500));
    fill_screen(C_BLACK);
}

void run_all_tests(void) {
    test_solid_colours();
    test_concentric_rings();
    test_colour_wedges();
    test_grid();
    test_clock_marks();
    test_checkerboard();
    test_greyscale_ramp();
}
