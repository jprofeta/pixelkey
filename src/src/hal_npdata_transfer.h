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

/** NeoPixel frame buffer. */
extern volatile color_rgb_t g_npdata_frame[PIXELKEY_NEOPIXEL_COUNT];

void npdata_open(void);
void npdata_send_frame(void);

/** @} */

#endif  // HAL_NPDATA_TRANSFER_H
