/* generated HAL source file - do not edit */
#include "hal_data.h"

rtc_instance_ctrl_t g_rtc_ctrl;
const rtc_error_adjustment_cfg_t g_rtc_err_cfg =
{ .adjustment_mode = RTC_ERROR_ADJUSTMENT_MODE_AUTOMATIC,
  .adjustment_period = RTC_ERROR_ADJUSTMENT_PERIOD_10_SECOND,
  .adjustment_type = RTC_ERROR_ADJUSTMENT_NONE,
  .adjustment_value = 0, };
const rtc_cfg_t g_rtc_cfg =
{ .clock_source = RTC_CLOCK_SOURCE_LOCO,
  .freq_compare_value_loco = 255,
  .p_err_cfg = &g_rtc_err_cfg,
  .p_callback = NULL,
  .p_context = NULL,
  .alarm_ipl = (BSP_IRQ_DISABLED),
  .periodic_ipl = (BSP_IRQ_DISABLED),
  .carry_ipl = (12),
#if defined(VECTOR_NUMBER_RTC_ALARM)
    .alarm_irq               = VECTOR_NUMBER_RTC_ALARM,
#else
  .alarm_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_RTC_PERIOD)
    .periodic_irq            = VECTOR_NUMBER_RTC_PERIOD,
#else
  .periodic_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_RTC_CARRY)
    .carry_irq               = VECTOR_NUMBER_RTC_CARRY,
#else
  .carry_irq = FSP_INVALID_VECTOR,
#endif
        };
/* Instance structure to use this module. */
const rtc_instance_t g_rtc =
{ .p_ctrl = &g_rtc_ctrl, .p_cfg = &g_rtc_cfg, .p_api = &g_rtc_on_rtc };
dmac_instance_ctrl_t g_npdata_transfer_ctrl;
transfer_info_t g_npdata_transfer_info =
{ .transfer_settings_word_b.dest_addr_mode = TRANSFER_ADDR_MODE_FIXED,
  .transfer_settings_word_b.repeat_area = TRANSFER_REPEAT_AREA_SOURCE,
  .transfer_settings_word_b.irq = TRANSFER_IRQ_END,
  .transfer_settings_word_b.chain_mode = TRANSFER_CHAIN_MODE_DISABLED,
  .transfer_settings_word_b.src_addr_mode = TRANSFER_ADDR_MODE_INCREMENTED,
  .transfer_settings_word_b.size = TRANSFER_SIZE_2_BYTE,
  .transfer_settings_word_b.mode = TRANSFER_MODE_NORMAL,
  .p_dest = (void*) NULL,
  .p_src = (void const*) NULL,
  .num_blocks = 1,
  .length = 16, };
const dmac_extended_cfg_t g_npdata_transfer_extend =
{ .offset = 1, .src_buffer_size = 1,
#if defined(VECTOR_NUMBER_DMAC2_INT)
    .irq                 = VECTOR_NUMBER_DMAC2_INT,
#else
  .irq = FSP_INVALID_VECTOR,
#endif
  .ipl = (BSP_IRQ_DISABLED),
  .channel = 2, .p_callback = NULL, .p_context = NULL, .activation_source = ELC_EVENT_GPT5_COUNTER_OVERFLOW, };
const transfer_cfg_t g_npdata_transfer_cfg =
{ .p_info = &g_npdata_transfer_info, .p_extend = &g_npdata_transfer_extend, };
/* Instance structure to use this module. */
const transfer_instance_t g_npdata_transfer =
{ .p_ctrl = &g_npdata_transfer_ctrl, .p_cfg = &g_npdata_transfer_cfg, .p_api = &g_transfer_on_dmac };
gpt_instance_ctrl_t g_npdata_timer_ctrl;
#if 0
const gpt_extended_pwm_cfg_t g_npdata_timer_pwm_extend =
{
    .trough_ipl          = (BSP_IRQ_DISABLED),
#if defined(VECTOR_NUMBER_GPT5_COUNTER_UNDERFLOW)
    .trough_irq          = VECTOR_NUMBER_GPT5_COUNTER_UNDERFLOW,
#else
    .trough_irq          = FSP_INVALID_VECTOR,
#endif
    .poeg_link           = GPT_POEG_LINK_POEG0,
    .output_disable      = (gpt_output_disable_t) ( GPT_OUTPUT_DISABLE_NONE),
    .adc_trigger         = (gpt_adc_trigger_t) ( GPT_ADC_TRIGGER_NONE),
    .dead_time_count_up  = 0,
    .dead_time_count_down = 0,
    .adc_a_compare_match = 0,
    .adc_b_compare_match = 0,
    .interrupt_skip_source = GPT_INTERRUPT_SKIP_SOURCE_NONE,
    .interrupt_skip_count  = GPT_INTERRUPT_SKIP_COUNT_0,
    .interrupt_skip_adc    = GPT_INTERRUPT_SKIP_ADC_NONE,
    .gtioca_disable_setting = GPT_GTIOC_DISABLE_PROHIBITED,
    .gtiocb_disable_setting = GPT_GTIOC_DISABLE_PROHIBITED,
};
#endif
const gpt_extended_cfg_t g_npdata_timer_extend =
        { .gtioca =
        { .output_enabled = false, .stop_level = GPT_PIN_LEVEL_LOW },
          .gtiocb =
          { .output_enabled = true, .stop_level = GPT_PIN_LEVEL_LOW },
          .start_source = (gpt_source_t) (GPT_SOURCE_NONE), .stop_source = (gpt_source_t) (GPT_SOURCE_NONE), .clear_source =
                  (gpt_source_t) (GPT_SOURCE_NONE),
          .count_up_source = (gpt_source_t) (GPT_SOURCE_NONE), .count_down_source = (gpt_source_t) (GPT_SOURCE_NONE), .capture_a_source =
                  (gpt_source_t) (GPT_SOURCE_NONE),
          .capture_b_source = (gpt_source_t) (GPT_SOURCE_NONE), .capture_a_ipl = (BSP_IRQ_DISABLED), .capture_b_ipl =
                  (BSP_IRQ_DISABLED),
#if defined(VECTOR_NUMBER_GPT5_CAPTURE_COMPARE_A)
    .capture_a_irq       = VECTOR_NUMBER_GPT5_CAPTURE_COMPARE_A,
#else
          .capture_a_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_GPT5_CAPTURE_COMPARE_B)
    .capture_b_irq       = VECTOR_NUMBER_GPT5_CAPTURE_COMPARE_B,
#else
          .capture_b_irq = FSP_INVALID_VECTOR,
#endif
          .capture_filter_gtioca = GPT_CAPTURE_FILTER_NONE,
          .capture_filter_gtiocb = GPT_CAPTURE_FILTER_NONE,
#if 0
    .p_pwm_cfg                   = &g_npdata_timer_pwm_extend,
#else
          .p_pwm_cfg = NULL,
#endif
#if 1
          .gtior_setting.gtior_b.gtioa = (0U << 4U) | (0U << 2U) | (0U << 0U),
          .gtior_setting.gtior_b.oadflt = (uint32_t) GPT_PIN_LEVEL_LOW, .gtior_setting.gtior_b.oahld = 0U, .gtior_setting.gtior_b.oae =
                  (uint32_t) false,
          .gtior_setting.gtior_b.oadf = (uint32_t) GPT_GTIOC_DISABLE_PROHIBITED, .gtior_setting.gtior_b.nfaen =
                  ((uint32_t) GPT_CAPTURE_FILTER_NONE & 1U),
          .gtior_setting.gtior_b.nfcsa = ((uint32_t) GPT_CAPTURE_FILTER_NONE >> 1U), .gtior_setting.gtior_b.gtiob = (1U
                  << 4U) | (0U << 2U) | (1U << 0U),
          .gtior_setting.gtior_b.obdflt = (uint32_t) GPT_PIN_LEVEL_LOW, .gtior_setting.gtior_b.obhld = 0U, .gtior_setting.gtior_b.obe =
                  (uint32_t) true,
          .gtior_setting.gtior_b.obdf = (uint32_t) GPT_GTIOC_DISABLE_PROHIBITED, .gtior_setting.gtior_b.nfben =
                  ((uint32_t) GPT_CAPTURE_FILTER_NONE & 1U),
          .gtior_setting.gtior_b.nfcsb = ((uint32_t) GPT_CAPTURE_FILTER_NONE >> 1U),
#else
    .gtior_setting.gtior = 0U,
#endif
        };
const timer_cfg_t g_npdata_timer_cfg =
{ .mode = TIMER_MODE_PERIODIC,
/* Actual period: 1.3981013333333334 seconds. Actual duty: 50%. */.period_counts = (uint32_t) 0x10000,
  .duty_cycle_counts = 0x8000, .source_div = (timer_source_div_t) 10, .channel = 5, .p_callback = NULL,
  /** If NULL then do not add & */
#if defined(NULL)
    .p_context           = NULL,
#else
  .p_context = &NULL,
#endif
  .p_extend = &g_npdata_timer_extend,
  .cycle_end_ipl = (BSP_IRQ_DISABLED),
#if defined(VECTOR_NUMBER_GPT5_COUNTER_OVERFLOW)
    .cycle_end_irq       = VECTOR_NUMBER_GPT5_COUNTER_OVERFLOW,
#else
  .cycle_end_irq = FSP_INVALID_VECTOR,
#endif
        };
/* Instance structure to use this module. */
const timer_instance_t g_npdata_timer =
{ .p_ctrl = &g_npdata_timer_ctrl, .p_cfg = &g_npdata_timer_cfg, .p_api = &g_timer_on_gpt };
dmac_instance_ctrl_t g_usb_tx_transfer_ctrl;
transfer_info_t g_usb_tx_transfer_info =
{ .transfer_settings_word_b.dest_addr_mode = TRANSFER_ADDR_MODE_INCREMENTED,
  .transfer_settings_word_b.repeat_area = TRANSFER_REPEAT_AREA_SOURCE,
  .transfer_settings_word_b.irq = TRANSFER_IRQ_END,
  .transfer_settings_word_b.chain_mode = TRANSFER_CHAIN_MODE_DISABLED,
  .transfer_settings_word_b.src_addr_mode = TRANSFER_ADDR_MODE_FIXED,
  .transfer_settings_word_b.size = TRANSFER_SIZE_2_BYTE,
  .transfer_settings_word_b.mode = TRANSFER_MODE_BLOCK,
  .p_dest = (void*) 0,
  .p_src = (void const*) USB_SRC_ADDRESS,
  .num_blocks = 0,
  .length = 0, };
const dmac_extended_cfg_t g_usb_tx_transfer_extend =
{ .offset = 0, .src_buffer_size = 1,
#if defined(VECTOR_NUMBER_DMAC1_INT)
    .irq                 = VECTOR_NUMBER_DMAC1_INT,
#else
  .irq = FSP_INVALID_VECTOR,
#endif
  .ipl = (3),
  .channel = 1, .p_callback = NULL, .p_context = NULL, .activation_source = ELC_EVENT_USBFS_FIFO_0, };
const transfer_cfg_t g_usb_tx_transfer_cfg =
{ .p_info = &g_usb_tx_transfer_info, .p_extend = &g_usb_tx_transfer_extend, };
/* Instance structure to use this module. */
const transfer_instance_t g_usb_tx_transfer =
{ .p_ctrl = &g_usb_tx_transfer_ctrl, .p_cfg = &g_usb_tx_transfer_cfg, .p_api = &g_transfer_on_dmac };
dmac_instance_ctrl_t g_usb_rx_transfer_ctrl;
transfer_info_t g_usb_rx_transfer_info =
{ .transfer_settings_word_b.dest_addr_mode = TRANSFER_ADDR_MODE_FIXED,
  .transfer_settings_word_b.repeat_area = TRANSFER_REPEAT_AREA_DESTINATION,
  .transfer_settings_word_b.irq = TRANSFER_IRQ_END,
  .transfer_settings_word_b.chain_mode = TRANSFER_CHAIN_MODE_DISABLED,
  .transfer_settings_word_b.src_addr_mode = TRANSFER_ADDR_MODE_INCREMENTED,
  .transfer_settings_word_b.size = TRANSFER_SIZE_2_BYTE,
  .transfer_settings_word_b.mode = TRANSFER_MODE_BLOCK,
  .p_dest = (void*) USB_DEST_ADDRESS,
  .p_src = (void const*) 0,
  .num_blocks = 0,
  .length = 0, };
const dmac_extended_cfg_t g_usb_rx_transfer_extend =
{ .offset = 0, .src_buffer_size = 1,
#if defined(VECTOR_NUMBER_DMAC0_INT)
    .irq                 = VECTOR_NUMBER_DMAC0_INT,
#else
  .irq = FSP_INVALID_VECTOR,
#endif
  .ipl = (3),
  .channel = 0, .p_callback = usb_ip0_d1fifo_callback, .p_context = NULL, .activation_source = ELC_EVENT_USBFS_FIFO_1, };
const transfer_cfg_t g_usb_rx_transfer_cfg =
{ .p_info = &g_usb_rx_transfer_info, .p_extend = &g_usb_rx_transfer_extend, };
/* Instance structure to use this module. */
const transfer_instance_t g_usb_rx_transfer =
{ .p_ctrl = &g_usb_rx_transfer_ctrl, .p_cfg = &g_usb_rx_transfer_cfg, .p_api = &g_transfer_on_dmac };
usb_instance_ctrl_t g_usb_ctrl;

#if !defined(g_usb_descriptor)
extern usb_descriptor_t g_usb_descriptor;
#endif
#define RA_NOT_DEFINED (1)
const usb_cfg_t g_usb_cfg =
{ .usb_mode = USB_MODE_PERI,
  .usb_speed = USB_SPEED_FS,
  .module_number = 0,
  .type = USB_CLASS_PCDC,
#if defined(g_usb_descriptor)
                .p_usb_reg = g_usb_descriptor,
#else
  .p_usb_reg = &g_usb_descriptor,
#endif
  .usb_complience_cb = NULL,
#if defined(VECTOR_NUMBER_USBFS_INT)
                .irq       = VECTOR_NUMBER_USBFS_INT,
#else
  .irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_USBFS_RESUME)
                .irq_r     = VECTOR_NUMBER_USBFS_RESUME,
#else
  .irq_r = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_USBFS_FIFO_0)
                .irq_d0    = VECTOR_NUMBER_USBFS_FIFO_0,
#else
  .irq_d0 = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_USBFS_FIFO_1)
                .irq_d1    = VECTOR_NUMBER_USBFS_FIFO_1,
#else
  .irq_d1 = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_USBHS_USB_INT_RESUME)
                .hsirq     = VECTOR_NUMBER_USBHS_USB_INT_RESUME,
#else
  .hsirq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_USBHS_FIFO_0)
                .hsirq_d0  = VECTOR_NUMBER_USBHS_FIFO_0,
#else
  .hsirq_d0 = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_USBHS_FIFO_1)
                .hsirq_d1  = VECTOR_NUMBER_USBHS_FIFO_1,
#else
  .hsirq_d1 = FSP_INVALID_VECTOR,
#endif
  .ipl = (12),
  .ipl_r = (12),
  .ipl_d0 = (12),
  .ipl_d1 = (12),
  .hsipl = (BSP_IRQ_DISABLED),
  .hsipl_d0 = (BSP_IRQ_DISABLED),
  .hsipl_d1 = (BSP_IRQ_DISABLED),
#if (BSP_CFG_RTOS != 0)
                .p_usb_apl_callback = NULL,
#else
  .p_usb_apl_callback = NULL,
#endif
#if defined(NULL)
                .p_context = NULL,
#else
  .p_context = &NULL,
#endif
#if (RA_NOT_DEFINED == g_usb_rx_transfer)
#else
  .p_transfer_tx = &g_usb_rx_transfer,
#endif
#if (RA_NOT_DEFINED == g_usb_tx_transfer)
#else
  .p_transfer_rx = &g_usb_tx_transfer,
#endif
        };
#undef RA_NOT_DEFINED

/* Instance structure to use this module. */
const usb_instance_t g_usb =
{ .p_ctrl = &g_usb_ctrl, .p_cfg = &g_usb_cfg, .p_api = &g_usb_on_usb, };

void g_hal_init(void)
{
    g_common_init ();
}
