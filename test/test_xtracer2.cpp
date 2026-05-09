#include "perf/xtracer2.h"
#include <gtest/gtest.h>
#include <functional>

using namespace au::perf;

static std::string captureStdout(const std::function<void()>& fn) {
    testing::internal::CaptureStdout();
    fn();
    std::fflush(stdout);
    return testing::internal::GetCapturedStdout();
}

TEST(XTracer2Test, TracerOnly) {
    XPerfContext ctx("TestTracer");
    PerfConfig cfg;
    cfg.tracerEnabled = true;
    ctx.setConfig(cfg);

    {
        XTracer2Scoped t(&ctx, "TraceRoot");
        t.sub("TracePhase1");
        t.sub("TracePhase2");
    }
    // Hard to verify without android system, just test it doesn't crash
    SUCCEED();
}

TEST(XTracer2Test, PerfScopedCombined) {
    XPerfContext ctx("TestCombined");
    PerfConfig cfg;
    cfg.timerEnabled = true;
    cfg.tracerEnabled = true;
    cfg.treeMode = true;
    ctx.setConfig(cfg);

    std::string out = captureStdout([&]() {
        XPerfScoped perf(&ctx, "CombinedRoot");
        perf.sub("CombinedPhase1");
        perf.sub("CombinedPhase2");
    });

    EXPECT_NE(out.find("=== Performance Tree [TestCombined] ==="), std::string::npos);
    EXPECT_NE(out.find("CombinedRoot:"), std::string::npos);
    EXPECT_NE(out.find("├── CombinedPhase1:"), std::string::npos);
    EXPECT_NE(out.find("└── CombinedPhase2:"), std::string::npos);
}

TEST(XTracer2Test, TracerDisabled) {
    XPerfContext ctx("TestTracerDisabled");
    PerfConfig cfg;
    cfg.tracerEnabled = false;
    ctx.setConfig(cfg);

    {
        XTracer2Scoped t(&ctx, "HiddenTrace");
        t.sub("HiddenPhase");
    }
    SUCCEED();
}

TEST(XTracer2Test, TracerEmptyNames) {
    XPerfContext ctx("TestEmpty");
    PerfConfig cfg;
    cfg.tracerEnabled = true;
    ctx.setConfig(cfg);

    {
        XTracer2Scoped t(&ctx, "");
        t.sub("");
        t.sub(nullptr);
    }
    SUCCEED();
}

TEST(XTracer2Test, TracerSubTransitions) {
    XPerfContext ctx("TestSubTracer");
    PerfConfig cfg;
    cfg.tracerEnabled = true;
    ctx.setConfig(cfg);

    {
        XTracer2Scoped t(&ctx, "Root");
        t.sub("Phase1");
        t.sub(); // Explicit close
        t.sub(); // Duplicate close shouldn't crash
        t.sub("Phase2");
    }
    SUCCEED();
}
