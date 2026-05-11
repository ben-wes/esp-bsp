/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "bsp_err_check.h"
#include "button_gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "iot_button.h"

#include "bsp/waveshare_esp32_s3_audio.h"

static const char *TAG = "Waveshare-Audio-Buttons";

/* Buttons from the korvo 2 are connected to an adc not gpio */
/* Buttons from the waveshare are connected to TCA9555 IO expander */
#define BSP_BUTTON_K1_IO (GPIO_NUM_MAX + 9)  /* TCA9555 EXIO9 */
#define BSP_BUTTON_K2_IO (GPIO_NUM_MAX + 10) /* TCA9555 EXIO10 */
#define BSP_BUTTON_K3_IO (GPIO_NUM_MAX + 11) /* TCA9555 EXIO11 */

typedef enum {
  BSP_BUTTON_TYPE_EXPANDER, // Button connected to TCA9555
  BSP_BUTTON_TYPE_DUMMY,    // Does nothing
} bsp_button_type_t;

typedef struct {
  bsp_button_type_t type;
  button_gpio_config_t gpio;
} bsp_button_config_t;

static const bsp_button_config_t bsp_button_config[BSP_BUTTON_NUM] = {
    [BSP_BUTTON_VOLUP] = {.type = BSP_BUTTON_TYPE_EXPANDER,
                          .gpio = {.gpio_num = BSP_BUTTON_K1_IO,
                                   .active_level = 0}},
    [BSP_BUTTON_PLAY] = {.type = BSP_BUTTON_TYPE_EXPANDER,
                         .gpio = {.gpio_num = BSP_BUTTON_K2_IO,
                                  .active_level = 0}},
    [BSP_BUTTON_VOLDOWN] = {.type = BSP_BUTTON_TYPE_EXPANDER,
                            .gpio = {.gpio_num = BSP_BUTTON_K3_IO,
                                     .active_level = 0}},
    [BSP_BUTTON_REC] = {.type = BSP_BUTTON_TYPE_DUMMY,
                        .gpio = {.gpio_num = -1, .active_level = 0}},
    [BSP_BUTTON_SET] = {.type = BSP_BUTTON_TYPE_DUMMY,
                        .gpio = {.gpio_num = -1, .active_level = 0}}};

typedef struct {
  button_driver_t base;
  int32_t gpio_num;
  uint8_t active_level;
} button_custom_obj_t;

static uint8_t button_expander_get_level(button_driver_t *button_driver) {
  button_custom_obj_t *btn =
      __containerof(button_driver, button_custom_obj_t, base);
  return gpio_get_level(btn->gpio_num) == btn->active_level ? 1 : 0;
}

static esp_err_t button_custom_del(button_driver_t *button_driver) {
  button_custom_obj_t *btn =
      __containerof(button_driver, button_custom_obj_t, base);
  free(btn);
  return ESP_OK;
}

static uint8_t button_dummy_get_level(button_driver_t *button_driver) {
  return 0; // Never pressed
}

esp_err_t bsp_iot_button_create(button_handle_t btn_array[], int *btn_cnt,
                                int btn_array_size) {
  /* Init IO expander */
  BSP_ERROR_CHECK_RETURN_ERR(bsp_io_expander_init() ? ESP_OK : ESP_FAIL);

  esp_err_t ret = ESP_OK;
  const button_config_t btn_config = {0};

  if ((btn_array_size < BSP_BUTTON_NUM) || (btn_array == NULL)) {
    return ESP_ERR_INVALID_ARG;
  }

  if (btn_cnt) {
    *btn_cnt = 0;
  }

  for (int i = 0; i < BSP_BUTTON_NUM; i++) {
    button_custom_obj_t *custom_btn = calloc(1, sizeof(button_custom_obj_t));
    if (!custom_btn)
      return ESP_ERR_NO_MEM;

    custom_btn->base.del = button_custom_del;

    if (bsp_button_config[i].type == BSP_BUTTON_TYPE_EXPANDER) {
      custom_btn->gpio_num = bsp_button_config[i].gpio.gpio_num;
      custom_btn->active_level = bsp_button_config[i].gpio.active_level;
      custom_btn->base.get_key_level = button_expander_get_level;

      gpio_set_direction(custom_btn->gpio_num, GPIO_MODE_INPUT);
      ret |= iot_button_create(&btn_config, &custom_btn->base, &btn_array[i]);

    } else if (bsp_button_config[i].type == BSP_BUTTON_TYPE_DUMMY) {
      custom_btn->base.get_key_level = button_dummy_get_level;
      ret |= iot_button_create(&btn_config, &custom_btn->base, &btn_array[i]);

    } else {
      ESP_LOGW(TAG, "Unsupported button type!");
      free(custom_btn);
    }

    if (btn_cnt) {
      (*btn_cnt)++;
    }
  }
  return ret;
}