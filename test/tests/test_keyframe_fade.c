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

#define FRAMERATE 60

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

static void test_curve(color_hsv_t color1, color_hsv_t color2, cubic_bezier_t const * const curve)
{
    fade.args.colors_len = 2;
    fade.args.colors[0] = color1;
    fade.args.colors[1] = color2;
    fade.args.curve = *curve;
    fade.args.fade_type = FADE_TYPE_CUBIC;
    fade.args.period = 1;
    fade.args.push_current = false;

    fade.base.p_api->render_init(p_keyframe, FRAMERATE, (color_rgb_t){ 0, 0, 0});

    color_rgb_t output[FRAMERATE] = {0};  // Do one whole frame
    for (int i = 0; i < FRAMERATE; i++)
    {
        fade.base.p_api->render_frame(p_keyframe, i, &output[i]);
    }

    printf("\n");
    for (int i = 0; i < FRAMERATE; i++)
    {
        color_rgb_t * rgb = &output[i];
        color_hsv_t hsv;
        color_convert2(COLOR_SPACE_RGB, COLOR_SPACE_HSV, (color_kind_t *)rgb, (color_kind_t *)&hsv);
        printf("%2d\t#%02X%02X%02X\t%6.3g\t%u\t%u\n", i, rgb->red, rgb->green, rgb->blue, HUE_FP_TO_F32(hsv.hue), hsv.saturation, hsv.value);
    }
}

TEST(keyframe_fade, linear)
{
    test_curve(color_red.hsv, color_blue.hsv, &cb_linear);
}

TEST(keyframe_fade, ease)
{
    test_curve(color_red.hsv, color_blue.hsv, &cb_ease);
}

TEST(keyframe_fade, ease_in)
{
    test_curve(color_red.hsv, color_blue.hsv, &cb_ease_in);
}

TEST(keyframe_fade, ease_out)
{
    test_curve(color_red.hsv, color_blue.hsv, &cb_ease_out);
}

TEST(keyframe_fade, ease_in_out)
{
    test_curve(color_red.hsv, color_blue.hsv, &cb_ease_in_out);
}

TEST_GROUP_RUNNER(keyframe_fade)
{
    RUN_TEST_CASE(keyframe_fade, linear);
    RUN_TEST_CASE(keyframe_fade, ease);
    RUN_TEST_CASE(keyframe_fade, ease_in);
    RUN_TEST_CASE(keyframe_fade, ease_out);
    RUN_TEST_CASE(keyframe_fade, ease_in_out);
}
