/**
 * @file
 * @defgroup hal__version__internals Version Internals
 * @ingroup hal__version
 * @{
*/

#include "version.h"

/** PixelKey version number. */
const version_t pixelkey_version __attribute__ ((section (".version"))) =
{
    .version_b =
    {
        .patch = PIXELKEY_VERSION_PATCH,
        .minor = PIXELKEY_VERSION_MINOR,
        .major = PIXELKEY_VERSION_MAJOR
    }
};

/** PixelKey version number string. */
const char g_pixelkey_version_str[PIXELKEY_VERSION_STR_MAX_LENGTH] __attribute__ ((section (".version"))) = PIXELKEY_VERSION_STR;

/** PixelKey product string. */
const char g_pixelkey_product_str[PIXELKEY_PRODUCT_STR_LENGTH] __attribute__ ((section (".version"))) = PIXELKEY_PRODUCT_STR;

/** @} */
