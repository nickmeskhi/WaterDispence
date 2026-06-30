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

static const char *TAG = "gc9a01_test";

// ── Pin config ── adjust to your wiring ──────────────────
#define PIN_MOSI    23
#define PIN_SCLK    18
#define PIN_CS      15
#define PIN_DC       2
#define PIN_RST      4
// No backlight pin — BL wire goes to 3.3V directly

#define SPI_HOST    SPI2_HOST
#define SPI_CLK_HZ  (40 * 1000 * 1000)

// draw helpers are implemented in draw_func.c


// Test patterns implemented in test_patterns.c
#include "test_patterns.h"

// ── LCD init ──────────────────────────────────────────────

static void lcd_init(void) {
    ESP_LOGI(TAG, "Init SPI bus");
    spi_bus_config_t bus_cfg = {
        .mosi_io_num     = PIN_MOSI,
        .miso_io_num     = -1,
        .sclk_io_num     = PIN_SCLK,
        .quadwp_io_num   = -1,
        .quadhd_io_num   = -1,
        .max_transfer_sz = LCD_W * 4,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI_HOST, &bus_cfg, SPI_DMA_CH_AUTO));

    ESP_LOGI(TAG, "Init LCD IO");
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_cfg = {
        .dc_gpio_num       = PIN_DC,
        .cs_gpio_num       = PIN_CS,
        .pclk_hz           = SPI_CLK_HZ,
        .lcd_cmd_bits      = 8,
        .lcd_param_bits    = 8,
        .spi_mode          = 0,
        .trans_queue_depth = 10,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(
        (esp_lcd_spi_bus_handle_t)SPI_HOST, &io_cfg, &io_handle));

    ESP_LOGI(TAG, "Init GC9A01 panel");
    esp_lcd_panel_dev_config_t panel_cfg = {
        .reset_gpio_num  = PIN_RST,
        .rgb_ele_order   = LCD_RGB_ELEMENT_ORDER_BGR,
        .bits_per_pixel  = 16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_gc9a01(io_handle, &panel_cfg, &panel));

    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel));
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel, true));  // GC9A01 needs inversion
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel, true));
}

// ── Entry point ───────────────────────────────────────────


// Draw Clock

static void draw_colon(uint16_t colour)
{
    fill_rect(117,101,122,110,colour);
    fill_rect(117,127,122,137,colour);
}




void app_main(void) {
    lcd_init();
    fill_screen(C_BLACK);
    vTaskDelay(pdMS_TO_TICKS(300));
   // draw_line(0, 119, 239, 119, C_RED);
 //   draw_line(119, 0, 119, 239, C_RED);
   // draw_rect(11, 69, 228, 171, C_BLUE);
   draw_rect(15,69,61,171,C_BLUE);
   draw_rect(69,69,115,171,C_GREEN);
    draw_rect(123,69,169,171,C_RED);
    draw_rect(177,69,223,171,C_YELLOW);



   // fill_rect(117,101,122,137,C_BLUE);
    
    vTaskDelay(pdMS_TO_TICKS(2500));
    


    fill_rect(117,101,122,110,C_BLUE);
    fill_rect(117,127,122,137,C_BLUE);

}




//All the tested functions
/*
    lcd_init();
    fill_screen(C_BLACK);
    vTaskDelay(pdMS_TO_TICKS(300));

    test_solid_colours();
 while (1){

  draw_line(0, 120, 240, 120, C_RED);
  draw_line(120, 0, 120, 240, C_GREEN);
  //draw_rect(10, 10, 230, 230, C_BLUE);
  //fill_rect(20, 20, 220, 220, C_YELLOW);
  vTaskDelay(pdMS_TO_TICKS(25));

 }

    
 
   test_colour_wedges();
  test_concentric_rings();
    test_grid();
    test_clock_marks();
    test_checkerboard();
    test_greyscale_ramp();

    // Done
    fill_screen(C_BLACK);
    draw_circle(CX, CY, CR - 1, C_GREEN);
    ESP_LOGI(TAG, "All tests complete"); 











*/