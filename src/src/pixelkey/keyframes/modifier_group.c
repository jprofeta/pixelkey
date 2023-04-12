
#include "pixelkey.h"
#include "keyframes.h"
#include "keyframe_modifiers.h"

/** Maximum allowed child keyframes. */
#define GROUP_CHILDREN_MAX_COUNT    (16)

typedef struct st_keyframe_mod_group
{
    /** Keyframe base; MUST be the first entry in the struct. */
    keyframe_base_t base;

    /** Render state variables. */
    struct
    {
        framerate_t framerate;      ///< The framerate used when the repeat keyframe was started.
        color_rgb_t current_color;  ///< The current color to use when init'ing the child.
        bool        init_keyframe;  ///< Initialize the child keyframe on the next call to render.
        timestep_t  start_offset;   ///< Start timestep when the child was last initialized.
    } state;

    uint8_t current_child_idx;      ///< Index of the current child being rendered.
    uint8_t children_len;           ///< Total number of children in this group.
    keyframe_base_t * const children[GROUP_CHILDREN_MAX_COUNT]; ///< List of child keyframes in this group.
} keyframe_mod_group_t;

static bool keyframe_mod_group_render_frame(keyframe_base_t * const p_keyframe, timestep_t time, color_rgb_t * p_color_out);
static void keyframe_mod_group_render_init(keyframe_base_t * const p_keyframe, framerate_t framerate, color_rgb_t current_color);

static const keyframe_base_api_t keyframe_mod_group_api =
{
    .render_frame = keyframe_mod_group_render_frame,
    .render_init = keyframe_mod_group_render_init,
};

static const keyframe_mod_group_t keyframe_mod_group_init = 
{
    .base = { .p_api = &keyframe_mod_group_api },
};

static bool keyframe_mod_group_render_frame(keyframe_base_t * const p_keyframe, timestep_t time, color_rgb_t * p_color_out)
{
    keyframe_mod_group_t * const p_mod_group = (keyframe_mod_group_t * const) p_keyframe;

    keyframe_base_t * const p_child_keyframe = p_mod_group->children[p_mod_group->current_child_idx];

    // Initialize the child keyframe if needed.
    if (p_mod_group->state.init_keyframe)
    {
        p_mod_group->state.init_keyframe = false;

        // Copy the start time so we can calculate the relative time later.
        p_mod_group->state.start_offset = time;

        // Init the child.
        p_child_keyframe->p_api->render_init(p_child_keyframe,
                                                p_mod_group->state.framerate,
                                                p_mod_group->state.current_color);
    }

    // Calculate the relative time for the child.
    timestep_t child_time = time - p_mod_group->state.start_offset;

    // Let the child render.
    bool is_child_done = p_child_keyframe->p_api->render_frame(p_child_keyframe, child_time, p_color_out);

    if (is_child_done)
    {
        // Copy the current color so we can init the next child.
        p_mod_group->state.current_color = *p_color_out;

        // Increment the child index and set it to init next call.
        p_mod_group->current_child_idx += 1;
        p_mod_group->state.init_keyframe = true;
    }

    return (p_mod_group->current_child_idx >= p_mod_group->children_len);
}

static void keyframe_mod_group_render_init(keyframe_base_t * const p_keyframe, framerate_t framerate, color_rgb_t current_color)
{
    keyframe_mod_group_t * const p_mod_group = (keyframe_mod_group_t * const) p_keyframe;

    p_mod_group->state.framerate = framerate;
    p_mod_group->state.current_color = current_color;
    p_mod_group->state.init_keyframe = true;
    p_mod_group->state.start_offset = 0;

    p_mod_group->current_child_idx = 0;
}
