/**
 * @file
 * @defgroup hal__usb__internals USB Comms Internals
 * @ingroup hal
 * @{
 */

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "hal_data.h"
#include "hal_device.h"
#include "hal_npdata_transfer.h"
#include "hal_tasks.h"

#include "neopixel.h"
#include "pixelkey.h"
#include "serial.h"
#include "config.h"

#include "r_usb_basic.h"
#include "r_usb_basic_api.h"
#include "r_usb_basic_cfg.h"

/** Buffer length for USB operations, must be a multiple of 64. */
#define USB_BUFFER_LENGTH     (64)

/** USB operational state. */
typedef enum e_usb_op_state
{
    USB_OP_STATE_OFFLINE,           ///< USB is offline or not yet enumerated.
    USB_OP_STATE_ONLINE,            ///< USB is online and enumerated, possibly idle.
    USB_OP_STATE_CONTROL_SEQUENCE,  ///< Control sequence processing is active.
    USB_OP_STATE_READ,              ///< Read data processing is active.
    USB_OP_STATE_WRITE              ///< Write data processing is active.
} usb_op_state_t;

static void input_buffer_shift(uint32_t count, uint32_t length);
static bool rx_start(void);
static void rx_end(usb_event_info_t const * const p_event_info);
static bool tx_start(void);
static void tx_end(void);

static pixelkey_error_t usb_serial_read(uint8_t * p_buffer, size_t * p_read_length);
static pixelkey_error_t usb_serial_write(uint8_t * p_buffer, size_t write_length);
static pixelkey_error_t usb_serial_flush(void);
static bool usb_serial_rts(void);

/** Current operational state of the USB. */
static usb_op_state_t op_state = USB_OP_STATE_OFFLINE;

/** The currently configured line coding. Unused for this application but required for USB CDC. */
static usb_pcdc_linecoding_t line_coding = {0};

static usb_pcdc_ctrllinestate_t line_state = {0};

/** Number of bytes received. */
static size_t rx_length = 0;
/** Number of bytes to transmit. */
static size_t tx_length = 0;

/** RX data buffer. */
static uint8_t rx_buffer[USB_BUFFER_LENGTH] __attribute__ (( aligned (2) )) = {0};
/** TX data buffer. */
static uint8_t tx_buffer[USB_BUFFER_LENGTH] __attribute__ (( aligned (2) )) = {0};

/** Serial instance for USB. */
const serial_api_t g_usb_serial = {
    .read =  usb_serial_read,
    .write = usb_serial_write,
    .flush = usb_serial_flush,
    .rts_get = usb_serial_rts,
};

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
        {
            op_state = USB_OP_STATE_ONLINE;
        }
        break;
        case USB_STATUS_READ_COMPLETE:
        {
            rx_end(&event_info);    // Changes the op_state
        }
        break;
        case USB_STATUS_WRITE_COMPLETE:
        {
            tx_end();   // Changes the op_state
        }
        break;
        /* Receive Class Request */
        case USB_STATUS_REQUEST:
        {
            op_state = USB_OP_STATE_CONTROL_SEQUENCE;

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
            else if (USB_PCDC_SET_CONTROL_LINE_STATE == (event_info.setup.request_type & USB_BREQUEST))
            {
                /* Line state is not sent as a data field, but as the control value instead.*/
                line_state = *((usb_pcdc_ctrllinestate_t *)&event_info.setup.request_value);
                err_code = g_usb.p_api->periControlStatusSet(&g_usb_ctrl, USB_SETUP_STATUS_ACK);
            }
            else
            {
                /* ACK all other status requests */
                err_code = g_usb.p_api->periControlStatusSet(&g_usb_ctrl, USB_SETUP_STATUS_ACK);
            }
        }
        break;
        case USB_STATUS_REQUEST_COMPLETE:
        {
            op_state = USB_OP_STATE_ONLINE;
        }
        break;
        case USB_STATUS_SUSPEND:
        case USB_STATUS_DETACH:
        {
            op_state = USB_OP_STATE_OFFLINE;
        }
        break;
        case USB_STATUS_NONE:
        {
            if (op_state == USB_OP_STATE_ONLINE)
            {
                // Try to transmit any data if available, and if not try to receive some.
                if (!tx_start())
                {
                    rx_start();
                }
            }
            break;
        }
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

/**
 * Starts read processing.
 */
static bool rx_start(void)
{
    if (rx_length > 0)
    {
        // Don't receive any more data until the existing has been read.
        // There was comments about ensuring a 64-byte read so unless a larger buffer is provided only read once.
        return false;
    }

    fsp_err_t err = g_usb.p_api->read(&g_usb_ctrl, rx_buffer, USB_BUFFER_LENGTH, USB_CLASS_PCDC);
    if (err == FSP_ERR_USB_BUSY)
    {
        // Do nothing there. The USB is currently doing something else and cannot read.
        return false;
    }
    else if (err != FSP_SUCCESS)
    {
        /// @todo Log USB error from data read.
        BKPT();
        return false;
    }

    // Keep it in the online state since there is no way to determine if a read did not take
    // place in the current USB driver.
    op_state = USB_OP_STATE_ONLINE;
    return true;
}

/**
 * Finishes read processing on USB_STATUS_READ_COMPLETE event.
 */
static void rx_end(usb_event_info_t const * const p_event_info)
{
    if (p_event_info->data_size != 0)
    {
        rx_length = p_event_info->data_size;
        tasks_queue(TASK_CMD_RX);
    }

    op_state = USB_OP_STATE_ONLINE;
}

/**
 * Starts write processing.
 */
static bool tx_start(void)
{
    if (tx_length == 0)
    {
        // No data to transmit.
        return false;
    }

    fsp_err_t err = g_usb.p_api->write(&g_usb_ctrl, tx_buffer, tx_length, USB_CLASS_PCDC);
    if (err == FSP_ERR_USB_BUSY)
    {
        // Do nothing there. The USB is currently doing something else and cannot write.
        return false;
    }
    else if (err != FSP_SUCCESS)
    {
        /// @todo Log USB error from data read.
        BKPT();
        return false;
    }

    op_state = USB_OP_STATE_WRITE;
    return true;
}

/**
 * Finishes write processing on USB_STATUS_WRITE_COMPLETE event.
 */
static void tx_end(void)
{
    if (op_state != USB_OP_STATE_WRITE)
    {
        // Only handle the end of writes generated from the buffer.
        // Other writes like echos should not modify the state.
        return;
    }

    tx_length = 0;
    op_state = USB_OP_STATE_ONLINE;
}

/**
 * Provides a read method for the @ref serial_api_t API.
 * @param[in]     p_buffer      Pointer to the buffer to store read data.
 * @param[in,out] p_read_length Pointer to the maximum read length, updated afterwards with the actual read length.
 * @retval PIXELKEY_ERROR_COMMUNICATION_ERROR The USB connection is offline or has yet to be enumerated.
 * @retval PIXELKEY_ERROR_NONE                Read was performed successfully.
 */
static pixelkey_error_t usb_serial_read(uint8_t * p_buffer, size_t * p_read_length)
{
    if (op_state == USB_OP_STATE_OFFLINE)
    {
        return PIXELKEY_ERROR_COMMUNICATION_ERROR;
    }

    if (rx_length > 0)
    {
        size_t bytes_to_read = (rx_length < *p_read_length) ? rx_length : *p_read_length;
        memcpy(p_buffer, rx_buffer, bytes_to_read);
        *p_read_length = bytes_to_read;
        rx_length -= bytes_to_read;
    }
    else
    {
        *p_read_length = 0;
    }

    return PIXELKEY_ERROR_NONE;
}

/**
 * Provides a write method for the @ref serial_api_t API.
 * @param[in] p_buffer     Pointer to the buffer of data to write.
 * @param     write_length Total number of bytes to write.
 * @retval PIXELKEY_ERROR_COMMUNICATION_ERROR The USB connection is offline or has yet to be enumerated.
 * @retval PIXELKEY_ERROR_BUFFER_FULL         Writing the requested amount of data will overrun the transmit buffer.
 * @retval PIXELKEY_ERROR_NONE                Write was performed successfully.
 */
static pixelkey_error_t usb_serial_write(uint8_t * p_buffer, size_t write_length)
{
    if (op_state == USB_OP_STATE_OFFLINE)
    {
        return PIXELKEY_ERROR_COMMUNICATION_ERROR;
    }

    while (op_state == USB_OP_STATE_WRITE)
    {
        // Run the state machine if another write is taking place.
        hal_usb_idle();
    }

    // Too many bytes to write.
    if (tx_length + write_length > USB_BUFFER_LENGTH)
    {
        return PIXELKEY_ERROR_BUFFER_FULL;
    }

    memcpy(&tx_buffer[tx_length], p_buffer, write_length);
    tx_length += write_length;

    return PIXELKEY_ERROR_NONE;
}

/**
 * Provides a flush method for the @ref serial_api_t API.
 * @retval PIXELKEY_ERROR_COMMUNICATION_ERROR The USB connection is offline or has yet to be enumerated.
 * @retval PIXELKEY_ERROR_NONE                Write was performed successfully.
 */
static pixelkey_error_t usb_serial_flush(void)
{
    if (op_state == USB_OP_STATE_OFFLINE)
    {
        return PIXELKEY_ERROR_COMMUNICATION_ERROR;
    }

    while (tx_length > 0)
    {
        // Run the state machine until the tx_length returns to zero.
        hal_usb_idle();
    }

    return PIXELKEY_ERROR_NONE;
}

/**
 * Gets the RTS signal state.
 */
static bool usb_serial_rts(void)
{
    return line_state.brts;
}

/** @} */
