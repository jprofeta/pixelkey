
#ifndef KEYFRAME_H
#define KEYFRAME_H

#include <stdint.h>

/** Minimum supported frame rate, frames per second. */
#define FRAME_RATE_MIN  (1U)

/** 
 *  Maximum supported frame rate, frames per second.
 *  This is based on the resolutions for the RTC peripheral.
 */
#define FRAME_RATE_MAX  (128U)

/** Indefinite time remaining. */
#define TIMESTEP_INDEFINITE ((timestep_t) UINT32_MAX)

/** Maximum value of hue. */
#define HUE_MAX         (359U)

/** Maximum value of saturation. */
#define SATURATION_MAX  (100U)

/** Maximum value of the value component of HSV. */
#define VALUE_MAX       (100U)

/** Maximum value of lightness. */
#define LIGHTNESS_MAX   (100U)

/** The base unit of time for animating keyframes; resolution of 1 frame. */
typedef uint32_t timestep_t;

/** Number of frames per second. */
typedef uint8_t framerate_t;

/** Supported color spaces. */
typedef enum e_color_space
{
    COLOR_SPACE_RGB, ///< Red-green-blue color space.
    COLOR_SPACE_HSV, ///< Hue-saturation-value color space.
    COLOR_SPACE_HSL, ///< Hue-saturation-lightness color space.
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

/** Represents a color in a defined color space. */
typedef struct st_color
{
    /** Color space being used to represented the color. */
    color_space_t color_space;
    union
    {
        /** Values in the RGB (red-green-blue) color space. */
        color_rgb_t rgb;
        /** Values in the HSV (hue-saturation-value) color space. */
        color_hsv_t hsv;
        /** Values in the HSL (hue-saturation-lightness) color space. */
        color_hsl_t hsl;
    };
} color_t;

/** Base for all keyframe types. */
typedef struct st_keyframe_base
{
    /**
     * Renders a keyframe for the given time step.
     * @param[in]  p_keyframe  Pointer to the keyframe.
     * @param[in]  time        Current time step for animation.
     * @param[out] p_color_out Pointer to the rendered RGB color for this time step.
     * @return Number of remaining time steps in the keyframe.
     */
    timestep_t (* render_frame)(keyframe_base_t * const p_keyframe, timestep_t time, color_rgb_t * p_color_out);

    /**
     * Update the keyframe to use the new framerate.
     * @param[in] p_keyframe    Pointer to the keyframe.
     * @param[in] new_framerate The new framerate to use.
     */
    void (* framerate_update)(keyframe_base_t * const p_keyframe, framerate_t new_framerate);

    /**
     * Gets whether or not this keyframe is a scheduled modifier.
     * @param[in] p_keyframe Pointer to the keyframe.
     * @return true for a scheduled modifier, false otherwise.
     */
    bool (* is_scheduled)(keyframe_base_t * const p_keyframe);
} keyframe_base_t;

/**
 * Convert a color to a different color space.
 * @param[in]  to    Desired color space to convert to.
 * @param[in]  p_in  Pointer to the color in the original color space.
 * @param[out] p_out Pointer to the color for the desired color space.
 */
void color_convert(color_space_t to, color_t const * p_in, color_t * p_out);

#endif // KEYFRAME_H
