#ifndef KEYFRAMES_H
#define KEYFRAMES_H

#include <stdlib.h>

#include "pixelkey.h"

/**
 * Default implementation for keyframe_base_t::modifier_type_get that always returns KEYFRAME_MODIFIER_TYPE_NONE.
 * @param[in] p_keyframe Pointer to the keyframe.
 * @return always KEYFRAME_MODIFIER_TYPE_NONE.
 */
keyframe_modifier_type_t keyframe_default_modifier_type_get(keyframe_base_t * const p_keyframe);

keyframe_base_t * keyframe_blink_parse(char * p_str, size_t len);
keyframe_base_t * keyframe_fade_parse(char * p_str, size_t len);
keyframe_base_t * keyframe_set_parse(char * p_str, size_t len);

#endif // KEYFRAMES_H
