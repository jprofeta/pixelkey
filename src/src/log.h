#ifndef LOG_H
#define LOG_H

#include <stdint.h>
#include <stdbool.h>

#include "helper_macros.h"

/**
 * @file
 * @defgroup log Diagnostics Logging
 * @{
 */

/** Diagnostic signals. */
typedef enum e_diag_signal
{
    DIAG_SIGNAL_RENDER_UNDERFLOW = FLAG32(0),   ///< The frame is stale and has not been updated since the last transmission.
    DIAG_SIGNAL_NPDATA_OVERFLOW  = FLAG32(1),   ///< The data transfer function was called before the previous transmission completed.
    DIAG_SIGNAL_USB_ERROR        = FLAG32(2),   ///< A USB error occurred.
    DIAG_SIGNAL_RENDER_ERROR     = FLAG32(3),   ///< A render error occurred.
} diag_signal_t;

typedef enum e_diag_counter
{
    DIAG_COUNTER_USB_READ_ERROR,
    DIAG_COUNTER_USB_WRITE_ERROR,
    DIAG_COUNTER_USB_OPERATION_ERROR,
    DIAG_COUNTER_COUNT,
} diag_counter_t;

typedef struct st_diag
{
    diag_signal_t signals;
    uint32_t      counters[DIAG_COUNTER_COUNT];
} diag_t;

#if DIAGNOSTICS_ENABLE
extern diag_t g_diag;

 #define LOG_SIGNAL(signal)         do { g_diag.signals |= (signal); } while (0)
 #define LOG_SIGNAL_CLEAR(signal)   do { g_diag.signals &= ~(signal); } while (0)
 #define LOG_SIGNAL_RESET()         do { g_diag.signals = 0; } while (0)

 #define LOG_COUNTER(counter)       do { g_diag.counters[(counter)] += 1; } while (0)
 #define LOG_COUNTER_CLEAR(counter) do { g_diag.counters[(counter)] = 0; } while (0)
 #define LOG_COUNTER_RESET()        do { for (int __i = 0; __i < DIAG_COUNTER_COUNT; __i++) g_diag.counters[__i] = 0; } while (0)
#else
 #define LOG_SIGNAL(signal)
 #define LOG_SIGNAL_CLEAR(signal)
 #define LOG_SIGNAL_RESET()

 #define LOG_COUNTER(counter)
 #define LOG_COUNTER_CLEAR(counter)
 #define LOG_COUNTER_RESET()
#endif

/** @} */

#endif
