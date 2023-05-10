#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "pixelkey.h"
#include "keyframes.h"

/**
 * @addtogroup pixelkey__keyframes__fade
 * @{
 */

static bool keyframe_fade_render_frame(keyframe_base_t * const p_keyframe, timestep_t time, color_rgb_t * p_color_out);
static void keyframe_fade_render_init(keyframe_base_t * const p_keyframe, framerate_t framerate, color_rgb_t current_color);

static void blend_colors(color_hsv_t const * p_a, color_hsv_t const * p_b, fade_axis_t axis, float ratio, color_hsv_t * p_out);
static void cubic_bezier_calc(cubic_bezier_t const * const p_curve, float t, point_t * p_point);

/**
 * @private
 * Fade keyframe API function pointers.
 */
static const keyframe_base_api_t keyframe_fade_api =
{
    .render_frame = keyframe_fade_render_frame,
    .render_init = keyframe_fade_render_init,
};

/**
 * @private
 * Default values for fade keyframe structs.
 */
static const keyframe_fade_t keyframe_fade_init = 
{
    .base = { .p_api = &keyframe_fade_api },
    .args =
    {
        .colors_len = 0,
        .push_current = false,
        .fade_type = FADE_TYPE_STEP,
        .period = 1
    }
};

/** Control points for linear fade. Fades linearly, in equal steps, between the start and end values. */
const cubic_bezier_t cb_linear      = { { 0.0f,  0.0f }, { 1.0f,  1.0f } };
/** Control points for ease fade. Quickly fades from the start value, then transitions slower to the end value. */
const cubic_bezier_t cb_ease        = { { 0.25f, 0.1f }, { 0.25f, 1.0f } };
/** Control points for ease-in fade. Slow fade at the start then quickly fades to the end value. */
const cubic_bezier_t cb_ease_in     = { { 0.42f, 0.0f }, { 1.0f,  1.0f } };
/** Control points for ease-out fade. Quickly fades from the start value then slowly fades to the end value. */
const cubic_bezier_t cb_ease_out    = { { 0.0f,  0.0f }, { 0.58f, 1.0f } };
/** Control points for ease-in-out fade. Quickly transitions from the start to the end; faster than normal ease. */
const cubic_bezier_t cb_ease_in_out = { { 0.42f, 0.0f }, { 0.58f, 1.0f } };

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
    point_t bezier_point = {0};

    // Calculate the ratio for the interpolation index, then get the bezier point.
    float t = ((float) relative_time) / ((float) p_fade->state.color_period);
    cubic_bezier_calc(&p_fade->args.curve, t, &bezier_point);

    // Scale the x by the fading period so it can be directly compared to relative_time.
    bezier_point.x *= (float) p_fade->state.color_period;
    if (((float) relative_time) >= bezier_point.x)
    {
        // This frame is "ahead" of the curve so update the current curve point.
        p_fade->state.curr_b_point = bezier_point;
    }
    // else: No updated to the curve for this round, this frame is "behind". Fade based on the stored point.

    color_hsv_t blended_color;
    blend_colors(&p_fade->args.colors[p_fade->state.pair_index],
                    &p_fade->args.colors[p_fade->state.pair_index + 1],
                    p_fade->state.fade_axis,
                    p_fade->state.curr_b_point.y,
                    &blended_color);
    color_convert2(COLOR_SPACE_HSV, COLOR_SPACE_RGB, (color_kind_t *) &blended_color, (color_kind_t *)p_color_out);

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
    p_fade->state.curr_b_point.x = 0.0f;
    p_fade->state.curr_b_point.y = 0.0f;
}

/**
 * @private
 * Blends two HSV colors based on a ratio between a to b.
 * @param[in]  p_a   Pointer to color a (ratio = 0).
 * @param[in]  p_b   Pointer to color b (ratio = 1).
 * @param      axis  The axes to blend across.
 * @param      ratio The ratio between color a and color b.
 * @param[out] p_out Pointer to store the blended color.
 * 
 */
static void blend_colors(color_hsv_t const * p_a, color_hsv_t const * p_b, fade_axis_t axis, float ratio, color_hsv_t * p_out)
{
    if (axis & FADE_AXIS_HUE)
    {
        float hue_delta = (float) (p_b->hue - p_a->hue);
        p_out->hue = p_a->hue + (uint16_t) (ratio * hue_delta);
    }
    else
    {
        p_out->hue = p_a->hue;
    }

    if (axis & FADE_AXIS_SAT)
    {
        float sat_delta = (float) (p_b->saturation - p_a->saturation);
        p_out->saturation = (uint8_t) (p_a->saturation + ratio * sat_delta);
    }
    else
    {
        p_out->saturation = p_a->saturation;
    }

    if (axis & FADE_AXIS_VAL)
    {
        float val_delta = (float) (p_b->value - p_a->value);
        p_out->value = p_a->value + (uint8_t) (ratio * val_delta);
    }
    else
    {
        p_out->value = p_a->value;
    }
}

/**
 * @private
 * Calculates a point on a cubic bezier curve at interpolation index, t.
 * 
 * This function assumes start, \f$ P_0 \f$, and end, \f$ P_3 \f$, points of \f$ (0,0) \f$ and \f$ (1,1) \f$ respectively.
 * The equation for the bezier curve is
 * \f[
 *  \vec{B}(t) = (1-t)^3 \vec{P}_0 + 3 (1-t)^2 t \vec{P}_1 + 3 (1-t) t^2 \vec{P}_2 + t^3 \vec{P}_3, \quad 0 \le t \le 1
 * \f]
 * or simplified using the assumptions for \f$ P_0 \f$ and \f$ P_3 \f$
 * \f[
 *  \vec{B}(t) = 3 (1-t)^2 t \vec{P}_1 + 3 (1-t) t^2 \vec{P}_2 + t^3, \quad 0 \le t \le 1
 * \f]
 * 
 * @param[in]  p_curve Pointer to the bezier control points.
 * @param      t       Interpolation index at which to calculate a point on the curve; 0 <= t <= 1.
 * @param[out] p_point Pointer to store the calculated point.
 */
static void cubic_bezier_calc(cubic_bezier_t const * const p_curve, float t, point_t * p_point)
{
    // Calculate the coefficients first because they have to be used twice.
    float t_ = 1.0f - t;

    float a1 = 3.0f * t_ * t_ * t;
    float a2 = 3.0f * t_ * t * t;
    float a3 = t * t * t;

    p_point->x = a1 * p_curve->p1.x + a2 * p_curve->p2.x + a3;
    p_point->y = a1 * p_curve->p1.y + a2 * p_curve->p2.y + a3;
}

/**
 * Parses a command string into a @ref pixelkey__keyframes__fade.
 * @param[in] p_str Pointer to the command string.
 * @return Pointer to the parsed keyframe or NULL on error.
 */
keyframe_base_t * keyframe_fade_parse(char * p_str)
{
    if (p_str == NULL)
    {
        return NULL;
    }

    keyframe_fade_t * p_fade = (keyframe_fade_t *) keyframe_fade_ctor(NULL);

    bool has_error = true;
    do
    {
        char * p_context = NULL;
        char * p_tok;

        if ((p_tok = strtok_r(p_str, " ", &p_context)) == NULL)
        {
            // Error: no arguments.
            break;
        }

        // Parse the period first
        float period = strtof(p_tok, NULL);
        if (period <= 0.0f)
        {
            // Period must be non-zero, positive number.
            // It is also set to zero on parse failure.
            break;
        }
        p_fade->args.period = period;

        if ((p_tok = strtok_r(NULL, " ", &p_context)) == NULL)
        {
            // Error: no more arguments, color list is not provided.
            break;
        }
        // else: parse the color list

        if (*p_tok == '&')
        {
            // Special case to push the current color onto the stack.
            p_fade->args.push_current = true;
            // Increment to skip the ampersand.
            p_tok++;
        }

        // Split the color list by ':'
        char * p_color_context = NULL;
        char * p_color_tok = strtok_r(p_tok, ":", &p_color_context);
        bool color_error = false;
        while (p_color_tok != NULL)
        {
            color_t color, hsv;
            if (!color_parse(p_color_tok, &color))
            {
                // Color parsing failed!
                color_error = true;
                break;
            }
            // else: Add the color to the list
            color_convert(COLOR_SPACE_HSV, &color, &hsv);
            if (p_fade->args.colors_len == KEYFRAME_FADE_COLORS_INPUT_MAX_LENGTH)
            {
                // Too many colors in the list
                color_error = true;
                break;
            }
            p_fade->args.colors[p_fade->args.colors_len++] = hsv.hsv;
        }

        // Exit parsing if an error occurred while parsing the colors.
        if (color_error)
        {
            break;
        }

        // Parse the fade type if provided.
        if ((p_tok = strtok_r(NULL, " ", &p_context)) == NULL)
        {
            // No more arguments, go ahead and exit.
            has_error = false;
            break;
        }

        if (strcmp(p_tok, "step") == 0)
        {
            p_fade->args.fade_type = FADE_TYPE_STEP;
        }
        else
        {
            p_fade->args.fade_type = FADE_TYPE_CUBIC;
            if (strcmp(p_tok, "linear") == 0)
            {
                p_fade->args.curve = cb_linear;
            }
            else if (strcmp(p_tok, "ease") == 0)
            {
                p_fade->args.curve = cb_ease;
            }
            else if (strcmp(p_tok, "ease-in") == 0)
            {
                p_fade->args.curve = cb_ease_in;
            }
            else if (strcmp(p_tok, "ease-out") == 0)
            {
                p_fade->args.curve = cb_ease_out;
            }
            else if (strcmp(p_tok, "ease-in-out") == 0)
            {
                p_fade->args.curve = cb_ease_in_out;
            }
            else if (memcmp(p_tok, "cubic(", 6) == 0)
            {
                // Move p_tok forward beyond the start of "cubic("
                p_tok  = &p_tok[6];

                // Find the closing parenthesis
                char * paren = strpbrk(p_tok, ")");
                if (paren == NULL)
                {
                    // Error: no closing parenthesis
                    break;
                }
                // else: replace it with a '\0' so we can tokenize the cubic points.
                *paren = '\0';

                char * p_point_tok = strtok(p_tok, ",");
                if (p_point_tok == NULL)
                {
                    // Error: no point list.
                    break;
                }
                float coord = strtof(p_tok, NULL);
                p_fade->args.curve.p1.x = coord;

                // Grab P1.Y
                p_point_tok = strtok(NULL, ",");
                if (p_point_tok == NULL)
                {
                    // Error: Not enough points in list.
                    break;
                }
                coord = strtof(p_tok, NULL);
                p_fade->args.curve.p1.y = coord;

                // Grab P2.X
                p_point_tok = strtok(NULL, ",");
                if (p_point_tok == NULL)
                {
                    // Error: Not enough points in list.
                    break;
                }
                coord = strtof(p_tok, NULL);
                p_fade->args.curve.p2.x = coord;

                // Grab P2.Y
                p_point_tok = strtok(NULL, ",");
                if (p_point_tok == NULL)
                {
                    // Error: Not enough points in list.
                    break;
                }
                coord = strtof(p_tok, NULL);
                p_fade->args.curve.p2.y = coord;

                // Make sure we are at the end of the list.
                if (strtok(NULL, ",") != NULL)
                {
                    // Error: Too many points in list.
                    break;
                }
            }
            else
            {
                // Error: unknown type.
                break;
            }
        }

        // Check to see if more arguments are available and break if so.
        if (strtok_r(NULL, " ", &p_context) != NULL)
        {
            // Error: too many arguments.
            break;
        }
    
        // Everything checked out so clear the error flag.
        has_error = false;
    } while (0);
    
    if (has_error)
    {
        // Cleanup on error.
        free(p_fade);
        return NULL;
    }
    else
    {
        return &p_fade->base;
    }
}

/**
 * Initialize a Fade keyframe with the appropriate keyframe_base_t and state values.
 * @param[in] p_fade Pointer to the fade keyframe to construct, or NULL to allocate a new one.
 * @return Pointer to the keyframe base portion of the fade keyframe.
 */
keyframe_base_t * keyframe_fade_ctor(keyframe_fade_t * p_fade)
{
    // If NULL, allocate a new set keyframe.
    if (p_fade == NULL)
    {
        p_fade = malloc(sizeof(keyframe_fade_t));
        memcpy(p_fade, &keyframe_fade_init, sizeof(*p_fade));
    }
    else
    {
        // Copy the base struct info (yes some of these fields are marked const... Just do it.)
        memcpy(&p_fade->base, &keyframe_fade_init.base, sizeof(keyframe_base_t));

        // Zero out the state
        memset(&p_fade->state, 0, sizeof(p_fade->state));
    }
    return &p_fade->base;
}

/** @} */
