/**
 * @defgroup pixelkey__tasks PixelKey Tasks
 * @ingroup pixelkey
 * @{
 */

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "hal_device.h"
#include "pixelkey.h"
#include "neopixel.h"

#include "hal_npdata_transfer.h"

/**
 * Renders and queues a frame to be transferred at the next frame interval.
 */
void pixelkey_task_do_frame(void)
{
    color_rgb_t temp_frame[PIXELKEY_NEOPIXEL_COUNT] = {0};

    pixelkey_error_t err = pixelkey_render_frame(temp_frame);

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
    memcpy(g_npdata_frame, temp_frame, sizeof(g_npdata_frame));
}

/**
 * Processes data from the USB input buffer and parses into a command.
 */
void pixelkey_task_command_rx(void)
{

}

/**
 * @}
 */
