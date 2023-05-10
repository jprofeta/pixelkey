/**
 * @file
 * @defgroup ringbuffer__internals Ring Buffer Internals
 * Simple ring buffer for queues.
 * @ingroup ringbuffer
 * @{
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "ring_buffer.h"
#include "hal_device.h"

/**
 * Initialize a ring buffer.
 * @param[in] p_buffer  Pointer to ring buffer control struct.
 * @param[in] ptr_array Array of pointers used as the backing store for the ring buffer.
 * @param     length    Number of elements available in ptr_array.
 */
void ring_buffer_init(ring_buffer_t * p_buffer, void * ptr_array, size_t length)
{
    p_buffer->p_data = ptr_array;
    p_buffer->length = length;
    p_buffer->head_idx = 0;
    p_buffer->tail_idx = 0;
}

/**
 * Gets a pointer to the next element in the buffer without removing it.
 * @param[in] p_buffer Pointer to the ring buffer control struct.
 * @return The pointer to the next element, or NULL if the buffer is empty.
 */
void * ring_buffer_peek(ring_buffer_t * p_buffer)
{
    if (p_buffer->head_idx == p_buffer->tail_idx)
    {
        return NULL;
    }
    else
    {
        return p_buffer->p_data[p_buffer->tail_idx];
    }
}

/**
 * Removes an element from the ring buffer.
 * @param[in]  p_buffer Pointer to the ring buffer control struct.
 * @param[out] p_element Pointer to copy the element's value to.
 * @return true on success, false if the buffer is empty.
 */
bool ring_buffer_pop(ring_buffer_t * p_buffer, void ** p_element)
{
    if (p_buffer->head_idx == p_buffer->tail_idx)
    {
        return false;
    }
    else
    {
        if (p_element != NULL)
        {
            *p_element = p_buffer->p_data[p_buffer->tail_idx];
        }

        if (p_buffer->tail_idx == p_buffer->length - 1)
        {
            p_buffer->tail_idx = 0;
        }
        else
        {
            p_buffer->tail_idx++;
        }

        return true;
    }
}

/**
 * Pushes a value onto the ring buffer.
 * @param[in] p_buffer Pointer to the ring buffer control struct.
 * @param[in] p_element Pointer to the element to push.
 * @return true on success, false if no room is available.
 */
bool ring_buffer_push(ring_buffer_t * p_buffer, void * p_element)
{
    size_t new_head = (p_buffer->head_idx + 1) % p_buffer->length;
    if (new_head == p_buffer->tail_idx)
    {
        return false;
    }
    else
    {
        p_buffer->p_data[p_buffer->head_idx] = p_element;
        p_buffer->head_idx = new_head;
        return true;
    }
}

/**
 * Gets whether or not the ring buffer is empty.
 * @param[in] p_buffer Pointer to the ring buffer control struct.
 * @return true on empty, false otherwise.
 */
bool ring_buffer_is_empty(ring_buffer_t * p_buffer)
{
    return (p_buffer->head_idx == p_buffer->tail_idx);
}

/**
 * Get the number of elements currently in the ring buffer.
 * @param[in] p_buffer Pointer to the ring buffer control struct.
 * @return Number of elements.
 */
size_t ring_buffer_count(ring_buffer_t * p_buffer)
{
    const int32_t n = (int32_t) p_buffer->head_idx - (int32_t) p_buffer->tail_idx;
    if (n < 0)
    {
        return (size_t)((int32_t)p_buffer->length + n + 1);
    }
    else
    {
        return (size_t)n;
    }
}

/** @} */
