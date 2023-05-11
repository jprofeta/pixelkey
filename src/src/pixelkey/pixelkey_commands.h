#ifndef PIXELKEY_COMMANDS_H
#define PIXELKEY_COMMANDS_H

/**
 * @file
 * @defgroup pixelkey__commands PixelKey Command Set
 * @ingroup pixelkey
 * @{
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "keyframes.h"

/** Prefix for non-keyframe commands. */
#define CMD_PREFIX                  ('$')

/** Prefix for repeat keyframe modifier command. */
#define CMD_REPEAT_MOD_PREFIX       ('^')

/** Prefix for schedule keyframe modifier command. */
#define CMD_SCHEDULE_MOD_PREFIX     ('@')

/** Prefix for group keyframe modifier command. */
#define CMD_GROUP_MOD_PREFIX        ('#')

/** Max string length for configuration keys. */
#define CMD_CONFIG_KEY_MAX_LENGTH   (32)
/** Max string length for time representations. */
#define CMD_TIME_SET_MAX_LENGTH     (26)    // YYYY-MM-DD HH:mm:ssZZZZZZ\0

/** Maximum length of the index array for keyfrmaes. */
#define CMD_KEYFRAME_WRAPPER_CHANNELS_MAX_LENGTH     (16)

/** Maximum channel number that can be specified for keyframes. */
#define CMD_KEYFRAME_MAX_CHANNEL_NUMBER (0x7FFF)

/** Command types. */
typedef enum e_cmd_type
{
    CMD_TYPE_UNDEFINED,             ///< Undefined command.
    CMD_TYPE_KEYFRAME_WRAPPER,      ///< Command wrapper around a keyframe.
    CMD_TYPE_KEYFRAME_MOD_REPEAT,   ///< Keyframe repeat modifier command.
    CMD_TYPE_KEYFRAME_MOD_SCHEDULE, ///< Keyframe schedule modifier command.
    CMD_TYPE_KEYFRAME_MOD_GROUP,    ///< Keyframe group modifier command.
    CMD_TYPE_CONFIG_GET,            ///< Get a configuration value.
    CMD_TYPE_CONFIG_SET,            ///< Set a configuration value.
    CMD_TYPE_RESUME,                ///< Resume keyframe processing.
    CMD_TYPE_STOP,                  ///< Stop keyframe processing and go idle.
    CMD_TYPE_STATUS,                ///< Display device status.
    CMD_TYPE_VERSION,               ///< Display device firmware version.
    CMD_TYPE_TIME_GET,              ///< Get the current system time.
    CMD_TYPE_TIME_SET               ///< Set the current system time.
} cmd_type_t;

/** Value types. */
typedef enum e_value_type
{
    VALUE_TYPE_BOOLEAN, ///< Boolean value.
    VALUE_TYPE_INTEGER, ///< Integer value.
    VALUE_TYPE_FLOAT    ///< Float value.
} value_type_t;

/** Command which wraps a keyframe. */
typedef struct st_cmd_args_keyframe_wrapper
{
    keyframe_base_t * p_keyframe; ///< Pointer to the parsed keyframe.
    uint16_t          channels[CMD_KEYFRAME_WRAPPER_CHANNELS_MAX_LENGTH]; ///< An array of channels to apply this keyframe for.
} cmd_args_keyframe_wrapper_t;

/** Arguments to config-set command. */
typedef struct st_cmd_args_config_get
{
    char key[CMD_CONFIG_KEY_MAX_LENGTH];    ///< Configuration key.
} cmd_args_config_get_t;

/** Arguments to config-set command. */
typedef struct st_cmd_args_config_set
{
    char key[CMD_CONFIG_KEY_MAX_LENGTH];    ///< Configuration key.

    value_type_t value_type;                ///< Type for value.
    union
    {
        int32_t  i32;   ///< Integer config value.
        float    f32;   ///< Float config value.
        bool     b;     ///< Boolean config value.
    } value;            ///< Configuration value.
    
} cmd_args_config_set_t;

/** Arguments to time-set command. */
typedef struct st_cmd_args_time_set
{
    /** BCD coded time. */
    struct
    {
        uint16_t year;      ///< Year.
        uint8_t  month;     ///< Month.
        uint8_t  day;       ///< Day.
        uint8_t  hour;      ///< Hour (24).
        uint8_t  minute;    ///< Minute.
        uint8_t  second;    ///< Second.
        uint8_t  tz_hour;   ///< Time-zone hour offset (MSB signifies sign).
        uint8_t  tz_minute; ///< Time-zone minute offset.
    } time_bcd;
} cmd_args_time_set_t;

/** Arguments for repeat keyframe modifier command. */
typedef struct st_cmd_args_keyframe_mod_repeat
{
    int32_t repeat_count;   ///< Number of repeats to perform, -1 is indefinite, 0 = 1.
} cmd_args_keyframe_mod_repeat_t;

/** Arguments for schedule keyframe modifier command. */
typedef struct st_cmd_args_keyframe_mod_schedule
{
    /// @todo Add support for schedule modifiers.
} cmd_args_keyframe_mod_schedule_t;

/** Parsed command and arguments. */
typedef struct st_cmd
{
    cmd_type_t type;    ///< Command type.
    void *     p_args;  ///< Pointer to command arguments.
} cmd_t;

/** Parsed command list. */
typedef struct st_cmd_list
{
    cmd_t * p_cmd;               ///< Pointer to the current command.
    struct st_cmd_list * p_next; ///< Pointer to the next command list element.
} cmd_list_t;

void pixelkey_cmd_list_free(cmd_list_t * p_cmd_list);
pixelkey_error_t pixelkey_command_parse(char * command_str, cmd_list_t ** p_cmd_list);

/** @} */

#endif
