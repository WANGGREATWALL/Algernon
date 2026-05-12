#if ENABLE_TEST_XTIMER

#include "gtest/gtest.h"
#include "perf/xtimer.h"

using namespace au::perf;

TEST(XTimer, Elapsed) {
    XTimer t;
    XTimer::sleepFor(10);
    float ms = t.elapsed();
    EXPECT_GT(ms, 5.0f);
    EXPECT_LT(ms, 100.0f);
}

TEST(XTimer, Restart) {
    XTimer t;
    XTimer::sleepFor(10);
    t.restart();
    float ms = t.elapsed();
    EXPECT_LT(ms, 5.0f);
}

TEST(XTimer, TimeFormatted) {
    auto s = XTimer::getTimeFormatted();
    EXPECT_FALSE(s.empty());
    EXPECT_GE(s.size(), 10u);
}

#endif  // ENABLE_TEST_XTIMER
