/**
 * @file
 * @defgroup hal__usb__internals USB Comms Internals
 * @ingroup hal
 * @{
 */

#include <stdint.h>

#include "hal_data.h"
#include "hal_device.h"
#include "hal_npdata_transfer.h"
#include "hal_tasks.h"

#include "neopixel.h"
#include "pixelkey.h"

#include "r_usb_basic.h"
#include "r_usb_basic_api.h"
#include "r_usb_basic_cfg.h"

#define USB_READ_LENGTH     (64)

/** Write index of the input buffer. */
static size_t input_buffer_idx = 0;

/** Input buffer for received command data over USB. */
static uint8_t input_buffer[PIXELKEY_INPUT_COMMAND_BUFFER_LENGTH] = {0};

/** The currently configured line coding. Unused for this application but required for USB CDC. */
static usb_pcdc_linecoding_t line_coding = {0};

static void input_buffer_shift(uint32_t count, uint32_t length);

/**
 * Idle function to check the status of the USB comms and handle CDC specific messaging.
 */
void hal_usb_idle(void)
{
    usb_event_info_t event_info;
    usb_status_t     event;
    fsp_err_t        err_code;

    // !IMPORTANT! This function does NOT conform to the API! The first argument
    //             is overloaded to be type usb_event_info_t not usb_ctrl_t!
    err_code = g_usb.p_api->eventGet(&event_info, &event);

    if (err_code != FSP_SUCCESS)
    {
        /// @todo Log USB error on eventGet failure.
        BKPT();
        return;
    }

    switch (event)
    {
        case USB_STATUS_CONFIGURED:
        case USB_STATUS_WRITE_COMPLETE:
        {
            // We have data that was received. Kick off the task to get it.
            tasks_queue(TASK_CMD_RX);
        }
        break;
        /* Receive Class Request */
        case USB_STATUS_REQUEST:
        {
            if (USB_PCDC_SET_LINE_CODING == (event_info.setup.request_type & USB_BREQUEST))
            {
                /* Configure virtual UART settings */
                err_code = g_usb.p_api->periControlDataGet(&g_usb_ctrl, (uint8_t *) &line_coding, event_info.setup.request_length);
            }
            else if (USB_PCDC_GET_LINE_CODING == (event_info.setup.request_type & USB_BREQUEST))
            {
                /* Send virtual UART settings back to host */
                err_code = g_usb.p_api->periControlDataSet(&g_usb_ctrl, (uint8_t *) &line_coding, event_info.setup.request_length);
            }
            else
            {
                /* ACK all other status requests */
                err_code = g_usb.p_api->periControlStatusSet(&g_usb_ctrl, USB_SETUP_STATUS_ACK);
            }
        }
        break;
        case USB_STATUS_SUSPEND:
        case USB_STATUS_DETACH:
        default:
            break;
    }

    if (err_code != FSP_SUCCESS)
    {
        /// @todo Log USB error on response operation failure.
        BKPT();
        return;
    }
}

void hal_task_command_rx(void)
{
    uint8_t read_buffer[64] __attribute__ (( aligned (2) )) = {0};

    uint32_t read_length = PIXELKEY_INPUT_COMMAND_BUFFER_LENGTH - input_buffer_idx;
    if (read_length > 64)
    {
        // Clip to 64 byte max read (length of read_buffer).
        read_length = 64;
    }
    fsp_err_t err = g_usb.p_api->read(&g_usb_ctrl, read_buffer, read_length, USB_CLASS_PCDC);
    if (err != FSP_SUCCESS)
    {
        /// @todo Log USB error from data read.
        BKPT();
        return;
    }

    // Make sure the read has finished and get the length.
    usb_event_info_t event_info;
    usb_status_t     event;

    // !IMPORTANT! This function does NOT conform to the API! The first argument
    //             is overloaded to be type usb_event_info_t not usb_ctrl_t!
    err = g_usb.p_api->eventGet(&event_info, &event);
    if (err != FSP_SUCCESS)
    {
        /// @todo Log USB error from event get
        BKPT();
        return;
    }
    if (event != USB_STATUS_READ_COMPLETE)
    {
        /// @todo Log read incomplete error.
        BKPT();
        return;
    }

    if (event_info.data_size != 0)
    {
#if DEBUG && defined(ECHO_USB)
        g_usb.p_api->write(&g_usb_ctrl, read_buffer, event_info.data_size, USB_CLASS_PCDC);
        usb_event_info_t echo_event_info;
        usb_status_t     echo_event;
        g_usb.p_api->eventGet(&echo_event_info, &echo_event);
        if (echo_event != USB_STATUS_WRITE_COMPLETE)
        {
            /// @todo Log echo failed.
            BKPT();
        }
#endif

        // Copy from the read buffer to the input buffer.
        memcpy(&input_buffer[input_buffer_idx], read_buffer, event_info.data_size);

        // Scan the input for NEW-LINE symbols.
        for (size_t i = event_info.data_size; i > 0; i--)
        {
            if (input_buffer[input_buffer_idx] == (uint8_t) '\n')
            {
                input_buffer[input_buffer_idx] = (uint8_t) '\0';

                // Parse the command string. This will queue amy multi-command inputs and start the handler task.
                //pixelkey_command_parse((char *)input_buffer);

                // Shift the input buffer down to remove the parsed command string.
                input_buffer_shift(input_buffer_idx + 1, input_buffer_idx + i);

                // Reset the buffer index and continue the scan.
                input_buffer_idx = 0;
            }
            else
            {
                input_buffer_idx++;
            }
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
}

/** @} */
