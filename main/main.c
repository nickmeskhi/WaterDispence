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

#include "draw_func.h"  // draw helpers are implemented in draw_func.c
#include "test_patterns.h" // Test patterns implemented in test_patterns.c



static const int SEGMENT_1_X = 15;
//static const int SEGMENT_2_X = 69;
//static const int SEGMENT_3_X = 123;
//static const int SEGMENT_4_X = 177;
static const int SEGMENT_Y = 69;

// Button configuration
#define OK  32
#define RIGHT  33
#define LEFT  25
#define UP  26
#define DOWN  27

static bool button_pressed_once(int pin, int *last_state)
{
    int level = gpio_get_level(pin);
    bool pressed = (level == 0);
    bool just_pressed = pressed && (*last_state != 0);
    *last_state = level;
    return just_pressed;
}

void setup() {
    gpio_set_direction(OK, GPIO_MODE_INPUT);
    gpio_set_pull_mode(OK, GPIO_PULLUP_ONLY);
    gpio_set_direction(RIGHT, GPIO_MODE_INPUT);
    gpio_set_pull_mode(RIGHT, GPIO_PULLUP_ONLY);
    gpio_set_direction(LEFT, GPIO_MODE_INPUT);
    gpio_set_pull_mode(LEFT, GPIO_PULLUP_ONLY);
    gpio_set_direction(UP, GPIO_MODE_INPUT);
    gpio_set_pull_mode(UP, GPIO_PULLUP_ONLY);
    gpio_set_direction(DOWN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(DOWN, GPIO_PULLUP_ONLY);
}




// ── Pin config ── adjust to your wiring ──────────────────
#define PIN_MOSI    23
#define PIN_SCLK    18
#define PIN_CS      15
#define PIN_DC       2
#define PIN_RST      4
// No backlight pin — BL wire goes to 3.3V directly

#define SPI_HOST    SPI2_HOST
#define SPI_CLK_HZ  (40 * 1000 * 1000)




// ── LCD init ──────────────────────────────────────────────

static void lcd_init(void) {
    ESP_LOGI("Screen", "Init SPI bus");
    spi_bus_config_t bus_cfg = {
        .mosi_io_num     = PIN_MOSI,
        .miso_io_num     = -1,
        .sclk_io_num     = PIN_SCLK,
        .quadwp_io_num   = -1,
        .quadhd_io_num   = -1,
        .max_transfer_sz = LCD_W * 4,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI_HOST, &bus_cfg, SPI_DMA_CH_AUTO));

    ESP_LOGI("Screen", "Init LCD IO");
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

    ESP_LOGI("Screen", "Init GC9A01 panel");
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





void app_main(void) {
    lcd_init();
    fill_screen(C_BLACK);
    vTaskDelay(pdMS_TO_TICKS(300));

 
 
 //  draw_rect(15,69,61,171,C_BLUE);
   draw_rect(69,69,115,171,C_GREEN);
    draw_rect(123,69,169,171,C_RED);
    draw_rect(177,69,223,171,C_YELLOW);


    for (int i = 0; i < 10; i++)
    {
        select_digit_1(i, SEGMENT_1_X, SEGMENT_Y, C_WHITE);
    }
   
    


   // fill_rect(117,101,122,137,C_BLUE);
    
    vTaskDelay(pdMS_TO_TICKS(2000));
    


   // fill_rect(117,101,122,110,C_BLUE);
  //  fill_rect(117,127,122,137,C_BLUE);

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
  











*/