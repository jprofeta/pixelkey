#ifndef SERIAL_H
#define SERIAL_H

/**
 * @file
 * @defgroup pixelkey__serial Serial Communications Abstraction
 * @ingroup pixelkey
 * @{
 */

#include <stdlib.h>
#include <stdint.h>

#include "pixelkey_errors.h"

/** Serial communications interface abstraction. */
typedef struct st_serial
{
    /**
     * Read data from the serial instance.
     * @param[out]    p_buffer      Pointer to store read bytes at.
     * @param[in,out] p_read_length Desired read length, updated with actual length after completion.
     * @retval PIXELKEY_ERROR_NONE                  Read was successful.
     * @retval PIXELKEY_ERROR_COMMUNICATION_ERROR   Communications error occurred.
     * @retval PIXELKEY_ERROR_INPUT_BUFFER_OVERFLOW Received data overflowed.
     */
    pixelkey_error_t (* read)(uint8_t * p_buffer, size_t * p_read_length);

    /**
     * Write data to the serial instance.
     * @param[out]    p_buffer      Pointer to write data from.
     * @param         write_length  Desired write length.
     * @retval PIXELKEY_ERROR_NONE                  Write was successful.
     * @retval PIXELKEY_ERROR_COMMUNICATION_ERROR   Communications error occurred.
     * @retval PIXELKEY_ERROR_BUFFER_FULL           Transmit buffer is full.
     */
    pixelkey_error_t (* write)(uint8_t * p_buffer, size_t write_length);

    /**
     * Flush the transmit buffer and wait for completion.
     * @retval PIXELKEY_ERROR_NONE                  Flush was successful.
     * @retval PIXELKEY_ERROR_COMMUNICATION_ERROR   Communications error occurred.
     */
    pixelkey_error_t (*flush)();
} serial_t;

serial_t const * serial(void);
void serial_register(serial_t const * p_instance);

/** @} */

#endif
