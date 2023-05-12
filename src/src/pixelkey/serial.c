/**
 * @file
 * @defgroup pixelkey__serial__internals Serial Communications Abstraction Internals
 * @ingroup pixelkey__serial
 * @{
 */

#include "serial.h"

static serial_t const * p_attached_serial_comms = NULL;

/**
 * Gets the currently registered serial instance.
 */
serial_t const * serial(void)
{
    return p_attached_serial_comms;
}

/**
 * Register a serial instance to be used.
 * @param[in] p_instance Pointer to the serial instance to register.
 */
void serial_register(serial_t const * p_instance)
{
    p_attached_serial_comms = p_instance;
}

/** @} */
