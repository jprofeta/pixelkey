
#include "pixelkey.h"
#include "keyframes.h"

typedef struct st_keyframe_set
{
    /** Keyframe base; MUST be the first entry in the struct. */
    keyframe_base_t base;
    /** Parsed arguments. */
    struct
    {
        color_rgb_t color;  ///< Color to set.
    } args;
} keyframe_set_t;

static bool keyframe_set_render_frame(keyframe_base_t * const p_keyframe, timestep_t time, color_rgb_t * p_color_out);
static void keyframe_set_render_init(keyframe_base_t * const p_keyframe, framerate_t framerate, color_rgb_t current_color);

static const keyframe_base_api_t keyframe_set_api =
{
    .render_frame = keyframe_set_render_frame,
    .render_init = keyframe_set_render_init,
    .modifier_type_get = keyframe_default_modifier_type_get,
};

static const keyframe_set_t keyframe_set_init = 
{
    .base = { .p_api = &keyframe_set_api },
    .args = {0}
};

static bool keyframe_set_render_frame(keyframe_base_t * const p_keyframe, timestep_t time, color_rgb_t * p_color_out)
{
    // Set only occurs for one frame so time can be ignored.
    ARG_NOT_USED(time);

    keyframe_set_t * const p_set = (keyframe_set_t * const) p_keyframe;

    // Copy the output color.
    *p_color_out = p_set->args.color;

    return true;   // No frames remaining.
}

static void keyframe_set_render_init(keyframe_base_t * const p_keyframe, framerate_t framerate, color_rgb_t current_color)
{
    // Do nothing here.

    ARG_NOT_USED(p_keyframe);
    ARG_NOT_USED(framerate);
    ARG_NOT_USED(current_color);

    return;
}

