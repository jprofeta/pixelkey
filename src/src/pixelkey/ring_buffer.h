#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * @file
 * @defgroup ringbuffer Ring Buffer
 * Simple ring buffer for queues.
 * @{
 */

/** Ring buffer control struct. */
typedef struct st_ring_buffer
{
    void ** p_data;  ///< Pointer to the underlying data region.
    size_t length;   ///< Number of elements in the buffer.
    size_t head_idx; ///< Index to the head of the ring buffer.
    size_t tail_idx; ///< Index to the tail of the ring buffer.
} ring_buffer_t;

void ring_buffer_init(ring_buffer_t * p_buffer, void * ptr_array, size_t length);
void * ring_buffer_peek(ring_buffer_t * p_buffer);
bool ring_buffer_pop(ring_buffer_t * p_buffer, void ** p_element);
bool ring_buffer_push(ring_buffer_t * p_buffer, void * p_element);
bool ring_buffer_is_empty(ring_buffer_t * p_buffer);
size_t ring_buffer_count(ring_buffer_t * p_buffer);

/** @} */

#endif
