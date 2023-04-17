#ifndef KEYFRAMES_H
#define KEYFRAMES_H

#include <stdlib.h>

#include "pixelkey.h"

/**
 * @ingroup pixelkey
 * @defgroup pixelkey__keyframes Keyframes
 * @{
 */

keyframe_base_t * keyframe_blink_parse(char * p_str);
keyframe_base_t * keyframe_fade_parse(char * p_str);
keyframe_base_t * keyframe_set_parse(char * p_str);

/** @} */

#endif // KEYFRAMES_H
