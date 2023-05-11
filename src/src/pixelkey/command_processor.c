/**
 * @file
 * @defgroup pixelkey__commandproc__internals Command Processor Internals
 * @ingroup pixelkey
 * @{
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "hal_device.h"
#include "ring_buffer.h"

#include "pixelkey.h"
#include "pixelkey_errors.h"
#include "pixelkey_commands.h"

static cmd_t * cmd_buffer_data[PIXELKEY_COMMAND_BUFFER_LENGTH] = {0};

static ring_buffer_t cmd_buffer = {0};

/**
 * Initialize the command processor.
 */
void pixelkey_commandproc_init(void)
{
    ring_buffer_init(&cmd_buffer, &cmd_buffer_data, PIXELKEY_COMMAND_BUFFER_LENGTH);
}

/**
 * Pushes a command to the processor buffer.
 * @param[in] p_cmd Pointer to the command to queue.
 * @retval PIXELKEY_ERROR_BUFFER_FULL No more space in the command queue.
 * @retval PIXELKEY_ERROR_NONE        Command was pushed to the queue.
 */
pixelkey_error_t pixelkey_commandproc_push(cmd_t * p_cmd)
{
    
}

/** @} */
