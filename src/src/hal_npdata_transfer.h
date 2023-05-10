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

/** Size of the frame buffer in bytes. */
#define NPDATA_FRAME_BUFFER_SIZE    (PIXELKEY_NEOPIXEL_COUNT * sizeof(color_rgb_t))

/**
 * Status of the NeoPixel data transfer.
 */
typedef enum e_transfer_status
{
    TRANSFER_STATUS_IDLE,   ///< No active transfer.
    TRANSFER_STATUS_WORKING ///< A transfer is currently active.
} transfer_status_t;

void npdata_open(void);
void npdata_frame_send(void);
void npdata_color_set(uint32_t index, color_rgb_t const * const p_color);
transfer_status_t npdata_status_get(void);

volatile color_rgb_t * npdata_frame_buffer_get(void);

/** @} */

#endif  // HAL_NPDATA_TRANSFER_H
