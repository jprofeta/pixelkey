#ifndef LOG_H
#define LOG_H

#include <stdint.h>
#include <stdbool.h>

#include "hal_data.h"
#include "helper_macros.h"

/**
 * @file
 * @defgroup log Diagnostics Logging
 * @{
 */

/** Diagnostic signals. */
typedef enum e_diag_signal
{
    DIAG_SIGNAL_NONE,
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
    DIAG_COUNTER_COUNT
} diag_counter_t;

typedef enum e_diag_timing
{
    DIAG_TIMING_FRAME_TX,
    DIAG_TIMING_FRAME_BLOCK_TX,
    DIAG_TIMING_FRAME_BLOCK_BUFFER,
    DIAG_TIMING_FRAME_RENDER,
    DIAG_TIMING_COUNT
} diag_timing_t;

typedef struct st_diag_timing_data
{
    uint32_t last;   ///< Last timing data.
    uint32_t min;    ///< Min timing data.
    uint32_t max;    ///< Max timing data.
    uint32_t sum;    ///< Total sum of timing data.
    uint32_t cnt;    ///< Number of entries in the timing data sum.
    uint32_t _start; ///< Start time for delta.
} diag_timing_data_t;

typedef struct st_diag
{
    diag_signal_t      signals;
    uint32_t           counters[DIAG_COUNTER_COUNT];
    diag_timing_data_t timing_data[DIAG_TIMING_COUNT];
} diag_t;

#if DIAGNOSTICS_ENABLE
extern diag_t g_diag;

 #define LOG_SIGNAL(signal)         do { g_diag.signals |= (signal); } while (0)
 #define LOG_SIGNAL_CLEAR(signal)   do { g_diag.signals &= ~(signal); } while (0)
 #define LOG_SIGNAL_RESET()         do { g_diag.signals = 0; } while (0)

 #define LOG_COUNTER(counter)       do { g_diag.counters[(counter)] += 1; } while (0)
 #define LOG_COUNTER_CLEAR(counter) do { g_diag.counters[(counter)] = 0; } while (0)
 #define LOG_COUNTER_RESET()        do { for (int _i = 0; _i < DIAG_COUNTER_COUNT; _i++) g_diag.counters[_i] = 0; } while (0)

 #define LOG_TIME_START(timer)      do { g_diag.timing_data[(timer)]._start = SysTick->VAL; } while (0)
 #define LOG_TIME(timer)            do { \
                                        uint32_t _t = SysTick->VAL; \
                                        uint32_t _d = (g_diag.timing_data[(timer)]._start - _t) & SysTick_VAL_CURRENT_Msk; \
                                        g_diag.timing_data[(timer)]._start = _t; \
                                        g_diag.timing_data[(timer)].last = _d; \
                                        g_diag.timing_data[(timer)].max = max(g_diag.timing_data[(timer)].max, _d); \
                                        g_diag.timing_data[(timer)].min = min(g_diag.timing_data[(timer)].min, _d); \
                                        g_diag.timing_data[(timer)].sum += _d; \
                                        g_diag.timing_data[(timer)].cnt++; \
                                    } while (0)
 #define LOG_TIME_RESET(timer)      do { g_diag.timing_data[(timer)] = (diag_timing_data_t) { ._start = 0, .last = 0, .min = UINT32_MAX, .max = 0, .sum = 0, .cnt = 0 }; } while (0)
 #define LOG_TIME_RESET_ALL()       do { for (int _i = 0; _i < DIAG_TIMING_COUNT; _i++) LOG_TIME_RESET(_i); } while (0)
#else
 #define LOG_SIGNAL(signal)
 #define LOG_SIGNAL_CLEAR(signal)
 #define LOG_SIGNAL_RESET()

 #define LOG_COUNTER(counter)
 #define LOG_COUNTER_CLEAR(counter)
 #define LOG_COUNTER_RESET()

 #define LOG_TIME_START(timer)
 #define LOG_TIME(timer)
 #define LOG_TIME_RESET(timer)
 #define LOG_TIME_RESET_ALL()
#endif

/** @} */

#endif
