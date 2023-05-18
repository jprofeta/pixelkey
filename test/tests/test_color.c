#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "unity_fixture.h"

#include "color.h"

TEST_GROUP(color);

TEST_SETUP(color)
{
}

TEST_TEAR_DOWN(color)
{
}

#define TEST_ASSERT_COLOR_COMPARE_HSV_RGB(hsv,expected) test_compare_hsv_rgb((hsv), (expected), __LINE__)
static void test_compare_hsv_rgb(color_hsv_t hsv, color_rgb_t expected, int line)
{
    color_rgb_t out;
    color_convert2(COLOR_SPACE_HSV, COLOR_SPACE_RGB, (color_kind_t *)&hsv, (color_kind_t *)&out);

    // Allow the colors to be within 1 value due to rounding and precision.
    UNITY_TEST_ASSERT_UINT8_WITHIN(1, expected.red, out.red, line, "Red channel does not match");
    UNITY_TEST_ASSERT_UINT8_WITHIN(1, expected.green, out.green, line, "Green channel does not match");
    UNITY_TEST_ASSERT_UINT8_WITHIN(1, expected.blue, out.blue, line, "Blue channel does not match");
}

#define TEST_ASSERT_COLOR_COMPARE_HSL_RGB(hsl,expected) test_compare_hsl_rgb((hsl), (expected), __LINE__)
static void test_compare_hsl_rgb(color_hsl_t hsl, color_rgb_t expected, int line)
{
    color_rgb_t out;
    color_convert2(COLOR_SPACE_HSL, COLOR_SPACE_RGB, (color_kind_t *)&hsl, (color_kind_t *)&out);

    // Allow the colors to be within 1 value due to rounding and precision.
    UNITY_TEST_ASSERT_UINT8_WITHIN(1, expected.red, out.red, line, "Red channel does not match");
    UNITY_TEST_ASSERT_UINT8_WITHIN(1, expected.green, out.green, line, "Green channel does not match");
    UNITY_TEST_ASSERT_UINT8_WITHIN(1, expected.blue, out.blue, line, "Blue channel does not match");
}

#define TEST_ASSERT_COLOR_COMPARE_HSV_HSL(hsv,expected) test_compare_hsv_hsl((hsv), (expected), __LINE__)
static void test_compare_hsv_hsl(color_hsv_t hsv, color_hsl_t expected, int line)
{
    color_hsl_t out;
    color_convert2(COLOR_SPACE_HSV, COLOR_SPACE_HSL, (color_kind_t *)&hsv, (color_kind_t *)&out);

    // Allow the colors to be within 1 value due to rounding and precision.
    UNITY_TEST_ASSERT_UINT16_WITHIN(1, expected.hue, out.hue, line, "Hue channel does not match");
    UNITY_TEST_ASSERT_UINT8_WITHIN(1, expected.saturation, out.saturation, line, "Saturation channel does not match");
    UNITY_TEST_ASSERT_UINT8_WITHIN(1, expected.lightness, out.lightness, line, "Lightness channel does not match");
}


#define TEST_ASSERT_COLOR_COMPARE_RGB_HSV(rgb,expected) test_compare_rgb_hsv((rgb), (expected), __LINE__)
static void test_compare_rgb_hsv(color_rgb_t rgb, color_hsv_t expected, int line)
{
    color_hsv_t out;
    color_convert2(COLOR_SPACE_RGB, COLOR_SPACE_HSV, (color_kind_t *)&rgb, (color_kind_t *)&out);

    // Allow the colors to be within 1 value due to rounding and precision.
    UNITY_TEST_ASSERT_UINT16_WITHIN(1, expected.hue, out.hue, line, "Hue channel does not match");
    UNITY_TEST_ASSERT_UINT8_WITHIN(1, expected.saturation, out.saturation, line, "Saturation channel does not match");
    UNITY_TEST_ASSERT_UINT8_WITHIN(1, expected.value, out.value, line, "Value channel does not match");
}

#define TEST_ASSERT_COLOR_COMPARE_RGB_HSL(rgb,expected) test_compare_rgb_hsl((rgb), (expected), __LINE__)
static void test_compare_rgb_hsl(color_rgb_t rgb, color_hsl_t expected, int line)
{
    color_hsl_t out;
    color_convert2(COLOR_SPACE_RGB, COLOR_SPACE_HSL, (color_kind_t *)&rgb, (color_kind_t *)&out);

    // Allow the colors to be within 1 value due to rounding and precision.
    UNITY_TEST_ASSERT_UINT16_WITHIN(1, expected.hue, out.hue, line, "Hue channel does not match");
    UNITY_TEST_ASSERT_UINT8_WITHIN(1, expected.saturation, out.saturation, line, "Saturation channel does not match");
    UNITY_TEST_ASSERT_UINT8_WITHIN(1, expected.lightness, out.lightness, line, "Lightness channel does not match");
}

#define TEST_ASSERT_COLOR_COMPARE_HSL_HSV(hsl,expected) test_compare_hsl_hsv((hsl), (expected), __LINE__)
static void test_compare_hsl_hsv(color_hsl_t hsl, color_hsv_t expected, int line)
{
    color_hsv_t out;
    color_convert2(COLOR_SPACE_HSL, COLOR_SPACE_HSV, (color_kind_t *)&hsl, (color_kind_t *)&out);

    // Allow the colors to be within 1 value due to rounding and precision.
    UNITY_TEST_ASSERT_UINT16_WITHIN(1, expected.hue, out.hue, line, "Hue channel does not match");
    UNITY_TEST_ASSERT_UINT8_WITHIN(1, expected.saturation, out.saturation, line, "Saturation channel does not match");
    UNITY_TEST_ASSERT_UINT8_WITHIN(1, expected.value, out.value, line, "Value channel does not match");
}

TEST(color, hsv_to_rgb)
{
    /* Red: #FF0000. */
    TEST_ASSERT_COLOR_COMPARE_HSV_RGB(((color_hsv_t) {   0, 100, 100 }), ((color_rgb_t) { .red = 0xFF, .green = 0x00, .blue = 0x00 }));

    /* Orange: #FF8000. */
    TEST_ASSERT_COLOR_COMPARE_HSV_RGB(((color_hsv_t) {  30, 100, 100 }), ((color_rgb_t) { .red = 0xFF, .green = 0x80, .blue = 0x00 }));

    /* Yellow: #FFFF00. */
    TEST_ASSERT_COLOR_COMPARE_HSV_RGB(((color_hsv_t) {  60, 100, 100 }), ((color_rgb_t) { .red = 0xFF, .green = 0xFF, .blue = 0x00 }));

    /* Neon: #80FF00. */
    TEST_ASSERT_COLOR_COMPARE_HSV_RGB(((color_hsv_t) {  90, 100, 100 }), ((color_rgb_t) { .red = 0x80, .green = 0xFF, .blue = 0x00 }));

    /* Green: #00FF00. */
    TEST_ASSERT_COLOR_COMPARE_HSV_RGB(((color_hsv_t) { 120, 100, 100 }), ((color_rgb_t) { .red = 0x00, .green = 0xFF, .blue = 0x00 }));

    /* Seafoam: #00FF80. */
    TEST_ASSERT_COLOR_COMPARE_HSV_RGB(((color_hsv_t) { 150, 100, 100 }), ((color_rgb_t) { .red = 0x00, .green = 0xFF, .blue = 0x80 }));

    /* Cyan: #00FFFF. */
    TEST_ASSERT_COLOR_COMPARE_HSV_RGB(((color_hsv_t) { 180, 100, 100 }), ((color_rgb_t) { .red = 0x00, .green = 0xFF, .blue = 0xFF }));

    /* Light Blue: #0080FF. */
    TEST_ASSERT_COLOR_COMPARE_HSV_RGB(((color_hsv_t) { 210, 100, 100 }), ((color_rgb_t) { .red = 0x00, .green = 0x80, .blue = 0xFF }));

    /* Blue: #0000FF. */
    TEST_ASSERT_COLOR_COMPARE_HSV_RGB(((color_hsv_t) { 240, 100, 100 }), ((color_rgb_t) { .red = 0x00, .green = 0x00, .blue = 0xFF }));

    /* Purple: #8000FF. */
    TEST_ASSERT_COLOR_COMPARE_HSV_RGB(((color_hsv_t) { 270, 100, 100 }), ((color_rgb_t) { .red = 0x80, .green = 0x00, .blue = 0xFF }));

    /* Magenta: #FF00FF. */
    TEST_ASSERT_COLOR_COMPARE_HSV_RGB(((color_hsv_t) { 300, 100, 100 }), ((color_rgb_t) { .red = 0xFF, .green = 0x00, .blue = 0xFF }));

    /* Pink: #FF0080. */
    TEST_ASSERT_COLOR_COMPARE_HSV_RGB(((color_hsv_t) { 330, 100, 100 }), ((color_rgb_t) { .red = 0xFF, .green = 0x00, .blue = 0x80 }));

    /* White: #FFFFFF. */
    TEST_ASSERT_COLOR_COMPARE_HSV_RGB(((color_hsv_t) {   0,   0, 100 }), ((color_rgb_t) { .red = 0xFF, .green = 0xFF, .blue = 0xFF }));

    /* Black: #000000. */
    TEST_ASSERT_COLOR_COMPARE_HSV_RGB(((color_hsv_t) {   0,   0,   0 }), ((color_rgb_t) { .red = 0x00, .green = 0x00, .blue = 0x00 }));
}

TEST(color, hsl_to_rgb)
{
    /* Red: #FF0000. */
    TEST_ASSERT_COLOR_COMPARE_HSL_RGB(((color_hsl_t) {   0, 100,  50 }), ((color_rgb_t) { .red = 0xFF, .green = 0x00, .blue = 0x00 }));

    /* Orange: #FF8000. */
    TEST_ASSERT_COLOR_COMPARE_HSL_RGB(((color_hsl_t) {  30, 100,  50 }), ((color_rgb_t) { .red = 0xFF, .green = 0x80, .blue = 0x00 }));

    /* Yellow: #FFFF00. */
    TEST_ASSERT_COLOR_COMPARE_HSL_RGB(((color_hsl_t) {  60, 100,  50 }), ((color_rgb_t) { .red = 0xFF, .green = 0xFF, .blue = 0x00 }));

    /* Neon: #80FF00. */
    TEST_ASSERT_COLOR_COMPARE_HSL_RGB(((color_hsl_t) {  90, 100,  50 }), ((color_rgb_t) { .red = 0x80, .green = 0xFF, .blue = 0x00 }));

    /* Green: #00FF00. */
    TEST_ASSERT_COLOR_COMPARE_HSL_RGB(((color_hsl_t) { 120, 100,  50 }), ((color_rgb_t) { .red = 0x00, .green = 0xFF, .blue = 0x00 }));

    /* Seafoam: #00FF80. */
    TEST_ASSERT_COLOR_COMPARE_HSL_RGB(((color_hsl_t) { 150, 100,  50 }), ((color_rgb_t) { .red = 0x00, .green = 0xFF, .blue = 0x80 }));

    /* Cyan: #00FFFF. */
    TEST_ASSERT_COLOR_COMPARE_HSL_RGB(((color_hsl_t) { 180, 100,  50 }), ((color_rgb_t) { .red = 0x00, .green = 0xFF, .blue = 0xFF }));

    /* Light Blue: #0080FF. */
    TEST_ASSERT_COLOR_COMPARE_HSL_RGB(((color_hsl_t) { 210, 100,  50 }), ((color_rgb_t) { .red = 0x00, .green = 0x80, .blue = 0xFF }));

    /* Blue: #0000FF. */
    TEST_ASSERT_COLOR_COMPARE_HSL_RGB(((color_hsl_t) { 240, 100,  50 }), ((color_rgb_t) { .red = 0x00, .green = 0x00, .blue = 0xFF }));

    /* Purple: #8000FF. */
    TEST_ASSERT_COLOR_COMPARE_HSL_RGB(((color_hsl_t) { 270, 100,  50 }), ((color_rgb_t) { .red = 0x80, .green = 0x00, .blue = 0xFF }));

    /* Magenta: #FF00FF. */
    TEST_ASSERT_COLOR_COMPARE_HSL_RGB(((color_hsl_t) { 300, 100,  50 }), ((color_rgb_t) { .red = 0xFF, .green = 0x00, .blue = 0xFF }));

    /* Pink: #FF0080. */
    TEST_ASSERT_COLOR_COMPARE_HSL_RGB(((color_hsl_t) { 330, 100,  50 }), ((color_rgb_t) { .red = 0xFF, .green = 0x00, .blue = 0x80 }));

    /* White: #FFFFFF. */
    TEST_ASSERT_COLOR_COMPARE_HSL_RGB(((color_hsl_t) {   0,   0, 100 }), ((color_rgb_t) { .red = 0xFF, .green = 0xFF, .blue = 0xFF }));

    /* Black: #000000. */
    TEST_ASSERT_COLOR_COMPARE_HSL_RGB(((color_hsl_t) {   0,   0,   0 }), ((color_rgb_t) { .red = 0x00, .green = 0x00, .blue = 0x00 }));
}

TEST(color, hsv_to_hsl)
{
    /* Red: hsl(0, 100.00%, 50.00%). */
    TEST_ASSERT_COLOR_COMPARE_HSV_HSL(((color_hsv_t) {   0, 100, 100 }), ((color_hsl_t) {   0, 100,  50 }));

    /* Orange: hsl(30, 100.00%, 50.00%). */
    TEST_ASSERT_COLOR_COMPARE_HSV_HSL(((color_hsv_t) {  30, 100, 100 }), ((color_hsl_t) {  30, 100,  50 }));

    /* Yellow: hsl(60, 100.00%, 50.00%). */
    TEST_ASSERT_COLOR_COMPARE_HSV_HSL(((color_hsv_t) {  60, 100, 100 }), ((color_hsl_t) {  60, 100,  50 }));

    /* Neon: hsl(90, 100.00%, 50.00%). */
    TEST_ASSERT_COLOR_COMPARE_HSV_HSL(((color_hsv_t) {  90, 100, 100 }), ((color_hsl_t) {  90, 100,  50 }));

    /* Green: hsl(120, 100.00%, 50.00%). */
    TEST_ASSERT_COLOR_COMPARE_HSV_HSL(((color_hsv_t) { 120, 100, 100 }), ((color_hsl_t) { 120, 100,  50 }));

    /* Seafoam: hsl(150, 100.00%, 50.00%). */
    TEST_ASSERT_COLOR_COMPARE_HSV_HSL(((color_hsv_t) { 150, 100, 100 }), ((color_hsl_t) { 150, 100,  50 }));

    /* Cyan: hsl(180, 100.00%, 50.00%). */
    TEST_ASSERT_COLOR_COMPARE_HSV_HSL(((color_hsv_t) { 180, 100, 100 }), ((color_hsl_t) { 180, 100,  50 }));

    /* Light Blue: hsl(210, 100.00%, 50.00%). */
    TEST_ASSERT_COLOR_COMPARE_HSV_HSL(((color_hsv_t) { 210, 100, 100 }), ((color_hsl_t) { 210, 100,  50 }));

    /* Blue: hsl(240, 100.00%, 50.00%). */
    TEST_ASSERT_COLOR_COMPARE_HSV_HSL(((color_hsv_t) { 240, 100, 100 }), ((color_hsl_t) { 240, 100,  50 }));

    /* Purple: hsl(270, 100.00%, 50.00%). */
    TEST_ASSERT_COLOR_COMPARE_HSV_HSL(((color_hsv_t) { 270, 100, 100 }), ((color_hsl_t) { 270, 100,  50 }));

    /* Magenta: hsl(300, 100.00%, 50.00%). */
    TEST_ASSERT_COLOR_COMPARE_HSV_HSL(((color_hsv_t) { 300, 100, 100 }), ((color_hsl_t) { 300, 100,  50 }));

    /* Pink: hsl(330, 100.00%, 50.00%). */
    TEST_ASSERT_COLOR_COMPARE_HSV_HSL(((color_hsv_t) { 330, 100, 100 }), ((color_hsl_t) { 330, 100,  50 }));

    /* White: hsl(0, 0.00%, 100.00%). */
    TEST_ASSERT_COLOR_COMPARE_HSV_HSL(((color_hsv_t) {   0,   0, 100 }), ((color_hsl_t) {   0,   0, 100 }));

    /* Black: hsl(0, 0.00%, 0.00%). */
    TEST_ASSERT_COLOR_COMPARE_HSV_HSL(((color_hsv_t) {   0,   0,   0 }), ((color_hsl_t) {   0,   0,   0 }));
}

TEST(color, rgb_to_hsv)
{
    /* Red: #FF0000. */
    TEST_ASSERT_COLOR_COMPARE_RGB_HSV(((color_rgb_t) { .red = 0xFF, .green = 0x00, .blue = 0x00 }), ((color_hsv_t) {   0, 100, 100 }));

    /* Orange: #FF8000. */
    TEST_ASSERT_COLOR_COMPARE_RGB_HSV(((color_rgb_t) { .red = 0xFF, .green = 0x80, .blue = 0x00 }), ((color_hsv_t) {  30, 100, 100 }));

    /* Yellow: #FFFF00. */
    TEST_ASSERT_COLOR_COMPARE_RGB_HSV(((color_rgb_t) { .red = 0xFF, .green = 0xFF, .blue = 0x00 }), ((color_hsv_t) {  60, 100, 100 }));

    /* Neon: #80FF00. */
    TEST_ASSERT_COLOR_COMPARE_RGB_HSV(((color_rgb_t) { .red = 0x80, .green = 0xFF, .blue = 0x00 }), ((color_hsv_t) {  90, 100, 100 }));

    /* Green: #00FF00. */
    TEST_ASSERT_COLOR_COMPARE_RGB_HSV(((color_rgb_t) { .red = 0x00, .green = 0xFF, .blue = 0x00 }), ((color_hsv_t) { 120, 100, 100 }));

    /* Seafoam: #00FF80. */
    TEST_ASSERT_COLOR_COMPARE_RGB_HSV(((color_rgb_t) { .red = 0x00, .green = 0xFF, .blue = 0x80 }), ((color_hsv_t) { 150, 100, 100 }));

    /* Cyan: #00FFFF. */
    TEST_ASSERT_COLOR_COMPARE_RGB_HSV(((color_rgb_t) { .red = 0x00, .green = 0xFF, .blue = 0xFF }), ((color_hsv_t) { 180, 100, 100 }));

    /* Light Blue: #0080FF. */
    TEST_ASSERT_COLOR_COMPARE_RGB_HSV(((color_rgb_t) { .red = 0x00, .green = 0x80, .blue = 0xFF }), ((color_hsv_t) { 210, 100, 100 }));

    /* Blue: #0000FF. */
    TEST_ASSERT_COLOR_COMPARE_RGB_HSV(((color_rgb_t) { .red = 0x00, .green = 0x00, .blue = 0xFF }), ((color_hsv_t) { 240, 100, 100 }));

    /* Purple: #8000FF. */
    TEST_ASSERT_COLOR_COMPARE_RGB_HSV(((color_rgb_t) { .red = 0x80, .green = 0x00, .blue = 0xFF }), ((color_hsv_t) { 270, 100, 100 }));

    /* Magenta: #FF00FF. */
    TEST_ASSERT_COLOR_COMPARE_RGB_HSV(((color_rgb_t) { .red = 0xFF, .green = 0x00, .blue = 0xFF }), ((color_hsv_t) { 300, 100, 100 }));

    /* Pink: #FF0080. */
    TEST_ASSERT_COLOR_COMPARE_RGB_HSV(((color_rgb_t) { .red = 0xFF, .green = 0x00, .blue = 0x80 }), ((color_hsv_t) { 330, 100, 100 }));

    /* White: #FFFFFF. */
    TEST_ASSERT_COLOR_COMPARE_RGB_HSV(((color_rgb_t) { .red = 0xFF, .green = 0xFF, .blue = 0xFF }), ((color_hsv_t) {   0,   0, 100 }));

    /* Black: #000000. */
    TEST_ASSERT_COLOR_COMPARE_RGB_HSV(((color_rgb_t) { .red = 0x00, .green = 0x00, .blue = 0x00 }), ((color_hsv_t) {   0,   0,   0 }));
}

TEST(color, rgb_to_hsl)
{
    /* Red: #FF0000. */
    TEST_ASSERT_COLOR_COMPARE_RGB_HSL(((color_rgb_t) { .red = 0xFF, .green = 0x00, .blue = 0x00 }), ((color_hsl_t) {   0, 100,  50 }));

    /* Orange: #FF8000. */
    TEST_ASSERT_COLOR_COMPARE_RGB_HSL(((color_rgb_t) { .red = 0xFF, .green = 0x80, .blue = 0x00 }), ((color_hsl_t) {  30, 100,  50 }));

    /* Yellow: #FFFF00. */
    TEST_ASSERT_COLOR_COMPARE_RGB_HSL(((color_rgb_t) { .red = 0xFF, .green = 0xFF, .blue = 0x00 }), ((color_hsl_t) {  60, 100,  50 }));

    /* Neon: #80FF00. */
    TEST_ASSERT_COLOR_COMPARE_RGB_HSL(((color_rgb_t) { .red = 0x80, .green = 0xFF, .blue = 0x00 }), ((color_hsl_t) {  90, 100,  50 }));

    /* Green: #00FF00. */
    TEST_ASSERT_COLOR_COMPARE_RGB_HSL(((color_rgb_t) { .red = 0x00, .green = 0xFF, .blue = 0x00 }), ((color_hsl_t) { 120, 100,  50 }));

    /* Seafoam: #00FF80. */
    TEST_ASSERT_COLOR_COMPARE_RGB_HSL(((color_rgb_t) { .red = 0x00, .green = 0xFF, .blue = 0x80 }), ((color_hsl_t) { 150, 100,  50 }));

    /* Cyan: #00FFFF. */
    TEST_ASSERT_COLOR_COMPARE_RGB_HSL(((color_rgb_t) { .red = 0x00, .green = 0xFF, .blue = 0xFF }), ((color_hsl_t) { 180, 100,  50 }));

    /* Light Blue: #0080FF. */
    TEST_ASSERT_COLOR_COMPARE_RGB_HSL(((color_rgb_t) { .red = 0x00, .green = 0x80, .blue = 0xFF }), ((color_hsl_t) { 210, 100,  50 }));

    /* Blue: #0000FF. */
    TEST_ASSERT_COLOR_COMPARE_RGB_HSL(((color_rgb_t) { .red = 0x00, .green = 0x00, .blue = 0xFF }), ((color_hsl_t) { 240, 100,  50 }));

    /* Purple: #8000FF. */
    TEST_ASSERT_COLOR_COMPARE_RGB_HSL(((color_rgb_t) { .red = 0x80, .green = 0x00, .blue = 0xFF }), ((color_hsl_t) { 270, 100,  50 }));

    /* Magenta: #FF00FF. */
    TEST_ASSERT_COLOR_COMPARE_RGB_HSL(((color_rgb_t) { .red = 0xFF, .green = 0x00, .blue = 0xFF }), ((color_hsl_t) { 300, 100,  50 }));

    /* Pink: #FF0080. */
    TEST_ASSERT_COLOR_COMPARE_RGB_HSL(((color_rgb_t) { .red = 0xFF, .green = 0x00, .blue = 0x80 }), ((color_hsl_t) { 330, 100,  50 }));

    /* White: #FFFFFF. */
    TEST_ASSERT_COLOR_COMPARE_RGB_HSL(((color_rgb_t) { .red = 0xFF, .green = 0xFF, .blue = 0xFF }), ((color_hsl_t) {   0,   0, 100 }));

    /* Black: #000000. */
    TEST_ASSERT_COLOR_COMPARE_RGB_HSL(((color_rgb_t) { .red = 0x00, .green = 0x00, .blue = 0x00 }), ((color_hsl_t) {   0,   0,   0 }));
}

TEST(color, hsl_to_hsv)
{
    /* Red: hsl(0, 100.00%, 50.00%). */
    TEST_ASSERT_COLOR_COMPARE_HSL_HSV(((color_hsl_t) {   0, 100,  50 }), ((color_hsv_t) {   0, 100, 100 }));

    /* Orange: hsl(30, 100.00%, 50.00%). */
    TEST_ASSERT_COLOR_COMPARE_HSL_HSV(((color_hsl_t) {  30, 100,  50 }), ((color_hsv_t) {  30, 100, 100 }));

    /* Yellow: hsl(60, 100.00%, 50.00%). */
    TEST_ASSERT_COLOR_COMPARE_HSL_HSV(((color_hsl_t) {  60, 100,  50 }), ((color_hsv_t) {  60, 100, 100 }));

    /* Neon: hsl(90, 100.00%, 50.00%). */
    TEST_ASSERT_COLOR_COMPARE_HSL_HSV(((color_hsl_t) {  90, 100,  50 }), ((color_hsv_t) {  90, 100, 100 }));

    /* Green: hsl(120, 100.00%, 50.00%). */
    TEST_ASSERT_COLOR_COMPARE_HSL_HSV(((color_hsl_t) { 120, 100,  50 }), ((color_hsv_t) { 120, 100, 100 }));

    /* Seafoam: hsl(150, 100.00%, 50.00%). */
    TEST_ASSERT_COLOR_COMPARE_HSL_HSV(((color_hsl_t) { 150, 100,  50 }), ((color_hsv_t) { 150, 100, 100 }));

    /* Cyan: hsl(180, 100.00%, 50.00%). */
    TEST_ASSERT_COLOR_COMPARE_HSL_HSV(((color_hsl_t) { 180, 100,  50 }), ((color_hsv_t) { 180, 100, 100 }));

    /* Light Blue: hsl(210, 100.00%, 50.00%). */
    TEST_ASSERT_COLOR_COMPARE_HSL_HSV(((color_hsl_t) { 210, 100,  50 }), ((color_hsv_t) { 210, 100, 100 }));

    /* Blue: hsl(240, 100.00%, 50.00%). */
    TEST_ASSERT_COLOR_COMPARE_HSL_HSV(((color_hsl_t) { 240, 100,  50 }), ((color_hsv_t) { 240, 100, 100 }));

    /* Purple: hsl(270, 100.00%, 50.00%). */
    TEST_ASSERT_COLOR_COMPARE_HSL_HSV(((color_hsl_t) { 270, 100,  50 }), ((color_hsv_t) { 270, 100, 100 }));

    /* Magenta: hsl(300, 100.00%, 50.00%). */
    TEST_ASSERT_COLOR_COMPARE_HSL_HSV(((color_hsl_t) { 300, 100,  50 }), ((color_hsv_t) { 300, 100, 100 }));

    /* Pink: hsl(330, 100.00%, 50.00%). */
    TEST_ASSERT_COLOR_COMPARE_HSL_HSV(((color_hsl_t) { 330, 100,  50 }), ((color_hsv_t) { 330, 100, 100 }));

    /* White: hsl(0, 0.00%, 100.00%). */
    TEST_ASSERT_COLOR_COMPARE_HSL_HSV(((color_hsl_t) {   0,   0, 100 }), ((color_hsv_t) {   0,   0, 100 }));

    /* Black: hsl(0, 0.00%, 0.00%). */
    TEST_ASSERT_COLOR_COMPARE_HSL_HSV(((color_hsl_t) {   0,   0,   0 }), ((color_hsv_t) {   0,   0,   0 }));
}

TEST_GROUP_RUNNER(color)
{
    RUN_TEST_CASE(color, hsv_to_rgb);
    RUN_TEST_CASE(color, hsl_to_rgb);
    RUN_TEST_CASE(color, hsv_to_hsl);
    RUN_TEST_CASE(color, rgb_to_hsv);
    RUN_TEST_CASE(color, rgb_to_hsl);
    RUN_TEST_CASE(color, hsl_to_hsv);
}
