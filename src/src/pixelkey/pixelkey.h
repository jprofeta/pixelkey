
#ifndef KEYFRAME_H
#define KEYFRAME_H

#include <stdbool.h>
#include <stdint.h>

#include "color.h"

/** Signifies an argument is not used. */
#define ARG_NOT_USED(arg) ((void)(arg))

/** Minimum supported frame rate, frames per second. */
#define FRAMERATE_MIN  (1U)

/** 
 *  Maximum supported frame rate, frames per second.
 *  This is based on the resolutions for the RTC peripheral.
 */
#define FRAMERATE_MAX  (128U)

/** No more timesteps are required. */
#define TIMESTEP_FINISHED   ((timestep_t) 0U)

/** Indefinite time remaining. */
#define TIMESTEP_INDEFINITE ((timestep_t) UINT32_MAX)

typedef enum e_keyframe_modifier_type
{
    KEYFRAME_MODIFIER_TYPE_NONE,
    KEYFRAME_MODIFIER_TYPE_REPEATING,
    KEYFRAME_MODIFIER_TYPE_TIMED,
    KEYFRAME_MODIFIER_TYPE_GROUP,
} keyframe_modifier_type_t;

/** The base unit of time for animating keyframes; resolution of 1 frame. */
typedef uint32_t timestep_t;

/** Number of frames per second. */
typedef uint8_t framerate_t;

typedef struct st_keyframe_base keyframe_base_t;

/** Base API for all keyframe types. */
typedef struct st_keyframe_base_api
{
    /**
     * Renders a keyframe for the given time step.
     * @param[in]  p_keyframe  Pointer to the keyframe.
     * @param[in]  time        Current time step for animation.
     * @param[out] p_color_out Pointer to the rendered RGB color for this time step.
     * @return true if the keyframe has completed, false if more frames remain.
     */
    bool (* render_frame)(keyframe_base_t * const p_keyframe, timestep_t time, color_rgb_t * p_color_out);

    /**
     * Initialize the renderer for the keyframe.
     * @param[in] p_keyframe    Pointer to the keyframe.
     * @param[in] framerate     Framerate for keyframe rendering.
     * @param[in] current_color The current color being used.
     */
    void (* render_init)(keyframe_base_t * const p_keyframe, framerate_t framerate, color_rgb_t current_color);

    /**
     * Gets the type of modifier this keyframe represents.
     * @param[in] p_keyframe Pointer to the keyframe.
     * @return the type of keyframe modifier or KEYFRAME_MODIFIER_TYPE_NONE for a regular keyframe.
     */
    keyframe_modifier_type_t (* modifier_type_get)(keyframe_base_t * const p_keyframe);
} keyframe_base_api_t;

/** Base struct for all keyframe types, must be the first element in child structs. */
struct st_keyframe_base
{
    /** Pointer to the base API struct for the keyframe instance. */
    keyframe_base_api_t const * const p_api;
};

#endif // KEYFRAME_H
