
/**
 * @addtogroup hal__npdata
 * @{
 */

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "hal_data.h"
#include "hal_device.h"
#include "pixelkey.h"
#include "neopixel.h"

#include "hal_npdata_transfer.h"

#include "r_dtc.h"

#define NPDATA_SHIFT_REG_MASK   (UINT32_C(1) << (NEOPIXEL_COLOR_BITS - 1))

volatile color_rgb_t g_npdata_frame[PIXELKEY_NEOPIXEL_COUNT] = {0};

static void push_data_to_buffer(uint32_t * const p_block);

/* *****************************************************************************
 * Static variables
 * ****************************************************************************/

/** @internal GPT compare buffer for generating NeoPixel timing waveforms. */
static volatile uint32_t npdata_gpt_buffer[NPDATA_GPT_BUFFER_LENGTH] = {0};

/** @internal Secondary buffer for NeoPixel data used to fill npdata_gpt_buffer. */
static volatile uint32_t npdata_secondary_buffer[NPDATA_SECONDARY_BUFFER_COUNT][NPDATA_GPT_BUFFER_LENGTH] = {0};

/** @internal Current index to write to the secondary buffer. */
static volatile int32_t npdata_frame_idx = -1;

/** @internal Current bit to write to the secondary buffer. */
static volatile uint8_t npdata_color_bit = UINT8_MAX;

/** @internal Current full shift-register word for the color being written. */
static volatile uint32_t npdata_color_word = 0;



/** @internal The address of the ping-pong buffer needs to be saved somewhere so it can be reloaded by the DTC. */
static void const * const default_npdata_secbuff_ptr = (void *) npdata_secondary_buffer;

/** @internal The number of blocks needs to be saved someplace so it can be reloaded by the DTC. */
static const uint16_t default_npdata_secbuff_block_count = NPDATA_SECONDARY_BUFFER_COUNT;



/**
 * @name DTC configuration and data
 * @{
 */

/**
 * @internal
 * Transfer info for the secondary buffer DTC operation. 
 * @note Since this requires a chain operation, it is simpler to configure it by hand than with e2 studio.
 * 
 * The idea here is to transfer a GPT buffer's worth of data to the DMAC when it runs out and keep going indefinitely.
 * Chain steps:
 * 1. Transfer NPDATA_SECONDARY_BUFFER_COUNT buffer blocks.
 * 2. Reset the source pointer of chain 1.
 * 3. Reset the block count of chain 1.
 * 
 * An interrupt occurs after each block has transferred, or at the end of chain step 3 if the last block was
 * transferred. This interrupt is used to push new data to the secondary buffer. Doing it here instead of at the DMAC
 * repeat interrupt allows the DTC to finish it's bus transaction before we start transferring new data to the same RAM
 * area.
 */
static transfer_info_t npdata_secondary_xfr_info[] =
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
        .p_src = (void const* ) npdata_secondary_buffer,
        .num_blocks = default_npdata_secbuff_block_count,
        .length = NPDATA_GPT_BUFFER_LENGTH,
    },
    {   // Reset the source address for the block transfer. This will "restart" the first transfer.
        .transfer_settings_word_b.dest_addr_mode = TRANSFER_ADDR_MODE_FIXED,
        .transfer_settings_word_b.repeat_area = TRANSFER_REPEAT_AREA_SOURCE,
        .transfer_settings_word_b.irq = TRANSFER_IRQ_END,
        .transfer_settings_word_b.chain_mode = TRANSFER_CHAIN_MODE_EACH,
        .transfer_settings_word_b.src_addr_mode = TRANSFER_ADDR_MODE_FIXED,
        .transfer_settings_word_b.size = TRANSFER_SIZE_4_BYTE,
        .transfer_settings_word_b.mode = TRANSFER_MODE_REPEAT,
        .p_dest = (void *) &(npdata_secondary_xfr_info[0].p_src),
        .p_src = (void const* ) &default_npdata_secbuff_ptr,
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
        .p_dest = (void *) &(npdata_secondary_xfr_info[0].num_blocks),
        .p_src = (void const* ) &default_npdata_secbuff_block_count,
        .num_blocks = 1,
        .length = 1,
    }
};

// Create the rest of the standard API elements for the DTC since e2studio isn't doing it for us this time.
/** @internal Secondary buffer DTC extended configuration. */
static const dtc_extended_cfg_t npdata_secondary_xfr_extended = 
{ .activation_source = VECTOR_NUMBER_DMAC2_INT };

/** @internal Secondary buffer transfer configuration. */
static const transfer_cfg_t npdata_secondary_xfr_cfg =
{
    .p_info = npdata_secondary_xfr_info,
    .p_extend = &npdata_secondary_xfr_extended
};

/** @internal Secondary buffer DTC instance control struct. */
static dtc_instance_ctrl_t npdata_secondary_xfr_ctrl;

/** @internal Secondary buffer DTC instance. */
static const transfer_instance_t npdata_secondary_xfr = 
{
    .p_ctrl = &npdata_secondary_xfr_ctrl,
    .p_cfg = &npdata_secondary_xfr_cfg,
    .p_api = &g_transfer_on_dtc
};

/** @} */



/**
 * @name ISR functions and callbacks
 * @{
 */

/**
 * DMAC2_INT ISR function.
 * @note This overrides the built-in FSP DMAC ISR.
 * 
 * This is setup to trigger on each repeat size completion.
 */
void dmac2_repeat_isr(void)
{
    IRQn_Type irq = R_FSP_CurrentIrqGet();

    // Clear IRQ to make sure it doesn't fire again after exiting.
    R_BSP_IrqStatusClear(irq);

    // If more blocks remain, we have to manually restart the DMAC.
    // If they are none left, stop the timer and keep the DMAC off.
    if (R_DMAC2->DMCRB > 0U)
    {
        R_DMAC2->DMCNT = 1;
    }
    else
    {
        // Shutdown the transfer.
        g_npdata_timer.p_api->stop(&g_npdata_timer_ctrl);
        npdata_secondary_xfr.p_api->disable(&npdata_secondary_xfr_ctrl);
        g_npdata_transfer.p_api->disable(&g_npdata_transfer_ctrl);
    }
}

/**
 * DMAC interrupt callback; unused.
 * @param p_args Pointer to callback arguments.
 * 
 * @note This is unused and is present as a formality to enable the FSP driver to configure the interrupt;
 *       see @ref dmac_repeat_isr.
 */
void npdata_transfer_callback(dmac_callback_args_t * p_args)
{
    FSP_PARAMETER_NOT_USED(p_args);
    // Do nothing.
}

void dtc_complete_isr()
{
    IRQn_Type irq = R_FSP_CurrentIrqGet();

    // Clear IRQ to make sure it doesn't fire again after exiting.
    R_BSP_IrqStatusClear(irq);

    // Calculate the block index that was just transferred to the DMAC area.
    // This isr will never see a num_blocks of 0 because the DTC rewrites the block count first if that happens.
    uint16_t block_idx = NPDATA_SECONDARY_BUFFER_COUNT - npdata_secondary_xfr_info[0].num_blocks;
    if (block_idx == 0)
    {
        // Loop back around
        block_idx = NPDATA_SECONDARY_BUFFER_COUNT - 1;
    }
    else
    {
        block_idx--;
    }

    push_data_to_buffer((uint32_t *) npdata_secondary_buffer[block_idx]);
}

/** @} */

/**
 * @internal
 * Push new data to the secondary (or primary) buffer.
 * @param[in] p_block Pointer to the block to write the data to.
 */
static void push_data_to_buffer(uint32_t * const p_block)
{
    if (npdata_frame_idx >= ((int32_t) PIXELKEY_NEOPIXEL_COUNT))
    {
        // No more elements left.
        return;
    }

    if (npdata_color_bit == UINT8_MAX)
    {
        // Load the next color word.
        npdata_color_bit = NEOPIXEL_COLOR_BITS - 1;
        npdata_frame_idx++;

        color_rgb_t color = g_npdata_frame[npdata_frame_idx];

        // NeoPixel data is transferred green-red-blue...
        npdata_color_word = (((uint32_t) color.green) << 16)
                            | (((uint32_t) color.red) << 8)
                            | ((uint32_t) color.blue);
    }

    // Write the data. NeoPixel data is written MSb first.
    for (size_t i = 0; i < NPDATA_GPT_BUFFER_LENGTH; i++)
    {
        p_block[i] = (npdata_color_word & (NPDATA_SHIFT_REG_MASK)) ? NPDATA_GPT_B1 : NPDATA_GPT_B0;
        npdata_color_word <<= 1;
        npdata_color_bit--;
    }
}

/**
 * Kicks off a frame transmission to the attached NeoPixels.
 */
void npdata_send_frame(void)
{
    // Check for a transmitting frame.
    timer_status_t status;
    g_npdata_timer.p_api->statusGet(&g_npdata_timer_ctrl, &status);
    if (status.state == TIMER_STATE_COUNTING)
    {
        /// @todo Add frame overrun logging/counter.
        return;
    }

    // Initialize the buffers and state variables.
    npdata_frame_idx = -1;
    npdata_color_bit = UINT8_MAX;
    npdata_color_word = 0;
    push_data_to_buffer((uint32_t *) npdata_gpt_buffer);
    for (size_t i = 0; i < NPDATA_SECONDARY_BUFFER_COUNT; i++)
    {
        push_data_to_buffer((uint32_t *) npdata_secondary_buffer[i]);
    }

    // Reconfigure the peripherals.
    // 1. Reset the DMAC source and block count.
    g_npdata_transfer.p_api->reset(&g_npdata_transfer_ctrl, (void *) npdata_gpt_buffer, NULL, PIXELKEY_NEOPIXEL_COUNT * NEOPIXEL_CHANNEL_COUNT);
    // 2. Reset the DTC.
    npdata_secondary_xfr_info[0].p_src = default_npdata_secbuff_ptr;
    npdata_secondary_xfr_info[0].num_blocks = default_npdata_secbuff_block_count;
    
    // Enable the peripherals
    npdata_secondary_xfr.p_api->enable(&npdata_secondary_xfr_ctrl);
    g_npdata_transfer.p_api->enable(&g_npdata_transfer_ctrl);

    // Prefill the capture registers to zero, and start the timer to transfer the data.
    R_GPT5->GTCCR[1] = 0;
    R_GPT5->GTCCR[3] = 0;
    g_npdata_timer.p_api->start(&g_npdata_timer_ctrl);
}

/** 
 * Opens the peripherals needed for data transmission to the NeoPixels.
*/
void npdata_open(void)
{
    g_npdata_timer.p_api->open(&g_npdata_timer_ctrl, &g_npdata_timer_cfg);
    g_npdata_transfer.p_api->open(&g_npdata_transfer_ctrl, &g_npdata_transfer_cfg);
    npdata_secondary_xfr.p_api->open(&npdata_secondary_xfr_ctrl, &npdata_secondary_xfr_cfg);

    R_BSP_IrqCfgEnable(VECTOR_NUMBER_DTC_COMPLETE, 1, NULL);
}

/** @} */
