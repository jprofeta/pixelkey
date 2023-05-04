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

#define DATA_LEN 16

uint8_t g_buf[512] = {0};

/** The currently configured line coding. Unused for this application but required for USB CDC. */
static usb_pcdc_linecoding_t line_coding = {0};

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
        case USB_STATUS_READ_COMPLETE:  // This might need to be used to write data back... not sure yet.
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

/** @} */
