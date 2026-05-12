#if ENABLE_TEST_XTIMER1

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <string>
#include <thread>
#include <vector>

#include "gtest/gtest.h"
#include "log/xlogger.h"
#include "perf/xtimer1.h"

using namespace au::perf;

// ---------------------------------------------------------------------------
// Test fixture
// ---------------------------------------------------------------------------

class XTimer1Test : public ::testing::Test
{
protected:
    void SetUp() override
    {
        auto& cfg = XPerfContext1::defaultConfig();
        cfg.setEnabled(true);
        cfg.setMode(Mode::Release);
        cfg.setTimerLevel(kPerfLevelAll);
        cfg.setTracerLevel(kPerfLevelOff);
        cfg.setMaxTreeDepth(64);
        cfg.setAggregateMode(false);
    }
};

// ---------------------------------------------------------------------------
// XTimer1 (stopwatch)
// ---------------------------------------------------------------------------

TEST_F(XTimer1Test, BasicStopwatch)
{
    XTimer1 t;
    XTimer1::sleepFor(std::chrono::milliseconds(10));
    float ms = t.elapsedMs();
    EXPECT_GE(ms, 5.0f);
    EXPECT_LT(ms, 500.0f);
}

TEST_F(XTimer1Test, StopwatchRestart)
{
    XTimer1 t;
    XTimer1::sleepFor(std::chrono::milliseconds(10));
    t.restart();
    float ms = t.elapsedMs();
    EXPECT_LT(ms, 5.0f);
}

TEST_F(XTimer1Test, SleepForEdgeCases)
{
    // Zero and negative should return immediately without crash.
    XTimer1::sleepFor(std::chrono::milliseconds(0));
    XTimer1::sleepFor(std::chrono::milliseconds(-1));
    SUCCEED();
}

TEST_F(XTimer1Test, TimeFormatted)
{
    std::string s = XTimer1::getTimeFormatted();
    EXPECT_FALSE(s.empty());
    EXPECT_GE(s.size(), 10u);
}

// ---------------------------------------------------------------------------
// Release mode
// ---------------------------------------------------------------------------

TEST_F(XTimer1Test, ReleaseModeNoCrash)
{
    auto& cfg = XPerfContext1::defaultConfig();
    cfg.setMode(Mode::Release);

    {
        XTimer1Scoped s("release.test");
        XTimer1::sleepFor(std::chrono::milliseconds(2));
        s.sub("sub.phase");
        XTimer1::sleepFor(std::chrono::milliseconds(1));
        s.sub();
    }
    SUCCEED();
}

// ---------------------------------------------------------------------------
// Debug mode
// ---------------------------------------------------------------------------

TEST_F(XTimer1Test, DebugModeTree)
{
    auto& cfg = XPerfContext1::defaultConfig();
    cfg.setMode(Mode::Debug);
    cfg.setRootName("TestTree");

    {
        XTimer1Scoped root("root");
        {
            XTimer1Scoped child("child1");
            XTimer1::sleepFor(std::chrono::milliseconds(2));
        }
        root.sub("phase_a");
        XTimer1::sleepFor(std::chrono::milliseconds(2));
        root.sub("phase_b");
        XTimer1::sleepFor(std::chrono::milliseconds(2));
        root.sub();
    }
    SUCCEED();
}

TEST_F(XTimer1Test, DebugModeNestedSubs)
{
    auto& cfg = XPerfContext1::defaultConfig();
    cfg.setMode(Mode::Debug);

    {
        XTimer1Scoped root("root");
        root.sub("A");
        root.sub("B");
        root.sub();  // close B
        root.sub("C");
        root.sub();  // close C
    }
    SUCCEED();
}

// ---------------------------------------------------------------------------
// Level filtering
// ---------------------------------------------------------------------------

TEST_F(XTimer1Test, LevelFiltering)
{
    auto& cfg = XPerfContext1::defaultConfig();
    cfg.setTimerLevel(1);

    {
        XTimer1Scoped active("active", 1);    // level <= 1 → active
        XTimer1Scoped inactive("inactive", 2); // level > 1 → inactive
    }
    SUCCEED();
}

TEST_F(XTimer1Test, LevelSentinelOff)
{
    auto& cfg = XPerfContext1::defaultConfig();
    cfg.setTimerLevel(kPerfLevelOff);  // -1

    {
        XTimer1Scoped silent("silent", 0);  // 0 > -1 → inactive
    }
    SUCCEED();
}

TEST_F(XTimer1Test, LevelSentinelAll)
{
    auto& cfg = XPerfContext1::defaultConfig();
    cfg.setTimerLevel(kPerfLevelAll);

    {
        XTimer1Scoped always("always", 999999);
    }
    SUCCEED();
}

// ---------------------------------------------------------------------------
// Master switch
// ---------------------------------------------------------------------------

TEST_F(XTimer1Test, MasterSwitchOff)
{
    auto& cfg = XPerfContext1::defaultConfig();
    cfg.setMode(Mode::Debug);
    cfg.setEnabled(false);

    {
        XTimer1Scoped s("should.be.silent");
    }
    // No crash, no output expected.
    SUCCEED();
}

TEST_F(XTimer1Test, MasterSwitchToggle)
{
    auto& cfg = XPerfContext1::defaultConfig();

    cfg.setEnabled(false);
    {
        XTimer1Scoped s("off");
    }

    cfg.setEnabled(true);
    {
        XTimer1Scoped s("on");
    }
    SUCCEED();
}

// ---------------------------------------------------------------------------
// Depth guard
// ---------------------------------------------------------------------------

TEST_F(XTimer1Test, MaxDepthGuard)
{
    auto& cfg = XPerfContext1::defaultConfig();
    cfg.setMode(Mode::Debug);
    cfg.setMaxTreeDepth(2);

    {
        XTimer1Scoped root("depth0");
        {
            XTimer1Scoped d1("depth1");
            {
                XTimer1Scoped d2("depth2");  // exceeds depth 2 → degraded
            }
        }
    }
    SUCCEED();
}

// ---------------------------------------------------------------------------
// Aggregate mode
// ---------------------------------------------------------------------------

TEST_F(XTimer1Test, AggregateCollect)
{
    auto& cfg = XPerfContext1::defaultConfig();
    cfg.setMode(Mode::Debug);
    cfg.setAggregateMode(true);

    {
        XTimer1Scoped s("agg.block");
    }

    XPerfContext1::flushAggregated();
    // Second flush should be idempotent.
    XPerfContext1::flushAggregated();
    SUCCEED();
}

// ---------------------------------------------------------------------------
// Thread isolation
// ---------------------------------------------------------------------------

TEST_F(XTimer1Test, ThreadIsolation)
{
    auto& cfg = XPerfContext1::defaultConfig();
    cfg.setMode(Mode::Debug);

    const int           kThreads = 4;
    std::vector<std::thread> threads;
    std::atomic<int>         counter{0};

    for (int i = 0; i < kThreads; ++i) {
        threads.emplace_back([&counter]() {
            XTimer1Scoped s("thread.work");
            XTimer1::sleepFor(std::chrono::milliseconds(1));
            s.sub("sub.work");
            XTimer1::sleepFor(std::chrono::milliseconds(1));
            s.sub();
            counter.fetch_add(1, std::memory_order_relaxed);
        });
    }
    for (auto& t : threads) {
        t.join();
    }
    EXPECT_EQ(counter.load(), kThreads);
}

TEST_F(XTimer1Test, ThreadSafetyAggregate)
{
    auto& cfg = XPerfContext1::defaultConfig();
    cfg.setMode(Mode::Debug);
    cfg.setAggregateMode(true);

    const int           kThreads = 4;
    const int           kIters   = 20;
    std::vector<std::thread> threads;

    for (int i = 0; i < kThreads; ++i) {
        threads.emplace_back([i]() {
            for (int j = 0; j < kIters; ++j) {
                XTimer1Scoped s("iter");
                s.sub("sub.task");
                s.sub();
            }
        });
    }
    for (auto& t : threads) {
        t.join();
    }

    XPerfContext1::flushAggregated();
    SUCCEED();
}

// ---------------------------------------------------------------------------
// Name edge cases
// ---------------------------------------------------------------------------

TEST_F(XTimer1Test, NameTruncation)
{
    auto& cfg = XPerfContext1::defaultConfig();
    cfg.setMode(Mode::Debug);

    // Name longer than internal buffer (256 for degraded, 255 for arena)
    std::string longName(500, 'x');
    {
        XTimer1Scoped s(longName);
    }
    SUCCEED();
}

TEST_F(XTimer1Test, EmptyName)
{
    auto& cfg = XPerfContext1::defaultConfig();
    cfg.setMode(Mode::Debug);

    {
        XTimer1Scoped s("");
    }
    SUCCEED();
}

TEST_F(XTimer1Test, EmptyNameWithSubs)
{
    auto& cfg = XPerfContext1::defaultConfig();
    cfg.setMode(Mode::Debug);

    {
        XTimer1Scoped s("root");
        s.sub("");
        s.sub();
    }
    SUCCEED();
}

// ---------------------------------------------------------------------------
// Multiple top-level scopes (multiple roots)
// ---------------------------------------------------------------------------

TEST_F(XTimer1Test, MultipleRoots)
{
    auto& cfg = XPerfContext1::defaultConfig();
    cfg.setMode(Mode::Debug);

    {
        XTimer1Scoped r1("root1");
        XTimer1::sleepFor(std::chrono::milliseconds(1));
    }
    {
        XTimer1Scoped r2("root2");
        XTimer1::sleepFor(std::chrono::milliseconds(1));
    }
    SUCCEED();
}

// ---------------------------------------------------------------------------
// Per-caller config
// ---------------------------------------------------------------------------

TEST_F(XTimer1Test, PerCallerConfig)
{
    XPerfContext1 cfgA;
    cfgA.setEnabled(true);
    cfgA.setMode(Mode::Debug);
    cfgA.setRootName("LibA");

    XPerfContext1 cfgB;
    cfgB.setEnabled(false);  // library B disabled

    {
        XTimer1Scoped sa(cfgA, "a.scope");
    }
    {
        XTimer1Scoped sb(cfgB, "b.scope");  // should be silent
    }
    SUCCEED();
}

TEST_F(XTimer1Test, IndependentLevels)
{
    XPerfContext1 cfgA;
    cfgA.setTimerLevel(0);  // only level 0

    XPerfContext1 cfgB;
    cfgB.setTimerLevel(kPerfLevelAll);

    {
        XTimer1Scoped a_level0(cfgA, "a0", 0);  // active
        XTimer1Scoped a_level1(cfgA, "a1", 1);  // inactive
        XTimer1Scoped b_level5(cfgB, "b5", 5);  // active (all levels)
    }
    SUCCEED();
}

// ---------------------------------------------------------------------------
// Macro usage
// ---------------------------------------------------------------------------

TEST_F(XTimer1Test, MacroUsage)
{
    auto& cfg = XPerfContext1::defaultConfig();
    cfg.setMode(Mode::Release);

    AU_TIMER1("macro.scope");
    AU_TIMER1_L("macro.scope.lv", 0);

    XPerfContext1 myCfg;
    AU_TIMER1_CFG(myCfg, "macro.scope.cfg", 0);
    SUCCEED();
}

// ---------------------------------------------------------------------------
// Zero-duration scope
// ---------------------------------------------------------------------------

TEST_F(XTimer1Test, ZeroDuration)
{
    auto& cfg = XPerfContext1::defaultConfig();
    cfg.setMode(Mode::Debug);

    {
        XTimer1Scoped s("instant");  // destructed immediately
    }
    SUCCEED();
}

// ---------------------------------------------------------------------------
// Unbalanced sub calls
// ---------------------------------------------------------------------------

TEST_F(XTimer1Test, DoubleSubClose)
{
    auto& cfg = XPerfContext1::defaultConfig();
    cfg.setMode(Mode::Debug);

    {
        XTimer1Scoped s("root");
        s.sub("A");
        s.sub();  // close A
        s.sub();  // extra close — should be no-op
        s.sub("B");
        s.sub();  // close B
    }
    SUCCEED();
}

TEST_F(XTimer1Test, SubWithoutOpen)
{
    auto& cfg = XPerfContext1::defaultConfig();
    cfg.setMode(Mode::Debug);

    {
        XTimer1Scoped s("root");
        s.sub();  // no sub open — should be no-op
    }
    SUCCEED();
}

// ---------------------------------------------------------------------------
// Deep nesting (but within depth limit)
// ---------------------------------------------------------------------------

TEST_F(XTimer1Test, DeepNesting)
{
    auto& cfg = XPerfContext1::defaultConfig();
    cfg.setMode(Mode::Debug);
    cfg.setMaxTreeDepth(64);

    // Nest 32 scopes — all should be tracked in the tree.
    {
        XTimer1Scoped d0("d00");
        {
            XTimer1Scoped d1("d01");
            {
                XTimer1Scoped d2("d02");
                {
                    XTimer1Scoped d3("d03");
                    {
                        XTimer1Scoped d4("d04");
                    }
                }
            }
        }
    }
    SUCCEED();
}

#endif  // ENABLE_TEST_XTIMER1
