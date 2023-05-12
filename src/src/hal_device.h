#ifndef HAL_DEVICE_H
#define HAL_DEVICE_H

#include "helper_macros.h"
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

/**
 * @defgroup hal PixelKey HAL
 * @{
 */

/** Number of NeoPixels present on the PCB. */
#define PIXELKEY_NEOPIXEL_COUNT         (4U)

/** Length of the GPT waveform buffer. */
#define NPDATA_GPT_BUFFER_LENGTH        (8U)

/** Number of secondary waveform buffers to keep. */
#define NPDATA_SECONDARY_BUFFER_COUNT   (2U)

/** Count for the high period of a 1-bit. */
#define NPDATA_GPT_B1                   (30)
/** Count for the high period of a 0-bit. */
#define NPDATA_GPT_B0                   (15)

/** Number of keyframes allowed to be queued. */
#define PIXELKEY_KEYFRAME_QUEUE_LENGTH  (4)

/** Command input buffer length. */
#define PIXELKEY_INPUT_COMMAND_BUFFER_LENGTH    (256)

/** Total number of commands that can be queued. */
#define PIXELKEY_COMMAND_BUFFER_LENGTH  (8)

#define PIXELKEY_DISABLE_GAMMA_CORRECTION (1)

#define PIXELKEY_DEFAULT_FRAMERATE      (30)

#define PIXELKEY_DEFAULT_COM_ECHO       (0)

#define ECHO_USB 1

void hal_usb_idle(void);

/** @} */

#endif // HAL_DEVICE_H
