#ifndef KEYFRAME_FADE_H
#define KEYFRAME_FADE_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "color.h"
#include "keyframes.h"

/**
 * @ingroup pixelkey__keyframes
 * @defgroup pixelkey__keyframes__fade Fade Keyframe
 * Keyframe for transitioning between two or more colors.
 * @{
*/

/** Maximum number of colors allowed to be input by the user. */
#define KEYFRAME_FADE_COLORS_INPUT_MAX_LENGTH (15)

/**
 * Maximum number of colors that can be faded in one keyframe.
 * Input + 1 to allow current color to be pushed to the top.
 */
#define KEYFRAME_FADE_COLORS_MAX_LENGTH  (KEYFRAME_FADE_COLORS_INPUT_MAX_LENGTH + 1)


/**  
 * The type of fade to perform.
 */
typedef enum e_fade_type
{
    FADE_TYPE_STEP,  ///< Colors are immediately transitioned.
    FADE_TYPE_CUBIC  ///< The transition curve is applied between every pair of colors.
} fade_type_t;

/**
 * The color axes/channels to fade over.
 */
typedef enum e_fade_axis
{
    FADE_AXIS_NONE = 0,         ///< No axis require fading.
    FADE_AXIS_HUE = (1U << 0),  ///< Fade is needed for the Hue axis.
    FADE_AXIS_SAT = (1U << 1),  ///< Fade is needed for the Saturation axis.
    FADE_AXIS_VAL = (1U << 2)   ///< Fade if needed for the Value axis.
} fade_axis_t;

/**
 * Single point on a cartesian plane.
 */
typedef struct st_point
{
    float x; ///< X-coordinate of the point.
    float y; ///< Y-coordinate of the point.
} point_t;

/**
 * Pair of control points for a cubic bezier curve where the start is (0,0) and end is (1,1).
 */
typedef struct st_cubic_bezier
{
    point_t p1; ///< First control point, from (0,0).
    point_t p2; ///< Second control point, to (1,1).
} cubic_bezier_t;

/**
 * Fade keyframe.
 */
typedef struct st_keyframe_fade
{
    /** Keyframe base; MUST be the first entry in the struct. */
    keyframe_base_t base;
    /** Parsed arguments. */
    struct
    {
        float          period;       ///< Number of seconds to fade over; max of 60 seconds.
        fade_type_t    fade_type;    ///< The type of transition to perform.
        cubic_bezier_t curve;        ///< The cubic bezier curve points for non-step transitions.
        bool           push_current; ///< Push the current color to be the first.
        uint8_t        colors_len;   ///< Number of colors provided.
        color_hsv_t    colors[KEYFRAME_FADE_COLORS_MAX_LENGTH]; ///< Array of colors provided by the user.
    } args;
    /** Keyframe render state. */
    struct
    {
        fade_axis_t fade_axis;    ///< Which axis of the colors need to be faded.
        timestep_t  color_period; ///< The period to transition between each pair of colors.
        timestep_t  finish_time;  ///< Total number of frames for this keyframe at the current framerate.
        uint8_t     pair_index;   ///< Index of the first color of the currently transitioning pair.
        point_t     curr_b_point; ///< The current bezier point to render.
    } state;
} keyframe_fade_t;

extern const cubic_bezier_t cb_linear;
extern const cubic_bezier_t cb_ease;
extern const cubic_bezier_t cb_ease_in;
extern const cubic_bezier_t cb_ease_out;
extern const cubic_bezier_t cb_ease_in_out; 

/** @} */

#endif
