/* generated HAL header file - do not edit */
#ifndef HAL_DATA_H_
#define HAL_DATA_H_
#include <stdint.h>
#include "bsp_api.h"
#include "common_data.h"
#include "r_crc.h"
#include "r_crc_api.h"
#include "r_flash_lp.h"
#include "r_flash_api.h"
#include "r_gpt.h"
#include "r_timer_api.h"
#include "r_rtc.h"
#include "r_rtc_api.h"
#include "r_dmac.h"
#include "r_transfer_api.h"
#include "r_usb_basic.h"
#include "r_usb_basic_api.h"
#include "r_usb_pcdc_api.h"
FSP_HEADER
extern const crc_instance_t g_crc0;
extern crc_instance_ctrl_t g_crc0_ctrl;
extern const crc_cfg_t g_crc0_cfg;
/* Flash on Flash LP Instance. */
extern const flash_instance_t g_flash0;

/** Access the Flash LP instance using these structures when calling API functions directly (::p_api is not used). */
extern flash_lp_instance_ctrl_t g_flash0_ctrl;
extern const flash_cfg_t g_flash0_cfg;

#ifndef NULL
void NULL(flash_callback_args_t *p_args);
#endif
/** Timer on GPT Instance. */
extern const timer_instance_t g_frame_timer;

/** Access the GPT instance using these structures when calling API functions directly (::p_api is not used). */
extern gpt_instance_ctrl_t g_frame_timer_ctrl;
extern const timer_cfg_t g_frame_timer_cfg;

#ifndef hal_frame_timer_callback
void hal_frame_timer_callback(timer_callback_args_t *p_args);
#endif
/* RTC Instance. */
extern const rtc_instance_t g_rtc;

/** Access the RTC instance using these structures when calling API functions directly (::p_api is not used). */
extern rtc_instance_ctrl_t g_rtc_ctrl;
extern const rtc_cfg_t g_rtc_cfg;

#ifndef hal_rtc_callback
void hal_rtc_callback(rtc_callback_args_t *p_args);
#endif
/* Transfer on DMAC Instance. */
extern const transfer_instance_t g_npdata_transfer;

/** Access the DMAC instance using these structures when calling API functions directly (::p_api is not used). */
extern dmac_instance_ctrl_t g_npdata_transfer_ctrl;
extern const transfer_cfg_t g_npdata_transfer_cfg;

#ifndef npdata_transfer_callback
void npdata_transfer_callback(dmac_callback_args_t *p_args);
#endif
/** Timer on GPT Instance. */
extern const timer_instance_t g_npdata_timer;

/** Access the GPT instance using these structures when calling API functions directly (::p_api is not used). */
extern gpt_instance_ctrl_t g_npdata_timer_ctrl;
extern const timer_cfg_t g_npdata_timer_cfg;

#ifndef NULL
void NULL(timer_callback_args_t *p_args);
#endif
/* Basic on USB Instance. */
extern const usb_instance_t g_usb;

/** Access the USB instance using these structures when calling API functions directly (::p_api is not used). */
extern usb_instance_ctrl_t g_usb_ctrl;
extern const usb_cfg_t g_usb_cfg;

#ifndef NULL
void NULL(void*);
#endif

#if 2 == BSP_CFG_RTOS
#ifndef NULL
void NULL(usb_event_info_t *, usb_hdl_t, usb_onoff_t);
#endif
#endif
/** CDC Driver on USB Instance. */
void hal_entry(void);
void g_hal_init(void);
FSP_FOOTER
#endif /* HAL_DATA_H_ */
