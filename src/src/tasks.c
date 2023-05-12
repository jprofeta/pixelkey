/**
 * @file
 * @defgroup pixelkey__tasks PixelKey Tasks
 * @ingroup pixelkey
 * @{
 */


#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "hal_device.h"
#include "hal_tasks.h"
#include "pixelkey.h"
#include "neopixel.h"
#include "serial.h"

#include "hal_npdata_transfer.h"

// Allow missing prototypes in this file.
// The prototypes are auto-generated from the task list when they are used in hal_tasks.c.
// The idea is that they should not be called by anyone other than the task manager.
WARNING_DISABLE("missing-prototypes")

static void input_buffer_shift(uint32_t count, uint32_t length);

/** Write index of the input buffer. */
static size_t input_buffer_idx = 0;

/** Input buffer for received command data over USB. */
static uint8_t input_buffer[PIXELKEY_INPUT_COMMAND_BUFFER_LENGTH] = {0};

/**
 * Renders and queues a frame to be transferred at the next frame interval.
 */
void pixelkey_task_do_frame(void)
{
    color_rgb_t temp_frame[PIXELKEY_NEOPIXEL_COUNT] = {0};

    pixelkey_error_t err = pixelkey_keyframeproc_render_frame(temp_frame);

    if (err != PIXELKEY_ERROR_NONE)
    {
        /// @todo Log render error
        return;
    }

    while (npdata_status_get() == TRANSFER_STATUS_WORKING)
    {
        // Wait until the transfer has completed.
    }

    // Copy the rendered frame to the frame buffer.
    /// @todo Change this to a copy function in npdata?
    memcpy((void *)npdata_frame_buffer_get(), temp_frame, NPDATA_FRAME_BUFFER_SIZE);
}

/**
 * Processes data from the USB input buffer and parses into a command.
 */
void pixelkey_task_command_rx(void)
{
    char err_str[64];

    size_t read_length = PIXELKEY_INPUT_COMMAND_BUFFER_LENGTH - input_buffer_idx;
    serial()->read(&input_buffer[input_buffer_idx], &read_length);

    // Scan the input for NEW-LINE symbols.
    for (int i = read_length; i > 0; i--)
    {
        if ((input_buffer[input_buffer_idx] == (uint8_t) '\n')
            || (input_buffer[input_buffer_idx] == (uint8_t) '\r'))
        {
            input_buffer[input_buffer_idx] = (uint8_t) '\0';

            // Parse the command string.
            cmd_list_t * p_cmd_list = NULL;
            pixelkey_error_t parse_err = pixelkey_command_parse((char *)input_buffer, &p_cmd_list);

            if (parse_err != PIXELKEY_ERROR_NONE)
            {
                // Respond with an error.
                snprintf((char *)err_str, sizeof(err_str), "%d NAK\n", (int)parse_err);
                serial()->write((uint8_t *)err_str, strlen(err_str));
            }
            else
            {
                int cmd_num = 0;
                while (p_cmd_list != NULL)
                {
                    cmd_num++;
                    if (pixelkey_commandproc_push(p_cmd_list->p_cmd) != PIXELKEY_ERROR_NONE)
                    {
                        // Respond with an error.
                        snprintf((char *)err_str, 
                                    sizeof(err_str),
                                    "Partial queue of %d commands.\n%d NAK\n",
                                    cmd_num, (int)PIXELKEY_ERROR_BUFFER_FULL);
                        serial()->write((uint8_t *)err_str, strlen(err_str));
                        pixelkey_cmd_list_free(p_cmd_list);
                        p_cmd_list = NULL;
                    }
                    else
                    {
                        p_cmd_list = p_cmd_list->p_next;
                        tasks_queue(TASK_CMD_HANDLER);
                    }
                }
            }

            // Shift the input buffer down to remove the parsed command string.
            input_buffer_shift(input_buffer_idx, input_buffer_idx + i);

            // Reset the buffer index and continue the scan.
            input_buffer_idx = 0;
        }
        else
        {
            // Increment the index to the next received character.
            input_buffer_idx++;
        }
    }
}


/**
 * Shifts the input buffer down once a command is parsed.
 * @param count  Number of elements to remove.
 * @param length Current length of the input buffer
 */
static void input_buffer_shift(uint32_t count, uint32_t length)
{
    for (uint32_t i = 0, j = count; j < length; i++, j++)
    {
        input_buffer[i] = input_buffer[j];
    }
    input_buffer_idx -= count;
}

/**
 * @}
 */
