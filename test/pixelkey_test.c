
#include <stdio.h>

#include "unity_fixture.h"

static void runAllTests(void);

int main(int argc, const char* argv[])
{
    return UnityMain(argc, argv, runAllTests);
}

static void runAllTests(void)
{
    RUN_TEST_GROUP(color);
    RUN_TEST_GROUP(command_parse);
    RUN_TEST_GROUP(keyframe_fade);
}
