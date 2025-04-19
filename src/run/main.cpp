#include "gtest/gtest.h"

#define TAG_LOGGER "[Algernon]"
#include "logger.h"
volatile int G_LEVEL_LOGGER = LEVEL_LOGGER_DEFAULT;


void showTitle()
{
    LOGGER_I("************************************************\n");
    LOGGER_I("*\n");
    LOGGER_I("* Algernon v%s\n", ALGERNON_VERSION);
    LOGGER_I("*\n");
    LOGGER_I("************************************************\n\n");
}

int main(int argc, char **argv)
{
    showTitle();

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}