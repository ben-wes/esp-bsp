
idf_component_register(
    SRCS "ws2812_strip.c"
    INCLUDE_DIRS "include"
    REQUIRES log led_strip esp_driver_rmt esp_driver_gpio
)
