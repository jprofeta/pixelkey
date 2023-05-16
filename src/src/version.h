#ifndef VERSION_H
#define VERSION_H

/**
 * @file
 * @defgroup hal__version Version
 * @ingroup hal
 * @{
*/

#include <stdint.h>
#include <assert.h>

/** Stringify symbols. */
#define XSTR(x)  #x
/** Expands symbols to be stringified. */
#define STR(x)   XSTR(x)

/** PixelKey major version number. */
#define PIXELKEY_VERSION_MAJOR   0      // Valid range: 0-255; do not add parentheses.
/** PixelKey minor version number. */
#define PIXELKEY_VERSION_MINOR   1      // Valid range: 0-255; do not add parentheses.
/** PixelKey patch version number. */
#define PIXELKEY_VERSION_PATCH   0      // Valid range: 0-999; do not add parentheses.

/** PixelKey version string. */
#define PIXELKEY_VERSION_STR     STR(PIXELKEY_VERSION_MAJOR)"."STR(PIXELKEY_VERSION_MINOR)"."STR(PIXELKEY_VERSION_PATCH)

static_assert(PIXELKEY_VERSION_MAJOR >= 0 && PIXELKEY_VERSION_MAJOR <= 255 && sizeof(STR(PIXELKEY_VERSION_MAJOR)) <= 4, "Major version field invalid.");
static_assert(PIXELKEY_VERSION_MINOR >= 0 && PIXELKEY_VERSION_MINOR <= 255 && sizeof(STR(PIXELKEY_VERSION_MINOR)) <= 4, "Minor version field invalid.");
static_assert(PIXELKEY_VERSION_PATCH >= 0 && PIXELKEY_VERSION_PATCH <= 999 && sizeof(STR(PIXELKEY_VERSION_PATCH)) <= 4, "Patch version field invalid.");

/** Maximum allowable length of the version string. */
#define PIXELKEY_VERSION_STR_MAX_LENGTH     (12)    // 3 major + . + 3 minor + . + 3 patch + \0

#define PIXELKEY_PRODUCT_STR        "PixelKey"
#define PIXELKEY_PRODUCT_STR_LENGTH sizeof(PIXELKEY_PRODUCT_STR)

/** Version struct. */
typedef union u_version
{
    struct
    {
        uint16_t patch; ///< Patch field.
        uint8_t  minor; ///< Minor field.
        uint8_t  major; ///< Major field.
    } version_b;        ///< Version fields.
    uint32_t version;   ///< Version word.
} version_t;

static_assert(sizeof(version_t) == sizeof(uint32_t), "Version must be 32-bits.");

extern const version_t g_pixelkey_version;
extern const char g_pixelkey_version_str[PIXELKEY_VERSION_STR_MAX_LENGTH];
extern const char g_pixelkey_product_str[PIXELKEY_PRODUCT_STR_LENGTH];

/** @} */

#endif
