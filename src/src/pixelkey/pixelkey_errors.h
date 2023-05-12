#ifndef PIXELKEY_ERRORS_H
#define PIXELKEY_ERRORS_H

/**
 * @addtogroup pixelkey
 * @{
 */

/** PixelKey errors. */
typedef enum e_pixelkey_error
{
    PIXELKEY_ERROR_NONE = 0,                        ///< Success; no error.
    PIXELKEY_ERROR_INVALID_ARGUMENT = 1,            ///< A command argument is invalid.
    PIXELKEY_ERROR_BUFFER_FULL = 2,                 ///< A buffer is full.
    PIXELKEY_ERROR_COMMUNICATION_ERROR = 3,         ///< USB communications error occurred.
    PIXELKEY_ERROR_INPUT_BUFFER_OVERFLOW = 4,       ///< USB input buffer has overrun.
    PIXELKEY_ERROR_INDEX_OUT_OF_RANGE = 5,          ///< An indexed NeoPixel does not exist.
    PIXELKEY_ERROR_KEYFRAME_PROCESSING_STOPPED = 6, ///< Keyframe processing is stopped.
    PIXELKEY_ERROR_RTC_NOT_SET = 7,                 ///< System time has not been set.
    PIXELKEY_ERROR_NOT_ENOUGH_ARGUMENTS = 8,        ///< Too few arguments are present in a command.
    PIXELKEY_ERROR_TOO_MANY_ARGUMENTS = 9,          ///< Too many arguments are present in a command.
    PIXELKEY_ERROR_UNKNOWN_COMMAND = 10,            ///< Command is not defined.

    PIXELKEY_ERROR_KEY_NOT_FOUND = 16,              ///< Configuration key is invalid.
    PIXELKEY_ERROR_NV_MEMORY_ERROR = 17,            ///< An error occurred while accessing NV memory.
    PIXELKEY_ERROR_VALUE_OUT_OF_RANGE = 18,         ///< An argument or configuration value is out of range.
    PIXELKEY_ERROR_NV_NOT_INITIALIZED = 19,         ///< NV memory has not yet been initialized.
    PIXELKEY_ERROR_NV_CRC_MISMATCH = 20,            ///< CRC of configuration struct does not match.

    PIXELKEY_ERROR_MISSING_BLOCKS = 32,             ///< Missing firmware upgrade image blocks.
    PIXELKEY_ERROR_CRC_MISMATCH = 33,               ///< Firmware upgrade image CRC does not match expected value.

    PIXELKEY_ERROR_OUT_OF_MEMORY = 48,              ///< No more memory is available in the heap.

} pixelkey_error_t;

/** @} */

#endif // PIXELKEY_ERRORS_H
