/**
 * @file
 * @defgroup pixelkey__config__internals Configuration Internals
 * @ingroup pixelkey__config
 * @{
*/

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "hal_device.h"
#include "pixelkey_errors.h"
#include "neopixel.h"

#include "config.h"

/** Currently registered API instance. */
static config_api_t const * registered_api = NULL;

/** Default configuration data. */
static config_data_t const config_data_default = 
{
    .version = CONFIG_DATA_VERSION,
    .flags_b = 
    {
        .echo_enabled = PIXELKEY_DEFAULT_COM_ECHO,
        .gamma_enabled = !PIXELKEY_DISABLE_GAMMA_CORRECTION,
    },
    .gamma_factor = NEOPIXEL_GAMMA_CORRECTION_DEFAULT,
    .framerate = PIXELKEY_DEFAULT_FRAMERATE,
    .num_neopixels = PIXELKEY_NEOPIXEL_COUNT,
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
config_data_t const * config_get_or_default(void)
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

/** @} */
