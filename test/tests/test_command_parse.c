#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "unity_fixture.h"

#include "pixelkey.h"
#include "pixelkey_commands.h"
#include "pixelkey_errors.h"

cmd_list_t * p_list = NULL;

TEST_GROUP(command_parse);

TEST_SETUP(command_parse)
{
    pixelkey_commandproc_init();
}

TEST_TEAR_DOWN(command_parse)
{
    pixelkey_cmd_list_free(p_list);
    p_list = NULL;
}

TEST(command_parse, invalid_inputs)
{
    char in[64] = {0};

    // Test zero length string
    TEST_ASSERT_EQUAL(PIXELKEY_ERROR_UNKNOWN_COMMAND, pixelkey_command_parse(in, &p_list));
    TEST_ASSERT_NULL(p_list);

    // Test pure white-space string
    strcpy(in, "  \t");
    TEST_ASSERT_EQUAL(PIXELKEY_ERROR_UNKNOWN_COMMAND, pixelkey_command_parse(in, &p_list));
    TEST_ASSERT_NULL(p_list);

    // Test blank command type string
    strcpy(in, "$");
    TEST_ASSERT_EQUAL(PIXELKEY_ERROR_UNKNOWN_COMMAND, pixelkey_command_parse(in, &p_list));
    TEST_ASSERT_NULL(p_list);
}

TEST(command_parse, config_get)
{
    char in[] = "$config-get somekey";
    TEST_ASSERT_EQUAL(PIXELKEY_ERROR_NONE, pixelkey_command_parse(in, &p_list));
    
    // Only one list element.
    TEST_ASSERT_NULL(p_list->p_next);

    // Proper command type and args
    TEST_ASSERT_EQUAL(CMD_TYPE_CONFIG_GET, p_list->p_cmd->type);
    cmd_args_config_get_t * p_args = (cmd_args_config_get_t *) p_list->p_cmd->p_args;
    TEST_ASSERT_EQUAL_STRING("somekey", p_args->key);

    pixelkey_cmd_list_free(p_list);
    p_list = NULL;
}

TEST(command_parse, config_get_extra_args)
{
    char in[] = "$config-get somekey another_arg";
    TEST_ASSERT_EQUAL(PIXELKEY_ERROR_TOO_MANY_ARGUMENTS, pixelkey_command_parse(in, &p_list));
    TEST_ASSERT_NULL(p_list);
}

TEST(command_parse, config_get_key_too_long)
{
    // $config-get supports a key length of up to 31 characters.
    char in[] = "$config-get 0123456789ABCDEF0123456789abcdef";
    TEST_ASSERT_EQUAL(PIXELKEY_ERROR_INVALID_ARGUMENT, pixelkey_command_parse(in, &p_list));
    TEST_ASSERT_NULL(p_list);
}

TEST(command_parse, config_set)
{
    char in[64] = {0};
    cmd_args_config_set_t * p_args = NULL;

    // Test integer config values
    strcpy(in, "$config-set somekey 1234");
    TEST_ASSERT_EQUAL(PIXELKEY_ERROR_NONE, pixelkey_command_parse(in, &p_list));
    
    // Only one list element.
    TEST_ASSERT_NULL(p_list->p_next);

    // Proper command type and args
    TEST_ASSERT_EQUAL(CMD_TYPE_CONFIG_SET, p_list->p_cmd->type);
    p_args = (cmd_args_config_set_t *) p_list->p_cmd->p_args;
    TEST_ASSERT_EQUAL_STRING("somekey", p_args->key);
    TEST_ASSERT_EQUAL(1234, p_args->value.i32);
    TEST_ASSERT_EQUAL(VALUE_TYPE_INTEGER, p_args->value_type);

    pixelkey_cmd_list_free(p_list);
    p_list = NULL;

    // Test float config values.
    strcpy(in, "$config-set somekey 1.234");
    TEST_ASSERT_EQUAL(PIXELKEY_ERROR_NONE, pixelkey_command_parse(in, &p_list));
    
    // Only one list element.
    TEST_ASSERT_NULL(p_list->p_next);

    // Proper command type and args
    TEST_ASSERT_EQUAL(CMD_TYPE_CONFIG_SET, p_list->p_cmd->type);
    p_args = (cmd_args_config_set_t *) p_list->p_cmd->p_args;
    TEST_ASSERT_EQUAL_STRING("somekey", p_args->key);
    TEST_ASSERT_EQUAL_FLOAT(1.234f, p_args->value.f32);
    TEST_ASSERT_EQUAL(VALUE_TYPE_FLOAT, p_args->value_type);

    pixelkey_cmd_list_free(p_list);
    p_list = NULL;

    // Test boolean config values.
    strcpy(in, "$config-set somekey true");
    TEST_ASSERT_EQUAL(PIXELKEY_ERROR_NONE, pixelkey_command_parse(in, &p_list));
    
    // Only one list element.
    TEST_ASSERT_NULL(p_list->p_next);

    // Proper command type and args
    TEST_ASSERT_EQUAL(CMD_TYPE_CONFIG_SET, p_list->p_cmd->type);
    p_args = (cmd_args_config_set_t *) p_list->p_cmd->p_args;
    TEST_ASSERT_EQUAL_STRING("somekey", p_args->key);
    TEST_ASSERT_EQUAL(true, p_args->value.b);
    TEST_ASSERT_EQUAL(VALUE_TYPE_BOOLEAN, p_args->value_type);

    pixelkey_cmd_list_free(p_list);
    p_list = NULL;
}

TEST(command_parse, config_set_extra_args)
{
    char in[] = "$config-set somekey 1234 another_arg";
    TEST_ASSERT_EQUAL(PIXELKEY_ERROR_TOO_MANY_ARGUMENTS, pixelkey_command_parse(in, &p_list));
    TEST_ASSERT_NULL(p_list);
}

TEST(command_parse, config_set_key_too_long)
{
    // $config-set supports a key length of up to 31 characters.
    char in[] = "$config-set 0123456789ABCDEF0123456789abcdef 1234";
    TEST_ASSERT_EQUAL(PIXELKEY_ERROR_INVALID_ARGUMENT, pixelkey_command_parse(in, &p_list));
    TEST_ASSERT_NULL(p_list);
}

TEST(command_parse, simple_cmds)
{
    char in[64] = {0};

    strcpy(in, "$resume");
    TEST_ASSERT_EQUAL(PIXELKEY_ERROR_NONE, pixelkey_command_parse(in, &p_list));
    TEST_ASSERT_EQUAL(CMD_TYPE_RESUME, p_list->p_cmd->type);
    TEST_ASSERT_NULL(p_list->p_cmd->p_args);
    pixelkey_cmd_list_free(p_list);
    p_list = NULL;

    strcpy(in, "$stop");
    TEST_ASSERT_EQUAL(PIXELKEY_ERROR_NONE, pixelkey_command_parse(in, &p_list));
    TEST_ASSERT_EQUAL(CMD_TYPE_STOP, p_list->p_cmd->type);
    TEST_ASSERT_NULL(p_list->p_cmd->p_args);
    pixelkey_cmd_list_free(p_list);
    p_list = NULL;

    strcpy(in, "$status");
    TEST_ASSERT_EQUAL(PIXELKEY_ERROR_NONE, pixelkey_command_parse(in, &p_list));
    TEST_ASSERT_EQUAL(CMD_TYPE_STATUS, p_list->p_cmd->type);
    TEST_ASSERT_NULL(p_list->p_cmd->p_args);
    pixelkey_cmd_list_free(p_list);
    p_list = NULL;

    strcpy(in, "$version");
    TEST_ASSERT_EQUAL(PIXELKEY_ERROR_NONE, pixelkey_command_parse(in, &p_list));
    TEST_ASSERT_EQUAL(CMD_TYPE_VERSION, p_list->p_cmd->type);
    TEST_ASSERT_NULL(p_list->p_cmd->p_args);
    pixelkey_cmd_list_free(p_list);
    p_list = NULL;

    strcpy(in, "$time-get");
    TEST_ASSERT_EQUAL(PIXELKEY_ERROR_NONE, pixelkey_command_parse(in, &p_list));
    TEST_ASSERT_EQUAL(CMD_TYPE_TIME_GET, p_list->p_cmd->type);
    TEST_ASSERT_NULL(p_list->p_cmd->p_args);
    pixelkey_cmd_list_free(p_list);
    p_list = NULL;
}

TEST(command_parse, simple_cmds_extra_args)
{
    char in[64] = {0};

    strcpy(in, "$resume extra");
    TEST_ASSERT_EQUAL(PIXELKEY_ERROR_TOO_MANY_ARGUMENTS, pixelkey_command_parse(in, &p_list));
    TEST_ASSERT_NULL(p_list);

    strcpy(in, "$stop extra");
    TEST_ASSERT_EQUAL(PIXELKEY_ERROR_TOO_MANY_ARGUMENTS, pixelkey_command_parse(in, &p_list));
    TEST_ASSERT_NULL(p_list);

    strcpy(in, "$status extra");
    TEST_ASSERT_EQUAL(PIXELKEY_ERROR_TOO_MANY_ARGUMENTS, pixelkey_command_parse(in, &p_list));
    TEST_ASSERT_NULL(p_list);

    strcpy(in, "$version extra");
    TEST_ASSERT_EQUAL(PIXELKEY_ERROR_TOO_MANY_ARGUMENTS, pixelkey_command_parse(in, &p_list));
    TEST_ASSERT_NULL(p_list);

    strcpy(in, "$time-get extra");
    TEST_ASSERT_EQUAL(PIXELKEY_ERROR_TOO_MANY_ARGUMENTS, pixelkey_command_parse(in, &p_list));
    TEST_ASSERT_NULL(p_list);
}

TEST(command_parse, cmds_need_args)
{
    char in[64] = {0};

    strcpy(in, "$config-get");
    TEST_ASSERT_EQUAL(PIXELKEY_ERROR_NOT_ENOUGH_ARGUMENTS, pixelkey_command_parse(in, &p_list));
    TEST_ASSERT_NULL(p_list);

    strcpy(in, "$config-set");
    TEST_ASSERT_EQUAL(PIXELKEY_ERROR_NOT_ENOUGH_ARGUMENTS, pixelkey_command_parse(in, &p_list));
    TEST_ASSERT_NULL(p_list);

    strcpy(in, "$time-set");
    TEST_ASSERT_EQUAL(PIXELKEY_ERROR_NOT_ENOUGH_ARGUMENTS, pixelkey_command_parse(in, &p_list));
    TEST_ASSERT_NULL(p_list);
}

TEST(command_parse, time_set)
{
    char in[64] = {0};

    strcpy(in, "$time-set 2023-05-09T20:09:33-04:00");
    TEST_ASSERT_EQUAL(PIXELKEY_ERROR_NONE, pixelkey_command_parse(in, &p_list));
    TEST_ASSERT_EQUAL(CMD_TYPE_TIME_SET, p_list->p_cmd->type);
    TEST_ASSERT_NOT_NULL(p_list->p_cmd->p_args);

    cmd_args_time_set_t * p_args = (cmd_args_time_set_t *)p_list->p_cmd->p_args;
    TEST_ASSERT_EQUAL(0x2023, p_args->time_bcd.year);
    TEST_ASSERT_EQUAL(  0x05, p_args->time_bcd.month);
    TEST_ASSERT_EQUAL(  0x09, p_args->time_bcd.day);
    TEST_ASSERT_EQUAL(  0x20, p_args->time_bcd.hour);
    TEST_ASSERT_EQUAL(  0x09, p_args->time_bcd.minute);
    TEST_ASSERT_EQUAL(  0x33, p_args->time_bcd.second);
    TEST_ASSERT_EQUAL(  0x84, p_args->time_bcd.tz_hour);
    TEST_ASSERT_EQUAL(  0x00, p_args->time_bcd.tz_minute);

    pixelkey_cmd_list_free(p_list);
    p_list = NULL;
}

TEST(command_parse, keyframe_set)
{
    char in[64] = {0};

    strcpy(in, "set blue");
    TEST_ASSERT_EQUAL(PIXELKEY_ERROR_NONE, pixelkey_command_parse(in, &p_list));
    TEST_ASSERT_EQUAL(CMD_TYPE_KEYFRAME_WRAPPER, p_list->p_cmd->type);
    TEST_ASSERT_NOT_NULL(p_list->p_cmd->p_args);

    cmd_args_keyframe_wrapper_t * p_wrapper = (cmd_args_keyframe_wrapper_t *)p_list->p_cmd->p_args;
    TEST_ASSERT_NOT_NULL(p_wrapper->p_keyframe);

    pixelkey_cmd_list_free(p_list);
    p_list = NULL;
}

TEST(command_parse, keyframe_set_invalid)
{
    char in[64] = {0};

    // no color
    strcpy(in, "set");
    TEST_ASSERT_EQUAL(PIXELKEY_ERROR_INVALID_ARGUMENT, pixelkey_command_parse(in, &p_list));
    TEST_ASSERT_NULL(p_list);

    // Invalid color
    strcpy(in, "set blurple");
    TEST_ASSERT_EQUAL(PIXELKEY_ERROR_INVALID_ARGUMENT, pixelkey_command_parse(in, &p_list));
    TEST_ASSERT_NULL(p_list);

    // Bad channels
    strcpy(in, "7,a set blurple");
    TEST_ASSERT_EQUAL(PIXELKEY_ERROR_INVALID_ARGUMENT, pixelkey_command_parse(in, &p_list));
    TEST_ASSERT_NULL(p_list);
}

TEST(command_parse, keyframe_blink)
{
    char in[64] = {0};
    cmd_args_keyframe_wrapper_t * p_wrapper = NULL;

    // Allow no colors
    strcpy(in, "blink 5");
    TEST_ASSERT_EQUAL(PIXELKEY_ERROR_NONE, pixelkey_command_parse(in, &p_list));
    TEST_ASSERT_EQUAL(CMD_TYPE_KEYFRAME_WRAPPER, p_list->p_cmd->type);
    TEST_ASSERT_NOT_NULL(p_list->p_cmd->p_args);

    p_wrapper = (cmd_args_keyframe_wrapper_t *)p_list->p_cmd->p_args;
    TEST_ASSERT_NOT_NULL(p_wrapper->p_keyframe);

    pixelkey_cmd_list_free(p_list);
    p_list = NULL;

    // Check one color
    strcpy(in, "blink 5 red");
    TEST_ASSERT_EQUAL(PIXELKEY_ERROR_NONE, pixelkey_command_parse(in, &p_list));
    TEST_ASSERT_EQUAL(CMD_TYPE_KEYFRAME_WRAPPER, p_list->p_cmd->type);
    TEST_ASSERT_NOT_NULL(p_list->p_cmd->p_args);

    p_wrapper = (cmd_args_keyframe_wrapper_t *)p_list->p_cmd->p_args;
    TEST_ASSERT_NOT_NULL(p_wrapper->p_keyframe);

    pixelkey_cmd_list_free(p_list);
    p_list = NULL;

    // Check two colors
    strcpy(in, "blink 5 red:green");
    TEST_ASSERT_EQUAL(PIXELKEY_ERROR_NONE, pixelkey_command_parse(in, &p_list));
    TEST_ASSERT_EQUAL(CMD_TYPE_KEYFRAME_WRAPPER, p_list->p_cmd->type);
    TEST_ASSERT_NOT_NULL(p_list->p_cmd->p_args);

    p_wrapper = (cmd_args_keyframe_wrapper_t *)p_list->p_cmd->p_args;
    TEST_ASSERT_NOT_NULL(p_wrapper->p_keyframe);

    pixelkey_cmd_list_free(p_list);
    p_list = NULL;

    // Allow duty cycle
    strcpy(in, "blink 5 red 25");
    TEST_ASSERT_EQUAL(PIXELKEY_ERROR_NONE, pixelkey_command_parse(in, &p_list));
    TEST_ASSERT_EQUAL(CMD_TYPE_KEYFRAME_WRAPPER, p_list->p_cmd->type);
    TEST_ASSERT_NOT_NULL(p_list->p_cmd->p_args);

    p_wrapper = (cmd_args_keyframe_wrapper_t *)p_list->p_cmd->p_args;
    TEST_ASSERT_NOT_NULL(p_wrapper->p_keyframe);

    pixelkey_cmd_list_free(p_list);
    p_list = NULL;
}

TEST(command_parse, keyframe_blink_invalid)
{
    char in[64] = {0};

    // no args
    strcpy(in, "blink");
    TEST_ASSERT_EQUAL(PIXELKEY_ERROR_INVALID_ARGUMENT, pixelkey_command_parse(in, &p_list));
    TEST_ASSERT_NULL(p_list);

    // Invalid color
    strcpy(in, "blink 10 blurple");
    TEST_ASSERT_EQUAL(PIXELKEY_ERROR_INVALID_ARGUMENT, pixelkey_command_parse(in, &p_list));
    TEST_ASSERT_NULL(p_list);

    // Too many colors
    strcpy(in, "blink 10 red:blue:green");
    TEST_ASSERT_EQUAL(PIXELKEY_ERROR_INVALID_ARGUMENT, pixelkey_command_parse(in, &p_list));
    TEST_ASSERT_NULL(p_list);

    // Bad channels
    strcpy(in, "7,a blink 10 blurple");
    TEST_ASSERT_EQUAL(PIXELKEY_ERROR_INVALID_ARGUMENT, pixelkey_command_parse(in, &p_list));
    TEST_ASSERT_NULL(p_list);

    // Bad duty cycle
    strcpy(in, "blink 10 red quarter");
    TEST_ASSERT_EQUAL(PIXELKEY_ERROR_INVALID_ARGUMENT, pixelkey_command_parse(in, &p_list));
    TEST_ASSERT_NULL(p_list);

    // Bad period
    strcpy(in, "blink 1a0");
    TEST_ASSERT_EQUAL(PIXELKEY_ERROR_INVALID_ARGUMENT, pixelkey_command_parse(in, &p_list));
    TEST_ASSERT_NULL(p_list);
}
TEST(command_parse, keyframe_fade)
{
    char in[64] = {0};

    strcpy(in, "fade 10 red:blue");
    TEST_ASSERT_EQUAL(PIXELKEY_ERROR_NONE, pixelkey_command_parse(in, &p_list));
    TEST_ASSERT_EQUAL(CMD_TYPE_KEYFRAME_WRAPPER, p_list->p_cmd->type);
    TEST_ASSERT_NOT_NULL(p_list->p_cmd->p_args);

    cmd_args_keyframe_wrapper_t * p_wrapper = (cmd_args_keyframe_wrapper_t *)p_list->p_cmd->p_args;
    TEST_ASSERT_NOT_NULL(p_wrapper->p_keyframe);

    pixelkey_cmd_list_free(p_list);
    p_list = NULL;
}

TEST(command_parse, keyframe_fade_invalid)
{
    char in[64] = {0};

    // no args
    strcpy(in, "fade");
    TEST_ASSERT_EQUAL(PIXELKEY_ERROR_INVALID_ARGUMENT, pixelkey_command_parse(in, &p_list));
    TEST_ASSERT_NULL(p_list);

    // Invalid color
    strcpy(in, "fade 10 blurple");
    TEST_ASSERT_EQUAL(PIXELKEY_ERROR_INVALID_ARGUMENT, pixelkey_command_parse(in, &p_list));
    TEST_ASSERT_NULL(p_list);

    // Bad channels
    strcpy(in, "7,a fade 10 blurple");
    TEST_ASSERT_EQUAL(PIXELKEY_ERROR_INVALID_ARGUMENT, pixelkey_command_parse(in, &p_list));
    TEST_ASSERT_NULL(p_list);
}

TEST(command_parse, keyframe_mod_repeat)
{
    char in[64] = {0};
    cmd_args_keyframe_mod_repeat_t * p_mod = NULL;

    // Test normal count
    strcpy(in, "^10");
    TEST_ASSERT_EQUAL(PIXELKEY_ERROR_NONE, pixelkey_command_parse(in, &p_list));
    TEST_ASSERT_EQUAL(CMD_TYPE_KEYFRAME_MOD_REPEAT, p_list->p_cmd->type);
    TEST_ASSERT_NOT_NULL(p_list->p_cmd->p_args);

    p_mod = (cmd_args_keyframe_mod_repeat_t *)p_list->p_cmd->p_args;
    TEST_ASSERT_EQUAL(10, p_mod->repeat_count);

    pixelkey_cmd_list_free(p_list);
    p_list = NULL;

    // Test negative count
    strcpy(in, "^-100");
    TEST_ASSERT_EQUAL(PIXELKEY_ERROR_NONE, pixelkey_command_parse(in, &p_list));
    TEST_ASSERT_EQUAL(CMD_TYPE_KEYFRAME_MOD_REPEAT, p_list->p_cmd->type);
    TEST_ASSERT_NOT_NULL(p_list->p_cmd->p_args);

    p_mod = (cmd_args_keyframe_mod_repeat_t *)p_list->p_cmd->p_args;
    TEST_ASSERT_EQUAL(-100, p_mod->repeat_count);

    pixelkey_cmd_list_free(p_list);
    p_list = NULL;
}

TEST(command_parse, keyframe_mod_repeat_invalid)
{
    char in[64] = {0};

    strcpy(in, "^");
    TEST_ASSERT_EQUAL(PIXELKEY_ERROR_INVALID_ARGUMENT, pixelkey_command_parse(in, &p_list));
    TEST_ASSERT_NULL(p_list);

    strcpy(in, "^abc");
    TEST_ASSERT_EQUAL(PIXELKEY_ERROR_INVALID_ARGUMENT, pixelkey_command_parse(in, &p_list));
    TEST_ASSERT_NULL(p_list);
}

TEST_GROUP_RUNNER(command_parse)
{
    RUN_TEST_CASE(command_parse, invalid_inputs);

    RUN_TEST_CASE(command_parse, simple_cmds);
    RUN_TEST_CASE(command_parse, simple_cmds_extra_args);

    RUN_TEST_CASE(command_parse, cmds_need_args);

    RUN_TEST_CASE(command_parse, config_get);
    RUN_TEST_CASE(command_parse, config_get_extra_args);
    RUN_TEST_CASE(command_parse, config_get_key_too_long);

    RUN_TEST_CASE(command_parse, config_set);
    RUN_TEST_CASE(command_parse, config_set_extra_args);
    RUN_TEST_CASE(command_parse, config_set_key_too_long);

    RUN_TEST_CASE(command_parse, time_set);

    RUN_TEST_CASE(command_parse, keyframe_set);
    RUN_TEST_CASE(command_parse, keyframe_set_invalid);
    RUN_TEST_CASE(command_parse, keyframe_blink);
    RUN_TEST_CASE(command_parse, keyframe_blink_invalid);
    RUN_TEST_CASE(command_parse, keyframe_fade);
    RUN_TEST_CASE(command_parse, keyframe_fade_invalid);

    RUN_TEST_CASE(command_parse, keyframe_mod_repeat);
    RUN_TEST_CASE(command_parse, keyframe_mod_repeat_invalid);
}
