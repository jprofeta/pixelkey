#ifndef PIXELKEY_H
#define PIXELKEY_H

#include <stdbool.h>
#include <stdint.h>

#include "pixelkey_errors.h"
#include "color.h"

/**
 * @defgroup pixelkey PixelKey
 * @{
 */

/** Signifies an argument is not used. */
#define ARG_NOT_USED(arg) ((void)(arg))

/** Minimum supported frame rate, frames per second. */
#define FRAMERATE_MIN  (1U)

/**
 * Maximum supported frame rate, frames per second.
 * This is based on the resolutions for the RTC peripheral.
 */
#define FRAMERATE_MAX  (128U)

/** No more timesteps are required. */
#define TIMESTEP_FINISHED   ((timestep_t) 0U)

/** Indefinite time remaining. */
#define TIMESTEP_INDEFINITE ((timestep_t) UINT32_MAX)

/** Maximum allowed child keyframes. */
#define GROUP_CHILDREN_MAX_COUNT    (16)

/** Types of supported keyframe schedules. */
typedef enum e_schedule_type
{
    SCHEDULE_TYPE_NONE,     ///< No schedule.
    SCHEDULE_TYPE_FIXED,    ///< Schedule is fixed at a specific time.
    SCHEDULE_TYPE_RELATIVE, ///< Schedule is for a relative time in the future.
    SCHEDULE_TYPE_INTERVAL, ///< Schedule is for a variable interval of a start and end time.
} schedule_type_t;

/** Management flags for keyframes. */
typedef enum e_keyframe_flag
{
    KEYFRAME_FLAG_NONE = 0UL,                ///< No flags are set.
    KEYFRAME_FLAG_INITIALIZED = (1UL << 30), ///< The keyframe has been initialized.
    KEYFRAME_FLAG_GROUP = (1UL << 31),       ///< The keyframe is a group keyframe.
} keyframe_flag_t;

/** The base unit of time for animating keyframes; resolution of 1 frame. */
typedef uint32_t timestep_t;

/** Number of frames per second. */
typedef uint8_t framerate_t;

/** Base struct for all keyframes. */
typedef struct st_keyframe_base keyframe_base_t;    // Defined ahead for use in the api struct.

/** 
 * Base API for all keyframe types.
 */
typedef struct st_keyframe_base_api
{
    /**
     * Renders a keyframe for the given time step.
     * @param[in]  p_keyframe  Pointer to the keyframe.
     * @param      time        Current time step for animation.
     * @param[out] p_color_out Pointer to the rendered RGB color for this time step.
     * @return true if the keyframe has completed, false if more frames remain.
     */
    bool (* render_frame)(keyframe_base_t * const p_keyframe, timestep_t time, color_rgb_t * p_color_out);

    /**
     * Initialize the renderer for the keyframe.
     * @param[in] p_keyframe    Pointer to the keyframe.
     * @param     framerate     Framerate for keyframe rendering.
     * @param     current_color The current color being used.
     */
    void (* render_init)(keyframe_base_t * const p_keyframe, framerate_t framerate, color_rgb_t current_color);
} keyframe_base_api_t;

/** Provides scheduled time information for keyframes. */
typedef struct st_keyframe_schedule
{
    schedule_type_t type;           ///< Type of schedule modifier.
    union
    {
        uint64_t fixed_time;        ///< Fixed/absolute timestamp of keyframe start.
        uint64_t relative_time;     ///< Relative time to start the keyframe.
        /** Start/end times for an interval schedule. */
        struct
        {
            uint64_t start;         ///< Interval start time.
            uint64_t end;           ///< Interval end time.
            bool start_is_relative; ///< true for start time is a relative time, false indicates fixed/absolute time.
            bool end_is_relative;   ///< true for end time is a relative time, false indicates fixed/absolute time.
        } interval;
    };
} keyframe_schedule_t;

/** Base struct for all keyframe types, must be the first element in child structs. */
struct st_keyframe_base
{
    /** Pointer to the base API struct for the keyframe instance. */
    keyframe_base_api_t const * const p_api;
    /** 
     * Flags used for identification and rendering. 
     * Upper 16-bits are reserved for PixelKey processing.
     * Lower 16-bit may be used in child implementations.
     * */
    uint32_t flags;
    /** Modifiers applied to this keyframe. */
    struct
    {
        keyframe_schedule_t schedule;              ///< Schedule times for this keyframe.
        int32_t             repeat_count;          ///< Total number of times to render the keyframe; negative is indefinite.
        bool                schedule_is_repeating; ///< Indicates the schedule should repeat instead of the frame.
    } modifiers;
};

/** Storage and state for keyframe groups. */
typedef struct st_keyframe_group
{
    /** Keyframe base; MUST be the first entry in the struct. */
    keyframe_base_t base;

    uint8_t children_len;       ///< Number of child keyframes in this group.
    uint8_t current_child_idx;  ///< Index of the child keyframe being rendered.
    /** List of child keyframes in this group. */
    keyframe_base_t * const children[GROUP_CHILDREN_MAX_COUNT];
} keyframe_group_t;

pixelkey_error_t pixelkey_render_frame(color_rgb_t * p_frame_buffer);

pixelkey_error_t pixelkey_enqueue_keyframe(uint8_t index, keyframe_base_t * p_keyframe);

void pixelkey_framerate_set(framerate_t framerate);

/** @} */

#endif // PIXELKEY_H
