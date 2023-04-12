
#include <stdint.h>
#include <math.h>
#include "pixelkey.h"

/** Number of degrees in each sector of the hue component. */
#define HUE_SECTOR_SIZE     (60U)
/** Number of degrees in each sector of the hue component as a float-32. */
#define HUE_SECTOR_SIZE_F32 (60.0f)

#ifndef max
/** Maximum of two values. */
#define max(a,b)    ((a) > (b) ? (a) : (b))
#endif

#ifndef min
/** Minimum of two values. */
#define min(a,b)    ((a) < (b) ? (a) : (b))
#endif

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
            uint16_t h = p_in->hsv.hue / HUE_SECTOR_SIZE;
            float s = (float) p_in->hsl.saturation / SATURATION_MAX_F32;
            float l = (float) p_in->hsl.lightness / LIGHTNESS_MAX_F32;

            float chroma = s * (1.0f - fabsf(2.0f * l - 1.0f));
            float x = chroma * (1.0f - fabsf((float)(h & 1) - 1.0f));
            float m = l - chroma / 2.0f;

            switch (h)
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
            uint16_t h = p_in->hsv.hue / HUE_SECTOR_SIZE;
            float s = (float) p_in->hsv.saturation / SATURATION_MAX_F32;
            float v = (float) p_in->hsv.value / VALUE_MAX_F32;

            float chroma = v * s;
            float x = chroma * (1.0f - fabsf((float)(h & 1) - 1.0f));
            float m = v - chroma;

            switch (h)
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

            uint8_t M = (uint8_t) max(rgb.red, max(rgb.green, rgb.blue));
            uint8_t m = (uint8_t) min(rgb.red, min(rgb.green, rgb.blue));
            uint8_t chroma = M - m;

            float h = 0;
            if (chroma > 0)
            {
                if (M == rgb.red)
                {
                    h = fmodf(((float) (rgb.green - rgb.blue)) / ((float) chroma), 6.0f);
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

            p_out->hue = (uint16_t) (HUE_SECTOR_SIZE_F32 * h);
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
            uint8_t chroma = M - m;

            float h = 0;
            if (chroma > 0)
            {
                if (M == rgb.red)
                {
                    h = fmodf(((float) (rgb.green - rgb.blue)) / ((float) chroma), 6.0f);
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

            p_out->hue = (uint16_t) (HUE_SECTOR_SIZE_F32 * h);
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
