#ifndef HAL_DEVICE_H
#define HAL_DEVICE_H

/**
 * @defgroup hal PixelKey HAL
 * @{
 */

#include <stdint.h>

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

/** @} */

#endif // HAL_DEVICE_H
