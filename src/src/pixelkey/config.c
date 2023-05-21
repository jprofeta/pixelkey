/**
 * @file
 * @defgroup pixelkey__config__internals Configuration Internals
 * @ingroup pixelkey__config
 * @{
*/

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "hal_device.h"
#include "helper_macros.h"
#include "pixelkey_errors.h"
#include "neopixel.h"

#include "config.h"

/** Currently registered API instance. */
static config_api_t const * registered_api = NULL;

/** Default configuration data. */
static config_data_t const config_data_default = 
{
    .header = 
    {
        .version = CONFIG_DATA_VERSION,
        .length = sizeof(config_data_t),
        .crc = UINT16_MAX,
    },
    .flags_b = 
    {
        .echo_enabled = PIXELKEY_DEFAULT_COM_ECHO,
        .gamma_enabled = !PIXELKEY_DISABLE_GAMMA_CORRECTION,
    },
    .gamma_factor = NEOPIXEL_GAMMA_CORRECTION_DEFAULT,
    .framerate = PIXELKEY_DEFAULT_FRAMERATE,
    .num_neopixels = PIXELKEY_NEOPIXEL_COUNT,
    .max_rgb_value = UINT8_MAX,
};

/**
 * Get a pointer to the current API instance.
 * @return pointer to instance.
 */
config_api_t const * config(void)
{
    return registered_api;
}

/**
 * Get a pointer to the default configuration values.
 * @return default config pointer.
 */
config_data_t const * config_default(void)
{
    return &config_data_default;
}

/**
 * Gets a pointer to the saved configuration values or the defaults if an error occurs.
 * @return Pointer to saved config or default values.
 */
inline config_data_t const * config_get_or_default(void)
{
    config_data_t * p_data;
    if (PIXELKEY_ERROR_NONE == registered_api->read(&p_data))
    {
        return p_data;
    }
    else
    {
        return &config_data_default;
    }
}

/**
 * Register an API to be used as the current instance.
 * @param[in] p_instance Pointer to the API instance.
 */
void config_register(config_api_t const * p_instance)
{
    registered_api = p_instance;
}

// Allow pointer arithmetic in validate.
WARNING_SAVE()
WARNING_DISABLE("pointer-arith")

/**
 * Validates and updates NV memory config if needed; can only be called after config_register.
 */
pixelkey_error_t config_validate(void)
{
    config_data_t * p_data = NULL;
    pixelkey_error_t err = registered_api->read(&p_data);
    if (err == PIXELKEY_ERROR_NV_NOT_INITIALIZED)
    {
        // Write the default values to memory.
        err = registered_api->write(&config_data_default);
    }
    else if (err != PIXELKEY_ERROR_NONE)
    {
        // Something happened to the NV memory...
        // Restore defaults.
        // Log an error? (these are separate clauses in case we want to do that.)
        err = registered_api->write(&config_data_default);
    }
    else
    {
        // Check to see if the NV memory needs advanced updating.
        if (p_data->header.version != config_data_default.header.version)
        {
            // This is a more complicated upgrade. Basically things are out of order.
            // Right now there is no use case for this.
        }
        // Assume if the lengths are different it is a simple append operation.
        else if (p_data->header.length != config_data_default.header.length)
        {
            // Load in the default values.
            config_data_t upgraded_data = config_data_default;
            const size_t header_len = sizeof(upgraded_data.header);
            memcpy(((void *)&upgraded_data) + header_len,
                    ((void *)p_data) + header_len,
                    p_data->header.length - header_len);

            // Write the updated config back.
            err = registered_api->write(&upgraded_data);
        }
    }

    return err;
}

WARNING_RESTORE()

/** @} */
