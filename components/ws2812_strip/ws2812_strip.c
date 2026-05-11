/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ws2812_strip.h"
#include "esp_check.h"
#include "esp_log.h"
#include "led_strip.h"
#include <stdlib.h>
#include <string.h>

static const char *TAG = "ws2812_strip";

/**
 * @brief Internal strip device structure
 */
typedef struct ws2812_strip_t {
  led_strip_handle_t led_strip; /*!< Underlying led_strip handle          */
  uint32_t num_leds;            /*!< Number of LEDs                        */
  uint8_t brightness;           /*!< Global brightness scaler [0-255]     */
  ws2812_color_t *buf;          /*!< Color buffer (one entry per LED)      */
} ws2812_strip_t;

/* --------------------------------------------------------------------------
 * Internal helpers
 * -------------------------------------------------------------------------- */

static inline uint8_t scale_brightness(uint8_t val, uint8_t brightness) {
  return (uint8_t)(((uint16_t)val * brightness) / 255);
}

static void hsv_to_rgb(uint16_t h, uint8_t s, uint8_t v, uint8_t *r, uint8_t *g,
                       uint8_t *b) {
  /* h: 0-360, s: 0-255, v: 0-255 */
  if (s == 0) {
    *r = *g = *b = v;
    return;
  }
  uint16_t region = h / 60;
  uint16_t rem = (h - (region * 60)) * 255 / 60;
  uint8_t p = (uint8_t)(((uint16_t)v * (255 - s)) / 255);
  uint8_t q = (uint8_t)(((uint16_t)v * (255 - ((s * rem) / 255))) / 255);
  uint8_t t =
      (uint8_t)(((uint16_t)v * (255 - ((s * (255 - rem)) / 255))) / 255);

  switch (region) {
  case 0:
    *r = v;
    *g = t;
    *b = p;
    break;
  case 1:
    *r = q;
    *g = v;
    *b = p;
    break;
  case 2:
    *r = p;
    *g = v;
    *b = t;
    break;
  case 3:
    *r = p;
    *g = q;
    *b = v;
    break;
  case 4:
    *r = t;
    *g = p;
    *b = v;
    break;
  default:
    *r = v;
    *g = p;
    *b = q;
    break;
  }
}

/* --------------------------------------------------------------------------
 * Public API
 * -------------------------------------------------------------------------- */

esp_err_t ws2812_strip_new(const ws2812_strip_config_t *config,
                           ws2812_strip_handle_t *handle) {
  ESP_RETURN_ON_FALSE(config != NULL && handle != NULL, ESP_ERR_INVALID_ARG,
                      TAG, "config and handle must not be NULL");
  ESP_RETURN_ON_FALSE(config->num_leds > 0, ESP_ERR_INVALID_ARG, TAG,
                      "num_leds must be > 0");

  esp_err_t ret = ESP_OK;
  ws2812_strip_t *strip = calloc(1, sizeof(ws2812_strip_t));
  ESP_RETURN_ON_FALSE(strip != NULL, ESP_ERR_NO_MEM, TAG,
                      "no memory for strip handle");

  strip->buf = calloc(config->num_leds, sizeof(ws2812_color_t));
  if (strip->buf == NULL) {
    free(strip);
    ESP_RETURN_ON_FALSE(false, ESP_ERR_NO_MEM, TAG,
                        "no memory for color buffer");
  }

  strip->num_leds = config->num_leds;
  strip->brightness = 255; /* full brightness by default */

  led_strip_config_t led_cfg = {
      .strip_gpio_num = config->gpio_num,
      .max_leds = config->num_leds,
      .led_model = LED_MODEL_WS2812,
      .flags.invert_out = false,
  };
  led_strip_rmt_config_t rmt_cfg = {
      .clk_src = RMT_CLK_SRC_DEFAULT,
      .resolution_hz = 10 * 1000 * 1000, /* 10 MHz */
      .flags.with_dma = config->with_dma,
  };

  ret = led_strip_new_rmt_device(&led_cfg, &rmt_cfg, &strip->led_strip);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to create led_strip RMT device: %s",
             esp_err_to_name(ret));
    free(strip->buf);
    free(strip);
    return ret;
  }

  *handle = strip;
  ESP_LOGI(TAG, "Created strip: %lu LEDs on GPIO %d", config->num_leds,
           config->gpio_num);
  return ESP_OK;
}

esp_err_t ws2812_strip_del(ws2812_strip_handle_t handle) {
  ESP_RETURN_ON_FALSE(handle != NULL, ESP_ERR_INVALID_ARG, TAG,
                      "handle must not be NULL");

  ws2812_strip_t *strip = (ws2812_strip_t *)handle;
  led_strip_clear(strip->led_strip);
  led_strip_del(strip->led_strip);
  free(strip->buf);
  free(strip);
  return ESP_OK;
}

esp_err_t ws2812_strip_set_pixel(ws2812_strip_handle_t handle, uint32_t index,
                                 ws2812_color_t color) {
  ESP_RETURN_ON_FALSE(handle != NULL, ESP_ERR_INVALID_ARG, TAG,
                      "handle must not be NULL");
  ws2812_strip_t *strip = (ws2812_strip_t *)handle;
  ESP_RETURN_ON_FALSE(index < strip->num_leds, ESP_ERR_INVALID_ARG, TAG,
                      "index %lu out of range [0, %lu]", index,
                      strip->num_leds - 1);

  strip->buf[index] = color;
  return ESP_OK;
}

esp_err_t ws2812_strip_set_pixel_rgb(ws2812_strip_handle_t handle,
                                     uint32_t index, uint8_t r, uint8_t g,
                                     uint8_t b) {
  return ws2812_strip_set_pixel(handle, index, (ws2812_color_t){r, g, b});
}

esp_err_t ws2812_strip_set_pixel_hsv(ws2812_strip_handle_t handle,
                                     uint32_t index, uint16_t h, uint8_t s,
                                     uint8_t v) {
  ESP_RETURN_ON_FALSE(handle != NULL, ESP_ERR_INVALID_ARG, TAG,
                      "handle must not be NULL");
  ws2812_strip_t *strip = (ws2812_strip_t *)handle;
  ESP_RETURN_ON_FALSE(index < strip->num_leds, ESP_ERR_INVALID_ARG, TAG,
                      "index %lu out of range [0, %lu]", index,
                      strip->num_leds - 1);
  ESP_RETURN_ON_FALSE(h <= 360, ESP_ERR_INVALID_ARG, TAG,
                      "hue must be in [0, 360]");

  uint8_t r, g, b;
  hsv_to_rgb(h, s, v, &r, &g, &b);
  strip->buf[index] = (ws2812_color_t){r, g, b};
  return ESP_OK;
}

esp_err_t ws2812_strip_fill(ws2812_strip_handle_t handle,
                            ws2812_color_t color) {
  ESP_RETURN_ON_FALSE(handle != NULL, ESP_ERR_INVALID_ARG, TAG,
                      "handle must not be NULL");
  ws2812_strip_t *strip = (ws2812_strip_t *)handle;

  for (uint32_t i = 0; i < strip->num_leds; i++) {
    strip->buf[i] = color;
  }
  return ESP_OK;
}

esp_err_t ws2812_strip_fill_range(ws2812_strip_handle_t handle, uint32_t start,
                                  uint32_t end, ws2812_color_t color) {
  ESP_RETURN_ON_FALSE(handle != NULL, ESP_ERR_INVALID_ARG, TAG,
                      "handle must not be NULL");
  ws2812_strip_t *strip = (ws2812_strip_t *)handle;
  ESP_RETURN_ON_FALSE(start <= end && end < strip->num_leds,
                      ESP_ERR_INVALID_ARG, TAG,
                      "invalid range [%lu, %lu] for strip of %lu LEDs", start,
                      end, strip->num_leds);

  for (uint32_t i = start; i <= end; i++) {
    strip->buf[i] = color;
  }
  return ESP_OK;
}

esp_err_t ws2812_strip_set_brightness(ws2812_strip_handle_t handle,
                                      uint8_t brightness) {
  ESP_RETURN_ON_FALSE(handle != NULL, ESP_ERR_INVALID_ARG, TAG,
                      "handle must not be NULL");
  ((ws2812_strip_t *)handle)->brightness = brightness;
  return ESP_OK;
}

esp_err_t ws2812_strip_refresh(ws2812_strip_handle_t handle) {
  ESP_RETURN_ON_FALSE(handle != NULL, ESP_ERR_INVALID_ARG, TAG,
                      "handle must not be NULL");
  ws2812_strip_t *strip = (ws2812_strip_t *)handle;

  for (uint32_t i = 0; i < strip->num_leds; i++) {
    uint8_t r = scale_brightness(strip->buf[i].r, strip->brightness);
    uint8_t g = scale_brightness(strip->buf[i].g, strip->brightness);
    uint8_t b = scale_brightness(strip->buf[i].b, strip->brightness);
    ESP_RETURN_ON_ERROR(led_strip_set_pixel(strip->led_strip, i, r, g, b), TAG,
                        "led_strip_set_pixel failed at index %lu", i);
  }
  return led_strip_refresh(strip->led_strip);
}

esp_err_t ws2812_strip_clear(ws2812_strip_handle_t handle) {
  ESP_RETURN_ON_FALSE(handle != NULL, ESP_ERR_INVALID_ARG, TAG,
                      "handle must not be NULL");
  ws2812_strip_t *strip = (ws2812_strip_t *)handle;

  memset(strip->buf, 0, strip->num_leds * sizeof(ws2812_color_t));
  return led_strip_clear(strip->led_strip);
}

esp_err_t ws2812_strip_get_num_leds(ws2812_strip_handle_t handle,
                                    uint32_t *num_leds) {
  ESP_RETURN_ON_FALSE(handle != NULL && num_leds != NULL, ESP_ERR_INVALID_ARG,
                      TAG, "handle and num_leds must not be NULL");
  *num_leds = ((ws2812_strip_t *)handle)->num_leds;
  return ESP_OK;
}
