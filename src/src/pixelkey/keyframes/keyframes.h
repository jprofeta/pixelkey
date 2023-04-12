#ifndef KEYFRAMES_H
#define KEYFRAMES_H

#include <stdlib.h>

#include "pixelkey.h"

keyframe_base_t * keyframe_blink_parse(char * p_str, size_t len);
keyframe_base_t * keyframe_fade_parse(char * p_str, size_t len);
keyframe_base_t * keyframe_set_parse(char * p_str, size_t len);

#endif // KEYFRAMES_H
