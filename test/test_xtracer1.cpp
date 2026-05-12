#if ENABLE_TEST_XTRACER1

#include <atomic>
#include <chrono>
#include <string>
#include <thread>
#include <vector>

#include "gtest/gtest.h"
#include "log/xlogger.h"
#include "perf/xtracer1.h"

using namespace au::perf;

// ---------------------------------------------------------------------------
// Test fixture
// ---------------------------------------------------------------------------

class XTracer1Test : public ::testing::Test
{
protected:
    void SetUp() override
    {
        auto& cfg = XPerfContext1::defaultConfig();
        cfg.setEnabled(true);
        cfg.setMode(Mode::Release);
        cfg.setTimerLevel(kPerfLevelAll);
        cfg.setTracerLevel(kPerfLevelAll);
    }
};

// ---------------------------------------------------------------------------
// Basic lifecycle
// ---------------------------------------------------------------------------

TEST_F(XTracer1Test, BasicLifecycle)
{
    {
        XTracer1Scoped t("basic.test");
    }
    SUCCEED();
}

TEST_F(XTracer1Test, LifecycleWithLevel)
{
    {
        XTracer1Scoped t("lv.test", 0);
    }
    SUCCEED();
}

// ---------------------------------------------------------------------------
// Sub-slice transitions
// ---------------------------------------------------------------------------

TEST_F(XTracer1Test, SubSliceTransitions)
{
    {
        XTracer1Scoped t("main");
        t.sub("phase.a");
        t.sub();         // close phase.a
        t.sub("phase.b");
        t.sub();         // close phase.b
    }
    SUCCEED();
}

TEST_F(XTracer1Test, SubWithoutOpen)
{
    {
        XTracer1Scoped t("main");
        t.sub();  // no sub open — should be no-op
    }
    SUCCEED();
}

TEST_F(XTracer1Test, MultipleSubs)
{
    {
        XTracer1Scoped t("main");
        t.sub("A");
        t.sub("B");  // closes A, opens B
        t.sub("C");  // closes B, opens C
        t.sub();     // closes C
    }
    SUCCEED();
}

// ---------------------------------------------------------------------------
// Disabled and level filtering
// ---------------------------------------------------------------------------

TEST_F(XTracer1Test, MasterSwitchOff)
{
    auto& cfg = XPerfContext1::defaultConfig();
    cfg.setEnabled(false);

    {
        XTracer1Scoped t("should.be.silent");
        t.sub("sub");
        t.sub();
    }
    SUCCEED();
}

TEST_F(XTracer1Test, LevelFiltered)
{
    auto& cfg = XPerfContext1::defaultConfig();
    cfg.setTracerLevel(1);

    {
        XTracer1Scoped active("active", 1);    // should activate
        XTracer1Scoped inactive("inactive", 2); // should be silent
    }
    SUCCEED();
}

TEST_F(XTracer1Test, TracerLevelOff)
{
    auto& cfg = XPerfContext1::defaultConfig();
    cfg.setTracerLevel(kPerfLevelOff);

    {
        XTracer1Scoped t("off");
    }
    SUCCEED();
}

// ---------------------------------------------------------------------------
// Name edge cases
// ---------------------------------------------------------------------------

TEST_F(XTracer1Test, EmptyName)
{
    {
        XTracer1Scoped t("");
    }
    SUCCEED();
}

TEST_F(XTracer1Test, VeryLongName)
{
    std::string longName(500, 'T');
    {
        XTracer1Scoped t(longName);
        t.sub(longName);
        t.sub();
    }
    SUCCEED();
}

TEST_F(XTracer1Test, LongSubName)
{
    std::string longName(300, 'S');
    {
        XTracer1Scoped t("main");
        t.sub(longName);  // should be truncated to 127 chars in trace_marker
        t.sub();
    }
    SUCCEED();
}

// ---------------------------------------------------------------------------
// Per-caller config
// ---------------------------------------------------------------------------

TEST_F(XTracer1Test, PerCallerConfig)
{
    XPerfContext1 cfgA;
    cfgA.setEnabled(true);
    cfgA.setTracerLevel(kPerfLevelAll);

    XPerfContext1 cfgB;
    cfgB.setEnabled(false);

    {
        XTracer1Scoped ta(cfgA, "libA.scope");
        XTracer1Scoped tb(cfgB, "libB.scope");  // should be silent
    }
    SUCCEED();
}

// ---------------------------------------------------------------------------
// Thread safety
// ---------------------------------------------------------------------------

TEST_F(XTracer1Test, ThreadSafety)
{
    const int           kThreads = 4;
    const int           kIters   = 50;
    std::vector<std::thread> threads;
    std::atomic<int>         counter{0};

    for (int i = 0; i < kThreads; ++i) {
        threads.emplace_back([&counter]() {
            for (int j = 0; j < kIters; ++j) {
                XTracer1Scoped t("thread.work");
                t.sub("sub.phase");
                t.sub();
            }
            counter.fetch_add(1, std::memory_order_relaxed);
        });
    }
    for (auto& t : threads) {
        t.join();
    }
    EXPECT_EQ(counter.load(), kThreads);
}

// ---------------------------------------------------------------------------
// Composite macro
// ---------------------------------------------------------------------------

TEST_F(XTracer1Test, CompositeMacro)
{
    auto& cfg = XPerfContext1::defaultConfig();
    cfg.setMode(Mode::Debug);
    cfg.setTimerLevel(kPerfLevelAll);
    cfg.setTracerLevel(kPerfLevelAll);

    {
        AU_PERF1_SCOPE("composite.test");
    }
    SUCCEED();
}

TEST_F(XTracer1Test, CompositeMacroLevel)
{
    {
        AU_PERF1_SCOPE_L("composite.lv", 0);
    }
    SUCCEED();
}

TEST_F(XTracer1Test, CompositeMacroCfg)
{
    XPerfContext1 myCfg;
    {
        AU_PERF1_SCOPE_CFG(myCfg, "composite.cfg", 0);
    }
    SUCCEED();
}

// ---------------------------------------------------------------------------
// Independent timer/tracer levels
// ---------------------------------------------------------------------------

TEST_F(XTracer1Test, IndependentTimerTracerLevels)
{
    auto& cfg = XPerfContext1::defaultConfig();

    // Timer off, tracer on — tracer should still work.
    cfg.setTimerLevel(kPerfLevelOff);
    cfg.setTracerLevel(kPerfLevelAll);

    {
        XTracer1Scoped t("only.trace");
    }
    SUCCEED();
}

TEST_F(XTracer1Test, TimerOnTracerOff)
{
    auto& cfg = XPerfContext1::defaultConfig();

    // Timer on, tracer off — timer should still work.
    cfg.setTimerLevel(kPerfLevelAll);
    cfg.setTracerLevel(kPerfLevelOff);

    {
        AU_TIMER1("only.timer");
    }
    SUCCEED();
}

#endif  // ENABLE_TEST_XTRACER1
