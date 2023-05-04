#include <stdint.h>

#include "r_dtc.h"

#include "hal_data.h"
#include "hal_device.h"
#include "hal_npdata_transfer.h"
#include "hal_tasks.h"

#include "neopixel.h"
#include "pixelkey.h"
#include "keyframes.h"

/* *****************************************************************************
 * Function declarations
 * ****************************************************************************/

FSP_CPP_HEADER
void R_BSP_WarmStart(bsp_warm_start_event_t event);
FSP_CPP_FOOTER

/* *****************************************************************************
 * Static variables
 * ****************************************************************************/

/* *****************************************************************************
 * Static functions
 * ****************************************************************************/

void hal_frame_timer_callback(timer_callback_args_t * p_args)
{
    npdata_frame_send();
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
    // Setup initial data first.
    pixelkey_framerate_set(30);
    color_build_gamma_table(NEOPIXEL_GAMMA_CORRECTION_DEFAULT);

    // Configure and open the peripherals
    npdata_open();
    g_frame_timer.p_api->open(&g_frame_timer_ctrl, &g_frame_timer_cfg);
    g_frame_timer.p_api->start(&g_frame_timer_ctrl);

    g_usb.p_api->open(&g_usb_ctrl, &g_usb_cfg);

    /* Initial hardware testing. */
    color_t c;
    keyframe_base_t * p_kf;

    keyframe_blink_t * p_kf_blink = (keyframe_blink_t *) keyframe_blink_ctor(NULL);
    p_kf_blink->args.color1 = color_blue;
    p_kf_blink->args.color1.hsv.value = 5;
    p_kf_blink->args.color1_provided = true;
    p_kf_blink->args.color2 = color_green;
    p_kf_blink->args.color2.hsv.value = 5;
    p_kf_blink->args.color2_provided = true;
    p_kf_blink->args.period = 2.0f;

    keyframe_fade_t * p_kf_fade = (keyframe_fade_t *) keyframe_fade_ctor(NULL);
    p_kf_fade->args.colors[0] = color_red.hsv;
    p_kf_fade->args.colors[0].value = 5;
    p_kf_fade->args.colors[1] = color_green.hsv;
    p_kf_fade->args.colors[1].value = 5;
    p_kf_fade->args.colors[2] = color_red.hsv;
    p_kf_fade->args.colors[2].value = 5;
    p_kf_fade->args.colors_len = 3;
    p_kf_fade->args.fade_type = FADE_TYPE_CUBIC;
    p_kf_fade->args.curve = cb_linear;
    p_kf_fade->args.period = 6;
    p_kf_fade->args.push_current = false;
    p_kf_fade->base.modifiers.repeat_count = -1;

    p_kf = keyframe_blink_ctor(NULL);
    memcpy(p_kf, p_kf_blink, sizeof(keyframe_blink_t));
    pixelkey_enqueue_keyframe(0, p_kf);

    p_kf = keyframe_blink_ctor(NULL);
    memcpy(p_kf, p_kf_blink, sizeof(keyframe_blink_t));
    pixelkey_enqueue_keyframe(1, p_kf);

    p_kf = keyframe_blink_ctor(NULL);
    memcpy(p_kf, p_kf_blink, sizeof(keyframe_blink_t));
    pixelkey_enqueue_keyframe(2, p_kf);

    p_kf = keyframe_blink_ctor(NULL);
    memcpy(p_kf, p_kf_blink, sizeof(keyframe_blink_t));
    pixelkey_enqueue_keyframe(3, p_kf);

    // Do the frame processing so it is ready on the first timer overflow.
    extern void pixelkey_task_do_frame(void);
    pixelkey_task_do_frame();

    tasks_run(hal_usb_idle);
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
