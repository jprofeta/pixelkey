#ifndef HELPER_MACROS_H
#define HELPER_MACROS_H

#include <inttypes.h>

/**
 * @defgroup helpers Misc helpers
 * @{
 */

/** 
 * @name Compiler and Linker Macros 
 * @{
 */

/** Wrapper for pragmas. */
#define DO_PRAGMA(x) _Pragma (#x)

/** Push the current diagnostic state. */
#define WARNING_SAVE()          DO_PRAGMA(GCC diagnostic push)

/** Disable a warning. */
#define WARNING_DISABLE(warn)   DO_PRAGMA(GCC diagnostic ignored "-W" warn)

/** Restore the diagnostic state from the last @ref WARNING_SAVE(). */
#define WARNING_RESTORE()       DO_PRAGMA(GCC diagnostic pop)

/** Wraps around a GCC attribute. */
#define ATTRIBUTE(x)            __attribute__ (( x ))

/** Specifies an alignment attribute. */
#define ALIGN(bytes)            ATTRIBUTE(aligned (bytes))

/** Function optimization disabled. */
#define OPTIMIZE_O0             ATTRIBUTE(optimize("O0"))

/** Function optimization level 1. */
#define OPTIMIZE_O1             ATTRIBUTE(optimize("O1"))

/** Function optimization level 2. */
#define OPTIMIZE_O2             ATTRIBUTE(optimize("O2"))

/** Function optimization level 3. */
#define OPTIMIZE_O3             ATTRIBUTE(optimize("O3"))

/** Function optimization for debug. */
#define OPTIMIZE_OG             ATTRIBUTE(optimize("Og"))

/** Function optimization for size. */
#define OPTIMIZE_OS             ATTRIBUTE(optimize("Os"))

/** Function optimization for speed. */
#define OPTIMIZE_OFAST          ATTRIBUTE(optimize("Ofast"))

/** @} */

/**
 * @name Bit-flag macros
 * @{
*/

/** Create a generic int flag. */
#define FLAG(i)      (          1 << i)

/** Create an uint8 flag. */
#define FLAG8(i)     ( UINT8_C(1) << i)

/** Create an uint16 flag. */
#define FLAG16(i)    (UINT16_C(1) << i)

/** Create an uint32 flag. */
#define FLAG32(i)    (UINT32_C(1) << i)

/** Create an uint64 flag. */
#define FLAG64(i)    (UINT64_C(1) << i)

/** @} */

#if DEBUG
/** Debug controlled breakpoint, only set if DEBUG=1. */
#   define BKPT()    __BKPT(0)
#else
/** Debug controlled breakpoint, only set if DEBUG=1. */
#   define BKPT()
#endif

/** Signifies an argument is not used. */
#define ARG_NOT_USED(arg) ((void)(arg))

/**
 * @name Simple Math Macros
 * @{
*/

#ifndef max
/** Selects the maximum between two values. */
#   define max(a,b)     ((a) > (b) ? (a) : (b))
#endif

#ifndef min
/** Selects the minimum between two values. */
#   define min(a,b)     ((a) <= (b) ? (a) : (b))
#endif

#ifndef sign
/** Gets the sign of a value, zero is considered positive. */
#   define sign(a)      ((a) >= 0 ? 1 : -1)
#endif

/** @} */

/** @} */

#endif
