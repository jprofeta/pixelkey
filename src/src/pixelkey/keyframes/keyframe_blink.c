#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "pixelkey.h"
#include "keyframes.h"

/**
 * @addtogroup pixelkey__keyframes__blink
 * @{
 */

/** @internal Default duty cycle if not specified. */
#define DUTY_CYCLE_DEFAULT  (50U)

static bool keyframe_blink_render_frame(keyframe_base_t * const p_keyframe, timestep_t time, color_rgb_t * p_color_out);
static void keyframe_blink_render_init(keyframe_base_t * const p_keyframe, framerate_t framerate, color_rgb_t current_color);

/**
 * @internal
 * Blink keyframe API function pointers.
 */
static const keyframe_base_api_t keyframe_blink_api =
{
    .render_frame = keyframe_blink_render_frame,
    .render_init = keyframe_blink_render_init,
};

/**
 * @internal
 * Default values for blink keyframe structs.
 */
static const keyframe_blink_t keyframe_blink_init = 
{
    .base =
    { 
        .p_api = &keyframe_blink_api,
        .modifiers = { .repeat_count = -1 } // Blink defaults to indefinite repeats.
    },
    .args =
    {
        .color2_provided = false,
        .color1_provided = false,
        .duty_cycle = 50,
        .period = 1,
    }
};

/**
 * @internal
 * Renders the frame.
 * See @ref keyframe_base_api_t::render_frame
 */
static bool keyframe_blink_render_frame(keyframe_base_t * const p_keyframe, timestep_t time, color_rgb_t * p_color_out)
{
    keyframe_blink_t * const p_blink = (keyframe_blink_t * const) p_keyframe;

    if (time < p_blink->state.transition_time)
    {
        *p_color_out = p_blink->args.color1.rgb;
    }
    else
    {
        *p_color_out = p_blink->args.color2.rgb;
    }

    return time >= p_blink->state.finish_time;
}

/**
 * @internal
 * Initialize the keyframe for rendering.
 * See @ref keyframe_base_api_t::render_init
 */
static void keyframe_blink_render_init(keyframe_base_t * const p_keyframe, framerate_t framerate, color_rgb_t current_color)
{
    keyframe_blink_t * const p_blink = (keyframe_blink_t * const) p_keyframe;

    if (!p_blink->args.color1_provided)
    {
        p_blink->args.color1.color_space = COLOR_SPACE_RGB;
        p_blink->args.color1.rgb = current_color;
    }
    else if (p_blink->args.color1.color_space != COLOR_SPACE_RGB)
    {
        color_t c1 = {0};
        color_convert(COLOR_SPACE_RGB,
                        &p_blink->args.color1,
                        &c1);
        p_blink->args.color1 = c1;
    }

    if (!p_blink->args.color2_provided)
    {
        color_t off = { .color_space = COLOR_SPACE_RGB, .rgb = { 0, 0, 0} };
        p_blink->args.color2 = off;
    }
    else if (p_blink->args.color2.color_space != COLOR_SPACE_RGB)
    {
        color_t c2 = {0};
        color_convert(COLOR_SPACE_RGB,
                        &p_blink->args.color2,
                        &c2);
        p_blink->args.color2 = c2;
    }

    p_blink->state.finish_time = (timestep_t) (p_blink->args.period * ((float) framerate));
    p_blink->state.transition_time = (timestep_t) ((p_blink->state.finish_time * p_blink->args.duty_cycle) / 100);
}

/**
 * Parses a command string into a @ref pixelkey__keyframes__blink.
 * @param[in] p_str Pointer to the command string.
 * @return Pointer to the parsed keyframe or NULL on error.
 */
keyframe_base_t * keyframe_blink_parse(char * p_str)
{
    // Allocate a new keyframe and copy the default values.
    keyframe_blink_t * p_blink = (keyframe_blink_t *) keyframe_blink_ctor(NULL);

    bool has_error = true;
    do
    {
        char * p_context = NULL;
        char * p_tok;

        if ((p_tok = strtok_r(p_str, " ", &p_context)) == NULL)
        {
            // No arguments
            break;
        }

        float period = strtof(p_tok, NULL);
        if (period <= 0.0f)
        {
            // Period must be non-zero, positive number.
            // It is also set to zero on parse failure.
            break;
        }
        p_blink->args.period = period;

        if ((p_tok = strtok_r(NULL, " ", &p_context)) == NULL)
        {
            // No more arguments, clear error flag and break.
            has_error = false;
            break;
        }

        // else: Parse the color list
        char * p_color_context = NULL;
        char * p_color_tok = strtok_r(p_tok, ":", &p_color_context);
        if (color_parse(p_color_tok, &p_blink->args.color1) == PIXELKEY_ERROR_NONE)
        {
            // Color parsing failed!
            break;
        }
        p_blink->args.color1_provided = true;

        // See if a second color was provided.
        p_color_tok = strtok_r(NULL, ":", &p_color_context);
        if (p_color_tok != NULL)
        {
            if (color_parse(p_color_tok, &p_blink->args.color2) == PIXELKEY_ERROR_NONE)
            {
                // Color parsing failed!
                break;
            }
            p_blink->args.color2_provided = true;
        }

        // Lastly check to see if a duty cycle was provided.
        if ((p_tok = strtok_r(NULL, " ", &p_context)) == NULL)
        {
            // No more arguments, clear error flag and break.
            has_error = false;
            break;
        }

        // else: Parse the duty cycle
        int duty_cycle = atoi(p_tok);
        if (duty_cycle <= 0 || duty_cycle >= 100)
        {
            // Duty cycle is only valid from 1-99.
            break;
        }
        p_blink->args.duty_cycle = (uint8_t) duty_cycle;

        // Check to see if more arguments are available and break if so.
        if (strtok_r(NULL, " ", &p_context) != NULL)
        {
            break;
        }

        // Everything checked out so clear the error flag.
        has_error = false;
    } while (0);
    
    if (has_error)
    {
        // Cleanup on error.
        free(p_blink);
        return NULL;
    }
    else
    {
        return &p_blink->base;
    }
}

/**
 * Initialize a Blink keyframe with the appropriate keyframe_base_t and state values.
 * @param[in] p_blink Pointer to the blink keyframe to construct, or NULL to allocate a new one.
 * @return Pointer to the keyframe base portion of the blink keyframe.
 */
keyframe_base_t * keyframe_blink_ctor(keyframe_blink_t * p_blink)
{
    // If NULL, allocate a new set keyframe.
    if (p_blink == NULL)
    {
        p_blink = malloc(sizeof(keyframe_blink_t));
        memcpy(p_blink, &keyframe_blink_init, sizeof(*p_blink));
    }
    else
    {
        // Copy the base struct info (yes some of these fields are marked const... Just do it.)
        memcpy(&p_blink->base, &keyframe_blink_init.base, sizeof(keyframe_base_t));

        // Zero out the state
        memset(&p_blink->state, 0, sizeof(p_blink->state));
    }

    return &p_blink->base;
}

/** @} */
