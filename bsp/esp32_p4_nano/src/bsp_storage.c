/*
 * SPDX-FileCopyrightText: 2024-2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include "esp_log.h"
#include "esp_check.h"
#include "esp_idf_version.h"
#include "esp_spiffs.h"
#include "esp_vfs_fat.h"
#include "sd_pwr_ctrl_by_on_chip_ldo.h"
#include "bsp_err_check.h"
#include "bsp/esp32_p4_nano.h"

static const char *TAG = "ESP32_P4_NANO";

#if CONFIG_ESP_HOSTED_SDIO_HOST_INTERFACE && (ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(6, 0, 0))
#define WORKAROUND_HOSTED_DOES_SDMMC_HOST_INIT 1
static esp_err_t sdmmc_host_init_dummy(void)
{
    return ESP_OK;
}
static esp_err_t sdmmc_host_deinit_dummy(void)
{
    return ESP_OK;
}
#else
#define WORKAROUND_HOSTED_DOES_SDMMC_HOST_INIT 0
#endif

sdmmc_card_t *bsp_sdcard = NULL;
static sd_pwr_ctrl_handle_t pwr_ctrl_handle = NULL;
static bool pwr_ctrl_adopted;

void bsp_sdcard_adopt_pwr_ctrl_handle(sd_pwr_ctrl_handle_t handle)
{
    pwr_ctrl_handle = handle;
    pwr_ctrl_adopted = (handle != NULL);
}

esp_err_t bsp_spiffs_mount(void)
{
    esp_vfs_spiffs_conf_t conf = {
        .base_path = CONFIG_BSP_SPIFFS_MOUNT_POINT,
        .partition_label = CONFIG_BSP_SPIFFS_PARTITION_LABEL,
        .max_files = CONFIG_BSP_SPIFFS_MAX_FILES,
#ifdef CONFIG_BSP_SPIFFS_FORMAT_ON_MOUNT_FAIL
        .format_if_mount_failed = true,
#else
        .format_if_mount_failed = false,
#endif
    };

    esp_err_t ret_val = esp_vfs_spiffs_register(&conf);

    BSP_ERROR_CHECK_RETURN_ERR(ret_val);

    size_t total = 0, used = 0;
    ret_val = esp_spiffs_info(conf.partition_label, &total, &used);
    if (ret_val != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret_val));
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }

    return ret_val;
}

esp_err_t bsp_spiffs_unmount(void)
{
    return esp_vfs_spiffs_unregister(CONFIG_BSP_SPIFFS_PARTITION_LABEL);
}

sdmmc_card_t *bsp_sdcard_get_handle(void)
{
    return bsp_sdcard;
}

void bsp_sdcard_get_sdmmc_host(const int slot, sdmmc_host_t *config)
{
    assert(config);
    sdmmc_host_t host_config = SDMMC_HOST_DEFAULT();
    host_config.slot = slot;
    host_config.max_freq_khz = SDMMC_FREQ_HIGHSPEED;
#if WORKAROUND_HOSTED_DOES_SDMMC_HOST_INIT
    host_config.init = &sdmmc_host_init_dummy;
    host_config.deinit = &sdmmc_host_deinit_dummy;
#endif
    memcpy(config, &host_config, sizeof(sdmmc_host_t));
}

void bsp_sdcard_get_sdspi_host(const int slot, sdmmc_host_t *config)
{
    assert(config);
    memset(config, 0, sizeof(sdmmc_host_t));
    ESP_LOGE(TAG, "SD card SPI mode is not supported!");
}

void bsp_sdcard_sdmmc_get_slot(const int slot, sdmmc_slot_config_t *config)
{
    assert(config);
    memset(config, 0, sizeof(sdmmc_slot_config_t));
    (void)slot;
    config->cd = SDMMC_SLOT_NO_CD;
    config->wp = SDMMC_SLOT_NO_WP;
    config->cmd = BSP_SD_CMD;
    config->clk = BSP_SD_CLK;
    config->d0 = BSP_SD_D0;
    config->d1 = BSP_SD_D1;
    config->d2 = BSP_SD_D2;
    config->d3 = BSP_SD_D3;
    config->width = 4;
    config->flags = 0;
}

void bsp_sdcard_sdspi_get_slot(const spi_host_device_t spi_host, sdspi_device_config_t *config)
{
    assert(config);
    memset(config, 0, sizeof(sdspi_device_config_t));
    (void)spi_host;
    ESP_LOGE(TAG, "SD card SPI mode is not supported!");
}

esp_err_t bsp_sdcard_sdmmc_mount(bsp_sdcard_cfg_t *cfg)
{
    sdmmc_host_t sdhost = {0};
    sdmmc_slot_config_t sdslot = {0};
    const esp_vfs_fat_sdmmc_mount_config_t mount_config = {
#ifdef CONFIG_BSP_SD_FORMAT_ON_MOUNT_FAIL
        .format_if_mount_failed = true,
#else
        .format_if_mount_failed = false,
#endif
        .max_files = 5,
        .allocation_unit_size = 64 * 1024
    };
    assert(cfg);

    if (!cfg->mount) {
        cfg->mount = &mount_config;
    }

    if (!cfg->host) {
        bsp_sdcard_get_sdmmc_host(SDMMC_HOST_SLOT_0, &sdhost);
        cfg->host = &sdhost;
    }

    if (!cfg->slot.sdmmc) {
        bsp_sdcard_sdmmc_get_slot(SDMMC_HOST_SLOT_0, &sdslot);
        cfg->slot.sdmmc = &sdslot;
    }

    if (!cfg->host->pwr_ctrl_handle) {
        if (pwr_ctrl_handle) {
            cfg->host->pwr_ctrl_handle = pwr_ctrl_handle;
        } else {
            sd_pwr_ctrl_ldo_config_t ldo_config = {
                .ldo_chan_id = 4,
            };
            esp_err_t ret = sd_pwr_ctrl_new_on_chip_ldo(&ldo_config, &pwr_ctrl_handle);
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "Failed to create a new on-chip LDO power control driver");
                return ret;
            }
            pwr_ctrl_adopted = false;
            cfg->host->pwr_ctrl_handle = pwr_ctrl_handle;
        }
    }

#if !defined(CONFIG_FATFS_LONG_FILENAMES) || defined(CONFIG_FATFS_LFN_NONE)
    ESP_LOGW(TAG, "Warning: Long filenames on SD card are disabled in menuconfig!");
#endif

    return esp_vfs_fat_sdmmc_mount(BSP_SD_MOUNT_POINT, cfg->host, cfg->slot.sdmmc, cfg->mount, &bsp_sdcard);
}

esp_err_t bsp_sdcard_sdspi_mount(bsp_sdcard_cfg_t *cfg)
{
    (void)cfg;
    ESP_LOGE(TAG, "SD card SPI mode is not supported!");
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t bsp_sdcard_mount(void)
{
    bsp_sdcard_cfg_t cfg = {0};
    return bsp_sdcard_sdmmc_mount(&cfg);
}

esp_err_t bsp_sdcard_unmount(void)
{
    esp_err_t ret = ESP_OK;

    if (pwr_ctrl_handle && !pwr_ctrl_adopted) {
        ret |= sd_pwr_ctrl_del_on_chip_ldo(pwr_ctrl_handle);
        pwr_ctrl_handle = NULL;
    }

    ret |= esp_vfs_fat_sdcard_unmount(BSP_SD_MOUNT_POINT, bsp_sdcard);
    bsp_sdcard = NULL;

    return ret;
}
