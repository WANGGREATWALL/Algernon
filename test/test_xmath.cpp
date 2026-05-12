#if ENABLE_TEST_XMATH

#include "gtest/gtest.h"
#include "math/xmath.h"

using namespace au::math;

TEST(XMath, Constants) {
    EXPECT_NEAR(kPi, 3.14159265358979323846, 1e-15);
    EXPECT_NEAR(kE,  2.71828182845904523536, 1e-15);
    EXPECT_FLOAT_EQ(kEpsilonF, 1e-6f);
}

TEST(XMath, MinMax) {
    EXPECT_EQ(minOf(3, 5), 3);
    EXPECT_EQ(maxOf(3, 5), 5);
    EXPECT_EQ(minOf(3, 1, 5), 1);
    EXPECT_EQ(maxOf(3, 1, 5), 5);
}

TEST(XMath, Clamp) {
    EXPECT_EQ(clamp(5, 0, 10), 5);
    EXPECT_EQ(clamp(-1, 0, 10), 0);
    EXPECT_EQ(clamp(15, 0, 10), 10);
    EXPECT_EQ(clampToU8(300), 255);
    EXPECT_EQ(clampToU8(-5), 0);
    EXPECT_EQ(clampToU16(70000), 65535);
}

TEST(XMath, Alignment) {
    EXPECT_TRUE(isAlignedTo2(4));
    EXPECT_FALSE(isAlignedTo2(3));
    EXPECT_TRUE(isAlignedTo8(16));
    EXPECT_TRUE(isAlignedToN(64, 16));
    EXPECT_FALSE(isAlignedToN(65, 16));

    EXPECT_EQ(ceilTo2(3), 4);
    EXPECT_EQ(ceilTo4(5), 8);
    EXPECT_EQ(ceilTo8(9), 16);
    EXPECT_EQ(ceilTo16(17), 32);
    EXPECT_EQ(ceilToN(13, 8), 16);

    EXPECT_EQ(floorTo2(3), 2);
    EXPECT_EQ(floorTo4(7), 4);
    EXPECT_EQ(floorToN(15, 8), 8);

    EXPECT_TRUE(isPowerOf2(1));
    EXPECT_TRUE(isPowerOf2(256));
    EXPECT_FALSE(isPowerOf2(3));
    EXPECT_FALSE(isPowerOf2(0));
}

TEST(XMath, RangeChecks) {
    EXPECT_TRUE(inRangeCC(5, 0, 10));
    EXPECT_TRUE(inRangeCC(0, 0, 10));
    EXPECT_FALSE(inRangeOO(0, 0, 10));
    EXPECT_TRUE(inRangeOC(10, 0, 10));
    EXPECT_FALSE(inRangeCO(10, 0, 10));
}

TEST(XMath, Trig) {
    EXPECT_NEAR(sinF(0.0f), 0.0f, kEpsilonF);
    EXPECT_NEAR(cosD(0.0), 1.0, kEpsilonD);
    EXPECT_NEAR(deg2rad(180.0), kPi, kEpsilonD);
    EXPECT_NEAR(rad2deg(kPi), 180.0, kEpsilonD);
}

#endif  // ENABLE_TEST_XMATH
