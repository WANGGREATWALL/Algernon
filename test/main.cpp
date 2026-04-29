#include "algernon/log/xlogger.h"
#include "algernon/version.h"
#include "gtest/gtest.h"

int main(int argc, char** argv) {
    printf("===========================================================\n");
    printf("       ___    __                                           \n");
    printf("      /   |  / /___  ___  _________  ____  ____            \n");
    printf("     / /| | / / __ `/ _ \\/ ___/ __ \\/ __ \\/ __ \\           \n");
    printf("    / ___ |/ / /_/ /  __/ /  / / / / /_/ / / / /           \n");
    printf("   /_/  |_/_/\\__, /\\___/_/  /_/ /_/\\____/_/ /_/            \n");
    printf("            /____/                                         \n");
    printf("                                                           \n");
    printf("                        Version %s                           \n", ALGERNON_VERSION);
    printf("===========================================================\n\n");

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
