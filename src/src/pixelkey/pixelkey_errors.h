#ifndef PIXELKEY_ERRORS_H
#define PIXELKEY_ERRORS_H

/** PixelKey errors. */
typedef enum e_pixelkey_error
{
    PIXELKEY_ERROR_NONE = 0,                        ///<
    PIXELKEY_ERROR_INVALID_ARGUMENT = 1,            ///<
    PIXELKEY_ERROR_BUFFER_FULL = 2,                 ///<
    PIXELKEY_ERROR_COMMUNICATION_ERROR = 3,         ///<
    PIXELKEY_ERROR_INPUT_BUFFER_OVERFLOW = 4,       ///<
    PIXELKEY_ERROR_INDEX_OUT_OF_RANGE = 5,          ///<
    PIXELKEY_ERROR_KEYFRAME_PROCESSING_STOPPED = 6, ///< Keyframe processing is stopped.
    PIXELKEY_ERROR_RTC_NOT_SET = 7,                 ///<

    PIXELKEY_ERROR_KEY_NOT_FOUND = 16,              ///<
    PIXELKEY_ERROR_NV_MEMORY_ERROR = 17,            ///<
    PIXELKEY_ERROR_VALUE_OUT_OF_RANGE = 18,         ///<

    PIXELKEY_ERROR_MISSING_BLOCKS = 32,             ///<
    PIXELKEY_ERROR_CRC_MISMATCH = 33                ///<

} pixelkey_error_t;

#endif // PIXELKEY_ERRORS_H
