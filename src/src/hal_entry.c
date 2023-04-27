#include <stdint.h>

#include "hal_data.h"
#include "r_dtc.h"
#include "hal_device.h"
#include "neopixel.h"
#include "pixelkey.h"

#define HARDWARE_TESTS

/* *****************************************************************************
 * Function declarations
 * ****************************************************************************/

FSP_CPP_HEADER
void R_BSP_WarmStart(bsp_warm_start_event_t event);
FSP_CPP_FOOTER

static void write_color_to_buffer(color_rgb_t const * const p_color, uint8_t byte_idx, volatile uint32_t * p_npdata_buffer);
static void init_npdata_buffers(void);
static void send_frame(void);

/* *****************************************************************************
 * Static variables
 * ****************************************************************************/

/** GPT compare buffer for generating NeoPixel timing waveforms. */
static volatile uint32_t npdata_gpt_buffer[NPDATA_GPT_BUFFER_LENGTH] = {0};

/** Double buffer for NeoPixel data used to fill npdata_gpt_buffer. */
static volatile uint32_t npdata_pingpong[2U][NPDATA_GPT_BUFFER_LENGTH] = {0};

/** Current NeoPixel frame. */
static volatile color_rgb_t npdata_frame[PIXELKEY_NEOPIXEL_COUNT] = {0};

/** Current index of the next color to write to npdata_pingpong. */
static volatile size_t npdata_frame_idx = 0;
static volatile uint8_t npdata_frame_byte_idx = 0;

/** Block index to write the next NeoPixel color block to. */
static volatile size_t npdata_pingpong_block = 0;

/** The address of the ping-pong buffer needs to be saved somewhere so it can be reloaded by the DTC. */
static void * const default_npdata_pingpong_xfr_src = (void *) npdata_pingpong;

/** The number of blocks needs to be saved someplace so it can be reloaded by the DTC. */
static const uint16_t default_npdata_pingpong_xfr_num_blocks = 2U;

/* *****************************************************************************
 * DTC configuration and data
 * ****************************************************************************/

/**
 * Transfer info for the ping-pong buffer DTC operation. 
 * @note Since this requires a chain operation, I don't believe e2studio can configure it for us. -JoeP
 */
static transfer_info_t g_npdata_pingpong_xfr_info[] =
{
    {   // Normal 2-block transfer to push a single NeoPixel's worth of data at a time into the GPT buffer.
        .transfer_settings_word_b.dest_addr_mode = TRANSFER_ADDR_MODE_INCREMENTED,
        .transfer_settings_word_b.repeat_area = TRANSFER_REPEAT_AREA_DESTINATION,
        .transfer_settings_word_b.irq = TRANSFER_IRQ_EACH,
        .transfer_settings_word_b.chain_mode = TRANSFER_CHAIN_MODE_END,
        .transfer_settings_word_b.src_addr_mode = TRANSFER_ADDR_MODE_INCREMENTED,
        .transfer_settings_word_b.size = TRANSFER_SIZE_4_BYTE,
        .transfer_settings_word_b.mode = TRANSFER_MODE_BLOCK,
        .p_dest = (void *) npdata_gpt_buffer,
        .p_src = (void const* ) npdata_pingpong,
        .num_blocks = 6,
        .length = 8,
    },
    {   // Reset the source address for the block transfer. This will "restart" the first transfer.
        .transfer_settings_word_b.dest_addr_mode = TRANSFER_ADDR_MODE_FIXED,
        .transfer_settings_word_b.repeat_area = TRANSFER_REPEAT_AREA_SOURCE,
        .transfer_settings_word_b.irq = TRANSFER_IRQ_END,
        .transfer_settings_word_b.chain_mode = TRANSFER_CHAIN_MODE_EACH,
        .transfer_settings_word_b.src_addr_mode = TRANSFER_ADDR_MODE_FIXED,
        .transfer_settings_word_b.size = TRANSFER_SIZE_4_BYTE,
        .transfer_settings_word_b.mode = TRANSFER_MODE_REPEAT,
        .p_dest = (void *) &(g_npdata_pingpong_xfr_info[0].p_src),
        .p_src = (void const* ) &default_npdata_pingpong_xfr_src,
        .num_blocks = 1,
        .length = 1,
    },
    {   // Reset the block count for the block transfer. This will "restart" the first transfer.
        .transfer_settings_word_b.dest_addr_mode = TRANSFER_ADDR_MODE_FIXED,
        .transfer_settings_word_b.repeat_area = TRANSFER_REPEAT_AREA_SOURCE,
        .transfer_settings_word_b.irq = TRANSFER_IRQ_EACH,
        .transfer_settings_word_b.chain_mode = TRANSFER_CHAIN_MODE_DISABLED,
        .transfer_settings_word_b.src_addr_mode = TRANSFER_ADDR_MODE_FIXED,
        .transfer_settings_word_b.size = TRANSFER_SIZE_2_BYTE,
        .transfer_settings_word_b.mode = TRANSFER_MODE_REPEAT,
        .p_dest = (void *) &(g_npdata_pingpong_xfr_info[0].num_blocks),
        .p_src = (void const* ) &default_npdata_pingpong_xfr_num_blocks,
        .num_blocks = 1,
        .length = 1,
    }
};
// Create the rest of the standard API elements for the DTC since e2studio isn't doing it for us this time.
const dtc_extended_cfg_t g_npdata_pingpong_xfr_extended = 
{ .activation_source = VECTOR_NUMBER_DMAC2_INT };
const transfer_cfg_t g_npdata_pingpong_xfr_cfg =
{ .p_info = g_npdata_pingpong_xfr_info, .p_extend = &g_npdata_pingpong_xfr_extended };
dtc_instance_ctrl_t g_npdata_pingpong_xfr_ctrl;
const transfer_instance_t g_npdata_pingpong_xfr = 
{ .p_ctrl = &g_npdata_pingpong_xfr_ctrl, .p_cfg = &g_npdata_pingpong_xfr_cfg, .p_api = &g_transfer_on_dtc };

/* *****************************************************************************
 * /END DTC configuration and data
 * ****************************************************************************/

/* *****************************************************************************
 * ISR and callback functions
 * ****************************************************************************/

void dmac_repeat_isr(void)
{
    IRQn_Type irq = R_FSP_CurrentIrqGet();

    /* Clear IRQ to make sure it doesn't fire again after exiting */
    R_BSP_IrqStatusClear(irq);

    if (R_DMAC2->DMCRB > 0U)
    {
        R_DMAC2->DMCNT = 1;
    }
    else
    {
        g_npdata_timer.p_api->stop(&g_npdata_timer_ctrl);
    }
}

/**
 * ISR for handling loading new data into the ping-pong buffer.
 * Trigger is DTC_COMPLETE.
 */
void dtc_complete_isr(void)
{
    IRQn_Type irq = R_FSP_CurrentIrqGet();

    /* Clear IRQ to make sure it doesn't fire again after exiting */
    R_BSP_IrqStatusClear(irq);

    if (npdata_frame_idx > PIXELKEY_NEOPIXEL_COUNT)
    {
        // Out of NeoPixels to write.
        // Turn off the DTC and return.
        g_npdata_pingpong_xfr.p_api->disable(&g_npdata_pingpong_xfr_ctrl);
        return;
    }

    // Copy the color from the frame buffer.
    const color_rgb_t rgb = npdata_frame[npdata_frame_idx];

    write_color_to_buffer(&rgb, npdata_frame_byte_idx, npdata_pingpong[npdata_pingpong_block]);

    // Increment the NeoPixel index and the block index.
    npdata_frame_byte_idx++;
    if (npdata_frame_byte_idx == 3)
    {
        npdata_frame_byte_idx = 0;
        npdata_frame_idx++;
    }
    npdata_pingpong_block = (npdata_pingpong_block + 1U) & 0x01U;
}

/**
 * Callback for stopping when the DMAC has completed all transfers for a frame.
 */
void hal_frame_complete_callback(dmac_callback_args_t * p_args)
{
    FSP_PARAMETER_NOT_USED(p_args);

    transfer_properties_t xfr_prop;
    g_npdata_transfer.p_api->infoGet(&g_npdata_transfer_ctrl, &xfr_prop);
    if (xfr_prop.block_count_remaining > 0)
    {
        // Only stop the data transfer if all the blocks have been written.
        return;
    }


    // Stop the timer and the transfers.
    /// @todo Check to make sure the last bit gets transferred out correctly; may need to wait until the timer cycles once more.
    g_npdata_timer.p_api->stop(&g_npdata_timer_ctrl);
    g_npdata_transfer.p_api->disable(&g_npdata_transfer_ctrl);
    g_npdata_pingpong_xfr.p_api->disable(&g_npdata_pingpong_xfr_ctrl);
}

/* *****************************************************************************
 * Static functions
 * ****************************************************************************/

/**
 * Writes the timing waveform representation of a color to the data buffer.
 * @param[in] p_color         Pointer to the color to transform.
 * @param[in] p_npdata_buffer Pointer to the waveform buffer for a single 24-bit NeoPixel burst.
 */
static void write_color_to_buffer(color_rgb_t const * const p_color, uint8_t byte_idx, volatile uint32_t * p_npdata_buffer)
{
    // Convert it to the NeoPixel data format.
    const neopixel_data_t data = {
        .gbr = {
            .green = p_color->green,
            .red = p_color->red,
            .blue = p_color->blue
        }
    };

    // Write each bit's compare value to the buffer.
    for (int8_t i = 7; i >= 0; i--)
    {
        p_npdata_buffer[7 - i] = ((data.array[byte_idx] >> i) & 0x01) ? NPDATA_GPT_B1 : NPDATA_GPT_B0;
    }
}

/**
 * Initialize/seed the npdata waveform buffers with up to the first three colors to transmit.
 */
static void init_npdata_buffers(void)
{
    npdata_frame_idx = 0;
    npdata_frame_byte_idx = 0;
    npdata_pingpong_block = 0;

    color_rgb_t rgb = npdata_frame[0];

    // Write the first NeoPixel directly to the gpt buffer.
    write_color_to_buffer(&rgb, 0, npdata_gpt_buffer);
    write_color_to_buffer(&rgb, 1, npdata_pingpong[0]);
    write_color_to_buffer(&rgb, 2, npdata_pingpong[1]);
    npdata_frame_idx++;
}

/**
 * Kicks off a frame transmission to the attached NeoPixels.
 */
static void send_frame(void)
{
    transfer_properties_t xfr_prop;
    g_npdata_transfer.p_api->infoGet(&g_npdata_transfer_ctrl, &xfr_prop);
    if (xfr_prop.block_count_remaining > 0)
    {
        /// @todo Add frame overflow log
        //return;
    }

    // Initialize the buffers and state variables.
    init_npdata_buffers();

    // Reconfigure the GPT DMAC transfer.
    g_npdata_transfer.p_api->reset(&g_npdata_transfer_ctrl, (void *)npdata_gpt_buffer, NULL, 12);

    // Reconfigure the ping-pong DTC transfer.
    // Make sure the source and block count is correct. Nothing else should be changed by the DTC.
    g_npdata_pingpong_xfr_info[0].p_src = (void *) npdata_pingpong;
    g_npdata_pingpong_xfr_info[0].num_blocks = 2;

    // Enable the DTC first so it is ready.
    g_npdata_pingpong_xfr.p_api->enable(&g_npdata_pingpong_xfr_ctrl);

    // Now enable the DMAC for sending the actual waveform to the GPT.
    R_DMAC2->DMINT_b.RPTIE = 1;
    R_DMAC2->DMINT_b.ESIE = 1;
    g_npdata_transfer.p_api->enable(&g_npdata_transfer_ctrl);

    // And lastly, enable the GPT to start sending data.
    R_GPT5->GTCCR[1] = 0x00;
    R_GPT5->GTCCR[3] = 0x00;
    g_npdata_timer.p_api->start(&g_npdata_timer_ctrl);
}

static void set_color(uint8_t index, color_t * const color)
{
    color_rgb_t rgb;
    color_convert2(color->color_space, COLOR_SPACE_RGB, (color_kind_t *)color, (color_kind_t *)&rgb);

    npdata_frame[index] = rgb;

    // const neopixel_data_t data = {
    //     .gbr = {
    //         .green = rgb.green,
    //         .red = rgb.red,
    //         .blue = rgb.blue
    //     }
    // };

    // const uint8_t offset_idx = index * NEOPIXEL_COLOR_BITS;
    // for (uint8_t i = 0; i < NEOPIXEL_COLOR_BITS; i++)
    // {
    //     const uint8_t byte = i >> 3;
    //     const uint8_t bit = (uint8_t) (7 - (i & 0x7));  // MSb first

    //     g_npdata_gpt_buffer[offset_idx + i] = ((data.array[byte] >> bit) & 0x01) ? NPDATA_GPT_B1 : NPDATA_GPT_B0;
    // }
}

void hal_rtc_callback(rtc_callback_args_t *p_args)
{
    switch (p_args->event)
    {
        case RTC_EVENT_PERIODIC_IRQ:
        {

        }
        break;
        default:
            break;
    }
}

/*******************************************************************************************************************//**
 * main() is generated by the RA Configuration editor and is used to generate threads if an RTOS is used.  This function
 * is called by main() when no RTOS is used.
 **********************************************************************************************************************/
void hal_entry(void)
{
    // Configure and open the peripherals
    g_npdata_timer.p_api->open(&g_npdata_timer_ctrl, &g_npdata_timer_cfg);
    g_npdata_transfer.p_api->open(&g_npdata_transfer_ctrl, &g_npdata_transfer_cfg);
    g_npdata_pingpong_xfr.p_api->open(&g_npdata_pingpong_xfr_ctrl, &g_npdata_pingpong_xfr_cfg);

    R_BSP_IrqCfgEnable(VECTOR_NUMBER_DTC_COMPLETE, 1, NULL);

#ifdef HARDWARE_TESTS
    /* Initial hardware testing. */
    //transfer_info_t * p_tfr = g_npdata_transfer_cfg.p_info; 
    //p_tfr->p_src = npdata_gpt_buffer;
    //p_tfr->length = NPDATA_GPT_BUFFER_LENGTH;

    color_t c = color_red;
    c.hsv.value = 50;
    set_color(0, &c);

    c = color_green;
    c.hsv.value = 50;
    set_color(1, &c);

    c = color_blue;
    c.hsv.value = 50;
    set_color(2, &c);

    c = color_white;
    c.hsv.value = 50;
    set_color(3, &c);

    send_frame();

    //g_npdata_timer.p_api->open(&g_npdata_timer_ctrl, &g_npdata_timer_cfg);
    //R_GPT5->GTCCR[1] = 0;
    //R_GPT5->GTCCR[3] = 0;

    //g_npdata_transfer.p_api->open(&g_npdata_transfer_ctrl, &g_npdata_transfer_cfg);
    //g_npdata_transfer.p_api->enable(&g_npdata_transfer_ctrl);

    //g_npdata_timer.p_api->start(&g_npdata_timer_ctrl);

    //transfer_properties_t tinfo;
    //do
    //{
    //    g_npdata_transfer.p_api->infoGet(&g_npdata_transfer_ctrl, &tinfo);
    //} while (tinfo.transfer_length_remaining > 0);

    //g_npdata_timer.p_api->stop(&g_npdata_timer_ctrl);

    return;
#endif

}

/*******************************************************************************************************************//**
 * This function is called at various points during the startup process.  This implementation uses the event that is
 * called right before main() to set up the pins.
 *
 * @param[in]  event    Where at in the start up process the code is currently at
 **********************************************************************************************************************/
void R_BSP_WarmStart(bsp_warm_start_event_t event)
{
    if (BSP_WARM_START_RESET == event)
    {
#if BSP_FEATURE_FLASH_LP_VERSION != 0

        /* Enable reading from data flash. */
        R_FACI_LP->DFLCTL = 1U;

        /* Would normally have to wait tDSTOP(6us) for data flash recovery. Placing the enable here, before clock and
         * C runtime initialization, should negate the need for a delay since the initialization will typically take more than 6us. */
#endif
    }

    if (BSP_WARM_START_POST_C == event)
    {
        /* C runtime environment and system clocks are setup. */

        /* Configure pins. */
        R_IOPORT_Open (&g_ioport_ctrl, g_ioport.p_cfg);
    }
}

#if BSP_TZ_SECURE_BUILD

FSP_CPP_HEADER
BSP_CMSE_NONSECURE_ENTRY void template_nonsecure_callable ();

/* Trustzone Secure Projects require at least one nonsecure callable function in order to build (Remove this if it is not required to build). */
BSP_CMSE_NONSECURE_ENTRY void template_nonsecure_callable ()
{

}
FSP_CPP_FOOTER

#endif
