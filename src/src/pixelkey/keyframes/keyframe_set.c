#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "pixelkey.h"
#include "keyframes.h"

/**
 * @addtogroup pixelkey__keyframes__set
 * @{
*/

static bool keyframe_set_render_frame(keyframe_base_t * const p_keyframe, timestep_t time, color_rgb_t * p_color_out);
static void keyframe_set_render_init(keyframe_base_t * const p_keyframe, framerate_t framerate, color_rgb_t current_color);

static const keyframe_base_api_t keyframe_set_api =
{
    .render_frame = keyframe_set_render_frame,
    .render_init = keyframe_set_render_init,
};

static const keyframe_set_t keyframe_set_init =
{
    .base = { .p_api = &keyframe_set_api },
};

static bool keyframe_set_render_frame(keyframe_base_t * const p_keyframe, timestep_t time, color_rgb_t * p_color_out)
{
    // Set only occurs for one frame so time can be ignored.
    ARG_NOT_USED(time);

    keyframe_set_t * const p_set = (keyframe_set_t * const) p_keyframe;

    // Copy the output color.
    *p_color_out = p_set->args.color.rgb;

    return true;   // No frames remaining.
}

static void keyframe_set_render_init(keyframe_base_t * const p_keyframe, framerate_t framerate, color_rgb_t current_color)
{
    ARG_NOT_USED(framerate);
    ARG_NOT_USED(current_color);

    // Make sure the color is in RGB.
    keyframe_set_t * const p_set = (keyframe_set_t * const) p_keyframe;
    if (p_set->args.color.color_space != COLOR_SPACE_RGB)
    {
        color_t color_rgb;
        color_convert(COLOR_SPACE_RGB, &p_set->args.color, &color_rgb);

        // Copy the RGB converted color since it cannot be converted in-place.
        p_set->args.color = color_rgb;
    }

    return;
}

/**
 * Parses a command string into a @ref pixelkey__keyframes__set.
 * @param[in] p_str Pointer to the command string.
 * @return Pointer to the parsed keyframe or NULL on error.
 */
keyframe_base_t * keyframe_set_parse(char * p_str)
{
    // Allocate a new keyframe and copy the default values.
    keyframe_set_t * p_set = malloc(sizeof(keyframe_set_t));
    memcpy(p_set, &keyframe_set_init, sizeof(keyframe_set_t));

    bool has_error = true;
    do
    {
        char * p_context = NULL;
        char * p_tok = strtok_r(p_str, " ", &p_context);
        // Check to see if the color parsing failed.
        if (color_parse(p_tok, &p_set->args.color) != PIXELKEY_ERROR_NONE)
        {
            break;
        }

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
        free(p_set);
        return NULL;
    }
    else
    {
        return &p_set->base;
    }
}

/**
 * Initialize a Set keyframe with the appropriate keyframe_base_t values.
 * @param[in] p_set Pointer to the set keyframe to construct, or NULL to allocate a new one.
 * @return Pointer to the keyframe base portion of the set keyframe.
 */
keyframe_base_t * keyframe_set_ctor(keyframe_set_t * p_set)
{
    // If NULL, allocate a new set keyframe.
    if (p_set == NULL)
    {
        p_set = malloc(sizeof(keyframe_set_t));
    }

    // Copy the base struct info (yes some of these fields are marked const... Just do it.)
    memcpy(&p_set->base, &keyframe_set_init.base, sizeof(keyframe_base_t));

    return &p_set->base;
}

/** @} */
