/**
 * @file
 * @defgroup color__internals Color Internals
 * @ingroup color
 * @{
 */

#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

#include "color.h"

/** Number of degrees in each sector of the hue component. */
#define HUE_SECTOR_SIZE     (60U)
/** Number of degrees in each sector of the hue component as a float-32. */
#define HUE_SECTOR_SIZE_F32 (60.0f)

#ifndef max
/** Maximum of two values. */
#define max(a,b)    ((a) >= (b) ? (a) : (b))
#endif

#ifndef min
/** Minimum of two values. */
#define min(a,b)    ((a) < (b) ? (a) : (b))
#endif

/** Maximum expected length of a hexadecimal color. */
#define COLOR_RGB_HEX_STR_LENGTH    (7U)

/** Red: #FF0000. */
const color_t color_red         = { .color_space = COLOR_SPACE_HSV, .hsv = { HUE(  0), 100, 100 } };

/** Orange: #FF8000. */
const color_t color_orange      = { .color_space = COLOR_SPACE_HSV, .hsv = { HUE( 30), 100, 100 } };

/** Yellow: #FFFF00. */
const color_t color_yellow      = { .color_space = COLOR_SPACE_HSV, .hsv = { HUE( 60), 100, 100 } };

/** Neon: #80FF00. */
const color_t color_neon        = { .color_space = COLOR_SPACE_HSV, .hsv = { HUE( 90), 100, 100 } };

/** Green: #00FF00. */
const color_t color_green       = { .color_space = COLOR_SPACE_HSV, .hsv = { HUE(120), 100, 100 } };

/** Seafoam: #00FF80. */
const color_t color_seafoam     = { .color_space = COLOR_SPACE_HSV, .hsv = { HUE(150), 100, 100 } };

/** Cyan: #00FFFF. */
const color_t color_cyan        = { .color_space = COLOR_SPACE_HSV, .hsv = { HUE(180), 100, 100 } };

/** Light Blue: #0080FF. */
const color_t color_lightblue   = { .color_space = COLOR_SPACE_HSV, .hsv = { HUE(210), 100, 100 } };

/** Blue: #0000FF. */
const color_t color_blue        = { .color_space = COLOR_SPACE_HSV, .hsv = { HUE(240), 100, 100 } };

/** Purple: #8000FF. */
const color_t color_purple      = { .color_space = COLOR_SPACE_HSV, .hsv = { HUE(270), 100, 100 } };

/** Magenta: #FF00FF. */
const color_t color_magenta     = { .color_space = COLOR_SPACE_HSV, .hsv = { HUE(300), 100, 100 } };

/** Pink: #FF0080. */
const color_t color_pink        = { .color_space = COLOR_SPACE_HSV, .hsv = { HUE(330), 100, 100 } };

/** White: #FFFFFF. */
const color_t color_white       = { .color_space = COLOR_SPACE_HSV, .hsv = { HUE(  0),   0, 100 } };

/** Black: #000000. */
const color_t color_black       = { .color_space = COLOR_SPACE_HSV, .hsv = { HUE(  0),   0,   0 } };

/** Off: #000000. */
const color_t color_off         = { .color_space = COLOR_SPACE_HSV, .hsv = { HUE(  0),   0,   0 } };

static const struct named_color
{
    char const * const    name;  ///< Color name stringZ.
    color_t const * const color; ///< Pointer to color.
} named_colors[] = 
{
    { "red",       &color_red },
    { "orange",    &color_orange },
    { "yellow",    &color_yellow },
    { "neon",      &color_neon },
    { "green",     &color_green },
    { "seafoam",   &color_seafoam },
    { "cyan",      &color_cyan },
    { "lightblue", &color_lightblue },
    { "blue",      &color_blue },
    { "purple",    &color_purple },
    { "magenta",   &color_magenta },
    { "pink",      &color_pink },
    { "white",     &color_white },
    { "black",     &color_black },
    { "off",       &color_off },
    { NULL, NULL }
}; /**< List of named colors. Pointers are NULL terminated at end of the list. */

/** Lookup table for gamma correction values. */
static uint8_t gamma_table[RGB_MAX + 1] = {0};

static int parse_next_hex_byte(char ** p_str);
static int parse_next_uint(char * p_str, int min, int max);
static float parse_next_f32(char * p_str, float min, float max);

/**
 * Converts a color to the RGB color space.
 * @param[in]  from  Color space to convert from.
 * @param[in]  p_in  Pointer to the color to convert from.
 * @param[out] p_out Pointer to the color converted to RGB.
 */
static void color_convert_rgb(color_space_t from, color_kind_t const * p_in, color_rgb_t * p_out)
{
    switch (from)
    {
        case COLOR_SPACE_HSL:
        {
            float h_f32 = HUE_FP_TO_F32(p_in->hsv.hue) / HUE_SECTOR_SIZE_F32;
            uint16_t h_sector = (uint16_t)(h_f32);
            float s = (float) p_in->hsl.saturation / SATURATION_MAX_F32;
            float l = (float) p_in->hsl.lightness / LIGHTNESS_MAX_F32;

            float chroma = s * (1.0f - fabsf(2.0f * l - 1.0f));
            float x = chroma * (1.0f - fabsf(fmodf(h_f32, 2.0f) - 1.0f));
            float m = l - chroma / 2.0f;

            switch (h_sector)
            {
                case 0:
                    p_out->red = (uint8_t) (RGB_MAX_F32 * (chroma + m));
                    p_out->green = (uint8_t) (RGB_MAX_F32 * (x + m));
                    p_out->blue = (uint8_t) (RGB_MAX_F32 * m);
                    break;
                case 1:
                    p_out->red = (uint8_t) (RGB_MAX_F32 * (x + m));
                    p_out->green = (uint8_t) (RGB_MAX_F32 * (chroma + m));
                    p_out->blue = (uint8_t) (RGB_MAX_F32 * m);
                    break;
                case 2:
                    p_out->red = (uint8_t) (RGB_MAX_F32 * m);
                    p_out->green = (uint8_t) (RGB_MAX_F32 * (chroma + m));
                    p_out->blue = (uint8_t) (RGB_MAX_F32 * (x + m));
                    break;
                case 3:
                    p_out->red = (uint8_t) (RGB_MAX_F32 * m);
                    p_out->green = (uint8_t) (RGB_MAX_F32 * (x + m));
                    p_out->blue = (uint8_t) (RGB_MAX_F32 * (chroma + m));
                    break;
                case 4:
                    p_out->red = (uint8_t) (RGB_MAX_F32 * (x + m));
                    p_out->green = (uint8_t) (RGB_MAX_F32 * m);
                    p_out->blue = (uint8_t) (RGB_MAX_F32 * (chroma + m));
                    break;
                default:
                    p_out->red = (uint8_t) (RGB_MAX_F32 * (chroma + m));
                    p_out->green = (uint8_t) (RGB_MAX_F32 * m);
                    p_out->blue = (uint8_t) (RGB_MAX_F32 * (x + m));
                    break;
            }
        }
        break;
        case COLOR_SPACE_HSV:
        {
            float h_f32 = HUE_FP_TO_F32(p_in->hsv.hue) / HUE_SECTOR_SIZE_F32;
            uint16_t h_sector = (uint16_t)(h_f32);
            float s = (float) p_in->hsv.saturation / SATURATION_MAX_F32;
            float v = (float) p_in->hsv.value / VALUE_MAX_F32;

            float chroma = v * s;
            float x = chroma * (1.0f - fabsf(fmodf(h_f32, 2.0f) - 1.0f));
            float m = v - chroma;

            switch (h_sector)
            {
                case 0:
                    p_out->red = (uint8_t) (RGB_MAX_F32 * v);
                    p_out->green = (uint8_t) (RGB_MAX_F32 * (x + m));
                    p_out->blue = (uint8_t) (RGB_MAX_F32 * m);
                    break;
                case 1:
                    p_out->red = (uint8_t) (RGB_MAX_F32 * (x + m));
                    p_out->green = (uint8_t) (RGB_MAX_F32 * v);
                    p_out->blue = (uint8_t) (RGB_MAX_F32 * m);
                    break;
                case 2:
                    p_out->red = (uint8_t) (RGB_MAX_F32 * m);
                    p_out->green = (uint8_t) (RGB_MAX_F32 * v);
                    p_out->blue = (uint8_t) (RGB_MAX_F32 * (x + m));
                    break;
                case 3:
                    p_out->red = (uint8_t) (RGB_MAX_F32 * m);
                    p_out->green = (uint8_t) (RGB_MAX_F32 * (x + m));
                    p_out->blue = (uint8_t) (RGB_MAX_F32 * v);
                    break;
                case 4:
                    p_out->red = (uint8_t) (RGB_MAX_F32 * (x + m));
                    p_out->green = (uint8_t) (RGB_MAX_F32 * m);
                    p_out->blue = (uint8_t) (RGB_MAX_F32 * v);
                    break;
                default:
                    p_out->red = (uint8_t) (RGB_MAX_F32 * v);
                    p_out->green = (uint8_t) (RGB_MAX_F32 * m);
                    p_out->blue = (uint8_t) (RGB_MAX_F32 * (x + m));
                    break;
            }
        }
        break;
        default:
            *p_out = p_in->rgb;
            break;
    }
}

/**
 * Converts a color to the HSV color space.
 * @param[in]  from  Color space to convert from.
 * @param[in]  p_in  Pointer to the color to convert from.
 * @param[out] p_out Pointer to the color converted to HSV.
 */
static void color_convert_hsv(color_space_t from, color_kind_t const * p_in, color_hsv_t * p_out)
{
    switch (from)
    {
        case COLOR_SPACE_HSL:
        {
            float s = (float) p_in->hsl.saturation / SATURATION_MAX_F32;
            float l = (float) p_in->hsl.lightness / LIGHTNESS_MAX_F32;

            float v = l + s * fminf(l, 1.0f - l);

            p_out->hue = p_in->hsl.hue; // Hue is the same in both HSV and HSL.
            p_out->value = (uint8_t) (VALUE_MAX_F32 * v);
            if (p_out->value == 0)
            {
                p_out->saturation = 0;
            }
            else
            {
                p_out->saturation = (uint8_t) (SATURATION_MAX_F32 * 2.0f * (1.0f - l / v));
            }
        }
        break;
        case COLOR_SPACE_RGB:
        {
            const color_rgb_t rgb = p_in->rgb;

            uint8_t M = (uint8_t) max(max(rgb.green, rgb.blue), rgb.red);
            uint8_t m = (uint8_t) min(min(rgb.green, rgb.blue), rgb.red);
            uint8_t chroma = (uint8_t)(M - m);  // This cast is here just to shut-up GCC when running unit tests.

            float h = 0;
            if (chroma > 0)
            {
                if (M == rgb.red)
                {
                    h = ((float) (rgb.green - rgb.blue)) / ((float) chroma);
                    // Correct if this wraps around to negative hue.
                    if (h < 0)
                    {
                        h = 6.0f + h;
                    }
                }
                else if (M == rgb.green)
                {
                    h = ((float) (rgb.blue - rgb.red)) / ((float) chroma) + 2.0f;
                }
                else if (M == rgb.blue)
                {
                    h = ((float) (rgb.red - rgb.green)) / ((float) chroma) + 4.0f;
                }
            }
            // else: keep default of 0 deg.

            p_out->hue = HUE_F32(HUE_SECTOR_SIZE_F32 * h);
            p_out->value = (uint8_t) ((VALUE_MAX * (uint16_t) M) / RGB_MAX);  // Convert 0-255 -> 0-100

            if (M == 0) // Value == 0
            {
                p_out->saturation = 0;
            }
            else
            {
                p_out->saturation = (uint8_t) ((SATURATION_MAX * (uint16_t) chroma) / ((uint16_t) M)); // No need to convert 0-255 here.
            }
        }
        break;
        default:
            *p_out = p_in->hsv;
            break;
    }
}

/**
 * Converts a color to the HSL color space.
 * @param[in]  from  Color space to convert from.
 * @param[in]  p_in  Pointer to the color to convert from.
 * @param[out] p_out Pointer to the color converted to HSL.
 */
static void color_convert_hsl(color_space_t from, color_kind_t const * p_in, color_hsl_t * p_out)
{
    switch (from)
    {
        case COLOR_SPACE_HSV:
        {
            float s = (float) p_in->hsv.saturation / SATURATION_MAX_F32;
            float v = (float) p_in->hsv.value / VALUE_MAX_F32;

            float l = v * (1.0f - s / 2.0f);


            p_out->hue = p_in->hsv.hue; // Hue is the same in both HSV and HSL.
            p_out->lightness = (uint8_t) (LIGHTNESS_MAX_F32 * l);
            if (p_out->lightness == 0 || p_out->lightness == LIGHTNESS_MAX)
            {
                p_out->saturation = 0;
            }
            else
            {
                p_out->saturation = (uint8_t) (SATURATION_MAX_F32 * (v - l) / fminf(l, 1.0f - l));
            }

        }
        break;
        case COLOR_SPACE_RGB:
        {
            const color_rgb_t rgb = p_in->rgb;

            uint8_t M = (uint8_t) max(rgb.red, max(rgb.green, rgb.blue));
            uint8_t m = (uint8_t) min(rgb.red, min(rgb.green, rgb.blue));
            uint8_t chroma = (uint8_t)(M - m);  // This cast is here just to shut-up GCC when running unit tests.

            float h = 0;
            if (chroma > 0)
            {
                if (M == rgb.red)
                {
                    h = ((float) (rgb.green - rgb.blue)) / ((float) chroma);
                    // Correct if this wraps around to negative hue.
                    if (h < 0)
                    {
                        h = 6.0f + h;
                    }
                }
                else if (M == rgb.green)
                {
                    h = ((float) (rgb.blue - rgb.red)) / ((float) chroma) + 2.0f;
                }
                else if (M == rgb.blue)
                {
                    h = ((float) (rgb.red - rgb.green)) / ((float) chroma) + 4.0f;
                }
            }
            // else: keep default of 0 deg.

            float l = ((float) M + (float) m) / (RGB_MAX_F32 * 2.0f);

            p_out->hue = HUE_F32(HUE_SECTOR_SIZE_F32 * h);
            p_out->lightness = (uint8_t) (LIGHTNESS_MAX_F32 * l);

            if (p_out->lightness == 0 || p_out->lightness == LIGHTNESS_MAX)
            {
                p_out->saturation = 0;
            }
            else
            {
                float chroma_f = (float) chroma / RGB_MAX_F32;
                p_out->saturation = (uint8_t) (SATURATION_MAX_F32 * chroma_f / (1.0f - fabsf(2.0f * l - 1.0f)));
            }
        }
        break;
        default:
            *p_out = p_in->hsl;
            break;
    }
}

/**
 * Convert a color to a different color space.
 * @param      to    Desired color space to convert to.
 * @param[in]  p_in  Pointer to the color in the original color space.
 * @param[out] p_out Pointer to the color for the desired color space.
 */
void color_convert(color_space_t to, color_t const * p_in, color_t * p_out)
{
    p_out->color_space = to;

    switch (to)
    {
        case COLOR_SPACE_RGB:
            color_convert_rgb(p_in->color_space, (color_kind_t const *)p_in, &p_out->rgb);
            break;
        case COLOR_SPACE_HSV:
            color_convert_hsv(p_in->color_space, (color_kind_t const *)p_in, &p_out->hsv);
            break;
        case COLOR_SPACE_HSL:
            color_convert_hsl(p_in->color_space, (color_kind_t const *)p_in, &p_out->hsl);
            break;
    }
}

/**
 * Convert a color to a different color space.
 * @param      from  Color space to convert from.
 * @param      to    Desired color space to convert to.
 * @param[in]  p_in  Pointer to the color in the original color space.
 * @param[out] p_out Pointer to the color for the desired color space.
 */
void color_convert2(color_space_t from, color_space_t to, color_kind_t const * p_in, color_kind_t * p_out)
{
    switch (to)
    {
        case COLOR_SPACE_RGB:
            color_convert_rgb(from, p_in, &p_out->rgb);
            break;
        case COLOR_SPACE_HSV:
            color_convert_hsv(from, p_in, &p_out->hsv);
            break;
        case COLOR_SPACE_HSL:
            color_convert_hsl(from, p_in, &p_out->hsl);
            break;
    }
}

/**
 * Parses the next hexadecimal byte from p_str and increments p_str.
 * @param[in,out] p_str Pointer to a string pointer to parse.
 * @return The next hexadecimal value parsed from p_str.
 */
static int parse_next_hex_byte(char ** p_str)
{
    if (p_str == NULL) { return 0; }

    char hex_str[] = { 0, 0, 0 };
    memcpy(hex_str, *p_str, 2);
    (*p_str)++;
    (*p_str)++;
    return (int) strtoul(hex_str, NULL, 16);
}

/**
 * Parses the next integer in a list separated by commas.
 * @param[in,out] p_str Pointer to the string to parse, or NULL to continue from the last string.
 * @param         min   Minumum accepted value (inclusive).
 * @param         max   Maximum accepted value (exclusive).
 * @return the parsed integer or -1 on error or out of range.
 */
static int parse_next_uint(char * p_str, int min, int max)
{
    char * p_tok = strtok(p_str, ",");
    if (p_tok == NULL)
    {
        return -1;
    }
    int value = atoi(p_tok);
    if (value < min || value >= max)
    {
        return -1;
    }
    return value;
}

/**
 * Parses the next float in a list separated by commas.
 * @param[in,out] p_str Pointer to the string to parse, or NULL to continue from the last string.
 * @param         min   Minimum accepted value (inclusive).
 * @param         max   Maximum accepted value (exclusive).
 * @return the parsed float or NaN on error or out of range.
*/
static float parse_next_f32(char * p_str, float min, float max)
{
    char * p_tok = strtok(p_str, ",");
    if (p_tok == NULL)
    {
        return -1;
    }
    float value = strtof(p_tok, NULL);
    if (value < min || value >= max)
    {
        return NAN;
    }
    return value;
}

/**
 * Parses a color from a string; must be NULL-terminated.
 * @param[in]  p_str       Pointer to the color string to parse; may be modified.
 * @param[out] p_color_out Pointer to store the parsed color.
 * @return true on success, false on failure.
 */
bool color_parse(char * p_str, color_t * p_color_out)
{
    if (p_str == NULL)
    {
        return false;
    }

    switch (*p_str)
    {
        case '#':
        {
            if (strlen(p_str) < COLOR_RGB_HEX_STR_LENGTH)
            {
                return false;
            }

            p_color_out->color_space = COLOR_SPACE_RGB;
            p_str++;    // Skip the indicator.

            p_color_out->rgb.red = (uint8_t) parse_next_hex_byte(&p_str);
            p_color_out->rgb.green = (uint8_t) parse_next_hex_byte(&p_str);
            p_color_out->rgb.blue = (uint8_t) parse_next_hex_byte(&p_str);
        }
        break;
        case '%':
        {
            p_color_out->color_space = COLOR_SPACE_RGB;
            p_str++;    // Skip the indicator.

            int pct_value = 0;

            pct_value = parse_next_uint(p_str, 0, 100);
            if (pct_value < 0) { return false; }
            p_color_out->rgb.red = (uint8_t) (UINT8_MAX * pct_value / 100);

            pct_value = parse_next_uint(NULL, 0, 100);
            if (pct_value < 0) { return false; }
            p_color_out->rgb.green = (uint8_t) (UINT8_MAX * pct_value / 100);

            pct_value = parse_next_uint(NULL, 0, 100);
            if (pct_value < 0) { return false; }
            p_color_out->rgb.blue = (uint8_t) (UINT8_MAX * pct_value / 100);

            if (strtok(NULL, ",") != NULL)
            {
                // Too many color parts.
                return false;
            }
        }
        break;
        case '!':
        {
            bool is_hsl = false;

            p_str++; // Skip the first indicator.
            if (*p_str == '!')
            {
                // This is an HSL color.
                p_color_out->color_space = COLOR_SPACE_HSL;
                p_str++; // Skip the second indicator.
                is_hsl = true;
            }
            else
            {
                // This is an HSV color.
                p_color_out->color_space = COLOR_SPACE_HSV;
            }

            float parsed_value_f32 = parse_next_f32(p_str, 0.0, HUE_RANGE_F32);
            if (isnan(parsed_value_f32)) { return false; }
            if (is_hsl)
            {
                p_color_out->hsl.hue = HUE_F32(parsed_value_f32);
            }
            else
            {
                p_color_out->hsv.hue = HUE_F32(parsed_value_f32);
            }

            int32_t parsed_value = parse_next_uint(NULL, 0, 100);
            if (is_hsl)
            {
                if (parsed_value < 0)
                {
                    p_color_out->hsl.saturation = SATURATION_MAX;
                    p_color_out->hsl.lightness = LIGHTNESS_MAX;
                    return true;
                }
                else
                {
                    p_color_out->hsl.saturation = (uint8_t) parsed_value;
                }
            }
            else
            {if (parsed_value < 0)
                {
                    p_color_out->hsv.saturation = SATURATION_MAX;
                    p_color_out->hsv.value = VALUE_MAX;
                    return true;
                }
                else
                {
                    p_color_out->hsv.saturation = (uint8_t) parsed_value;
                }
            }

            parsed_value = parse_next_uint(NULL, 0, 100);
            if (is_hsl)
            {
                if (parsed_value < 0)
                {
                    return false;
                }
                else
                {
                    p_color_out->hsl.lightness = (uint8_t) parsed_value;
                }
            }
            else
            {
                if (parsed_value < 0)
                {
                    // Allow shortcut to value. The saturation is actually the value to use and assume 100% saturation.
                    p_color_out->hsv.value = p_color_out->hsv.saturation;
                    p_color_out->hsv.saturation = SATURATION_MAX;
                }
                else
                {
                    p_color_out->hsv.value = (uint8_t) parsed_value;
                }
            }

            if (strtok(NULL, ",") != NULL)
            {
                // Too many color parts.
                return false;
            }
        }
        break;
        default:
        {
            // Assume this is a named color.
            for (size_t i = 0; named_colors[i].name != NULL; i++ )
            {
                if (strcmp(p_str, named_colors[i].name) == 0)
                {
                    *p_color_out = *named_colors[i].color;
                    return true;
                }
            }

            // The color isn't in the named color list.
            return false;
        }
        break;
    }

    return true;
}

/**
 * Applies gamma correction to an RGB color.
 * @param[in,out] p_in  The color to correct.
 * @param[out]    p_out Pointer to write the corrected color, if NULL p_in is edited in place.
 */
void color_gamma_correct(color_rgb_t * p_in, color_rgb_t * p_out)
{
    if (p_out == NULL)
    {
        p_out = p_in;
    }

    p_out->red = gamma_table[p_in->red];
    p_out->green = gamma_table[p_in->green];
    p_out->blue = gamma_table[p_in->blue];
}

/**
 * Updates the gamma table with the provided correction factor.
 * @param gamma The gamma correction factor to use.
 * 
 * Builds the gamma table using the equation 
 * @f[
 *  C_\gamma = 255 * \left( \frac{C}{255} \right)^\gamma + \frac{1}{2}
 * @f]
 */
void color_gamma_build(float gamma)
{
    for (size_t i = 0; i < sizeof(gamma_table)/sizeof(gamma_table[0]); i++)
    {
        gamma_table[i] = (uint8_t)(RGB_MAX_F32 * powf(((float) i) / RGB_MAX_F32, gamma) + 0.5);
    }
}

/** @} */
