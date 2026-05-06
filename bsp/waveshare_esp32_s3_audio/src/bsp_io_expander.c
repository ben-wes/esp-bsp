/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "bsp_err_check.h"
#include "esp_io_expander_tca95xx_16bit.h"
#include "esp_io_expander_gpio_wrapper.h"

#include "bsp/waveshare_esp32_s3_audio.h"

#include <sys/lock.h>

static esp_io_expander_handle_t io_expander = NULL;  // IO Expander
static _lock_t expander_lock;

esp_io_expander_handle_t bsp_io_expander_init(void)
{
    _lock_acquire(&expander_lock);
    if (io_expander) {
        _lock_release(&expander_lock);
        return io_expander;
    }
    /* Initilize I2C */
    if (bsp_i2c_init() != ESP_OK) {
        _lock_release(&expander_lock);
        return NULL;
    }

    /* Initialize TCA9555 (16-bit IO expander) at address 0x20 */
    if (esp_io_expander_new_i2c_tca95xx_16bit(bsp_i2c_get_handle(),
                                BSP_IO_EXPANDER_ADDRESS, &io_expander) != ESP_OK) {
        _lock_release(&expander_lock);
        return NULL;
    }

#if CONFIG_IO_EXPANDER_ENABLE_GPIO_API_WRAPPER
    // Map expander pins to virtual GPIO numbers starting at GPIO_NUM_MAX
    esp_io_expander_gpio_wrapper_append_handler(io_expander, GPIO_NUM_MAX);
#endif

    _lock_release(&expander_lock);
    return io_expander;
}
