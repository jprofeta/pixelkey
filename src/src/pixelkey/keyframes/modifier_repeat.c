
#include "pixelkey.h"
#include "keyframes.h"
#include "keyframe_modifiers.h"

typedef struct st_keyframe_mod_repeat
{
    /** Keyframe base; MUST be the first entry in the struct. */
    keyframe_base_t base;

    int32_t           repeat_count; ///< Number of repeats to perform.
    keyframe_base_t * child;        ///< Pointer to the child keyrame to repeat.

    /** Render state variables. */
    struct
    {
        framerate_t framerate;      ///< The framerate used when the repeat keyframe was started.
        color_rgb_t current_color;  ///< The current color to use when init'ing the child.
        bool        init_keyframe;  ///< Initialize the child keyframe on the next call to render.
        timestep_t  start_offset;   ///< Start timestep when the child was last initialized.
    } state;

} keyframe_mod_repeat_t;

static bool keyframe_mod_repeat_render_frame(keyframe_base_t * const p_keyframe, timestep_t time, color_rgb_t * p_color_out);
static void keyframe_mod_repeat_render_init(keyframe_base_t * const p_keyframe, framerate_t framerate, color_rgb_t current_color);

static const keyframe_base_api_t keyframe_mod_repeat_api =
{
    .render_frame = keyframe_mod_repeat_render_frame,
    .render_init = keyframe_mod_repeat_render_init,
};

static const keyframe_mod_repeat_t keyframe_mod_repeat_init = 
{
    .base = { .p_api = &keyframe_mod_repeat_api },
};

static bool keyframe_mod_repeat_render_frame(keyframe_base_t * const p_keyframe, timestep_t time, color_rgb_t * p_color_out)
{
    keyframe_mod_repeat_t * const p_mod_repeat = (keyframe_mod_repeat_t * const) p_keyframe;

    keyframe_base_t * const p_child_keyframe = p_mod_repeat->child;

    // Initialize the child keyframe if needed.
    if (p_mod_repeat->state.init_keyframe)
    {
        p_mod_repeat->state.init_keyframe = false;

        // Copy the start time so we can calculate the relative time later.
        p_mod_repeat->state.start_offset = time;

        // Init the child.
        p_child_keyframe->p_api->render_init(p_child_keyframe,
                                                p_mod_repeat->state.framerate,
                                                p_mod_repeat->state.current_color);
    }

    // Calculate the relative time for the child.
    timestep_t child_time = time - p_mod_repeat->state.start_offset;

    // Let the child render.
    bool is_child_done = p_child_keyframe->p_api->render_frame(p_child_keyframe, child_time, p_color_out);

    if (is_child_done)
    {
        // Copy the current color so we can init the next child.
        p_mod_repeat->state.current_color = *p_color_out;

        // Set to init the child again on the next call.
        p_mod_repeat->state.init_keyframe = true;

        // Only decrement if we have a positive number of repeats.
        // If repeat_count is negative then this will allow it to repeat forever.
        if (p_mod_repeat->repeat_count > 0)
        {
            p_mod_repeat->repeat_count -= 1;
        }
    }

    return (p_mod_repeat->repeat_count == 0);
}

static void keyframe_mod_repeat_render_init(keyframe_base_t * const p_keyframe, framerate_t framerate, color_rgb_t current_color)
{
    keyframe_mod_repeat_t * const p_mod_repeat = (keyframe_mod_repeat_t * const) p_keyframe;

    p_mod_repeat->state.framerate = framerate;
    p_mod_repeat->state.current_color = current_color;
    p_mod_repeat->state.init_keyframe = true;
    p_mod_repeat->state.start_offset = 0;
}
