/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 * SPDX-License-Identifier: Apache-2.0
 */

#include "bsp/waveshare_esp32_s3_audio.h"
#include "ws2812_strip.h"

static ws2812_strip_handle_t s_strip = NULL;

esp_err_t bsp_led_strip_init(void) {
  ws2812_strip_config_t cfg = {
      .gpio_num = BSP_LED_STRIP_IO,  // GPIO38
      .num_leds = BSP_LED_STRIP_NUM, // 7
      .with_dma = false,
  };
  return ws2812_strip_new(&cfg, &s_strip);
}

ws2812_strip_handle_t bsp_led_strip_get_handle(void) { return s_strip; }