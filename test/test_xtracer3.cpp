#if ENABLE_TEST_XTRACER3

#include <thread>
#include <vector>

#include "gtest/gtest.h"
#include "log/xlogger.h"
#include "perf/xtimer3.h"
#include "perf/xtracer3.h"
#include "sys/xplatform.h"

#if AU_OS_ANDROID
#include <fcntl.h>
#include <unistd.h>
#endif

class XTracer3Test : public ::testing::Test
{
protected:
    void SetUp() override
    {
        auto& cfg = au::perf::XPerfContext3::defaultConfig();
        cfg.setEnabled(true);
        cfg.setMode(au::perf::Mode::Release);
        cfg.setTimerLevel(au::perf::kPerfLevelOff);  // silence timer channel during tracer tests
        cfg.setTracerLevel(au::perf::kPerfLevelAll);
        cfg.setAggregateMode(false);
    }
};

TEST_F(XTracer3Test, BasicLifecycle)
{
    {
        au::perf::XTracer3Scoped t("tracer.basic");
        au::perf::XTimer3::sleepFor(std::chrono::milliseconds(1));
    }
    SUCCEED();
}

TEST_F(XTracer3Test, SubSlices)
{
    au::perf::XTracer3Scoped t("tracer.sub");
    t.sub("phase1");
    au::perf::XTimer3::sleepFor(std::chrono::milliseconds(1));
    t.sub("phase2");
    au::perf::XTimer3::sleepFor(std::chrono::milliseconds(1));
    t.sub();
    SUCCEED();
}

TEST_F(XTracer3Test, DisabledIsNoOp)
{
    auto& cfg = au::perf::XPerfContext3::defaultConfig();
    cfg.setEnabled(false);
    {
        au::perf::XTracer3Scoped t("tracer.off");
        t.sub("nope");
        t.sub();
    }
    cfg.setEnabled(true);
    SUCCEED();
}

TEST_F(XTracer3Test, LevelFilter)
{
    auto& cfg = au::perf::XPerfContext3::defaultConfig();
    cfg.setTracerLevel(2);
    {
        au::perf::XTracer3Scoped active("tracer.active", 2);
        au::perf::XTracer3Scoped rejected("tracer.rejected", 3);
        rejected.sub("shouldBeIgnored");
        rejected.sub();
    }
    SUCCEED();
}

TEST_F(XTracer3Test, PrivateConfig)
{
    au::perf::XPerfContext3 libCfg;
    libCfg.setTracerLevel(au::perf::kPerfLevelAll);
    {
        au::perf::XTracer3Scoped t(libCfg, "lib.scope");
        t.sub("step");
        t.sub();
    }
    SUCCEED();
}

TEST_F(XTracer3Test, ThreadSafety)
{
    constexpr int kThreads = 4;
    constexpr int kIter    = 50;

    std::vector<std::thread> ths;
    ths.reserve(kThreads);
    for (int i = 0; i < kThreads; ++i) {
        ths.emplace_back([] {
            for (int j = 0; j < kIter; ++j) {
                au::perf::XTracer3Scoped t("mt.scope");
                t.sub("a");
                t.sub("b");
                t.sub();
            }
        });
    }
    for (auto& t : ths) {
        t.join();
    }
    SUCCEED();
}

TEST_F(XTracer3Test, CompositeScopeMacro)
{
    auto& cfg = au::perf::XPerfContext3::defaultConfig();
    cfg.setMode(au::perf::Mode::Release);
    cfg.setTimerLevel(au::perf::kPerfLevelAll);
    cfg.setTracerLevel(au::perf::kPerfLevelAll);

    {
        AU_PERF3_SCOPE("composite.outer");
        {
            AU_PERF3_SCOPE_L("composite.inner", 0);
            au::perf::XTimer3::sleepFor(std::chrono::milliseconds(1));
        }
    }
    SUCCEED();
}

#endif  // ENABLE_TEST_XTRACER3
