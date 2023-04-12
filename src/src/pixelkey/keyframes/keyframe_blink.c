
#include "pixelkey.h"
#include "keyframes.h"

/** Default duty cycle if not specified. */
#define DUTY_CYCLE_DEFAULT  (50U)

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
        bool        color2_provided; ///< Specifies if color2 was set during parsing.
    } args;
    /** Keyframe render state. */
    struct
    {
        timestep_t  transition_time; ///< Time to transition from color1 to color2 for the current framerate.
        timestep_t  finish_time;     ///< Time at which the keyframe has completed for the current framerate.
    } state;
} keyframe_blink_t;

static bool keyframe_blink_render_frame(keyframe_base_t * const p_keyframe, timestep_t time, color_rgb_t * p_color_out);
static void keyframe_blink_render_init(keyframe_base_t * const p_keyframe, framerate_t framerate, color_rgb_t current_color);

static const keyframe_base_api_t keyframe_blink_api =
{
    .render_frame = keyframe_blink_render_frame,
    .render_init = keyframe_blink_render_init,
};

static const keyframe_blink_t keyframe_blink_init = 
{
    .base = { .p_api = &keyframe_blink_api },
    .args = {0},
    .state = {0},
};

static bool keyframe_blink_render_frame(keyframe_base_t * const p_keyframe, timestep_t time, color_rgb_t * p_color_out)
{
    keyframe_blink_t * const p_blink = (keyframe_blink_t * const) p_keyframe;

    if (time < p_blink->state.transition_time)
    {
        *p_color_out = p_blink->args.color1;
    }
    else
    {
        *p_color_out = p_blink->args.color2;
    }

    return time >= p_blink->state.finish_time;
}

static void keyframe_blink_render_init(keyframe_base_t * const p_keyframe, framerate_t framerate, color_rgb_t current_color)
{
    keyframe_blink_t * const p_blink = (keyframe_blink_t * const) p_keyframe;

    if (!p_blink->args.color2_provided)
    {
        p_blink->args.color2 = current_color;
    }

    p_blink->state.finish_time = (timestep_t) (p_blink->args.period * ((float) framerate));
    p_blink->state.transition_time = (timestep_t) ((p_blink->state.finish_time * p_blink->args.duty_cycle) / 100);
}
