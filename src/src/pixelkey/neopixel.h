#ifndef NEOPIXEL_H
#define NEOPIXEL_H

#include <stdint.h>

/**
 * @defgroup neopixel NeoPixel
 * Support for NeoPixel LEDs.
 * @{
 */

/**
 * @page neopixel_timing NeoPixel Timing Requirements
 * 
 * @section data_codes Data codes
 * NeoPixels use a NRZ (non-return-to-zero) code with a fixed timing width to transmit color data.
 * 
 * ```
 *  Code  | Timing waveform
 * ------ | ----------------------------------------------------------------------
 *        |      |<------- 1.20 us ----->|        (Nominal 1.25±0.6 us ≈ 800 kHz)
 *        |       _______
 *    0   |   XXX|       |________________XXX
 *        |      |<-T0H->|<-     TOL   ->|
 *        |        (0.3)        (0.9)                         us (25% duty-cycle)
 *        |
 *        |       ___________
 *    1   |   XXX|           |____________XXX
 *        |      |<-  T1H  ->|<-  T1L  ->|
 *        |          (0.6)      (0.6)                         us (50% duty-cycle)
 *        |
 *   RST  |   XXX_____________________________________XXX
 *        |      |<-              TRST             ->|
 *        |                       (80)                        us
 * ```
 * 
 * @section data_tx Data transmission
 * Data is transmitted during each refresh cycle. All NeoPixels color codes are transmitted in sequence
 * followed by a reset code. Each subsequent NeoPixel re-transmits any bit codes that follow after 
 * reception of its data.
 * 
 * ```
 * NeoPixel  | Data                                | Reset   | Data                               | Reset    
 * --------- | ----------------------------------- | ------- | ---------------------------------- | --------
 *           |                                     |         |                                    |
 *           | <--------------- Data frame 1 --------------->|<--------------- Data frame 2 --------------->
 *           |     Data1       Data2       Data3   |         |   Data1        Data2       Data3   |
 *     D1    |_<̅_̅_̅_̅_̅_̅_̅_̅_̅_̅_̅_><̅_̅_̅_̅_̅_̅_̅_̅_̅_̅_̅_><̅_̅_̅_̅_̅_̅_̅_̅_̅_̅_̅_>____//____<̅_̅_̅_̅_̅_̅_̅_̅_̅_̅_̅_̅_><̅_̅_̅_̅_̅_̅_̅_̅_̅_̅_̅_><̅_̅_̅_̅_̅_̅_̅_̅_̅_̅_̅_>____//____
 *           | <G1><R1><B1><G2><R2><B2><G3><R3><B3>|         |<G1><R1><B1><G2><R2><B2><G3><R3><B3>|
 *     D2    |_____________<̅_̅_̅_̅_̅_̅_̅_̅_̅_̅_̅_><̅_̅_̅_̅_̅_̅_̅_̅_̅_̅_̅_>____//_________________<̅_̅_̅_̅_̅_̅_̅_̅_̅_̅_̅_><̅_̅_̅_̅_̅_̅_̅_̅_̅_̅_̅_>____//____
 *           |             <G2><R2><B2><G3><R3><B3>|         |            <G2><R2><B2><G3><R3><B3>|
 *     D3    |_________________________<̅_̅_̅_̅_̅_̅_̅_̅_̅_̅_̅_>____//_____________________________<̅_̅_̅_̅_̅_̅_̅_̅_̅_̅_̅_>____//____
 *           |                         <G3><R3><B3>|         |                        <G3><R3><B3>|
 *     D4    |_________________________________________//_____________________________________________//____
 *           |                                     |         |                                    |
 * ```
 */

/** Number of color channels per neopixel. */
#define NEOPIXEL_CHANNEL_COUNT  (3U)

/** Total number of bits for a single NeoPixel. */
#define NEOPIXEL_COLOR_BITS     (NEOPIXEL_CHANNEL_COUNT * 8U)

/** Period of the 0-NRZ-code high assertion time, in nanoseconds. */
#define NEOPIXEL_CODE_T0H_NS    (300U)

/** Period of the 0-NRZ-code low assertion time, in nanoseconds. */
#define NEOPIXEL_CODE_T0L_NS    (900U)

/** Period of the 1-NRZ-code high assertion time, in nanoseconds. */
#define NEOPIXEL_CODE_T1H_NS    (600U)

/** Period of the 1-NRZ-code low assertion time, in nanoseconds. */
#define NEOPIXEL_CODE_T1L_NS    (600U)

/** Total period, in nanoseconds, for the high and low portions of the NRZ bit codes. */
#define NEOPIXEL_CODE_TBIT_NS   (1200U)

/** Minumum period, in nanoseconds, of the reset code. */
#define NEOPIXEL_CODE_TRST_NS   (80000U)

/** Individual data block for a NeoPixel: 24-bit color green-red-blue sent MSb first. */
typedef struct st_neopixel_data
{
    union
    {
        /** Data as individual color channels. */
        struct st_gbr
        {
            uint8_t green;  ///< Green channel.
            uint8_t red;    ///< Red channel.
            uint8_t blue;   ///< Blue channel.
        } gbr;
        uint8_t array[3];   ///< Access channels as a flat array.
    };
} neopixel_data_t;

/** @} */

#endif // NEOPIXEL_H
