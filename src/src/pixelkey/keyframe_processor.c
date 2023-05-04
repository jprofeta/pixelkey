
/**
 * @file
 * @defgroup pixelkey__keyframe_proc__internals PixelKey Keyframe Processor Internals
 * @ingroup pixelkey
 * @{
 */

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "hal_device.h"
#include "pixelkey.h"
#include "neopixel.h"

typedef struct st_keyframe_queue
{
    uint8_t           head;
    uint8_t           tail;
    keyframe_base_t * buffer[PIXELKEY_KEYFRAME_QUEUE_LENGTH];
} keyframe_queue_t;

static color_rgb_t       current_color[PIXELKEY_NEOPIXEL_COUNT] = {0};
static keyframe_queue_t  keyframe_queue[PIXELKEY_NEOPIXEL_COUNT] = {0};
static keyframe_base_t * current_keyframe[PIXELKEY_NEOPIXEL_COUNT] = {0};
static timestep_t        current_framecount[PIXELKEY_NEOPIXEL_COUNT] = {0};
static framerate_t       current_framerate = 0;

/**
 * Check if a keyframe queue is empty.
 * @param[in] p_queue Pointer to the queue.
 * @return true on empty, or false if not empty.
 */
static bool is_empty(keyframe_queue_t * const p_queue)
{
    return (p_queue->head == p_queue->tail);
}

/**
 * Push a keyframe onto a keyframe queue.
 * @param[in] p_queue Pointer to the queue.
 * @param[in] p_keyframe Pointer to the keyframe to enqueue.
 * @return true on success, or false if the queue was full.
 */
static bool push(keyframe_queue_t * p_queue, keyframe_base_t * p_keyframe)
{
    if (p_queue->head == ((p_queue->tail + 1) % PIXELKEY_KEYFRAME_QUEUE_LENGTH))
    {
        return false;
    }

    p_queue->buffer[p_queue->tail] = p_keyframe;
    p_queue->tail = (uint8_t) (((p_queue->tail + 1) % PIXELKEY_KEYFRAME_QUEUE_LENGTH));

    return true;
}

/**
 * Pop the first entry from a keyframe queue.
 * @param[in] p_queue Pointer to the queue.
 * @return pointer to the popped keyframe or NULL if empty.
 */
static keyframe_base_t * pop(keyframe_queue_t * p_queue)
{
    if (p_queue->head == p_queue->tail)
    {
        return NULL;
    }

    keyframe_base_t * p_keyframe = p_queue->buffer[p_queue->head];
    p_queue->head = (uint8_t) ((p_queue->head + 1) % PIXELKEY_KEYFRAME_QUEUE_LENGTH);

    return p_keyframe;
}

/**
 * Peek at a keyframe queue.
 * @param[in] p_queue Pointer to the queue.
 * @return pointer to the next available keyframe or NULL if empty.
 */
static keyframe_base_t * peek(keyframe_queue_t * p_queue)
{
    if (p_queue->head == p_queue->tail)
    {
        return NULL;
    }

    return p_queue->buffer[p_queue->head];
}

/**
 * Initialize a keyframe or keyframe group.
 * @param[in] p_keyframe Pointer to the keyframe to initialize.
 * @param[in] p_color    Pointer to the current rendered color.
 */
static void init_keyframe(keyframe_base_t * p_keyframe, color_rgb_t * p_color)
{
    if (p_keyframe->flags & KEYFRAME_FLAG_GROUP)
    {
        keyframe_group_t * p_grp = (keyframe_group_t *)p_keyframe;
        p_grp->current_child_idx = 0;
        init_keyframe(p_grp->children[0], p_color);
    }
    else
    {
        p_keyframe->p_api->render_init(p_keyframe, current_framerate, *p_color);
    }
    p_keyframe->flags |= KEYFRAME_FLAG_INITIALIZED;
}

/**
 * Performs a render of the current keyframes.
 * @retval PIXELKEY_ERROR_NONE Frame render was successful.
 * 
 * @todo Add support for scheduled keyframes.
 */
pixelkey_error_t pixelkey_render_frame(color_rgb_t * p_frame_buffer)
{
    // This should be called at the beginning of the frame period, directly after
    // the frame has been written to the neopixels.
    for (uint8_t i = 0; i < PIXELKEY_NEOPIXEL_COUNT; i++)
    {
        keyframe_base_t * p_kf = peek(&keyframe_queue[i]);
        if (p_kf != NULL)
        {
            (void)pop(&keyframe_queue[i]);
            current_keyframe[i] = p_kf;
            current_framecount[i] = 1;

            init_keyframe(p_kf, &current_color[i]);
            p_kf->flags |= KEYFRAME_FLAG_INITIALIZED;
        }
        else
        {
            p_kf = current_keyframe[i];
            current_framecount[i]++;
        }

        // Render a frame if a keyframe is available.
        if (p_kf != NULL)
        {
            bool finished = p_kf->p_api->render_frame(p_kf, current_framecount[i], &current_color[i]);
            if (finished)
            {
                // Decrement the repeat count only if positive.
                // This will allow for indefinite (negative) repeats and "0 is 1 repeat" behavior.
                if (p_kf->modifiers.repeat_count > 0)
                {
                    p_kf->modifiers.repeat_count--;
                }

                if (p_kf->modifiers.repeat_count == 0)
                {
                    current_keyframe[i] = NULL;
                    free(p_kf);
                }
                else
                {
                    // The keyframe is repeating. Prepare for a new render next frame.
                    current_framecount[i] = 0;
                    init_keyframe(p_kf, &current_color[i]);
                }
            }
        }
    }

    // Write the colors to the frame buffer
    for (uint8_t i = 0; i < PIXELKEY_NEOPIXEL_COUNT; i++)
    {
        p_frame_buffer[i] = current_color[i];
    }

    return PIXELKEY_ERROR_NONE;
}

/**
 * Pushes a keyframe into the queue for a given NeoPixel index.
 * @param     index      Index of NeoPixel.
 * @param[in] p_keyframe Pointer to keyframe to push.
 * @retval PIXELKEY_ERROR_NONE               Push was successful
 * @retval PIXELKEY_ERROR_INDEX_OUT_OF_RANGE Index is higher than maximum available NeoPixel.
 * @retval PIXELKEY_ERROR_BUFFER_FULL        Buffer if full for the given NeoPixel queue.
 */
pixelkey_error_t pixelkey_enqueue_keyframe(uint8_t index, keyframe_base_t * p_keyframe)
{
    if (index >= PIXELKEY_NEOPIXEL_COUNT)
    {
        return PIXELKEY_ERROR_INDEX_OUT_OF_RANGE;
    }
    if (!push(&keyframe_queue[index], p_keyframe))
    {
        return PIXELKEY_ERROR_BUFFER_FULL;
    }

    return PIXELKEY_ERROR_NONE;
}

/**
 * Sets the framerate used to render keyframes.
 * @param framerate The framerate to use.
 */
void pixelkey_framerate_set(framerate_t framerate)
{
    current_framerate = framerate;
}

/** @} */
