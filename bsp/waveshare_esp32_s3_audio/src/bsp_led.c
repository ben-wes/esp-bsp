/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 * SPDX-License-Identifier: Apache-2.0
 */

#include "bsp/waveshare_esp32_s3_audio.h"
#include "led_strip.h"

static led_strip_handle_t s_strip = NULL;

esp_err_t bsp_led_strip_init(void) {
  led_strip_config_t strip_config = {
      .strip_gpio_num = BSP_LED_STRIP_IO,
      .max_leds = BSP_LED_STRIP_NUM,
      //.led_pixel_format = LED_PIXEL_FORMAT_GRB,
      .led_model = LED_MODEL_WS2812,
      .flags.invert_out = false,
  };
  led_strip_rmt_config_t rmt_config = {
      .clk_src = RMT_CLK_SRC_DEFAULT,
      .resolution_hz = 10 * 1000 * 1000, // 10MHz
      .flags.with_dma = false,
  };
  return led_strip_new_rmt_device(&strip_config, &rmt_config, &s_strip);
}

led_strip_handle_t bsp_led_strip_get_handle(void) { return s_strip; }