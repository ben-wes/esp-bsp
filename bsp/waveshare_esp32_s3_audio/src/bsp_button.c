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

typedef struct {
    button_driver_t base;
    int32_t gpio_num;
    uint8_t active_level;
} button_exio_obj_t;

static uint8_t button_exio_get_key_level(button_driver_t *button_driver)
{
    button_exio_obj_t *gpio_btn = __containerof(button_driver, button_exio_obj_t, base);
    return gpio_get_level(gpio_btn->gpio_num) == gpio_btn->active_level ? 1 : 0;
}

static esp_err_t button_exio_del(button_driver_t *button_driver)
{
    button_exio_obj_t *gpio_btn = __containerof(button_driver, button_exio_obj_t, base);
    free(gpio_btn);
    return ESP_OK;
}

static uint8_t button_dummy_get_key_level(button_driver_t *button_driver)
{
    return 0; // Never pressed
}

static esp_err_t button_dummy_del(button_driver_t *button_driver)
{
    free(button_driver);
    return ESP_OK;
}

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
        if (bsp_button_config[i].gpio_num >= 0) {
            button_exio_obj_t *gpio_btn = calloc(1, sizeof(button_exio_obj_t));
            if (!gpio_btn) return ESP_ERR_NO_MEM;
            gpio_btn->gpio_num = bsp_button_config[i].gpio_num;
            gpio_btn->active_level = bsp_button_config[i].active_level;
            gpio_btn->base.get_key_level = button_exio_get_key_level;
            gpio_btn->base.del = button_exio_del;

            // TCA9555 has internal pull-ups, no need to configure pull mode
            gpio_set_direction(gpio_btn->gpio_num, GPIO_MODE_INPUT);

            ret |= iot_button_create(&btn_config, &gpio_btn->base, &btn_array[i]);
        } else {
            // Create a dummy button
            button_driver_t *dummy_btn = calloc(1, sizeof(button_driver_t));
            if (!dummy_btn) return ESP_ERR_NO_MEM;
            dummy_btn->get_key_level = button_dummy_get_key_level;
            dummy_btn->del = button_dummy_del;
            ret |= iot_button_create(&btn_config, dummy_btn, &btn_array[i]);
        }
        if (btn_cnt) {
            (*btn_cnt)++;
        }
    }
    return ret;
}
