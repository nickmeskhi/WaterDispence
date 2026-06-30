idf_component_register(
    SRCS "test_patterns.c" "draw_func.c" "main.c"
    INCLUDE_DIRS "."
    REQUIRES esp_lcd spi_flash driver
)