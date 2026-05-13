/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief ESP BSP: Waveshare ESP32-S3-AUDIO-Board
 */

#pragma once

#include "bsp/config.h"
#include "bsp/display.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "driver/i2s_std.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_codec_dev.h"
#include "esp_io_expander.h"
#include "esp_vfs_fat.h"
#include "iot_button.h"
#include "sdkconfig.h"
#include "led_strip.h"

#if (BSP_CONFIG_NO_GRAPHIC_LIB == 0)
#include "esp_lvgl_port.h"
#include "lvgl.h"
#endif // BSP_CONFIG_NO_GRAPHIC_LIB == 0

/**************************************************************************************************
 *  BSP Board Name
 **************************************************************************************************/
#define BSP_BOARD_WAVESHARE_ESP32_S3_AUDIO

/**************************************************************************************************
 *  BSP Capabilities
 **************************************************************************************************/
#define BSP_CAPS_DISPLAY 1
#define BSP_CAPS_TOUCH 1
#define BSP_CAPS_BUTTONS 1
#define BSP_CAPS_KNOB 0
#define BSP_CAPS_AUDIO 1
#define BSP_CAPS_AUDIO_SPEAKER 1
#define BSP_CAPS_AUDIO_MIC 1
#define BSP_CAPS_SDCARD 1
#define BSP_CAPS_LED 1
#define BSP_CAPS_CAMERA 1
#define BSP_CAPS_BAT 1
#define BSP_CAPS_IMU 0

/**************************************************************************************************
 *  Board pinout
 **************************************************************************************************/

/* Waveshare ESP32-S3-AUDIO-Board pinout */

/** @defgroup g01_i2c I2C
 *  @brief I2C BSP API
 *  @{
 */
#define BSP_I2C_SCL (GPIO_NUM_10)
#define BSP_I2C_SDA (GPIO_NUM_11)
/** @} */

/** @defgroup g03_audio Audio
 *  @brief Audio BSP API
 *  @{
 */
#define BSP_I2S_MCLK (GPIO_NUM_12)
#define BSP_I2S_SCLK (GPIO_NUM_13)
#define BSP_I2S_LCLK (GPIO_NUM_14)
#define BSP_I2S_DOUT (GPIO_NUM_16)
#define BSP_I2S_DSIN (GPIO_NUM_15)
#define BSP_POWER_AMP_IO (GPIO_NUM_MAX + 8) /* TCA9555 EXIO8 */
/** @} */

/** @defgroup g04_display Display and Touch
 *  @brief Display BSP API
 *  @{
 */
#define BSP_LCD_PCLK (GPIO_NUM_4)
#define BSP_LCD_DATA0 (GPIO_NUM_9)
#define BSP_LCD_DATA1 (GPIO_NUM_8)
#define BSP_LCD_DATA2 (GPIO_NUM_7)
#define BSP_LCD_DATA3 (GPIO_NUM_6)
#define BSP_LCD_DC (GPIO_NUM_7)
#define BSP_LCD_CS (GPIO_NUM_3)
#define BSP_LCD_RST (GPIO_NUM_MAX + 0) /* TCA9555 EXIO0 */
#define BSP_LCD_BACKLIGHT (GPIO_NUM_5)
#define BSP_LCD_EN (GPIO_NUM_5)
#define BSP_TOUCH_EN (GPIO_NUM_MAX + 1)      /* TCA9555 EXIO1 */
#define BSP_LCD_TOUCH_INT (GPIO_NUM_MAX + 2) /* TCA9555 EXIO2 */
/** @} */

/** @defgroup g12_camera Camera
 *  @brief Camera BSP API
 *  @{
 */
#define BSP_CAMERA_GPIO_XCLK (GPIO_NUM_43)
#define BSP_CAMERA_RST (GPIO_NUM_MAX + 6)  /* TCA9555 EXIO6 (SET) */
#define BSP_CAMERA_PWDN (GPIO_NUM_MAX + 5) /* TCA9555 EXIO5 */
#define BSP_CAMERA_PCLK (GPIO_NUM_44)
#define BSP_CAMERA_VSYNC (GPIO_NUM_21)
#define BSP_CAMERA_HSYNC (GPIO_NUM_1) /* HREF */
#define BSP_CAMERA_D0 (GPIO_NUM_2)
#define BSP_CAMERA_D1 (GPIO_NUM_17)
#define BSP_CAMERA_D2 (GPIO_NUM_18)
#define BSP_CAMERA_D3 (GPIO_NUM_39)
#define BSP_CAMERA_D4 (GPIO_NUM_45)
#define BSP_CAMERA_D5 (GPIO_NUM_46)
#define BSP_CAMERA_D6 (GPIO_NUM_47)
#define BSP_CAMERA_D7 (GPIO_NUM_48)
/** @} */

/** @defgroup g06_led Leds
 *  @brief Leds BSP API
 *  @{
 */
/* WS2812 RGB LED ring on GPIO38, 7 LEDs */
#define BSP_LED_STRIP_IO (GPIO_NUM_38)
#define BSP_LED_STRIP_NUM (7)
/** @} */

/** @defgroup g02_storage SD Card and SPIFFS
 *  @brief SPIFFS and SD card BSP API
 *  @{
 */
#define BSP_SD_D0 (GPIO_NUM_41)
#define BSP_SD_D1 (GPIO_NUM_NC)
#define BSP_SD_D2 (GPIO_NUM_NC)
#define BSP_SD_D3 (GPIO_NUM_NC)
#define BSP_SD_CMD (GPIO_NUM_42)
#define BSP_SD_CLK (GPIO_NUM_40)
#define BSP_SD_DET (GPIO_NUM_MAX + 3) /* TCA9555 EXIO3 */
/** @} */

/** @defgroup g09_battery Battery
 *  @brief Battery BSP API
 *  @{
 */
#define BSP_BATTERY_VOLTAGE_CHANNEL (ADC_CHANNEL_4)
#define BSP_BATTERY_VOLTAGE_DIV (2)
/** @} */

/** @defgroup g05_buttons Buttons
 *  @brief Buttons BSP API
 *  @{
 */
/* The Waveshare board has 3 user buttons: K1, K2, K3 (active low GPIO buttons)
 * We map them to the most useful Korvo-2 equivalents for compatibility. */
typedef enum {
  BSP_BUTTON_VOLUP,   /* K1 */
  BSP_BUTTON_PLAY,    /* K2 */
  BSP_BUTTON_VOLDOWN, /* K3 */
  BSP_BUTTON_REC,     /* Dummy */
  BSP_BUTTON_SET,     /* Dummy */
  BSP_BUTTON_NUM
} bsp_button_t;
/** @} */

/** @defgroup g06_led_enum Led enum
 *  @brief Led types
 *  @{
 */
/* LED definitions - WS2812 strip (treated as a single logical LED for BSP API)
 */
typedef enum { BSP_LED_STRIP, BSP_LED_NUM } bsp_led_t;

/* Default LED effects */
typedef enum {
  BSP_LED_ON,
  BSP_LED_OFF,
  BSP_LED_BLINK_FAST,
  BSP_LED_BLINK_SLOW,
  BSP_LED_MAX,
} bsp_led_effect_t;
/** @} */

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************************
 *
 * I2S audio interface
 *
 **************************************************************************************************/

/**
 * @brief Init audio
 *
 * @param[in]  i2s_config I2S configuration. Pass NULL to use default values
 * @return
 *      - ESP_OK on success
 *      - other error codes
 */
esp_err_t bsp_audio_init(const i2s_std_config_t *i2s_config);

/**
 * @brief Get codec I2S interface (initialized in bsp_audio_init)
 * @return Pointer to codec I2S interface handle or NULL when error occurred
 */
const audio_codec_data_if_t *bsp_audio_get_codec_itf(void);

/**
 * @brief Initialize speaker codec device
 * @return Pointer to codec device handle or NULL when error occurred
 */
esp_codec_dev_handle_t bsp_audio_codec_speaker_init(void);

/**
 * @brief Initialize microphone codec device
 * @return Pointer to codec device handle or NULL when error occurred
 */
esp_codec_dev_handle_t bsp_audio_codec_microphone_init(void);

/**************************************************************************************************
 *
 * I2C interface
 *
 **************************************************************************************************/
#define BSP_I2C_NUM CONFIG_BSP_I2C_NUM

esp_err_t bsp_i2c_init(void);
esp_err_t bsp_i2c_deinit(void);
i2c_master_bus_handle_t bsp_i2c_get_handle(void);

/**************************************************************************************************
 *
 * SPIFFS
 *
 **************************************************************************************************/
#define BSP_SPIFFS_MOUNT_POINT CONFIG_BSP_SPIFFS_MOUNT_POINT

esp_err_t bsp_spiffs_mount(void);
esp_err_t bsp_spiffs_unmount(void);

/**************************************************************************************************
 *
 * SD card
 *
 **************************************************************************************************/
#define BSP_SD_MOUNT_POINT CONFIG_BSP_SD_MOUNT_POINT

typedef struct {
  const esp_vfs_fat_sdmmc_mount_config_t *mount;
  sdmmc_host_t *host;
  union {
    const sdmmc_slot_config_t *sdmmc;
    const sdspi_device_config_t *sdspi;
  } slot;
} bsp_sdcard_cfg_t;

esp_err_t bsp_sdcard_mount(void);
esp_err_t bsp_sdcard_unmount(void);
sdmmc_card_t *bsp_sdcard_get_handle(void);
void bsp_sdcard_get_sdmmc_host(const int slot, sdmmc_host_t *config);
void bsp_sdcard_get_sdspi_host(const int slot, sdmmc_host_t *config);
void bsp_sdcard_sdmmc_get_slot(const int slot, sdmmc_slot_config_t *config);
void bsp_sdcard_sdspi_get_slot(const spi_host_device_t spi_host,
                               sdspi_device_config_t *config);
esp_err_t bsp_sdcard_sdmmc_mount(bsp_sdcard_cfg_t *cfg);
esp_err_t bsp_sdcard_sdspi_mount(bsp_sdcard_cfg_t *cfg);

/**************************************************************************************************
 *
 * LCD interface
 *
 **************************************************************************************************/
#define BSP_LCD_PIXEL_CLOCK_HZ (40 * 1000 * 1000)
#define BSP_LCD_SPI_NUM (SPI3_HOST)

#if (BSP_CONFIG_NO_GRAPHIC_LIB == 0)

typedef struct {
  lvgl_port_cfg_t lvgl_port_cfg;
  uint32_t buffer_size;
  bool double_buffer;
  struct {
    unsigned int buff_dma : 1;
    unsigned int buff_spiram : 1;
    unsigned int sw_rotate : 1;
  } flags;
} bsp_display_cfg_t;

lv_display_t *bsp_display_start(void);
lv_display_t *bsp_display_start_with_config(const bsp_display_cfg_t *cfg);
lv_indev_t *bsp_display_get_input_dev(void);
bool bsp_display_lock(uint32_t timeout_ms);
void bsp_display_unlock(void);
esp_err_t bsp_display_enter_sleep(void);
esp_err_t bsp_display_exit_sleep(void);
void bsp_display_rotate(lv_display_t *disp, lv_disp_rotation_t rotation);
#endif // BSP_CONFIG_NO_GRAPHIC_LIB == 0

/**************************************************************************************************
 *
 * Camera interface
 *
 **************************************************************************************************/
#define BSP_CAMERA_DEVICE (ESP_VIDEO_DVP_DEVICE_NAME)
#define BSP_CAMERA_VFLIP (0)
#define BSP_CAMERA_HFLIP (0)
#define BSP_CAMERA_XCLK_CLOCK_MHZ (20)

typedef struct {
  uint8_t dummy;
} bsp_camera_cfg_t;

esp_err_t bsp_camera_start(const bsp_camera_cfg_t *cfg);

/**************************************************************************************************
 *
 * ADC interface
 *
 **************************************************************************************************/
#define BSP_ADC_UNIT ADC_UNIT_1

esp_err_t bsp_adc_initialize(void);
adc_oneshot_unit_handle_t bsp_adc_get_handle(void);

/**************************************************************************************************
 *
 * BSP Features
 *
 **************************************************************************************************/
typedef enum { BSP_FEATURE_LCD, BSP_FEATURE_TOUCH } bsp_feature_t;

esp_err_t bsp_feature_enable(bsp_feature_t feature, bool enable);

/**************************************************************************************************
 *
 * LEDs
 *
 **************************************************************************************************/
esp_err_t bsp_led_strip_init(void);
led_strip_handle_t bsp_led_strip_get_handle(void);

/**************************************************************************************************
 *
 * Button
 *
 **************************************************************************************************/
esp_err_t bsp_iot_button_create(button_handle_t btn_array[], int *btn_cnt,
                                int btn_array_size);

/**************************************************************************************************
 *
 * IO Expander Interface
 * IO expander: TCA9555 (16-bit, address 0x20)
 *
 **************************************************************************************************/
#define BSP_IO_EXPANDER_ADDRESS (0x20)

esp_io_expander_handle_t bsp_io_expander_init(void);

/**************************************************************************************************
 *
 * Voltage measurements
 *
 **************************************************************************************************/
esp_err_t bsp_voltage_init(void);
int bsp_voltage_battery_get(void);

#ifdef __cplusplus
}
#endif
