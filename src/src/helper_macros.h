#ifndef HELPER_MACROS_H
#define HELPER_MACROS_H

#include <inttypes.h>

/**
 * @defgroup helpers Misc helpers
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

#if DEBUG
#   define BKPT()    __BKPT(0)
#else
#   define BKPT()
#endif

/** Signifies an argument is not used. */
#define ARG_NOT_USED(arg) ((void)(arg))

/** @} */

#endif
