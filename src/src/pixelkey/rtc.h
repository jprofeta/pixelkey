#ifndef SERIAL_H
#define SERIAL_H

/**
 * @file
 * @defgroup pixelkey__rtc Real-Time Clock Abstraction
 * @ingroup pixelkey
 * @{
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#include "pixelkey_errors.h"

/** Time-zone offset from UTC in minutes. */
typedef int16_t rtc_tzoffset_t;

/** Time period in milliseconds. */
typedef uint32_t rtc_period_ms_t;

/** Real-time clock interface abstraction. */
typedef struct st_rtc_api
{
    /**
     * Gets the current system time in UTC.
     * @param[out] p_time Pointer to time struct to populate.
     */
    pixelkey_error_t (* time_get)(struct tm * p_time);

    /**
     * Sets the current system time in UTC.
     * @param[in] p_time Pointer to the current time.
     */
    pixelkey_error_t (* time_set)(struct tm * const p_time);

    rtc_tzoffset_t   (* tzoffset_get)(void);
    pixelkey_error_t (* tzoffset_set)(rtc_tzoffset_t tzoffset);

    /**
     * Sets a timer alarm to go off in the near future after a set period.
     * @param period Number of milliseconds in the future to alarm at.
     */
    pixelkey_error_t (* timer_alarm_set)(rtc_period_ms_t period);

    /**
     * Sets a calendar alarm to go off at a set date-time in the future.
     * @param[in] p_alarm_time Data-time in UTC for the calendar alarm.
     */
    pixelkey_error_t (* calendar_alarm_set)(struct tm * const p_alarm_time);
} rtc_api_t;

rtc_api_t const * rtc(void);
void rtc_register(rtc_api_t const * p_instance);

/** @} */

#endif
