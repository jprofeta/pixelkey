
#include <stdint.h>
#include <math.h>
#include "pixelkey.h"

#ifndef max
#define max(a,b)    ((a) < (b) ? (b) : (a))
#endif

#ifndef min
#define min(a,b)    ((a) < (b) ? (a) : (b))
#endif

/**
 * Converts a color to the RGB color space.
 * @param[in]  p_in  Pointer to the color to convert from.
 * @param[out] p_out Pointer to the color converted to RGB.
 */
static void color_convert_rgb(color_t const * p_in, color_t * p_out)
{
    p_out->color_space = COLOR_SPACE_RGB;

    switch (p_in->color_space)
    {
        case COLOR_SPACE_HSL:
        {
            uint16_t h = p_in->hsv.hue / 60;
            float s = (float) p_in->hsl.saturation / 100.0f;
            float l = (float) p_in->hsl.lightness / 100.0f;

            float chroma = s * (1.0f - fabsf(2.0f * l - 1.0f));
            float x = chroma * (1.0f - fabsf((float)(h & 1) - 1.0f));
            float m = l - chroma / 2.0f;

            switch (h)
            {
                case 0:
                    p_out->rgb.red = (uint8_t) (255.0f * (chroma + m));
                    p_out->rgb.green = (uint8_t) (255.0f * (x + m));
                    p_out->rgb.blue = (uint8_t) (255.0f * m);
                    break;
                case 1:
                    p_out->rgb.red = (uint8_t) (255.0f * (x + m));
                    p_out->rgb.green = (uint8_t) (255.0f * (chroma + m));
                    p_out->rgb.blue = (uint8_t) (255.0f * m);
                    break;
                case 2:
                    p_out->rgb.red = (uint8_t) (255.0f * m);
                    p_out->rgb.green = (uint8_t) (255.0f * (chroma + m));
                    p_out->rgb.blue = (uint8_t) (255.0f * (x + m));
                    break;
                case 3:
                    p_out->rgb.red = (uint8_t) (255.0f * m);
                    p_out->rgb.green = (uint8_t) (255.0f * (x + m));
                    p_out->rgb.blue = (uint8_t) (255.0f * (chroma + m));
                    break;
                case 4:
                    p_out->rgb.red = (uint8_t) (255.0f * (x + m));
                    p_out->rgb.green = (uint8_t) (255.0f * m);
                    p_out->rgb.blue = (uint8_t) (255.0f * (chroma + m));
                    break;
                default:
                    p_out->rgb.red = (uint8_t) (255.0f * (chroma + m));
                    p_out->rgb.green = (uint8_t) (255.0f * m);
                    p_out->rgb.blue = (uint8_t) (255.0f * (x + m));
                    break;
            }
        }
        break;
        case COLOR_SPACE_HSV:
        {
            uint16_t h = p_in->hsv.hue / 60;
            float s = (float) p_in->hsv.saturation / 100.0f;
            float v = (float) p_in->hsv.value / 100.0f;

            float chroma = v * s;
            float x = chroma * (1.0f - fabsf((float)(h & 1) - 1.0f));
            float m = v - chroma;

            switch (h)
            {
                case 0:
                    p_out->rgb.red = (uint8_t) (255.0f * (chroma + m));
                    p_out->rgb.green = (uint8_t) (255.0f * (x + m));
                    p_out->rgb.blue = (uint8_t) (255.0f * m);
                    break;
                case 1:
                    p_out->rgb.red = (uint8_t) (255.0f * (x + m));
                    p_out->rgb.green = (uint8_t) (255.0f * (chroma + m));
                    p_out->rgb.blue = (uint8_t) (255.0f * m);
                    break;
                case 2:
                    p_out->rgb.red = (uint8_t) (255.0f * m);
                    p_out->rgb.green = (uint8_t) (255.0f * (chroma + m));
                    p_out->rgb.blue = (uint8_t) (255.0f * (x + m));
                    break;
                case 3:
                    p_out->rgb.red = (uint8_t) (255.0f * m);
                    p_out->rgb.green = (uint8_t) (255.0f * (x + m));
                    p_out->rgb.blue = (uint8_t) (255.0f * (chroma + m));
                    break;
                case 4:
                    p_out->rgb.red = (uint8_t) (255.0f * (x + m));
                    p_out->rgb.green = (uint8_t) (255.0f * m);
                    p_out->rgb.blue = (uint8_t) (255.0f * (chroma + m));
                    break;
                default:
                    p_out->rgb.red = (uint8_t) (255.0f * (chroma + m));
                    p_out->rgb.green = (uint8_t) (255.0f * m);
                    p_out->rgb.blue = (uint8_t) (255.0f * (x + m));
                    break;
            }
        }
        break;
        default:
            *p_out = *p_in;
            break;
    }
}

/**
 * Converts a color to the HSV color space.
 * @param[in]  p_in  Pointer to the color to convert from.
 * @param[out] p_out Pointer to the color converted to HSV.
 */
static void color_convert_hsv(color_t const * p_in, color_t * p_out)
{
    p_out->color_space = COLOR_SPACE_HSV;

    switch (p_in->color_space)
    {
        case COLOR_SPACE_HSL:
        {
            float s = (float) p_in->hsl.saturation / 100.0f;
            float l = (float) p_in->hsl.lightness / 100.0f;

            float v = l + s * fminf(l, 1.0f - l);

            p_out->hsv.hue = p_in->hsl.hue; // Hue is the same in both HSV and HSL.
            p_out->hsv.value = (uint8_t) (100.0f * v);
            if (p_out->hsv.value == 0)
            {
                p_out->hsv.saturation = 0;
            }
            else
            {
                p_out->hsv.saturation = (uint8_t) (100.0f * 2.0f * (1.0f - l / v));
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
                    h = fmodf(((float) (rgb.green - rgb.blue)) / ((float) chroma), 6.0);
                }
                else if (M == rgb.green)
                {
                    h = ((float) (rgb.blue - rgb.red)) / ((float) chroma) + 2.0;
                }
                else if (M == rgb.blue)
                {
                    h = ((float) (rgb.red - rgb.green)) / ((float) chroma) + 4.0;
                }
            }
            // else: keep default of 0 deg.

            p_out->hsv.hue = (uint16_t) (60.0f * h);
            p_out->hsv.value = (uint8_t) ((100 * (uint16_t) M) / 255);  // Convert 0-255 -> 0-100

            if (M == 0) // Value == 0
            {
                p_out->hsv.saturation = 0;
            }
            else
            {
                p_out->hsv.saturation = (uint8_t) ((100 * (uint16_t) chroma) / ((uint16_t) M)); // No need to convert 0-255 here.
            }
        }
        break;
        default:
            *p_out = *p_in;
            break;
    }
}

/**
 * Converts a color to the HSL color space.
 * @param[in]  p_in  Pointer to the color to convert from.
 * @param[out] p_out Pointer to the color converted to HSL.
 */
static void color_convert_hsl(color_t const * p_in, color_t * p_out)
{
    p_out->color_space = COLOR_SPACE_HSL;

    switch (p_in->color_space)
    {
        case COLOR_SPACE_HSV:
        {
            float s = (float) p_in->hsv.saturation / 100.0f;
            float v = (float) p_in->hsv.value / 100.0f;

            float l = v * (1.0f - s / 2.0f);


            p_out->hsl.hue = p_in->hsv.hue; // Hue is the same in both HSV and HSL.
            p_out->hsl.lightness = (uint8_t) (100.0f * l);
            if (p_out->hsl.lightness == 0 || p_out->hsl.lightness == 100)
            {
                p_out->hsl.saturation = 0;
            }
            else
            {
                p_out->hsl.saturation = (uint8_t) (100.0f * (v - l) / fminf(l, 1.0 - l));
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
                    h = fmodf(((float) (rgb.green - rgb.blue)) / ((float) chroma), 6.0);
                }
                else if (M == rgb.green)
                {
                    h = ((float) (rgb.blue - rgb.red)) / ((float) chroma) + 2.0;
                }
                else if (M == rgb.blue)
                {
                    h = ((float) (rgb.red - rgb.green)) / ((float) chroma) + 4.0;
                }
            }
            // else: keep default of 0 deg.

            float l = ((float) M + (float) m) / (255.0f * 2.0f);

            p_out->hsl.hue = (uint16_t) (60.0f * h);
            p_out->hsl.lightness = (uint8_t) (100.0f * l);

            if (p_out->hsl.lightness == 0 || p_out->hsl.lightness == 100)
            {
                p_out->hsl.saturation = 0;
            }
            else
            {
                float chroma_f = (float) chroma / 255.0f;
                p_out->hsl.saturation = (uint8_t) (100.0f * chroma_f / (1.0f - fabsf(2.0f * l - 1.0f)));
            }
        }
        break;
        default:
            *p_out = *p_in;
            break;
    }
}

void color_convert(color_space_t to, color_t const * p_in, color_t * p_out)
{
    switch (to)
    {
        case COLOR_SPACE_RGB:
            color_convert_rgb(p_in, p_out);
            break;
        case COLOR_SPACE_HSV:
            color_convert_hsv(p_in, p_out);
            break;
        case COLOR_SPACE_HSL:
            color_convert_hsl(p_in, p_out);
            break;
    }
}
