/**
 * @file
 * @defgroup pixelkey__commandproc__internals Command Processor Internals
 * @ingroup pixelkey
 * @{
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "hal_device.h"
#include "ring_buffer.h"
#include "serial.h"
#include "config.h"
#include "version.h"

#include "pixelkey.h"
#include "pixelkey_errors.h"
#include "pixelkey_commands.h"

typedef void (*handler_fn_t)(void * p_cmd_args);

static void send_trailer(bool is_nak, pixelkey_error_t error);

static void handler_undefined(void * p_cmd_args);
static void handler_config_get(void * p_cmd_args);
static void handler_config_set(void * p_cmd_args);
static void handler_resume(void * p_cmd_args);
static void handler_stop(void * p_cmd_args);
static void handler_status(void * p_cmd_args);
static void handler_version(void * p_cmd_args);
static void handler_time_get(void * p_cmd_args);
static void handler_time_set(void * p_cmd_args);
static void handler_keyframe_wrapper(void * p_cmd_args);
static void handler_keyframe_mod_repeat(void * p_cmd_args);
static void handler_keyframe_mod_schedule(void * p_cmd_args);
static void handler_keyframe_mod_group(void * p_cmd_args);

static cmd_t * cmd_buffer_data[PIXELKEY_COMMAND_BUFFER_LENGTH] = {0};

static ring_buffer_t cmd_buffer = {0};

static handler_fn_t cmd_handlers[CMD_TYPE_COUNT] = 
{
    [CMD_TYPE_CONFIG_GET] = handler_config_get,
    [CMD_TYPE_CONFIG_SET] = handler_config_set,
    [CMD_TYPE_RESUME]     = handler_resume,
    [CMD_TYPE_STOP]       = handler_stop,
    [CMD_TYPE_STATUS]     = handler_status,
    [CMD_TYPE_VERSION]    = handler_version,
    [CMD_TYPE_TIME_GET]   = handler_time_get,
    [CMD_TYPE_TIME_SET]   = handler_time_set,
    [CMD_TYPE_KEYFRAME_WRAPPER] = handler_keyframe_wrapper,
    [CMD_TYPE_KEYFRAME_MOD_REPEAT] = handler_keyframe_mod_repeat,
    [CMD_TYPE_KEYFRAME_MOD_SCHEDULE] = handler_keyframe_mod_schedule,
    [CMD_TYPE_KEYFRAME_MOD_GROUP] = handler_keyframe_mod_group,
};

static bool has_repeat_modifier = false;
static int32_t repeat_modifier = 0;

/**
 * Initialize the command processor.
 */
void pixelkey_commandproc_init(void)
{
    ring_buffer_init(&cmd_buffer, &cmd_buffer_data, PIXELKEY_COMMAND_BUFFER_LENGTH);
}

/**
 * Pushes a command to the processor buffer.
 * @param[in] p_cmd Pointer to the command to queue.
 * @retval PIXELKEY_ERROR_BUFFER_FULL No more space in the command queue.
 * @retval PIXELKEY_ERROR_NONE        Command was pushed to the queue.
 */
pixelkey_error_t pixelkey_commandproc_push(cmd_t * p_cmd)
{
    if (!ring_buffer_push(&cmd_buffer, p_cmd))
    {
        return PIXELKEY_ERROR_BUFFER_FULL;
    }
    
    return PIXELKEY_ERROR_NONE;
}

/**
 * Executes queued commands.
 */
void pixelkey_commandproc_task(void)
{
    cmd_t * p_cmd = NULL;
    while (ring_buffer_pop(&cmd_buffer, (void **)&p_cmd))
    {
        if (p_cmd->type <= CMD_TYPE_UNDEFINED || p_cmd->type >= CMD_TYPE_COUNT)
        {
            handler_undefined(NULL);
        }
        else
        {
            /// @todo Remove this check once all the handlers are implemented.
            handler_fn_t handler = cmd_handlers[p_cmd->type];
            if (handler)
            {
                handler(p_cmd->p_args);
            }
            else
            {
                send_trailer(true, PIXELKEY_ERROR_NONE);
            }
        }
        
        pixelkey_cmd_free(p_cmd);
    }
}

static void send_trailer(bool is_nak, pixelkey_error_t error)
{
    char trailer[16] = "OK\n";
    int len = 3;
    
    // Make sure any pending writes are complete first.
    serial()->flush();

    // Send the trailer NAK/OK sequence.
    if (is_nak)
    {
        len = sprintf(trailer, "%d NAK\n", (int)error);
    }
    serial()->write((uint8_t *)trailer, (size_t)len);
    serial()->flush();

}

static void handler_undefined(void * p_cmd_args)
{
    ARG_NOT_USED(p_cmd_args);

    char msg[] = "Error: unknown command\n";
    serial()->write((uint8_t *)msg, (size_t)strlen(msg));
    serial()->flush();

    send_trailer(true, PIXELKEY_ERROR_UNKNOWN_COMMAND);
}

static void handler_config_get(void * p_cmd_args)
{
    cmd_args_config_get_t * p_args = (cmd_args_config_get_t *)p_cmd_args;
    char msg[64];
    int len = 0;

    config_data_t * p_config = NULL;
    pixelkey_error_t config_error = config()->read(&p_config);

    if (config_error != PIXELKEY_ERROR_NONE)
    {
        send_trailer(true, config_error);
        return;
    }

    if (!strcmp("crc", p_args->key))
    {
        len = sprintf(msg, "0x%04X\n", p_config->header.crc);
    }
    else if (!strcmp("echo_enabled", p_args->key))
    {
        len = sprintf(msg, "%s\n", (p_config->flags_b.echo_enabled ? "true" : "false"));
    }
    else if (!strcmp("gamma_enabled", p_args->key))
    {
        len = sprintf(msg, "%s\n", (p_config->flags_b.gamma_enabled ? "true" : "false"));
    }
    else if (!strcmp("gamma_factor", p_args->key))
    {
        len = sprintf(msg, "%.3g\n", p_config->gamma_factor);
    }
    else if (!strcmp("framerate", p_args->key))
    {
        len = sprintf(msg, "%"PRIu32"\n", p_config->framerate);
    }
    else if (!strcmp("num_neopixels", p_args->key))
    {
        len = sprintf(msg, "%"PRIu32"\n", p_config->num_neopixels);
    }
    else
    {
        send_trailer(true, PIXELKEY_ERROR_KEY_NOT_FOUND);
        return;
    }

    serial()->write((uint8_t *)msg, (size_t)len);
    send_trailer(false, PIXELKEY_ERROR_NONE);
}

static void handler_config_set(void * p_cmd_args)
{
    cmd_args_config_set_t * p_args = (cmd_args_config_set_t *)p_cmd_args;

    config_data_t * p_config = NULL;
    pixelkey_error_t config_error = config()->read(&p_config);

    if (config_error != PIXELKEY_ERROR_NONE)
    {
        send_trailer(true, config_error);
        return;
    }

    config_data_t new_config = *p_config;

    if (!strcmp("echo_enabled", p_args->key))
    {
        if (p_args->value_type != VALUE_TYPE_BOOLEAN)
        {
            send_trailer(true, PIXELKEY_ERROR_INVALID_ARGUMENT);
            return;
        }

        new_config.flags_b.echo_enabled = p_args->value.b;
        config_error = config()->write(&new_config);
    }
    else if (!strcmp("gamma_enabled", p_args->key))
    {
        if (p_args->value_type != VALUE_TYPE_BOOLEAN)
        {
            send_trailer(true, PIXELKEY_ERROR_INVALID_ARGUMENT);
            return;
        }

        new_config.flags_b.gamma_enabled = p_args->value.b;
        config_error = config()->write(&new_config);
    }
    else if (!strcmp("gamma_factor", p_args->key))
    {
        if (p_args->value_type != VALUE_TYPE_FLOAT)
        {
            send_trailer(true, PIXELKEY_ERROR_INVALID_ARGUMENT);
            return;
        }

        new_config.gamma_factor = p_args->value.f32;
        config_error = config()->write(&new_config);

        if (config_error == PIXELKEY_ERROR_NONE)
        {
            color_gamma_build(new_config.gamma_factor);
        }
    }
    else if (!strcmp("framerate", p_args->key))
    {
        if (p_args->value_type != VALUE_TYPE_INTEGER)
        {
            send_trailer(true, PIXELKEY_ERROR_INVALID_ARGUMENT);
            return;
        }

        new_config.framerate = (uint32_t) p_args->value.i32;
        config_error = config()->write(&new_config);

        if (config_error == PIXELKEY_ERROR_NONE)
        {
            pixelkey_keyframeproc_framerate_set((framerate_t)new_config.framerate);
        }
    }
    else if (!strcmp("num_neopixels", p_args->key))
    {
        // For the time being, don't allow this to be changed.
        /// @todo Add support to set number of neopixels.
        send_trailer(true, PIXELKEY_ERROR_KEY_NOT_FOUND);
        return;
    }
    else
    {
        send_trailer(true, PIXELKEY_ERROR_KEY_NOT_FOUND);
        return;
    }

    send_trailer((config_error != PIXELKEY_ERROR_NONE), config_error);
}

static void handler_resume(void * p_cmd_args)
{
    send_trailer(true, PIXELKEY_ERROR_NONE);
}

static void handler_stop(void * p_cmd_args)
{
    send_trailer(true, PIXELKEY_ERROR_NONE);
}

static void handler_status(void * p_cmd_args)
{
    char msg[64];
    int len = 0;

    len = snprintf(msg, sizeof(msg), "%s v%s\n", g_pixelkey_product_str, g_pixelkey_version_str);
    serial()->write((uint8_t *)msg, (size_t)len);
    serial()->flush();

    len = snprintf(msg, sizeof(msg), "Current state: %s\n", "active");
    serial()->write((uint8_t *)msg, (size_t)len);
    serial()->flush();

    send_trailer(false, PIXELKEY_ERROR_NONE);
}

static void handler_version(void * p_cmd_args)
{
    char msg[64];
    int len = 0;

    len = snprintf(msg, sizeof(msg), "%s\n", g_pixelkey_version_str);
    serial()->write((uint8_t *)msg, (size_t)len);
    serial()->flush();

    send_trailer(false, PIXELKEY_ERROR_NONE);
}

static void handler_time_get(void * p_cmd_args)
{
    send_trailer(true, PIXELKEY_ERROR_NONE);
}

static void handler_time_set(void * p_cmd_args)
{
    send_trailer(true, PIXELKEY_ERROR_NONE);
}

static void handler_keyframe_wrapper(void * p_cmd_args)
{
    cmd_args_keyframe_wrapper_t * p_args = (cmd_args_keyframe_wrapper_t *)p_cmd_args;
    if (p_args->channels[0] == 0)
    {
        // No channels specified.
        for (uint8_t i = 0; i < config_get_or_default()->num_neopixels; i++)
        {
            keyframe_base_t * p_keyframe = p_args->p_keyframe->p_api->clone(p_args->p_keyframe);
            if (p_keyframe == NULL)
            {
                send_trailer(true, PIXELKEY_ERROR_OUT_OF_MEMORY);
                return;
            }

            // Apply modifiers.
            if (has_repeat_modifier)
            {
                p_keyframe->modifiers.repeat_count = repeat_modifier;
            }

            pixelkey_keyframeproc_push(i, p_keyframe);
        }
    }
    else
    {
        // Loop through the channels.
        // Values in p_args->channels are 1-based instead of 0-based like the actual indexes.
        // Use 0 as a flag for the end of the channel list.
        for (size_t i = 0; p_args->channels[i] != 0 && i < CMD_KEYFRAME_WRAPPER_CHANNELS_MAX_LENGTH; i++)
        {
            keyframe_base_t * p_keyframe = p_args->p_keyframe->p_api->clone(p_args->p_keyframe);
            if (p_keyframe == NULL)
            {
                send_trailer(true, PIXELKEY_ERROR_OUT_OF_MEMORY);
                return;
            }

            // Apply modifiers.
            if (has_repeat_modifier)
            {
                p_keyframe->modifiers.repeat_count = repeat_modifier;
            }

            pixelkey_keyframeproc_push((uint8_t)(p_args->channels[i] - 1U), p_keyframe);
        }
    }

    // Clear the modifiers
    has_repeat_modifier = false;
    repeat_modifier = 0;

    send_trailer(false, PIXELKEY_ERROR_NONE);
}

static void handler_keyframe_mod_repeat(void * p_cmd_args)
{
    cmd_args_keyframe_mod_repeat_t * p_args = (cmd_args_keyframe_mod_repeat_t *)p_cmd_args;
    repeat_modifier = p_args->repeat_count;
    has_repeat_modifier = true;

    send_trailer(false, PIXELKEY_ERROR_NONE);
}

static void handler_keyframe_mod_schedule(void * p_cmd_args)
{
    send_trailer(true, PIXELKEY_ERROR_NONE);
}

static void handler_keyframe_mod_group(void * p_cmd_args)
{
    send_trailer(true, PIXELKEY_ERROR_NONE);
}

/** @} */
