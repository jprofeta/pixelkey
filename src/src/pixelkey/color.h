#ifndef COLOR_H
#define COLOR_H

#include <stdint.h>

/**
 * @defgroup color Color
 * Color conversions and types.
 * @{
 */

/** Maximum value of an RGB color component. */
#define RGB_MAX            (255U)
/** Maximum value of an RGB color component as a float-32. */
#define RGB_MAX_F32        (255.0f)

/** Maximum value of hue. */
#define HUE_MAX            (359U)
/** Maximum value of hue as a float-32. */
#define HUE_MAX_F32        (359.0f)

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

/** Color represented in red-green-blue color space. */
typedef struct st_color_rgb
{
    uint8_t red;         ///< Red component.
    uint8_t green;       ///< Green component.
    uint8_t blue;        ///< Blue component. 
} color_rgb_t;

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

/**
 * Convert a color to a different color space.
 * @param[in]  to    Desired color space to convert to.
 * @param[in]  p_in  Pointer to the color in the original color space.
 * @param[out] p_out Pointer to the color for the desired color space.
 */
void color_convert(color_space_t to, color_t const * p_in, color_t * p_out);

/**
 * Convert a color to a different color space.
 * @param[in]  from  Color space to convert from.
 * @param[in]  to    Desired color space to convert to.
 * @param[in]  p_in  Pointer to the color in the original color space.
 * @param[out] p_out Pointer to the color for the desired color space.
 */
void color_convert2(color_space_t from, color_space_t to, color_kind_t const * p_in, color_kind_t * p_out);

/**
 * Parses a color from a string; must be NULL-terminated.
 * @param[in]  p_str       Pointer to the color string to parse; may be modified.
 * @param[out] p_color_out Pointer to store the parsed color.
 * @retval PIXELKEY_ERROR_NONE             Parse was successful.
 * @retval PIXELKEY_ERROR_INVALID_ARGUMENT The color string was malformed or a component was out of range.
 */
pixelkey_error_t color_parse(char * p_str, color_t * p_color_out);

/** @} */

#endif // COLOR_H
