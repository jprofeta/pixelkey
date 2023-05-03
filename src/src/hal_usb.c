#include <stdint.h>

#include "hal_data.h"
#include "hal_device.h"
#include "hal_npdata_transfer.h"

#include "neopixel.h"
#include "pixelkey.h"

#include "r_usb_basic.h"
#include "r_usb_basic_api.h"
#include "r_usb_basic_cfg.h"

#define DATA_LEN 16

uint8_t g_buf[512] = {0};

usb_pcdc_linecoding_t g_line_coding = {0};

static fsp_err_t    usb__err;
static usb_pipe_t   usb__pipe_info;

void usb_test(void)
{
    g_usb.p_api->open(&g_usb_ctrl, &g_usb_cfg);

    usb_event_info_t event_info;
    usb_status_t     event;
    usb_info_t       info;

    while (1)
    {
        /* Get USB event data */
        g_usb.p_api->eventGet(&event_info, &event);

        /* Handle the received event (if any) */
        switch (event)
        {
            case USB_STATUS_CONFIGURED:
            case USB_STATUS_WRITE_COMPLETE:
                /* Initialization complete; get data from host */
                g_usb.p_api->read(&g_usb_ctrl, g_buf, DATA_LEN, USB_CLASS_PCDC);
                break;
            case USB_STATUS_READ_COMPLETE:
                /* Loop back received data to host */
                g_usb.p_api->write(&g_usb_ctrl, g_buf, event_info.data_size, USB_CLASS_PCDC);
                break;
            case USB_STATUS_REQUEST:   /* Receive Class Request */
                if (USB_PCDC_SET_LINE_CODING == (event_info.setup.request_type & USB_BREQUEST))
                {
                    /* Configure virtual UART settings */
                    usb__err = g_usb.p_api->periControlDataGet(&g_usb_ctrl, (uint8_t *) &g_line_coding, event_info.setup.request_length);
                    //usb__err = g_usb.p_api->periControlStatusSet(&g_usb_ctrl, USB_SETUP_STATUS_ACK);
                }
                else if (USB_PCDC_GET_LINE_CODING == (event_info.setup.request_type & USB_BREQUEST))
                {
                    /* Send virtual UART settings back to host */
                    usb__err = g_usb.p_api->periControlDataSet(&g_usb_ctrl, (uint8_t *) &g_line_coding, event_info.setup.request_length);
                    //usb__err = g_usb.p_api->periControlStatusSet(&g_usb_ctrl, USB_SETUP_STATUS_ACK);
                }
                else
                {
                    /* ACK all other status requests */
                    usb__err = g_usb.p_api->periControlStatusSet(&g_usb_ctrl, USB_SETUP_STATUS_ACK);
                }
                break;
            case USB_STATUS_SUSPEND:
            case USB_STATUS_DETACH:
                break;
            default:
                break;
        }

        if (usb__err != FSP_SUCCESS)
        {
            __BKPT(0);
            usb__err = FSP_SUCCESS;
        }
    }
}
