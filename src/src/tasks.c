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
#include <ctype.h>

#include "hal_device.h"
#include "hal_tasks.h"
#include "pixelkey.h"
#include "neopixel.h"
#include "serial.h"
#include "config.h"

#include "hal_npdata_transfer.h"

/** Position to begin looking for the CSI end character. */
#define CSI_CHAR_END_START      (2U)
/** Minimum value for the CSI end character. */
#define CSI_CHAR_END_MIN_VALUE  (0x40U)
/** Maximum value of the CSI end character. */
#define CSI_CHAR_END_MAX_VALUE  (0x7EU)

// Allow missing prototypes in this file.
// The prototypes are auto-generated from the task list when they are used in hal_tasks.c.
// The idea is that they should not be called by anyone other than the task manager.
WARNING_DISABLE("missing-prototypes")

static void echo_str(uint8_t * str, size_t length);

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

    LOG_TIME_START(DIAG_TIMING_FRAME_RENDER);
    pixelkey_error_t err = pixelkey_keyframeproc_render_frame(temp_frame);
    LOG_TIME(DIAG_TIMING_FRAME_RENDER);

    if (err != PIXELKEY_ERROR_NONE)
    {
        LOG_SIGNAL(DIAG_SIGNAL_RENDER_ERROR);
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
 * Sends initial strings for a connected terminal.
 */
void pixelkey_task_terminal_connected(void)
{
    pixelkey_commandproc_terminal_connected();
}

/**
 * Sends the command prompt.
 */
void pixelkey_task_command_prompt(void)
{
    pixelkey_commandproc_send_prompt();
}

/**
 * Processes data from the USB input buffer and parses into a command.
 */
void pixelkey_task_command_rx(void)
{
    char err_str[64];
    static int csi_start = -1;

    size_t read_length = PIXELKEY_INPUT_COMMAND_BUFFER_LENGTH - input_buffer_idx;
    serial()->read(&input_buffer[input_buffer_idx], &read_length);

    // Scan the input for NEW-LINE symbols.
    for (size_t i = read_length; i > 0; i--)
    {
        if (csi_start >= 0)
        {
            uint8_t c = input_buffer[input_buffer_idx];
            if ((input_buffer_idx - (size_t)csi_start) >= CSI_CHAR_END_START
                    && c >= CSI_CHAR_END_MIN_VALUE && c <= CSI_CHAR_END_MAX_VALUE)
            {
                // This is the end of the CSI sequence, throw it away.
                for (size_t j = 0; j < i - 1; j++)
                {
                    input_buffer[(size_t)csi_start + j] = input_buffer[input_buffer_idx + j + 1];
                }
                input_buffer_idx = (size_t)csi_start;
                csi_start = -1;
            }
            else
            {
                input_buffer_idx++;
            }
        }
        else if (input_buffer[input_buffer_idx] == '\x1B')
        {
            // Ignore any ASCII escape sequences
            csi_start = (int)input_buffer_idx;
            input_buffer_idx++;
        }
        else if (input_buffer[input_buffer_idx] == (uint8_t) '\b')
        {
            // Check the current index to make sure we don't go negative.
            if (input_buffer_idx > 0)
            {
                // Handle backspace
                char backspace_seq[] = "\b\x1B[0K"; // \b normally just moves the cursor back so throw in the escape code to clear the line.
                echo_str((uint8_t *)backspace_seq, sizeof(backspace_seq) - 1);

                // Decrement the write index and shift the buffer down.
                input_buffer_idx--;
                for (size_t j = 0; j < i - 1; j++) // i is the number of bytes remaining to be read.
                {
                    input_buffer[input_buffer_idx + j] = input_buffer[input_buffer_idx + j + 2];
                }
            }
        }
        else if ((input_buffer[input_buffer_idx] == (uint8_t) '\n')
            || (input_buffer[input_buffer_idx] == (uint8_t) '\r'))
        {
            echo_str(&input_buffer[input_buffer_idx], 1);

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
            // i is the remaining bytes to read.
            for (size_t j = 0; j < i; j++)
            {
                input_buffer[j] = input_buffer[input_buffer_idx + j + 1];
            }

            // Reset the buffer index and continue the scan.
            input_buffer_idx = 0;

            if (i == 1 && serial()->rts_get())
            {
                tasks_queue(TASK_CMD_PROMPT);
            }
        }
        else
        {
            // Increment the index to the next received character.
            echo_str(&input_buffer[input_buffer_idx], 1);

            input_buffer_idx++;
        }
    }
}

/**
 * Echos a string back to the serial interface if echo is enabled or if a terminal is detected.
 * @param[in] str    Data to echo.
 * @param     length Number of bytes to write.
 */
static void echo_str(uint8_t * str, size_t length)
{
    if (serial()->rts_get() || config_get_or_default()->flags_b.echo_enabled)
    {
        serial()->write(str, length);
        serial()->flush();
    }
}

/**
 * @}
 */
