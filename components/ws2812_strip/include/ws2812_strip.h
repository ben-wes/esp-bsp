/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief WS2812 LED strip driver
 *
 * High-level driver for WS2812/NeoPixel LED strips over RMT.
 * Supports individual pixel control, fill, brightness scaling,
 * and HSV color input — similar in spirit to CircuitPython's NeoPixel library.
 */

#pragma once

#include "driver/gpio.h"
#include "esp_err.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque handle for a WS2812 strip instance
 */
typedef struct ws2812_strip_t *ws2812_strip_handle_t;

/**
 * @brief Configuration for a WS2812 strip
 */
typedef struct {
    gpio_num_t gpio_num;        /*!< GPIO connected to the strip data line */
    uint32_t   num_leds;        /*!< Total number of LEDs in the strip */
    bool       with_dma;        /*!< Use DMA for RMT transfer (recommended when using WiFi/BT) */
} ws2812_strip_config_t;

/**
 * @brief RGB color value
 */
typedef struct {
    uint8_t r; /*!< Red   [0-255] */
    uint8_t g; /*!< Green [0-255] */
    uint8_t b; /*!< Blue  [0-255] */
} ws2812_color_t;

/* Convenience color constants */
#define WS2812_COLOR_RED     ((ws2812_color_t){255,   0,   0})
#define WS2812_COLOR_GREEN   ((ws2812_color_t){  0, 255,   0})
#define WS2812_COLOR_BLUE    ((ws2812_color_t){  0,   0, 255})
#define WS2812_COLOR_WHITE   ((ws2812_color_t){255, 255, 255})
#define WS2812_COLOR_OFF     ((ws2812_color_t){  0,   0,   0})
#define WS2812_COLOR_YELLOW  ((ws2812_color_t){255, 255,   0})
#define WS2812_COLOR_CYAN    ((ws2812_color_t){  0, 255, 255})
#define WS2812_COLOR_MAGENTA ((ws2812_color_t){255,   0, 255})

/**
 * @brief Create a new WS2812 strip instance
 *
 * Allocates resources and initializes the RMT peripheral.
 * Call ws2812_strip_del() to free resources when done.
 *
 * @param[in]  config  Strip configuration (GPIO, LED count, DMA)
 * @param[out] handle  Returned handle on success
 *
 * @return
 *      - ESP_OK                Success
 *      - ESP_ERR_INVALID_ARG   config or handle is NULL
 *      - ESP_ERR_NO_MEM        Not enough heap memory
 *      - Other                 RMT driver error
 */
esp_err_t ws2812_strip_new(const ws2812_strip_config_t *config, ws2812_strip_handle_t *handle);

/**
 * @brief Delete a WS2812 strip instance and free all resources
 *
 * @param handle Strip handle returned by ws2812_strip_new()
 *
 * @return
 *      - ESP_OK                Success
 *      - ESP_ERR_INVALID_ARG   handle is NULL
 */
esp_err_t ws2812_strip_del(ws2812_strip_handle_t handle);

/**
 * @brief Set a single pixel to an RGB color
 *
 * Changes are buffered until ws2812_strip_refresh() is called.
 *
 * @param handle Strip handle
 * @param index  LED index [0, num_leds - 1]
 * @param color  RGB color value
 *
 * @return
 *      - ESP_OK                Success
 *      - ESP_ERR_INVALID_ARG   handle is NULL or index out of range
 */
esp_err_t ws2812_strip_set_pixel(ws2812_strip_handle_t handle, uint32_t index, ws2812_color_t color);

/**
 * @brief Set a single pixel using raw RGB components
 *
 * Convenience wrapper around ws2812_strip_set_pixel().
 * Changes are buffered until ws2812_strip_refresh() is called.
 *
 * @param handle Strip handle
 * @param index  LED index [0, num_leds - 1]
 * @param r      Red   [0-255]
 * @param g      Green [0-255]
 * @param b      Blue  [0-255]
 *
 * @return
 *      - ESP_OK                Success
 *      - ESP_ERR_INVALID_ARG   handle is NULL or index out of range
 */
esp_err_t ws2812_strip_set_pixel_rgb(ws2812_strip_handle_t handle, uint32_t index,
                                     uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief Set a single pixel using HSV color space
 *
 * Useful for smooth hue animations. Changes are buffered until
 * ws2812_strip_refresh() is called.
 *
 * @param handle Strip handle
 * @param index  LED index [0, num_leds - 1]
 * @param h      Hue        [0-360] degrees
 * @param s      Saturation [0-255]
 * @param v      Value/brightness [0-255]
 *
 * @return
 *      - ESP_OK                Success
 *      - ESP_ERR_INVALID_ARG   handle is NULL or index out of range
 */
esp_err_t ws2812_strip_set_pixel_hsv(ws2812_strip_handle_t handle, uint32_t index,
                                     uint16_t h, uint8_t s, uint8_t v);

/**
 * @brief Fill the entire strip with one color
 *
 * Changes are buffered until ws2812_strip_refresh() is called.
 *
 * @param handle Strip handle
 * @param color  RGB color to apply to all LEDs
 *
 * @return
 *      - ESP_OK                Success
 *      - ESP_ERR_INVALID_ARG   handle is NULL
 */
esp_err_t ws2812_strip_fill(ws2812_strip_handle_t handle, ws2812_color_t color);

/**
 * @brief Fill a range of pixels with one color
 *
 * Changes are buffered until ws2812_strip_refresh() is called.
 *
 * @param handle     Strip handle
 * @param start      First LED index (inclusive)
 * @param end        Last LED index (inclusive)
 * @param color      RGB color to apply
 *
 * @return
 *      - ESP_OK                Success
 *      - ESP_ERR_INVALID_ARG   handle is NULL or range invalid
 */
esp_err_t ws2812_strip_fill_range(ws2812_strip_handle_t handle, uint32_t start,
                                  uint32_t end, ws2812_color_t color);

/**
 * @brief Set global brightness for the strip
 *
 * Brightness is applied as a scaling factor when ws2812_strip_refresh()
 * sends data to the hardware. The internal color buffer is not modified.
 *
 * @param handle     Strip handle
 * @param brightness Brightness [0-255]; 255 = full, 0 = off
 *
 * @return
 *      - ESP_OK                Success
 *      - ESP_ERR_INVALID_ARG   handle is NULL
 */
esp_err_t ws2812_strip_set_brightness(ws2812_strip_handle_t handle, uint8_t brightness);

/**
 * @brief Push the pixel buffer to the hardware
 *
 * Must be called after any set/fill operation to make changes visible.
 * Applies the brightness scaling before transmission.
 *
 * @param handle Strip handle
 *
 * @return
 *      - ESP_OK                Success
 *      - ESP_ERR_INVALID_ARG   handle is NULL
 *      - Other                 RMT transmission error
 */
esp_err_t ws2812_strip_refresh(ws2812_strip_handle_t handle);

/**
 * @brief Turn off all LEDs immediately
 *
 * Clears the internal buffer and refreshes the hardware.
 *
 * @param handle Strip handle
 *
 * @return
 *      - ESP_OK                Success
 *      - ESP_ERR_INVALID_ARG   handle is NULL
 */
esp_err_t ws2812_strip_clear(ws2812_strip_handle_t handle);

/**
 * @brief Get the number of LEDs in the strip
 *
 * @param handle   Strip handle
 * @param num_leds Output: number of LEDs
 *
 * @return
 *      - ESP_OK                Success
 *      - ESP_ERR_INVALID_ARG   handle or num_leds is NULL
 */
esp_err_t ws2812_strip_get_num_leds(ws2812_strip_handle_t handle, uint32_t *num_leds);

#ifdef __cplusplus
}
#endif
