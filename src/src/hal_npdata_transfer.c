
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

/** Default value for @ref npdata_frame_idx. */
#define NPDATA_FRAME_IDX_DEFAULT    (0U)
/** Default value for @ref npdata_color_bit. */
#define NPDATA_COLOR_BIT_DEFAULT    (NPDATA_SHIFT_REG_MASK)

/** Mask for selecting the current data bit when converting color bytes to timing data. */
#define NPDATA_SHIFT_REG_MASK       (UINT32_C(1) << (NEOPIXEL_COLOR_BITS - 1))

static void push_data_to_buffer(uint32_t * const p_block);

/** NeoPixel frame buffer. */
volatile color_rgb_t g_npdata_frame[PIXELKEY_NEOPIXEL_COUNT] = {0};

/** GPT compare ping-pong buffer for generating NeoPixel timing waveforms. */
static volatile uint32_t npdata_gpt_buffer[2][NPDATA_GPT_BUFFER_LENGTH] ALIGN(4) = {0};

static uint32_t npdata_frame_cnt = 0;

/** Current NeoPixel index to write to the buffers. */
static uint32_t npdata_frame_idx = NPDATA_FRAME_IDX_DEFAULT;

/** Current bit to write to the buffers. */
static uint32_t npdata_color_bit = NPDATA_COLOR_BIT_DEFAULT;

/** Current full shift-register word for the color being written. */
static uint32_t npdata_color_word = 0U;

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
    // Clear IRQ to make sure it doesn't fire again after exiting.
    IRQn_Type irq = R_FSP_CurrentIrqGet();
    R_BSP_IrqStatusClear(irq);

    // If more blocks remain, we have to manually restart the DMAC.
    // If they are none left, stop the timer and keep the DMAC off.
    if (R_DMAC0->DMCRB > 0U)
    {
        const bool use_buffer2 = R_DMAC0->DMSAR == (uint32_t)(void *)npdata_gpt_buffer[0];

        if (use_buffer2)
            R_DMAC0->DMSAR = (uint32_t)(void *)npdata_gpt_buffer[1];
        else
            R_DMAC0->DMSAR = (uint32_t)(void *)npdata_gpt_buffer[0];

        R_DMAC0->DMCNT = 1;

        if (use_buffer2)
            push_data_to_buffer((uint32_t *) npdata_gpt_buffer[0]);
        else
            push_data_to_buffer((uint32_t *) npdata_gpt_buffer[1]);

        LOG_TIME(DIAG_TIMING_FRAME_BLOCK_TX);
        LOG_TIME_START(DIAG_TIMING_FRAME_BLOCK_BUFFER);
    }
    else
    {
        // Shutdown the transfer.
        g_npdata_timer.p_api->stop(&g_npdata_timer_ctrl);
        g_npdata_transfer.p_api->disable(&g_npdata_transfer_ctrl);
        LOG_TIME(DIAG_TIMING_FRAME_TX);
        LOG_TIME(DIAG_TIMING_FRAME_BLOCK_TX);
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

/** @} */

/**
 * Push new data to the secondary (or primary) buffer.
 * @param[in] p_block Pointer to the block to write the data to.
 */
static inline void push_data_to_buffer(uint32_t * const p_block)
{
    if (npdata_frame_cnt == 0)
    {
        // No more elements left.
        return;
    }

    // Write the data. NeoPixel data is written MSb first.
    for (size_t i = 0; i < NPDATA_GPT_BUFFER_LENGTH; i++)
    {
        p_block[i] = (npdata_color_word & npdata_color_bit) ? NPDATA_GPT_B1 : NPDATA_GPT_B0;
        npdata_color_bit >>= 1;

        if (npdata_color_bit == 0)
        {
            // Load the next color word.
            npdata_color_bit = NPDATA_COLOR_BIT_DEFAULT;
            npdata_frame_idx++;
            npdata_frame_cnt--;

            if (npdata_frame_cnt == 0)
            {
                // No more elements left.
                return;
            }

            // NeoPixel data is transferred green-red-blue... MSb first
            // Copy it directly from the color_rgb_t struct.
            // color_rgb_t should be in the correct order unless the static_asserts were changed.
            uint32_t * p_color_u32 = (uint32_t *)(void *)(&g_npdata_frame[npdata_frame_idx]);
            npdata_color_word = *p_color_u32;
        }
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
#if CHECK_RENDER_UNDERFLOW
    static uint32_t last_framecount = 0;
    uint32_t current_framecount = pixelkey_keyframeproc_framecount_get();
    if (current_framecount == last_framecount)
    {
        LOG_SIGNAL(DIAG_SIGNAL_RENDER_UNDERFLOW);
    }
#endif

    // Check for a transmitting frame.
    if (npdata_status_get() == TRANSFER_STATUS_WORKING)
    {
        LOG_SIGNAL(DIAG_SIGNAL_NPDATA_OVERFLOW);
        return;
    }

    // Initialize the buffers and state variables.
    npdata_frame_idx = NPDATA_FRAME_IDX_DEFAULT;
    npdata_color_bit = NPDATA_COLOR_BIT_DEFAULT;
    npdata_frame_cnt = config_get_or_default()->num_neopixels;
    // NeoPixel data is transferred green-red-blue...
    // Copy it directly from the color_rgb_t struct.
    // color_rgb_t should be in the correct order unless the static_asserts were changed.
    uint32_t * p_color_u32 = (uint32_t *)(void *)(&g_npdata_frame[0]);
    npdata_color_word = *p_color_u32;

    push_data_to_buffer((uint32_t *) npdata_gpt_buffer[0]);
    push_data_to_buffer((uint32_t *) npdata_gpt_buffer[1]);

    // Reconfigure the peripherals.
    // 1. Reset the DMAC source and block count.
    const uint16_t num_blocks = (uint16_t)((config_get_or_default()->num_neopixels * NEOPIXEL_COLOR_BITS) / NPDATA_GPT_BUFFER_LENGTH);
    g_npdata_transfer.p_api->reset(&g_npdata_transfer_ctrl, (void *) npdata_gpt_buffer[0], NULL, num_blocks);

    // Manually pre-fill the first two timings into the GPT buffers.
    // The source address will reload based on the high-word of the count register.
    g_npdata_transfer_ctrl.p_reg->DMSAR += 4;
    g_npdata_transfer_ctrl.p_reg->DMCRA_b.DMCRAL -= 1;
    g_npdata_transfer.p_api->enable(&g_npdata_transfer_ctrl);

    // Prefill the capture registers to zero, and start the timer to transfer the data.
    R_GPT5->GTCNT = 0;
    R_GPT5->GTCCR[3] = npdata_gpt_buffer[0][0];
    LOG_TIME_START(DIAG_TIMING_FRAME_TX);
    LOG_TIME_START(DIAG_TIMING_FRAME_BLOCK_TX);
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
