#ifndef COLOR_H
#define COLOR_H

#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @defgroup color Color
 * Color conversions and types.
 * @{
 */

/** Maximum value of an RGB color component. */
#define RGB_MAX            (255U)
/** Maximum value of an RGB color component as a float-32. */
#define RGB_MAX_F32        (255.0f)
/** Total range of RGB values. */
#define RGB_RANGE          (256U)
/** Total range of RGB values as a float-32. */
#define RGB_RANGE_F32      (256.0f)

/** Maximum value of hue. */
#define HUE_MAX            (359U)
/** Maximum value of hue as a float-32. */
#define HUE_MAX_F32        (359.0f)
/** Total range of hue values. */
#define HUE_RANGE          (360U)
/** Total range of hue values as a float-32. */
#define HUE_RANGE_F32      (360.0f)

/** Maximum value of saturation. */
#define SATURATION_MAX     (100U)
/** Maximum value of saturation as a float-32. */
#define SATURATION_MAX_F32 (100.0f)

/** Maximum value of the value component of HSV. */
#define VALUE_MAX          (100U)
/** Maximum value of the value component of HSV as a float-32. */
#define VALUE_MAX_F32      (100.0f)

/** Maximum value of lightness. */
#define LIGHTNESS_MAX      (100U)
/** Maximum value of lightness as a float-32. */
#define LIGHTNESS_MAX_F32  (100.0f)

/** Supported color spaces. */
typedef enum e_color_space
{
    COLOR_SPACE_RGB,     ///< Red-green-blue color space.
    COLOR_SPACE_HSV,     ///< Hue-saturation-value color space.
    COLOR_SPACE_HSL,     ///< Hue-saturation-lightness color space.
} color_space_t;

/**
 * Color represented in red-green-blue color space.
 * @note The colors are represented internally as [blue, red, green] to be
 *       binary compatible with the NeoPixel color format.
 */
typedef struct st_color_rgb
{
    uint8_t blue;        ///< Blue component.
    uint8_t red;         ///< Red component.
    uint8_t green;       ///< Green component.
} color_rgb_t;

static_assert(offsetof(color_rgb_t, blue) == 0, "color_rgb_t layout must be B-R-G for ease of use converting to NeoPixel format.");
static_assert(offsetof(color_rgb_t, red) == 1, "color_rgb_t layout must be B-R-G for ease of use converting to NeoPixel format.");
static_assert(offsetof(color_rgb_t, green) == 2, "color_rgb_t layout must be B-R-G for ease of use converting to NeoPixel format.");

/** Color represented in hue-saturation-value color space. */
typedef struct st_color_hsv
{
    uint16_t hue;        ///< Hue component; ranges from 0-359.
    uint8_t  saturation; ///< Saturation component; ranges from 0-100.
    uint8_t  value;      ///< Value component; ranges from 0-100.
} color_hsv_t;

/** Color represented in hue-saturation-lightness color space. */
typedef struct st_color_hsl
{
    uint16_t hue;        ///< Hue component; ranges from 0-359.
    uint8_t  saturation; ///< Saturation component; ranges from 0-100.
    uint8_t  lightness;  ///< Lightness component; ranges from 0-100.
} color_hsl_t;

/**
 * Union for the three color space representations.
 * @note Layout must match the union in color_t.
 */
typedef union u_color_kind
{
    /** Values in the RGB (red-green-blue) color space. */
    color_rgb_t rgb;
    /** Values in the HSV (hue-saturation-value) color space. */
    color_hsv_t hsv;
    /** Values in the HSL (hue-saturation-lightness) color space. */
    color_hsl_t hsl;
} color_kind_t;

/** Represents a color in a defined color space. */
typedef struct st_color
{
    /* !IMPORTANT! Keep this union at the top so color_t can be cast directly to color_kind_t. */
    union
    {
        /** Values in the RGB (red-green-blue) color space. */
        color_rgb_t rgb;
        /** Values in the HSV (hue-saturation-value) color space. */
        color_hsv_t hsv;
        /** Values in the HSL (hue-saturation-lightness) color space. */
        color_hsl_t hsl;
    };
    /** Color space being used for this color. */
    color_space_t color_space;
} color_t;

static_assert(offsetof(color_t, rgb) == offsetof(color_kind_t, rgb), "The layout of color_kind_t must match that of color_t.");
static_assert(offsetof(color_t, hsv) == offsetof(color_kind_t, hsv), "The layout of color_kind_t must match that of color_t.");
static_assert(offsetof(color_t, hsl) == offsetof(color_kind_t, hsl), "The layout of color_kind_t must match that of color_t.");
static_assert(offsetof(color_t, rgb) == 0, "The color representations must be at the top of the color_t struct.");

/**
 * @defgroup named_colors Named Colors
 * @{
 */
extern const color_t color_red;
extern const color_t color_orange;
extern const color_t color_yellow;
extern const color_t color_neon;
extern const color_t color_green;
extern const color_t color_seafoam;
extern const color_t color_cyan;
extern const color_t color_lightblue;
extern const color_t color_blue;
extern const color_t color_purple;
extern const color_t color_magenta;
extern const color_t color_pink;
extern const color_t color_white;
extern const color_t color_black;
extern const color_t color_off;

/** @} */

void color_convert(color_space_t to, color_t const * p_in, color_t * p_out);

void color_convert2(color_space_t from, color_space_t to, color_kind_t const * p_in, color_kind_t * p_out);

bool color_parse(char * p_str, color_t * p_color_out);

void color_gamma_correct(color_rgb_t * p_in, color_rgb_t * p_out);

void color_gamma_build(float gamma);

/** @} */

#endif // COLOR_H
