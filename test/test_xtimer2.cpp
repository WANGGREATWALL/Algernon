#if ENABLE_TEST_XTIMER2

#include "perf/xtimer2.h"
#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <functional>

using namespace au::perf;

static std::string captureStdout(const std::function<void()>& fn) {
    testing::internal::CaptureStdout();
    fn();
    std::fflush(stdout);
    return testing::internal::GetCapturedStdout();
}

TEST(XTimer2Test, ReleaseModeLinearLog) {
    XPerfContext ctx("TestLinear");
    PerfConfig cfg;
    cfg.treeMode = false;
    ctx.setConfig(cfg);

    std::string out = captureStdout([&]() {
        XTimer2Scoped root(&ctx, "RootTask");
        XTimer2::sleepFor(10);
        root.sub("Phase1");
        XTimer2::sleepFor(20);
        root.sub("Phase2");
        XTimer2::sleepFor(15);
    });

    EXPECT_NE(out.find("[Timer:TestLinear] RootTask (pre-sub):"), std::string::npos);
    EXPECT_NE(out.find("[Timer:TestLinear] RootTask -> Phase1:"), std::string::npos);
    EXPECT_NE(out.find("[Timer:TestLinear] RootTask -> Phase2:"), std::string::npos);
    EXPECT_NE(out.find("[Timer:TestLinear] RootTask (Total):"), std::string::npos);
}

TEST(XTimer2Test, TreeFormatVerification) {
    XPerfContext ctx("TestTreeFormat");
    PerfConfig cfg;
    cfg.treeMode = true;
    ctx.setConfig(cfg);

    std::string out = captureStdout([&]() {
        XTimer2Scoped root(&ctx, "MainLoop");
        root.sub("ProcessInput");
        root.sub("UpdateLogic");
        {
            XTimer2Scoped logic(&ctx, "Physics");
            logic.sub("Collision");
        }
        root.sub("Render");
    });

    EXPECT_NE(out.find("=== Performance Tree [TestTreeFormat] ==="), std::string::npos);
    EXPECT_NE(out.find("MainLoop:"), std::string::npos);
    EXPECT_NE(out.find("├── ProcessInput:"), std::string::npos);
    EXPECT_NE(out.find("├── UpdateLogic:"), std::string::npos);
    EXPECT_NE(out.find("│   └── Physics:"), std::string::npos);
    EXPECT_NE(out.find("│       └── Collision:"), std::string::npos);
    EXPECT_NE(out.find("└── Render:"), std::string::npos);
}

TEST(XTimer2Test, MultiThreadTreeLog) {
    XPerfContext ctx("TestMultiThread");
    PerfConfig cfg;
    cfg.treeMode = true;
    ctx.setConfig(cfg);

    auto worker = [&ctx](int threadIdx) {
        XTimer2Scoped root(&ctx, "ThreadWork");
        root.sub("ThreadPhase1");
        root.sub("ThreadPhase2");
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back(worker, i);
    }
    for (auto& t : threads) {
        t.join();
    }
    SUCCEED(); // No garbled output verification natively, just crash check
}

TEST(XTimer2Test, PropertyControlLevel) {
    XPerfContext ctx("TestLevel");
    PerfConfig cfg;
    cfg.timerEnabled = true;
    cfg.timerLevel = 0;
    cfg.treeMode = true;
    ctx.setConfig(cfg);

    std::string out = captureStdout([&]() {
        XTimer2Scoped root(&ctx, "VisibleRoot", 0);
        root.sub("VisibleSub1");
        XTimer2Scoped hidden(&ctx, "HiddenSub", 1);
        hidden.sub("HiddenInner");
    });

    EXPECT_NE(out.find("VisibleRoot"), std::string::npos);
    EXPECT_EQ(out.find("HiddenSub"), std::string::npos);
}

TEST(XTimer2Test, NegativeOrZeroDuration) {
    XPerfContext ctx("TestZero");
    PerfConfig cfg;
    cfg.treeMode = true;
    ctx.setConfig(cfg);

    std::string out = captureStdout([&]() {
        XTimer2Scoped root(&ctx, "FastTask");
        root.sub("FastSub");
        // No sleep, duration might be 0.00
    });
    EXPECT_NE(out.find("FastTask: 0."), std::string::npos);
}

TEST(XTimer2Test, UnbalancedSubCalls) {
    XPerfContext ctx("TestUnbalanced");
    PerfConfig cfg;
    cfg.treeMode = true;
    ctx.setConfig(cfg);

    std::string out = captureStdout([&]() {
        XTimer2Scoped root(&ctx, "Root");
        root.sub("Sub1");
        root.sub(); // Explicit close
        root.sub(); // Extra close should be safe
        root.sub("Sub2");
    });
    EXPECT_NE(out.find("├── Sub1:"), std::string::npos);
    EXPECT_NE(out.find("└── Sub2:"), std::string::npos);
}

TEST(XTimer2Test, DeepNesting) {
    XPerfContext ctx("TestDeepNesting");
    PerfConfig cfg;
    cfg.treeMode = true;
    ctx.setConfig(cfg);

    std::string out = captureStdout([&]() {
        XTimer2Scoped* timers[20];
        std::vector<std::string> names(20);
        for (int i = 0; i < 20; ++i) {
            names[i] = "Level" + std::to_string(i);
            timers[i] = new XTimer2Scoped(&ctx, names[i].c_str());
        }
        for (int i = 19; i >= 0; --i) {
            delete timers[i];
        }
    });
    EXPECT_NE(out.find("Level0"), std::string::npos);
    EXPECT_NE(out.find("Level19"), std::string::npos);
}

TEST(XTimer2Test, DisabledTimerBehavior) {
    XPerfContext ctx("TestDisabled");
    PerfConfig cfg;
    cfg.timerEnabled = false;
    cfg.treeMode = true;
    ctx.setConfig(cfg);

    std::string out = captureStdout([&]() {
        XTimer2Scoped root(&ctx, "HiddenRoot");
        root.sub("HiddenPhase");
        XTimer2Scoped child(&ctx, "HiddenChild");
    });
    EXPECT_TRUE(out.empty());
}

#endif  // ENABLE_TEST_XTIMER2
