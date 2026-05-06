/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "esp_err.h"
#include "esp_log.h"
#include "bsp_err_check.h"
#include "iot_button.h"
#include "button_gpio.h"

#include "bsp/waveshare_esp32_s3_audio.h"

static const char *TAG = "Waveshare-Audio";

/*
 * Waveshare ESP32-S3-AUDIO-Board has 3 user buttons (active low GPIO):
 *   K1 -> mapped to BSP_BUTTON_VOLUP
 *   K2 -> mapped to BSP_BUTTON_PLAY
 *   K3 -> mapped to BSP_BUTTON_VOLDOWN
 *
 * These are directly connected to TCA9555 extended IO pins.
 * EXIO0 = K1, EXIO1 = K2, EXIO2 = K3
 * Virtual GPIO numbers = GPIO_NUM_MAX + pin_offset
 */
#define BSP_BUTTON_K1_IO  (GPIO_NUM_MAX + 9)  /* TCA9555 EXIO9 */
#define BSP_BUTTON_K2_IO  (GPIO_NUM_MAX + 10) /* TCA9555 EXIO10 */
#define BSP_BUTTON_K3_IO  (GPIO_NUM_MAX + 11) /* TCA9555 EXIO11 */

static const button_gpio_config_t bsp_button_config[BSP_BUTTON_NUM] = {
    [BSP_BUTTON_VOLUP] = {
        .gpio_num = BSP_BUTTON_K1_IO,
        .active_level = 0,
    },
    [BSP_BUTTON_PLAY] = {
        .gpio_num = BSP_BUTTON_K2_IO,
        .active_level = 0,
    },
    [BSP_BUTTON_VOLDOWN] = {
        .gpio_num = BSP_BUTTON_K3_IO,
        .active_level = 0,
    },
    [BSP_BUTTON_REC] = {
        .gpio_num = -1,
        .active_level = 0,
    },
    [BSP_BUTTON_SET] = {
        .gpio_num = -1,
        .active_level = 0,
    },
};

esp_err_t bsp_iot_button_create(button_handle_t btn_array[], int *btn_cnt, int btn_array_size)
{
    /* Initialize IO expander first (buttons are on TCA9555) */
    bsp_io_expander_init();

    esp_err_t ret = ESP_OK;
    const button_config_t btn_config = {0};
    if ((btn_array_size < BSP_BUTTON_NUM) ||
            (btn_array == NULL)) {
        return ESP_ERR_INVALID_ARG;
    }

    if (btn_cnt) {
        *btn_cnt = 0;
    }
    for (int i = 0; i < BSP_BUTTON_NUM; i++) {
        ret |= iot_button_new_gpio_device(&btn_config, &bsp_button_config[i], &btn_array[i]);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to create button %d", i);
        }
        if (btn_cnt) {
            (*btn_cnt)++;
        }
    }
    return ret;
}
