#include "log/xlogger.h"
#include "version.h"
#include "gtest/gtest.h"

int main(int argc, char** argv) {
    printf("===========================================================\n");
    printf("      ------    ----    ---- -----------     ------    \n");
    printf("     ********   ****    **** ***********    ********   \n");
    printf("    ----------  ----    ---- ----    ---   ----------  \n");
    printf("   ****    **** ****    **** *********    ****    **** \n");
    printf("   ------------ ----    ---- ---------    ------------ \n");
    printf("   ************ ************ ****  ****   ************ \n");
    printf("   ----    ---- ------------ ----   ----  ----    ---- \n");
    printf("   ****    **** ************ ****    **** ****    **** \n");
    printf("                                                           \n");
    printf("                        Version %s                           \n", AURA_VERSION);
    printf("===========================================================\n\n");

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
