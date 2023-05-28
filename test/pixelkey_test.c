
#include <stdio.h>

#include "unity_fixture.h"

#define TEST_PRINT_BEZIER_CURVE 1

static void runAllTests(void);

int main(int argc, const char* argv[])
{
    return UnityMain(argc, argv, runAllTests);
}

static void runAllTests(void)
{
    RUN_TEST_GROUP(color);
    RUN_TEST_GROUP(command_parse);

#if TEST_PRINT_BEZIER_CURVE
    RUN_TEST_GROUP(keyframe_fade);
#endif
}
