#ifndef PIXELKEY_HAL
#define PIXELKEY_HAL

/**
 * @file
 * @defgroup pixelkey__hal Hardware Abstraction Layer for PixelKey
 * Provides abstraction functions for various hardware methods.
 * @ingroup pixelkey
 * @{
*/

#include "pixelkey.h"
#include "pixelkey_errors.h"

pixelkey_error_t pixelkey_hal_frame_timer_update(framerate_t new_framerate);

/** @} */

#endif
