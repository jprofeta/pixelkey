#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "unity_fixture.h"

#include "pixelkey.h"
#include "pixelkey_commands.h"
#include "pixelkey_errors.h"

#include "keyframes.h"

#include "color.h"

keyframe_fade_t fade;
keyframe_base_t * p_keyframe = NULL;

TEST_GROUP(keyframe_fade);

TEST_SETUP(keyframe_fade)
{
    p_keyframe = keyframe_fade_ctor(&fade);
}

TEST_TEAR_DOWN(keyframe_fade)
{
    
}

TEST(keyframe_fade, linear)
{
    fade.args.colors_len = 2;
    fade.args.colors[0] = color_red.hsv;
    fade.args.colors[1] = color_blue.hsv;
    fade.args.curve = cb_linear;
    fade.args.fade_type = FADE_TYPE_CUBIC;
    fade.args.period = 1;
    fade.args.push_current = false;

    fade.base.p_api->render_init(p_keyframe, 30, (color_rgb_t){ 0, 0, 0});

    color_rgb_t output[30] = {0};  // Do one whole frame
    for (int i = 0; i < 30; i++)
    {
        fade.base.p_api->render_frame(p_keyframe, i, &output[i]);
    }

    printf("\n");
    for (int i = 0; i < 30; i++)
    {
        color_rgb_t * rgb = &output[i];
        color_hsv_t hsv;
        color_convert2(COLOR_SPACE_RGB, COLOR_SPACE_HSV, (color_kind_t *)rgb, (color_kind_t *)&hsv);
        printf("%2d: #%02X%02X%02X (%u, %u, %u)\n", i, rgb->red, rgb->green, rgb->blue, hsv.hue, hsv.saturation, hsv.value);
    }
}

TEST(keyframe_fade, ease_in_out)
{
    fade.args.colors_len = 2;
    fade.args.colors[0] = color_red.hsv;
    fade.args.colors[1] = color_blue.hsv;
    fade.args.curve = cb_ease_in_out;
    fade.args.fade_type = FADE_TYPE_CUBIC;
    fade.args.period = 1;
    fade.args.push_current = false;

    fade.base.p_api->render_init(p_keyframe, 30, (color_rgb_t){ 0, 0, 0});

    color_rgb_t output[30] = {0};  // Do one whole frame
    for (int i = 0; i < 30; i++)
    {
        fade.base.p_api->render_frame(p_keyframe, i, &output[i]);
    }

    printf("\n");
    for (int i = 0; i < 30; i++)
    {
        color_rgb_t * rgb = &output[i];
        color_hsv_t hsv;
        color_convert2(COLOR_SPACE_RGB, COLOR_SPACE_HSV, (color_kind_t *)rgb, (color_kind_t *)&hsv);
        printf("%2d: #%02X%02X%02X (%3u, %u, %u)\n", i, rgb->red, rgb->green, rgb->blue, hsv.hue, hsv.saturation, hsv.value);
    }
}

TEST_GROUP_RUNNER(keyframe_fade)
{
    RUN_TEST_CASE(keyframe_fade, linear);
    RUN_TEST_CASE(keyframe_fade, ease_in_out);
}
