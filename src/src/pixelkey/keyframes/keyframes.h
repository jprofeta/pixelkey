#ifndef KEYFRAMES_H
#define KEYFRAMES_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "pixelkey.h"
#include "keyframe_fade.h"

/**
 * @ingroup pixelkey
 * @defgroup pixelkey__keyframes Keyframes
 * @{
 */

/**
 * @defgroup pixelkey__keyframes__blink Blink Keyframe
 * Keyframe to blink between two colors.
 * @{
 */

/**
 * Blink keyframe.
 */
typedef struct st_keyframe_blink
{
    /** Keyframe base; MUST be the first entry in the struct. */
    keyframe_base_t base;
    /** Parsed arguments. */
    struct
    {
        color_rgb_t color1;          ///< Color to blink during ON cycle.
        color_rgb_t color2;          ///< Color to blink during OFF cycle.
        float       period;          ///< Number of seconds to blink over; max of 60 seconds.
        uint8_t     duty_cycle;      ///< Blink duty cycle.
        bool        color1_provided; ///< Specifies if color1 was set during parsing.
        bool        color2_provided; ///< Specifies if color2 was set during parsing.
    } args;
    /** Keyframe render state. */
    struct
    {
        timestep_t  transition_time; ///< Time to transition from color1 to color2 for the current framerate.
        timestep_t  finish_time;     ///< Time at which the keyframe has completed for the current framerate.
    } state;
} keyframe_blink_t;

/** @} */

/**
 * @defgroup pixelkey__keyframes__set Set Keyframe
 * Keyframe to set a NeoPixel to a constant color.
 * @{
 */

/**
 * Set keyframe.
 */
typedef struct st_keyframe_set
{
    /** Keyframe base; MUST be the first entry in the struct. */
    keyframe_base_t base;
    /** Parsed arguments. */
    struct
    {
        color_t color; ///< Color to set.
    } args;
} keyframe_set_t;

/** @} */

keyframe_base_t * keyframe_blink_parse(char * p_str);
keyframe_base_t * keyframe_blink_ctor(keyframe_blink_t * p_blink);

keyframe_base_t * keyframe_fade_parse(char * p_str);
keyframe_base_t * keyframe_fade_ctor(keyframe_fade_t * p_fade);

keyframe_base_t * keyframe_set_parse(char * p_str);
keyframe_base_t * keyframe_set_ctor(keyframe_set_t * p_set);

/** @} */

#endif // KEYFRAMES_H
