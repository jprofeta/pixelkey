/**
 * @file
 * @defgroup hal__flash__internals Flash NVMem Internals
 * @ingroup hal
 * @{
*/

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "hal_data.h"
#include "pixelkey_errors.h"

#include "config.h"

/** Number of data flash blocks required by @ref config_data_t (rounds up). */
#define DATA_FLASH_BLOCKS_PER_CONFIG    ((sizeof(config_data_t) + BSP_FEATURE_FLASH_LP_DF_BLOCK_SIZE - 1)/BSP_FEATURE_FLASH_LP_DF_BLOCK_SIZE)

// Grab the start of data flash from the linker script.
extern char const __Data_Flash_Start;

static pixelkey_error_t flash_config_write(config_data_t const * const p_config_data);
static pixelkey_error_t flash_config_read(config_data_t ** pp_config_data);

/** Pointer to the configuration struct at the start of the Data Flash section. */
config_data_t const * const p_nv_config = (config_data_t *)((void *)&__Data_Flash_Start);

/** Calculated NV memory CRC to validate the data. */
static uint32_t nv_crc = UINT32_MAX;

const config_api_t g_hal_config =
{
    .write = flash_config_write,
    .read = flash_config_read,
};

static pixelkey_error_t flash_config_write(config_data_t const * const p_config_data)
{
    // Copy the config struct locally so we can CRC and set the length.
    config_data_t data = *p_config_data;
    data.header.length = sizeof(config_data_t);

    if (FSP_SUCCESS != g_crc0.p_api->open(&g_crc0_ctrl, &g_crc0_cfg))
    {
        return PIXELKEY_ERROR_NV_MEMORY_ERROR;
    }
    crc_input_t crc_in =
    {
        .p_input_buffer = &data.header.length,
        .num_bytes = p_nv_config->header.length - sizeof(data.header.crc),
        .crc_seed = 0,
    };
    uint32_t crc = 0;
    g_crc0.p_api->calculate(&g_crc0_ctrl, &crc_in, &crc);
    data.header.crc = crc & UINT16_MAX;  // Mask to make sure there are only 16-bits.

    g_crc0.p_api->close(&g_crc0_ctrl);

    if (FSP_SUCCESS != g_flash0.p_api->open(&g_flash0_ctrl, &g_flash0_cfg))
    {
        return PIXELKEY_ERROR_NV_MEMORY_ERROR;
    }

    if (FSP_SUCCESS != g_flash0.p_api->erase(&g_flash0_ctrl, (uint32_t)((void *)p_nv_config), DATA_FLASH_BLOCKS_PER_CONFIG))
    {
        g_flash0.p_api->close(&g_flash0_ctrl);
        return PIXELKEY_ERROR_NV_MEMORY_ERROR;
    }

    if (FSP_SUCCESS != g_flash0.p_api->write(&g_flash0_ctrl, (uint32_t)((void *)&data), (uint32_t)((void *)p_nv_config), sizeof(config_data_t)))
    {
        g_flash0.p_api->close(&g_flash0_ctrl);
        return PIXELKEY_ERROR_NV_MEMORY_ERROR;
    }

    g_flash0.p_api->close(&g_flash0_ctrl);
    nv_crc = data.header.crc;
    return PIXELKEY_ERROR_NONE;
}

static pixelkey_error_t flash_config_read(config_data_t ** pp_config_data)
{
    if (p_nv_config->header.crc == UINT16_MAX && p_nv_config->header.length == UINT8_MAX)
    {
        return PIXELKEY_ERROR_NV_NOT_INITIALIZED;
    }

    if (nv_crc == UINT32_MAX)
    {
        if (FSP_SUCCESS != g_crc0.p_api->open(&g_crc0_ctrl, &g_crc0_cfg))
        {
            return PIXELKEY_ERROR_NV_MEMORY_ERROR;
        }
        crc_input_t crc_in =
        {
            .p_input_buffer = (void *)&p_nv_config->header.length,
            .num_bytes = p_nv_config->header.length - sizeof(p_nv_config->header.crc),
            .crc_seed = 0,
        };
        uint32_t crc = 0;
        g_crc0.p_api->calculate(&g_crc0_ctrl, &crc_in, &crc);
        nv_crc = crc & UINT16_MAX;  // Mask to make sure there are only 16-bits.

        g_crc0.p_api->close(&g_crc0_ctrl);
    }

    if (nv_crc != (uint32_t)p_nv_config->header.crc)
    {
        return PIXELKEY_ERROR_NV_CRC_MISMATCH;
    }

    *pp_config_data = p_nv_config;

    return PIXELKEY_ERROR_NONE;
}

/** @} */
