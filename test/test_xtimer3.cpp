#if ENABLE_TEST_XTIMER3

#include <thread>
#include <vector>

#include "gtest/gtest.h"
#include "log/xlogger.h"
#include "perf/xtimer3.h"

class XTimer3Test : public ::testing::Test
{
protected:
    void SetUp() override
    {
        auto& cfg = au::perf::XPerfContext3::defaultConfig();
        cfg.setEnabled(true);
        cfg.setMode(au::perf::Mode::Release);
        cfg.setTimerLevel(au::perf::kPerfLevelAll);
        cfg.setAggregateMode(false);
    }
};

TEST_F(XTimer3Test, BasicTimer)
{
    au::perf::XTimer3 t;
    au::perf::XTimer3::sleepFor(std::chrono::milliseconds(10));
    float elapsed = t.elapsedMs();
    EXPECT_GE(elapsed, 10.0f);

    t.restart();
    elapsed = t.elapsedMs();
    EXPECT_LT(elapsed, 5.0f);
}

TEST_F(XTimer3Test, ReleaseModeLogging)
{
    // Verify that it doesn't crash and prints something (visual check)
    {
        au::perf::XTimer3Scoped s("release.test");
        au::perf::XTimer3::sleepFor(std::chrono::milliseconds(5));
    }
    SUCCEED();
}

TEST_F(XTimer3Test, DebugModeTree)
{
    auto& cfg = au::perf::XPerfContext3::defaultConfig();
    cfg.setMode(au::perf::Mode::Debug);
    cfg.setRootName("MyTree");

    {
        au::perf::XTimer3Scoped root("root");
        {
            au::perf::XTimer3Scoped child1("child1");
            au::perf::XTimer3::sleepFor(std::chrono::milliseconds(2));
        }
        root.sub("sub1");
        au::perf::XTimer3::sleepFor(std::chrono::milliseconds(2));
        root.sub("sub2");
        au::perf::XTimer3::sleepFor(std::chrono::milliseconds(2));
        root.sub();
    }
    SUCCEED();
}

TEST_F(XTimer3Test, LevelFiltering)
{
    auto& cfg = au::perf::XPerfContext3::defaultConfig();
    cfg.setTimerLevel(1);

    {
        au::perf::XTimer3Scoped active("active", 1);
        au::perf::XTimer3Scoped inactive("inactive", 2);
    }
    SUCCEED();
}

TEST_F(XTimer3Test, AggregateMode)
{
    auto& cfg = au::perf::XPerfContext3::defaultConfig();
    cfg.setMode(au::perf::Mode::Debug);
    cfg.setAggregateMode(true);

    {
        au::perf::XTimer3Scoped s("agg.test");
    }

    au::perf::XPerfContext3::flushAggregated();
    SUCCEED();
}

TEST_F(XTimer3Test, ThreadIsolation)
{
    auto& cfg = au::perf::XPerfContext3::defaultConfig();
    cfg.setMode(au::perf::Mode::Debug);

    std::thread t1([]() { au::perf::XTimer3Scoped s("thread1"); });
    std::thread t2([]() { au::perf::XTimer3Scoped s("thread2"); });
    t1.join();
    t2.join();
    SUCCEED();
}

TEST_F(XTimer3Test, MacroUsage)
{
    AU_TIMER3("macro.test");
    AU_TIMER3_L("macro.test.level", 0);

    au::perf::XPerfContext3 myCfg;
    AU_TIMER3_CFG(myCfg, "macro.test.cfg", 0);
    SUCCEED();
}

#endif  // ENABLE_TEST_XTIMER3
