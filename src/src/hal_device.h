#ifndef HAL_DEVICE_H
#define HAL_DEVICE_H

#include <stdint.h>

#define PIXELKEY_NEOPIXEL_COUNT         (4U)

#define NPDATA_GPT_BUFFER_LENGTH        (PIXELKEY_NEOPIXEL_COUNT * NEOPIXEL_COLOR_BITS)

#define NPDATA_GPT_B1                   (30)
#define NPDATA_GPT_B0                   (15)

#define PIXELKEY_KEYFRAME_QUEUE_LENGTH  (4)

extern uint32_t g_npdata_gpt_buffer[];

#endif // HAL_DEVICE_H
