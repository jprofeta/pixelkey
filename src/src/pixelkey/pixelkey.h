#ifndef PIXELKEY_H
#define PIXELKEY_H

#include <stdbool.h>
#include <stdint.h>

#include "helper_macros.h"

#include "color.h"

#include "pixelkey_errors.h"
#include "pixelkey_commands.h"

#include "keyframes.h"

/**
 * @defgroup pixelkey PixelKey
 * PixelKey generic processes.
 * 
 * The PixelKey processes are designed to be processor agnostic. All implementation
 * specific details should be contained in @ref hal.
 * @{
 */

void pixelkey_frameproc_init(framerate_t framerate);
void pixelkey_keyframeproc_framerate_set(framerate_t framerate);
uint32_t pixelkey_keyframeproc_framecount_get(void);
pixelkey_error_t pixelkey_keyframeproc_render_frame(color_rgb_t * p_frame_buffer);
pixelkey_error_t pixelkey_keyframeproc_push(uint8_t index, keyframe_base_t * p_keyframe);

void pixelkey_commandproc_init(void);
void pixelkey_commandproc_task(void);
void pixelkey_commandproc_terminal_connected(void);
void pixelkey_commandproc_send_prompt(void);
pixelkey_error_t pixelkey_commandproc_push(cmd_t * p_cmd);

/** @} */

#endif // PIXELKEY_H
