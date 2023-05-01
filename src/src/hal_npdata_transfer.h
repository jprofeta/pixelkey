#ifndef HAL_NPDATA_TRANSFER_H
#define HAL_NPDATA_TRANSFER_H

/**
 * @defgroup hal__npdata NeoPixel Data Transfer
 * @ingroup hal
 * @{
 */

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "hal_device.h"
#include "pixelkey.h"

typedef enum e_transfer_status
{
    TRANSFER_STATUS_IDLE,
    TRANSFER_STATUS_WORKING
} transfer_status_t;

/** NeoPixel frame buffer. */
extern volatile color_rgb_t g_npdata_frame[PIXELKEY_NEOPIXEL_COUNT];

void npdata_open(void);
void npdata_frame_send(void);
void npdata_color_set(uint32_t index, color_rgb_t const * const p_color);
transfer_status_t npdata_status_get(void);

/** @} */

#endif  // HAL_NPDATA_TRANSFER_H
