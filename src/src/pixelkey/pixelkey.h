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

pixelkey_error_t pixelkey_render_frame(color_rgb_t * p_frame_buffer);

pixelkey_error_t pixelkey_enqueue_keyframe(uint8_t index, keyframe_base_t * p_keyframe);

void pixelkey_frameproc_init(framerate_t framerate);
void pixelkey_framerate_set(framerate_t framerate);

void pixelkey_commandproc_init(void);
void pixelkey_cmd_list_free(cmd_list_t * p_cmd_list);
pixelkey_error_t pixelkey_command_parse(char * command_str, cmd_list_t ** p_cmd_list);

/** @} */

#endif // PIXELKEY_H
