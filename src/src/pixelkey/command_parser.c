/**
 * @file
 * @defgroup pixelkey__commandparser__internals Command Parser Internals
 * @ingroup pixelkey__commands
 * @{
*/

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "hal_device.h"
#include "ring_buffer.h"

#include "pixelkey_errors.h"
#include "pixelkey_commands.h"

static char * trim(char * str);
static void lower(char * str);

static pixelkey_error_t parse_no_args(cmd_type_t type, char * arg_ctx, cmd_t * p_cmd);
static pixelkey_error_t parse_config_get(char * arg_ctx, cmd_t * p_cmd);
static pixelkey_error_t parse_config_set(char * arg_ctx, cmd_t * p_cmd);
static pixelkey_error_t parse_time_set(char * arg_ctx, cmd_t * p_cmd);
static pixelkey_error_t parse_keyframe(char * cmd_tok, cmd_t * p_cmd);

/**
 * Parses a command string.
 * @param[in]  command_str Pointer to the command string to parse.
 * @param[out] p_cmd_list  Pointer to store the command list. 
 */
pixelkey_error_t pixelkey_command_parse(char * command_str, cmd_list_t ** p_cmd_list)
{
    char * cmd_tok_ctx = NULL;
    char * cmd_tok = NULL;
    cmd_tok = strtok_r(command_str, ";", &cmd_tok_ctx);

    if (cmd_tok == NULL)
    {
        return PIXELKEY_ERROR_UNKNOWN_COMMAND;
    }

    pixelkey_error_t parse_error = PIXELKEY_ERROR_NONE;

    *p_cmd_list = (cmd_list_t *)malloc(sizeof(cmd_list_t));
    if (*p_cmd_list == NULL)
    {
        return PIXELKEY_ERROR_OUT_OF_MEMORY;
    }
    memset(*p_cmd_list, 0, sizeof(cmd_list_t));

    // Current command list node 
    cmd_list_t * p_list = *p_cmd_list;

    do
    {
        cmd_tok = trim(cmd_tok);
        if (*cmd_tok == '\0')
        {
            parse_error = PIXELKEY_ERROR_UNKNOWN_COMMAND;
            break;
        }

        cmd_t * p_cmd = (cmd_t *)malloc(sizeof(cmd_t));
        if (p_cmd == NULL)
        {
            parse_error = PIXELKEY_ERROR_OUT_OF_MEMORY;
            break;
        }
        memset(p_cmd, 0, sizeof(cmd_t));

        p_list->p_cmd = p_cmd;

        if (*cmd_tok == CMD_PREFIX)
        {
            // Parse as non-keyframe command.
            char * arg_ctx = NULL;  // This should be the start of the next argument token in cmd_tok.
            char * cmd_name = strtok_r(cmd_tok, " ", &arg_ctx);
            lower(cmd_name);
            if (!strcmp(cmd_name, "$config-get"))
            {
                parse_error = parse_config_get(arg_ctx, p_cmd);
            }
            else if (!strcmp(cmd_name, "$config-set"))
            {
                parse_error = parse_config_set(arg_ctx, p_cmd);
            }
            else if (!strcmp(cmd_name, "$help"))
            {
                parse_error = parse_no_args(CMD_TYPE_HELP, arg_ctx, p_cmd);
            }
            else if (!strcmp(cmd_name, "$resume"))
            {
                parse_error = parse_no_args(CMD_TYPE_RESUME, arg_ctx, p_cmd);
            }
            else if (!strcmp(cmd_name, "$stop"))
            {
                parse_error = parse_no_args(CMD_TYPE_STOP, arg_ctx, p_cmd);
            }
            else if (!strcmp(cmd_name, "$status"))
            {
                parse_error = parse_no_args(CMD_TYPE_STATUS, arg_ctx, p_cmd);
            }
            else if (!strcmp(cmd_name, "$version"))
            {
                parse_error = parse_no_args(CMD_TYPE_VERSION, arg_ctx, p_cmd);
            }
            else if (!strcmp(cmd_name, "$time-get"))
            {
                parse_error = parse_no_args(CMD_TYPE_TIME_GET, arg_ctx, p_cmd);
            }
            else if (!strcmp(cmd_name, "$time-set"))
            {
                parse_error = parse_time_set(arg_ctx, p_cmd);
            }
            else
            {
                parse_error = PIXELKEY_ERROR_UNKNOWN_COMMAND;
            }
        }
        else if (*cmd_tok == CMD_HELP_PREFIX)
        {
            parse_error = parse_no_args(CMD_TYPE_HELP, NULL, p_cmd);
        }
        else if (*cmd_tok == CMD_REPEAT_MOD_PREFIX)
        {
            cmd_tok++;  // Move forward past the prefix.
            char c = *cmd_tok;
            if (!isdigit(c) && c != '-')
            {
                parse_error = PIXELKEY_ERROR_INVALID_ARGUMENT;
            }
            else
            {
                p_cmd->type = CMD_TYPE_KEYFRAME_MOD_REPEAT;
                p_cmd->p_args = malloc(sizeof(cmd_args_keyframe_mod_repeat_t));
                if (p_cmd->p_args == NULL)
                {
                    parse_error = PIXELKEY_ERROR_OUT_OF_MEMORY;
                }
                else
                {
                    cmd_args_keyframe_mod_repeat_t * p_mod = (cmd_args_keyframe_mod_repeat_t *)p_cmd->p_args;
                    p_mod->repeat_count = atoi(cmd_tok);

                    parse_error = PIXELKEY_ERROR_NONE;
                }
            }
        }
        else if (*cmd_tok == CMD_SCHEDULE_MOD_PREFIX)
        {
            // Not supported yet
            parse_error = PIXELKEY_ERROR_UNKNOWN_COMMAND;
        }
        else if (*cmd_tok == CMD_GROUP_BEGIN_MOD_PREFIX)
        {
            // Not supported yet
            parse_error = PIXELKEY_ERROR_UNKNOWN_COMMAND;
        }
        else if (*cmd_tok == CMD_GROUP_END_MOD_PREFIX)
        {
            // Not supported yet
            parse_error = PIXELKEY_ERROR_UNKNOWN_COMMAND;
        }
        else if (!strcmp(cmd_tok, "help"))
        {
            parse_error = parse_no_args(CMD_TYPE_HELP, NULL, p_cmd);
        }
        else
        {
            // Parse as keyframe
            parse_error = parse_keyframe(cmd_tok, p_cmd);
        }

        cmd_tok = strtok_r(NULL, ";", &cmd_tok_ctx);
        if (cmd_tok != NULL && parse_error == PIXELKEY_ERROR_NONE)
        {
            p_list->p_next = (cmd_list_t *)malloc(sizeof(cmd_list_t));
            if (p_list->p_next == NULL)
            {
                parse_error = PIXELKEY_ERROR_OUT_OF_MEMORY;
            }
            else
            {
                memset(p_list->p_next, 0, sizeof(cmd_list_t));
                p_list = p_list->p_next;
            }
        }
    } while (cmd_tok != NULL && parse_error == PIXELKEY_ERROR_NONE);

    // Perform cleanup.
    // MUST ensure malloc'd memory is freed!
    if (parse_error != PIXELKEY_ERROR_NONE)
    {
        pixelkey_cmd_list_free(*p_cmd_list);

        // Null out p_list so it can't be used.
        *p_cmd_list = NULL;
    }

    return parse_error;
}

/**
 * Removes white-space from an input string; replaces trailing white-space with NULL character.
 * @param[in] str String to trim.
 * @return Pointer to start of the first non-white-space character in str.
 */
static char * trim(char * str)
{
    size_t str_len = strlen(str);
    for (size_t i = 0; i < str_len; i++)
    {
        char c = *str;
        if (isspace(c))
        {
            str++;
            str_len--;
        }
        else
        {
            break;
        }
    }
    char * str_end = &str[str_len - 1];
    for (size_t i = str_len; i > 0; i--)
    {
        char c = *str_end;
        if (isspace(c))
        {
            *str_end = '\0';
            str_end--;
        }
        else
        {
            break;
        }
    }

    return str;
}

/**
 * Converts a string to lower-case.
 * @param[in,out] str String to transform.
 */
static void lower(char * str)
{
    for (; *str != '\0'; str++)
    {
        *str = (char)tolower(*str);
    }
}

/**
 * Converts a character to an integer.
 * @param c Character to convert.
 * @return The integer value of c.
 * @retval UINT8_MAX On non-digit character.
 */
static uint8_t char_to_u8(char c)
{
    if (c < '0' || c > '9')
    {
        return UINT8_MAX;
    }
    else
    {
        return (uint8_t)(c - '0');
    }
}

/**
 * Parses a command that takes no arguments.
 * @param         type    The command type to return.
 * @param[in]     arg_ctx Argument tokenizer context.
 * @param[in,out] p_cmd   Pointer to the command structure to populate.
 * @retval PIXELKEY_ERROR_TOO_MANY_ARGUMENTS Additional arguments were specified; expects no arguments.
 * @retval PIXELKEY_ERROR_NONE               Parsing was successful.
 */
static pixelkey_error_t parse_no_args(cmd_type_t type, char * arg_ctx, cmd_t * p_cmd)
{
    char * next_arg = strtok_r(NULL, " ", &arg_ctx);

    p_cmd->type = type;
    if (next_arg != NULL)
    {
        // Extra args; bad command.
        return PIXELKEY_ERROR_TOO_MANY_ARGUMENTS;
    }
    else
    {
        return PIXELKEY_ERROR_NONE;
    }
}

/**
 * Parses config-get command arguments.
 * @param[in]     arg_ctx Argument tokenizer context.
 * @param[in,out] p_cmd   Pointer to the command structure to populate.
 * @retval PIXELKEY_ERROR_NOT_ENOUGH_ARGUMENTS Configuration key was not provided.
 * @retval PIXELKEY_ERROR_INVALID_ARGUMENT     Configuration key is invalid or too long.
 * @retval PIXELKEY_ERROR_TOO_MANY_ARGUMENTS   Additional, unexpected arguments were specified.
 * @retval PIXELKEY_ERROR_OUT_OF_MEMORY        Failed to malloc argument structure.
 * @retval PIXELKEY_ERROR_NONE                 Parsing was successful.
 */
static pixelkey_error_t parse_config_get(char * arg_ctx, cmd_t * p_cmd)
{
    char * next_arg = strtok_r(NULL, " ", &arg_ctx);

    p_cmd->type = CMD_TYPE_CONFIG_GET;
    if (next_arg == NULL)
    {
        return PIXELKEY_ERROR_NOT_ENOUGH_ARGUMENTS;
    }
    if (strlen(next_arg) > CMD_CONFIG_KEY_MAX_LENGTH - 1)    // leave room for the '\0'.
    {
        return PIXELKEY_ERROR_INVALID_ARGUMENT;
    }

    p_cmd->p_args = malloc(sizeof(cmd_args_config_get_t));
    if (p_cmd->p_args == NULL)
    {
        return PIXELKEY_ERROR_OUT_OF_MEMORY;
    }
    strcpy(((cmd_args_config_get_t *)p_cmd->p_args)->key, next_arg);

    if (strtok_r(NULL, " ", &arg_ctx) != NULL)
    {
        // Extra args; bad command.
        free(p_cmd->p_args);
        p_cmd->p_args = NULL;
        return PIXELKEY_ERROR_TOO_MANY_ARGUMENTS;
    }
    else
    {
        return PIXELKEY_ERROR_NONE;
    }
}

/**
 * Parses config-set command arguments.
 * @param[in]     arg_ctx Argument tokenizer context.
 * @param[in,out] p_cmd   Pointer to the command structure to populate.
 * @retval PIXELKEY_ERROR_NOT_ENOUGH_ARGUMENTS Configuration key or value was not provided.
 * @retval PIXELKEY_ERROR_INVALID_ARGUMENT     Configuration key or value is invalid or too long.
 * @retval PIXELKEY_ERROR_TOO_MANY_ARGUMENTS   Additional, unexpected arguments were specified.
 * @retval PIXELKEY_ERROR_OUT_OF_MEMORY        Failed to malloc argument structure.
 * @retval PIXELKEY_ERROR_NONE                 Parsing was successful.
 */
static pixelkey_error_t parse_config_set(char * arg_ctx, cmd_t * p_cmd)
{
    char * next_arg = strtok_r(NULL, " ", &arg_ctx);

    p_cmd->type = CMD_TYPE_CONFIG_SET;
    if (next_arg == NULL)
    {
        return PIXELKEY_ERROR_NOT_ENOUGH_ARGUMENTS;
    }
    if (strlen(next_arg) > CMD_CONFIG_KEY_MAX_LENGTH - 1)    // leave room for the '\0'.
    {
        return PIXELKEY_ERROR_INVALID_ARGUMENT;
    }

    p_cmd->p_args = malloc(sizeof(cmd_args_config_set_t));
    if (p_cmd->p_args == NULL)
    {
        return PIXELKEY_ERROR_OUT_OF_MEMORY;
    }
    cmd_args_config_set_t * p_args = p_cmd->p_args;
    strcpy(p_args->key, next_arg);

    next_arg = strtok_r(NULL, " ", &arg_ctx);
    if (next_arg == NULL)
    {
        return PIXELKEY_ERROR_NOT_ENOUGH_ARGUMENTS;
    }

    lower(next_arg);
    if (!strcmp(next_arg, "true"))
    {
        p_args->value_type = VALUE_TYPE_BOOLEAN;
        p_args->value.b = true;
    }
    else if (!strcmp(next_arg, "false"))
    {
        p_args->value_type = VALUE_TYPE_BOOLEAN;
        p_args->value.b = false;
    }
    else if (strpbrk(next_arg, ".e") != NULL)
    {
        // Parse as a float
        p_args->value_type = VALUE_TYPE_FLOAT;
        char * end_ptr = NULL;
        p_args->value.f32 = strtof(next_arg, &end_ptr);
        if (end_ptr == next_arg || *end_ptr != '\0')
        {
            free(p_cmd->p_args);
            p_cmd->p_args = NULL;
            return PIXELKEY_ERROR_INVALID_ARGUMENT;
        }
    }
    else
    {
        // Try to parse as an int.
        p_args->value_type = VALUE_TYPE_INTEGER;
        char * end_ptr = NULL;
        p_args->value.i32 = (int32_t)strtol(next_arg, &end_ptr, 0);
        if (end_ptr == next_arg || *end_ptr != '\0')
        {
            free(p_cmd->p_args);
            p_cmd->p_args = NULL;
            return PIXELKEY_ERROR_INVALID_ARGUMENT;
        }
    }

    if (strtok_r(NULL, " ", &arg_ctx) != NULL)
    {
        // Extra args; bad command.
        free(p_cmd->p_args);
        p_cmd->p_args = NULL;
        return PIXELKEY_ERROR_TOO_MANY_ARGUMENTS;
    }
    else
    {
        return PIXELKEY_ERROR_NONE;
    }
}

/**
 * Parses time-set command arguments.
 * @param[in]     arg_ctx Argument tokenizer context.
 * @param[in,out] p_cmd   Pointer to the command structure to populate.
 * @retval PIXELKEY_ERROR_NOT_ENOUGH_ARGUMENTS Time string was not provided.
 * @retval PIXELKEY_ERROR_INVALID_ARGUMENT     Time string is invalid or too long.
 * @retval PIXELKEY_ERROR_TOO_MANY_ARGUMENTS   Additional, unexpected arguments were specified.
 * @retval PIXELKEY_ERROR_OUT_OF_MEMORY        Failed to malloc argument structure.
 * @retval PIXELKEY_ERROR_NONE                 Parsing was successful.
 */
static pixelkey_error_t parse_time_set(char * arg_ctx, cmd_t * p_cmd)
{
    // Make a local enum to track the state of the time string as the contents are verified.
    enum e_time_parse_state
    {
        YEAR,
        MONTH,
        DAY,
        HOUR,
        MINUTE,
        SECOND,
        TIMEZONE_HOUR,
        TIMEZONE_MIN
    } parse_state = YEAR;

    char * next_arg = strtok_r(NULL, " ", &arg_ctx);

    p_cmd->type =  CMD_TYPE_TIME_SET;
    if (next_arg == NULL)
    {
        return PIXELKEY_ERROR_NOT_ENOUGH_ARGUMENTS;
    }
    if (strlen(next_arg) > CMD_TIME_SET_MAX_LENGTH)
    {
        return PIXELKEY_ERROR_INVALID_ARGUMENT;
    }

    p_cmd->p_args = malloc(sizeof(cmd_args_time_set_t));
    if (p_cmd->p_args == NULL)
    {
        return PIXELKEY_ERROR_OUT_OF_MEMORY;
    }
    cmd_args_time_set_t * p_args = p_cmd->p_args;
    memset(p_args, 0, sizeof(*p_args));

    lower(next_arg);

    const size_t timestr_len = strlen(next_arg);
    int8_t bcd_shift = 4;
    bool bad_timestr = false;

    for (size_t i = 0; i < timestr_len && !bad_timestr; i++)
    {
        uint8_t char_val = char_to_u8(next_arg[i]);
        switch (parse_state)
        {
            case YEAR:
                if (char_val != UINT8_MAX && bcd_shift > 0)
                {
                    p_args->time_bcd.year |= (uint16_t)(char_val << (4 * (bcd_shift - 1)));
                    bcd_shift--;
                }
                else
                {
                    if (bcd_shift != 0 || next_arg[i] != '-')
                    {
                        bad_timestr = true;
                    }
                    else
                    {
                        bcd_shift = 2;
                        parse_state = MONTH;
                    }
                }
                break;
            case MONTH:
                if (char_val != UINT8_MAX && bcd_shift > 0)
                {
                    p_args->time_bcd.month |= (uint8_t)(char_val << (4 * (bcd_shift - 1)));
                    bcd_shift--;
                }
                else
                {
                    if (bcd_shift != 0 || next_arg[i] != '-')
                    {
                        bad_timestr = true;
                    }
                    else
                    {
                        bcd_shift = 2;
                        parse_state = DAY;
                    }
                }
                break;
            case DAY:
                if (char_val != UINT8_MAX && bcd_shift > 0)
                {
                    p_args->time_bcd.day |= (uint8_t)(char_val << (4 * (bcd_shift - 1)));
                    bcd_shift--;
                }
                else
                {
                    if (bcd_shift != 0 || (next_arg[i] != ' ' && next_arg[i] != 't' && next_arg[i] != '_'))
                    {
                        bad_timestr = true;
                    }
                    else
                    {
                        bcd_shift = 2;
                        parse_state = HOUR;
                    }
                }
                break;
            case HOUR:
                if (char_val != UINT8_MAX && bcd_shift > 0)
                {
                    p_args->time_bcd.hour |= (uint8_t)(char_val << (4 * (bcd_shift - 1)));
                    bcd_shift--;
                }
                else
                {
                    if (bcd_shift != 0 || (next_arg[i] != ':'))
                    {
                        bad_timestr = true;
                    }
                    else
                    {
                        bcd_shift = 2;
                        parse_state = MINUTE;
                    }
                }
                break;
            case MINUTE:
                if (char_val != UINT8_MAX && bcd_shift > 0)
                {
                    p_args->time_bcd.minute |= (uint8_t)(char_val << (4 * (bcd_shift - 1)));
                    bcd_shift--;
                }
                else
                {
                    if (bcd_shift != 0 || (next_arg[i] != ':'))
                    {
                        bad_timestr = true;
                    }
                    else
                    {
                        bcd_shift = 2;
                        parse_state = SECOND;
                    }
                }
                break;
            case SECOND:
                if (char_val != UINT8_MAX && bcd_shift > 0)
                {
                    p_args->time_bcd.second |= (uint8_t)(char_val << (4 * (bcd_shift - 1)));
                    bcd_shift--;

                    if (bcd_shift == 0)
                    {
                        bcd_shift = 2;
                        parse_state = TIMEZONE_HOUR;
                    }
                }
                else
                {
                    bad_timestr = true;
                }
                break;
            case TIMEZONE_HOUR:
                if (bcd_shift == 2 && char_val == UINT8_MAX)
                {
                    if (next_arg[i] == '-')
                    {
                        p_args->time_bcd.tz_hour |= 0x80;
                    }
                    else if (next_arg[i] != '+')
                    {
                        bad_timestr = true;
                    }
                }
                else if (char_val != UINT8_MAX && bcd_shift > 0)
                {
                    p_args->time_bcd.tz_hour |= (uint8_t)(char_val << (4 * (bcd_shift - 1)));
                    bcd_shift--;
                }
                else
                {
                    if (bcd_shift != 0 || (next_arg[i] != ':'))
                    {
                        bad_timestr = true;
                    }
                    else
                    {
                        bcd_shift = 2;
                        parse_state = TIMEZONE_MIN;
                    }
                }
                break;
            case TIMEZONE_MIN:
                if (char_val != UINT8_MAX && bcd_shift > 0)
                {
                    p_args->time_bcd.tz_minute |= (uint8_t)(char_val << (4 * (bcd_shift - 1)));
                    bcd_shift--;
                }
                else
                {
                    bad_timestr = true;
                }
                break;
        }
    }

    if (bad_timestr || parse_state < TIMEZONE_HOUR)
    {
        free(p_cmd->p_args);
        p_cmd->p_args = NULL;
        return PIXELKEY_ERROR_INVALID_ARGUMENT;
    }

    if (strtok_r(NULL, " ", &arg_ctx) != NULL)
    {
        // Extra args; bad command.
        free(p_cmd->p_args);
        p_cmd->p_args = NULL;
        return PIXELKEY_ERROR_TOO_MANY_ARGUMENTS;
    }
    else
    {
        return PIXELKEY_ERROR_NONE;
    }
}

/**
 * Parses a keyframe command.
 * @param[in]     cmd_tok Command token representing the keyframe.
 * @param[in,out] p_cmd   Pointer to the command structure to populate.
 * @retval PIXELKEY_ERROR_NOT_ENOUGH_ARGUMENTS Required keyframe arguments were not provided.
 * @retval PIXELKEY_ERROR_INVALID_ARGUMENT     A keyframe argument was invalid or too long.
 * @retval PIXELKEY_ERROR_TOO_MANY_ARGUMENTS   Additional, unexpected arguments were specified.
 * @retval PIXELKEY_ERROR_OUT_OF_MEMORY        Failed to malloc argument structure.
 * @retval PIXELKEY_ERROR_NONE                 Parsing was successful.
 */
static pixelkey_error_t parse_keyframe(char * cmd_tok, cmd_t * p_cmd)
{
    p_cmd->type =  CMD_TYPE_KEYFRAME_WRAPPER;

    // Lowercase the whole command token.
    lower(cmd_tok);
    
    // Start argument parsing
    char * arg_ctx = NULL;
    char * next_arg = strtok_r(cmd_tok, " ", &arg_ctx);

    if (next_arg == NULL)
    {
        return PIXELKEY_ERROR_NOT_ENOUGH_ARGUMENTS;
    }

    p_cmd->p_args = malloc(sizeof(cmd_args_keyframe_wrapper_t));
    if (p_cmd->p_args == NULL)
    {
        return PIXELKEY_ERROR_OUT_OF_MEMORY;
    }
    cmd_args_keyframe_wrapper_t * p_wrapper = p_cmd->p_args;
    memset(p_wrapper, 0, sizeof(*p_wrapper));

    // Check if any channel numbers are provided
    char x = *next_arg;
    if (isdigit(x))
    {
        size_t i = 0;
        char * ch_ctx = NULL;
        char * ch = strtok_r(cmd_tok, ",", &ch_ctx);
        while (ch != NULL && i < CMD_KEYFRAME_WRAPPER_CHANNELS_MAX_LENGTH)
        {
            // Check for a channel list
            char * dash = strchr(ch, '-');
            if (dash != NULL)
            {
                // Make sure the dash isn't first.
                if (dash == ch)
                {
                    return PIXELKEY_ERROR_INVALID_ARGUMENT;
                }

                *dash = '\0';
                dash++;
                int start = atoi(ch);
                int end = atoi(dash);
                if ((start >= end) 
                    || (start <= 0 || end <= 0)
                    || (start > CMD_KEYFRAME_MAX_CHANNEL_NUMBER || end > CMD_KEYFRAME_MAX_CHANNEL_NUMBER))
                {
                    // Start must be less than end, and both most be positive, non-zero integers.
                    return PIXELKEY_ERROR_INVALID_ARGUMENT;
                }

                if (i > CMD_KEYFRAME_WRAPPER_CHANNELS_MAX_LENGTH - 2)
                {
                    // There must be enough space to push the channel numbers.
                    return PIXELKEY_ERROR_INVALID_ARGUMENT;
                }

                // Mark the MSB so the handler knows this is a range.
                p_wrapper->channels[i++] = (uint16_t)(start | 0x8000);
                p_wrapper->channels[i++] = (uint16_t)(end);
            }
            else
            {
                int ch_num = atoi(ch);
                if ((ch_num <= 0) || (ch_num > CMD_KEYFRAME_MAX_CHANNEL_NUMBER))
                {
                    // Channel number must be a positive, non-zero integer less than the max.
                    return PIXELKEY_ERROR_INVALID_ARGUMENT;
                }
                p_wrapper->channels[i++] = (uint16_t)ch_num;
            }

            ch = strtok_r(NULL, ",", &ch_ctx);
        }

        if (ch != NULL)
        {
            // Too many channels were specified. Channels remain to be parsed.
            return PIXELKEY_ERROR_INVALID_ARGUMENT;
        }

        // Move the arg parser forward.
        next_arg = strtok_r(NULL, " ", &arg_ctx);
    }

    char * remaining_args = &next_arg[strlen(next_arg) + 1];

    if (!strcmp("set", next_arg))
    {
        p_wrapper->p_keyframe = keyframe_set_parse(remaining_args);
        if (p_wrapper->p_keyframe == NULL)
        {
            return PIXELKEY_ERROR_INVALID_ARGUMENT;
        }
    }
    else if (!strcmp("blink", next_arg))
    {
        p_wrapper->p_keyframe = keyframe_blink_parse(remaining_args);
        if (p_wrapper->p_keyframe == NULL)
        {
            return PIXELKEY_ERROR_INVALID_ARGUMENT;
        }
    }
    else if (!strcmp("fade", next_arg))
    {
        p_wrapper->p_keyframe = keyframe_fade_parse(remaining_args);
        if (p_wrapper->p_keyframe == NULL)
        {
            return PIXELKEY_ERROR_INVALID_ARGUMENT;
        }
    }
    else
    {
        // Unknown keyframe type.
        return PIXELKEY_ERROR_UNKNOWN_COMMAND;
    }

    return PIXELKEY_ERROR_NONE;
}

void pixelkey_cmd_free(cmd_t * p_cmd)
{
    if (p_cmd == NULL)
    {
        return;
    }

    if (p_cmd->p_args != NULL)
    {
        if (p_cmd->type == CMD_TYPE_KEYFRAME_WRAPPER)
        {
            cmd_args_keyframe_wrapper_t * p_wrapper = (cmd_args_keyframe_wrapper_t *)p_cmd->p_args;
            free(p_wrapper->p_keyframe);
            p_wrapper->p_keyframe = NULL;
        }
        free(p_cmd->p_args);
        p_cmd->p_args = NULL;
    }

    free(p_cmd);
}

void pixelkey_cmd_list_free(cmd_list_t * p_cmd_list)
{
    while (p_cmd_list != NULL)
    {
        // Free the command struct
        if (p_cmd_list->p_cmd != NULL)
        {
            pixelkey_cmd_free(p_cmd_list->p_cmd);
            p_cmd_list->p_cmd = NULL;
        }
        // Save p_cmd_list to a temp variable, move the list forward, then free
        cmd_list_t * x = p_cmd_list;
        p_cmd_list = p_cmd_list->p_next;
        x->p_next = NULL;
        free(x);
    }
}

/** @} */
