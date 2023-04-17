#ifndef KEYFRAMES_H
#define KEYFRAMES_H

#include <stdlib.h>

#include "pixelkey.h"

/**
 * @ingroup pixelkey
 * @defgroup pixelkey__keyframes Keyframes
 * @{
 */

/**
 * Parses a command string into a @ref pixelkey__keyframes__blink.
 * @param[in] p_str Pointer to the command string.
 * @return Pointer to the parsed keyframe or NULL on error.
 */
keyframe_base_t * keyframe_blink_parse(char * p_str);
/**
 * Parses a command string into a @ref pixelkey__keyframes__fade.
 * @param[in] p_str Pointer to the command string.
 * @return Pointer to the parsed keyframe or NULL on error.
 */
keyframe_base_t * keyframe_fade_parse(char * p_str);
/**
 * Parses a command string into a @ref pixelkey__keyframes__set.
 * @param[in] p_str Pointer to the command string.
 * @return Pointer to the parsed keyframe or NULL on error.
 */
keyframe_base_t * keyframe_set_parse(char * p_str);

/** @} */

#endif // KEYFRAMES_H
