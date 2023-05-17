
/**
 * @file
 * @defgroup hal__npdata__internals NeoPixel Data Transfer Internals
 * @ingroup hal__npdata
 * @{
 */

/**
 * @page Operation
 * The mechanism for data transmission to the NeoPixels uses the GPT peripheral available on the RA4M1. A pulse-code
 * waveform is transmitted at 800&nbsp;kHz. To achieve this, the Capture Compare B channel is utilized.
 * 
 * @startuml
 * hide footbox
 * 
 * skinparam lifelineStrategy solid
 * skinparam sequenceMessageAlign center
 * skinparam sequence {
 * ArrowFontBackgroundColor white
 * }
 * 
 * actor FW as fw
 * control GPT as gpt
 * queue DMAC as dmac
 * collections DTC as dtc
 * 
 * fw --> gpt : Start
 * activate gpt
 * loop PIXELKEY_NEOPIXEL_COUNT * NEOPIXEL_CHANNEL_COUNT times
 *      loop NPDATA_SECONDARY_BUFFER_COUNT times
 *          loop NPDATA_GPT_BUFFER_LENGTH times
 *              gpt --> dmac : GPT5_OVF signal
 *              activate dmac
 *              dmac -> gpt : Write next compare value
 *          end
 *          dmac --> dtc : DMAC2_INT signal
 *          activate dtc
 *          dtc -> dmac : [chain 1] Write next timing block
 *          dtc --> fw : DTC_COMPLETE signal
 *          fw <- fw : Write next block to buffer
 *      end
 *      dtc -> dtc : [chain 2] Reset chain 1 source
 *      dtc -> dtc : [chain 3] Reset chain 1 blocks
 *      dtc --> fw : DTC_COMPLETE signal
 *      deactivate dtc
 *      fw <- fw : Write next block to buffer
 * end
 * dmac --> fw : DMAC2_INT
 * deactivate dmac
 * fw --> gpt : Stop
 * deactivate gpt
 * @enduml
 */

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "hal_data.h"
#include "hal_device.h"
#include "hal_tasks.h"
#include "pixelkey.h"
#include "neopixel.h"
#include "config.h"

#include "hal_npdata_transfer.h"

#include "r_dtc.h"

/** Default value for @ref npdata_frame_idx. */
#define NPDATA_FRAME_IDX_DEFAULT    (-1)
/** Default value for @ref npdata_color_bit. */
#define NPDATA_COLOR_BIT_DEFAULT    (UINT8_MAX)
/** Default value for @ref npdata_color_word. */
#define NPDATA_COLOR_WORD_DEFAULT   (0U)
/** Mask for selecting the current data bit when converting color bytes to timing data. */
#define NPDATA_SHIFT_REG_MASK       (UINT32_C(1) << (NEOPIXEL_COLOR_BITS - 1))

static void push_data_to_buffer(uint32_t * const p_block);

/** NeoPixel frame buffer. */
volatile color_rgb_t g_npdata_frame[PIXELKEY_NEOPIXEL_COUNT] = {0};

/** Primary GPT compare buffer for generating NeoPixel timing waveforms. */
static volatile uint32_t npdata_gpt_buffer[NPDATA_GPT_BUFFER_LENGTH] = {0};

/** Secondary buffer for NeoPixel data used to fill npdata_gpt_buffer. */
static volatile uint32_t npdata_secondary_buffer[NPDATA_SECONDARY_BUFFER_COUNT][NPDATA_GPT_BUFFER_LENGTH] = {0};

/** Current index to write to the secondary buffer. */
static volatile int32_t npdata_frame_idx = NPDATA_FRAME_IDX_DEFAULT;

/** Current bit to write to the secondary buffer. */
static volatile uint8_t npdata_color_bit = NPDATA_COLOR_BIT_DEFAULT;

/** Current full shift-register word for the color being written. */
static volatile uint32_t npdata_color_word = NPDATA_COLOR_WORD_DEFAULT;

/**
 * @name DTC configuration and data
 * @{
 */

/** The address of the ping-pong buffer needs to be saved somewhere so it can be reloaded by the DTC. */
static void const * const default_npdata_secbuff_ptr = (void *) npdata_secondary_buffer;

/** The number of blocks needs to be saved someplace so it can be reloaded by the DTC. */
static const uint16_t default_npdata_secbuff_block_count = NPDATA_SECONDARY_BUFFER_COUNT;


/**
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
/** Secondary buffer DTC extended configuration. */
static const dtc_extended_cfg_t npdata_secondary_xfr_extended = 
{ .activation_source = VECTOR_NUMBER_DMAC0_INT };

/** Secondary buffer transfer configuration. */
static const transfer_cfg_t npdata_secondary_xfr_cfg =
{
    .p_info = npdata_secondary_xfr_info,
    .p_extend = &npdata_secondary_xfr_extended
};

/** Secondary buffer DTC instance control struct. */
static dtc_instance_ctrl_t npdata_secondary_xfr_ctrl;

/** Secondary buffer DTC instance. */
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
 * DMAC_INT ISR handler.
 * @note This overrides the built-in FSP DMAC ISR.
 * 
 * This is setup to trigger on each repeat size completion.
 */
void dmac0_repeat_isr(void)
{
    IRQn_Type irq = R_FSP_CurrentIrqGet();

    // Clear IRQ to make sure it doesn't fire again after exiting.
    R_BSP_IrqStatusClear(irq);

    // If more blocks remain, we have to manually restart the DMAC.
    // If they are none left, stop the timer and keep the DMAC off.
    if (R_DMAC0->DMCRB > 0U)
    {
        R_DMAC0->DMCNT = 1;
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

/**
 * DTC_COMPLETE ISR handler.
 * Responsible for pushing new data to the secondary buffer.
 */
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

        volatile color_rgb_t * p_color = &g_npdata_frame[npdata_frame_idx];

        // NeoPixel data is transferred green-red-blue...
        // Copy it directly from the color_rgb_t struct.
        // color_rgb_t should be in the correct order unless the static_asserts were changed.
        uint32_t * p_color_u32 = (uint32_t *)(void *)p_color;
        npdata_color_word = *p_color_u32;
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
 * Gets a pointer to the frame buffer.
 * @warning Do not write to the frame buffer while @ref npdata_status_get() returns @ref TRANSFER_STATUS_WORKING.
 */
volatile color_rgb_t * npdata_frame_buffer_get(void)
{
    return g_npdata_frame;
}

/**
 * Kicks off a frame transmission to the attached NeoPixels.
 */
void npdata_frame_send(void)
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
    npdata_frame_idx = NPDATA_FRAME_IDX_DEFAULT;
    npdata_color_bit = NPDATA_COLOR_BIT_DEFAULT;
    npdata_color_word = NPDATA_COLOR_WORD_DEFAULT;

    push_data_to_buffer((uint32_t *) npdata_gpt_buffer);
    for (size_t i = 0; i < NPDATA_SECONDARY_BUFFER_COUNT; i++)
    {
        push_data_to_buffer((uint32_t *) npdata_secondary_buffer[i]);
    }

    // Reconfigure the peripherals.
    // 1. Reset the DMAC source and block count.
    const uint16_t num_blocks = (uint16_t)((config_get_or_default()->num_neopixels * NEOPIXEL_COLOR_BITS) / NPDATA_GPT_BUFFER_LENGTH);
    g_npdata_transfer.p_api->reset(&g_npdata_transfer_ctrl, (void *) npdata_gpt_buffer, NULL, num_blocks);
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

    // Queue the task to render the next frame.
    tasks_queue(TASK_FRAME_RENDER);
}

/**
 * Opens the peripherals needed for data transmission to the NeoPixels.
*/
void npdata_open(void)
{
    // Update the transfer info with the macro values
    extern transfer_info_t g_npdata_transfer_info;
    const uint16_t num_blocks = (uint16_t)((config_get_or_default()->num_neopixels * NEOPIXEL_COLOR_BITS) / NPDATA_GPT_BUFFER_LENGTH);
    g_npdata_transfer_info.length = NPDATA_GPT_BUFFER_LENGTH;
    g_npdata_transfer_info.num_blocks = num_blocks;

    g_npdata_timer.p_api->open(&g_npdata_timer_ctrl, &g_npdata_timer_cfg);

    g_npdata_transfer.p_api->open(&g_npdata_transfer_ctrl, &g_npdata_transfer_cfg);
    npdata_secondary_xfr.p_api->open(&npdata_secondary_xfr_ctrl, &npdata_secondary_xfr_cfg);

    R_BSP_IrqCfgEnable(VECTOR_NUMBER_DTC_COMPLETE, 1, NULL);
}

/**
 * Copies a color to the specified index of the frame buffer.
 * @param     index   The index to write.
 * @param[in] p_color Pointer to the color to copy.
 */
void npdata_color_set(uint32_t index, color_rgb_t const * const p_color)
{
    if (index > PIXELKEY_NEOPIXEL_COUNT)
    {
        /// @todo Log frame index out of range?
        return;
    }

    g_npdata_frame[index] = *p_color;
}

/**
 * Gets the current status of the NeoPixel data transfer.
 * @retval TRANSFER_STATUS_IDLE     No transfers are active.
 * @retval TRANSFER_STATUS_WORKING  Data is currently being transferred.
*/
transfer_status_t npdata_status_get(void)
{
    timer_status_t status;
    g_npdata_timer.p_api->statusGet(&g_npdata_timer_ctrl, &status);
    if (status.state == TIMER_STATE_COUNTING)
    {
        return TRANSFER_STATUS_WORKING;
    }
    else
    {
        return TRANSFER_STATUS_IDLE;
    }
}

/** @} */
