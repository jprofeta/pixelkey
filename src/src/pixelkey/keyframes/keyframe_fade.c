
#include <string.h>

#include "pixelkey.h"
#include "keyframes.h"

/** Maximum number of colors allowed to be input by the user. */
#define COLORS_INPUT_MAX_LENGTH (15)

/**
 * Maximum number of colors that can be faded in one keyframe.
 * Input + 1 to allow current color to be pushed to the top.
 */
#define COLORS_MAX_LENGTH  (COLORS_INPUT_MAX_LENGTH + 1)

typedef enum e_fade_type
{
    FADE_TYPE_STEP,  ///< Colors are immediately transitioned.
    FADE_TYPE_CUBIC  ///< The transition curve is applied between every pair of colors.
} fade_type_t;

typedef enum e_fade_axis
{
    FADE_AXIS_NONE = 0,         ///< No axis require fading.
    FADE_AXIS_HUE = (1U << 0),  ///< Fade is needed for the Hue axis.
    FADE_AXIS_SAT = (1U << 1),  ///< Fade is needed for the Saturation axis.
    FADE_AXIS_VAL = (1U << 2)   ///< Fade if needed for the Value axis.
} fade_axis_t;

/** Single point on a cartesian plane. */
typedef struct st_point
{
    float x; ///< X-coordinate of the point.
    float y; ///< Y-coordinate of the point.
} point_t;

/** Pair of control points for a cubic bezier curve where the start is (0,0) and end is (1,1). */
typedef struct st_cubic_bezier
{
    point_t p1; ///< First control point, from (0,0).
    point_t p2; ///< Second control point, to (1,1).
} cubic_bezier_t;

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
        color_hsv_t    colors[COLORS_MAX_LENGTH]; ///< Array of colors provided by the user.
    } args;
    /** Keyframe render state. */
    struct
    {
        fade_axis_t fade_axis;    ///< Which axis of the colors need to be faded.
        timestep_t  color_period; ///< The period to transition between each pair of colors.
        timestep_t  finish_time;  ///< Total number of frames for this keyframe at the current framerate.
        uint8_t     pair_index;   ///< Index of the first color of the currently transitioning pair.
    } state;
} keyframe_fade_t;

static bool keyframe_fade_render_frame(keyframe_base_t * const p_keyframe, timestep_t time, color_rgb_t * p_color_out);
static void keyframe_fade_render_init(keyframe_base_t * const p_keyframe, framerate_t framerate, color_rgb_t current_color);

static const keyframe_base_api_t keyframe_fade_api =
{
    .render_frame = keyframe_fade_render_frame,
    .render_init = keyframe_fade_render_init,
};

static const keyframe_fade_t keyframe_fade_init = 
{
    .base = { .p_api = &keyframe_fade_api },
};

static const cubic_bezier_t linear      = { { 0.0f,  0.0f }, { 1.0f,  1.0f } };
static const cubic_bezier_t ease        = { { 0.25f, 0.1f }, { 0.25f, 1.0f } };
static const cubic_bezier_t ease_in     = { { 0.42f, 0.0f }, { 1.0f,  1.0f } };
static const cubic_bezier_t ease_out    = { { 0.0f,  0.0f }, { 0.58f, 1.0f } };
static const cubic_bezier_t ease_in_out = { { 0.42f, 0.0f }, { 0.58f, 1.0f } };

static bool keyframe_fade_render_frame(keyframe_base_t * const p_keyframe, timestep_t time, color_rgb_t * p_color_out)
{
    keyframe_fade_t * const p_fade = (keyframe_fade_t * const) p_keyframe;

    // Increment the pair index if we should be at a new one.
    // This assumes time increases monotonically. If for some reason it doesn't, the fade will be broken anyway.
    if (time == (p_fade->state.pair_index + 1) * p_fade->state.color_period)
    {
        p_fade->state.pair_index += 1;
    }

    // Get the time relative to the start of this pair's transition.
    // This is used to index the bezier curve later.
    timestep_t relative_time = time - (p_fade->state.color_period * p_fade->state.pair_index);

    return time >= p_fade->state.finish_time;
}

static void keyframe_fade_render_init(keyframe_base_t * const p_keyframe, framerate_t framerate, color_rgb_t current_color)
{
    keyframe_fade_t * const p_fade = (keyframe_fade_t * const) p_keyframe;

    // Determine if the current color should be pushed onto the color list.
    if (p_fade->args.colors_len == 1 || p_fade->args.push_current)
    {
        // Move all the colors down one spot.
        for (uint8_t i = p_fade->args.colors_len - 1; i >= 1; i--)
        {
            p_fade->args.colors[i] = p_fade->args.colors[i - 1];
        }
        p_fade->args.colors_len += 1;

        // Insert the current color as HSV.
        color_convert2(COLOR_SPACE_RGB, COLOR_SPACE_HSV, (color_kind_t *)&current_color, (color_kind_t *)&p_fade->args.colors[0]);
    }

    // Determine which axis require fading. This allows for some optimization later.
    fade_axis_t axis = FADE_AXIS_NONE;
    for (uint8_t i = 1; i < p_fade->args.colors_len; i++)
    {
        if (p_fade->args.colors[i].hue != p_fade->args.colors[0].hue)
        {
            axis |= FADE_AXIS_HUE;
        }

        if (p_fade->args.colors[i].saturation != p_fade->args.colors[0].saturation)
        {
            axis |= FADE_AXIS_SAT;
        }

        if (p_fade->args.colors[i].value != p_fade->args.colors[0].value)
        {
            axis |= FADE_AXIS_VAL;
        }
    }
    p_fade->state.fade_axis = axis;

    // Calculate the total time for the keyframe.
    p_fade->state.finish_time = (timestep_t) (p_fade->args.period * ((float) framerate));

    // Calculate the period between each color.
    p_fade->state.color_period = (timestep_t) (p_fade->state.finish_time / (p_fade->args.colors_len - 1)); 

    // Set default values.
    p_fade->state.pair_index = 0;
}
