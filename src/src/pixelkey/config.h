#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <stddef.h>

#include "pixelkey_errors.h"

/**
 * @file
 * @defgroup pixelkey__config Configuration
 * @ingroup pixelkey
 * @{
 */

#define CONFIG_DATA_VERSION     (1)

// Saved to memory; must be packed!
#pragma pack(push,1)
/**
 * Configuration structure.
 * 
 * @note The @ref config_data_t::version element should be incremented if the struct layout changes in such 
 *       a way that it is incompatible with previous versions, e.g. fields are moved out of order. Simple additions
 *       to the end of the struct do not need data version to be updated. 
 */
typedef struct st_config_data
{
    /** 
     * Config header.
     * @warning Modification or moving of this struct will break the config functions.
     * @warning These fields should only be modified by the code in hal_flash.c.
     */
    struct
    {
        uint16_t crc;                   ///< CRC-CCITT of all the following fields in the structure.
        uint8_t  length;                ///< Size of the configuration data struct at the time of save.
        uint8_t  version;               ///< Configuration struct version.
    } header;
    union
    {
        struct
        {
            uint32_t echo_enabled       :  1; ///< COM echo is enabled.
            uint32_t gamma_enabled      :  1; ///< Gamma correction is enabled. 
            uint32_t                    : 30;
        } flags_b;                      ///< Configuration flags bit-field.
        uint32_t flags;                 ///< Configuration flags as a word.
    };
    float gamma_factor;                 ///< Gamma correction factor.
    uint32_t framerate;                 ///< Frame rate.
    uint32_t num_neopixels;             ///< Number of attached neopixels.
    uint8_t max_rgb_value;              ///< Maximum brightness allowed for any RGB channel.
} config_data_t;
#pragma pack(pop)

static_assert(offsetof(config_data_t, header) == 0, "Config header must be the first element of config_data_t.");
static_assert(offsetof(config_data_t, header.crc) == 0, "CRC must start at byte 0.");
static_assert(offsetof(config_data_t, header.length) == 2, "Length must start at byte 2.");
static_assert(offsetof(config_data_t, header.version) == 3, "Version must start at byte 3.");

/** Configuration instance API. */
typedef struct st_config_api
{
    /**
     * Writes a configuration data struct to NV memory.
     * @param[in] p_config_data Pointer to the data to save.
     * @retval PIXELKEY_ERROR_NONE            Write was successful.
     * @retval PIXELKEY_ERROR_NV_MEMORY_ERROR NV memory error occurred on write.
     */
    pixelkey_error_t (* write)(config_data_t const * const p_config_data);
    /**
     * Gets a pointer to the configuration data struct.
     * @param[out] pp_config_data Pointer to write the config data pointer.
     * @retval PIXELKEY_ERROR_NONE            Read was successful.
     * @retval PIXELKEY_ERROR_NV_MEMORY_ERROR NV memory error occurred on read.
     */
    pixelkey_error_t (* read)(config_data_t ** pp_config_data);
} config_api_t;

config_api_t const * config(void);
config_data_t const * config_default(void);
config_data_t const * config_get_or_default(void);
void config_register(config_api_t const * p_instance);
pixelkey_error_t config_validate(void);

/** @} */

#endif
