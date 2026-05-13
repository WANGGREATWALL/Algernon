#if ENABLE_TEST_XTRACER0

#include <string>
#include <thread>

#include "gtest/gtest.h"
#include "perf/xtracer0.h"

using namespace au::perf;

// ============================================================================
// Helpers
// ============================================================================

static std::string captureStdout(const std::function<void()>& fn)
{
    testing::internal::CaptureStdout();
    fn();
    std::fflush(stdout);
    return testing::internal::GetCapturedStdout();
}

static bool contains(const std::string& haystack, const std::string& needle)
{
    return haystack.find(needle) != std::string::npos;
}

// ============================================================================
// 1. XTracer0Context
// ============================================================================

TEST(XTracer0Context, GlobalDefaults)
{
    auto& ctx = XTracer0Context::global();
    // Reset to known defaults (singletons persist across tests)
    ctx.setEnabled(false);
    ctx.setLevel(-1);
    ctx.setMode(XTimer0Context::Mode::Release);

    EXPECT_FALSE(ctx.isEnabled());
    EXPECT_EQ(ctx.getLevel(), -1);
    EXPECT_EQ(ctx.getMode(), XTimer0Context::Mode::Release);
}

TEST(XTracer0Context, SetEnabled)
{
    auto& ctx = XTracer0Context::global();
    ctx.setEnabled(true);
    EXPECT_TRUE(ctx.isEnabled());
    ctx.timerContext().setEnabled(true);
    EXPECT_TRUE(ctx.timerContext().isEnabled());

    ctx.setEnabled(false);
    EXPECT_FALSE(ctx.isEnabled());
    EXPECT_FALSE(ctx.timerContext().isEnabled());
}

TEST(XTracer0Context, IndependentLevel)
{
    auto& ctx = XTracer0Context::global();
    ctx.setEnabled(true);

    // Tracer level and timer level are independent
    ctx.setLevel(2);
    ctx.timerContext().setLevel(5);

    EXPECT_EQ(ctx.getLevel(), 2);
    EXPECT_EQ(ctx.timerContext().getLevel(), 5);
}

TEST(XTracer0Context, ShouldPrint)
{
    auto& ctx = XTracer0Context::global();
    ctx.setEnabled(false);
    EXPECT_FALSE(ctx.shouldPrint(0));

    ctx.setEnabled(true);
    ctx.setLevel(1);
    EXPECT_TRUE(ctx.shouldPrint(0));
    EXPECT_TRUE(ctx.shouldPrint(1));
    EXPECT_FALSE(ctx.shouldPrint(2));

    ctx.setLevel(-1);
    EXPECT_TRUE(ctx.shouldPrint(100));
}

TEST(XTracer0Context, NamedContexts)
{
    auto& a = XTracer0Context::get("trace_lib_a");
    auto& b = XTracer0Context::get("trace_lib_b");
    auto& a2 = XTracer0Context::get("trace_lib_a");

    EXPECT_EQ(&a, &a2);
    EXPECT_NE(&a, &b);
}

TEST(XTracer0Context, TimerContextDelegation)
{
    auto& ctx = XTracer0Context::get("delegation_test");
    ctx.timerContext().setEnabled(true);
    ctx.timerContext().setLevel(3);

    EXPECT_TRUE(ctx.timerContext().isEnabled());
    EXPECT_EQ(ctx.timerContext().getLevel(), 3);
}

// ============================================================================
// 2. XTracer0Scoped — basic
// ============================================================================

TEST(XTracer0Scoped, DisabledNoOutput)
{
    auto& ctx = XTracer0Context::global();
    ctx.setEnabled(false);

    auto out = captureStdout([] {
        XTracer0Scoped s("noop");
    });
    EXPECT_TRUE(out.empty());
}

TEST(XTracer0Scoped, EnabledTimerOutput)
{
    auto& ctx = XTracer0Context::global();
    ctx.setEnabled(true);
    ctx.timerContext().setEnabled(true);
    ctx.timerContext().setLevel(-1);
    ctx.timerContext().setMode(XTimer0Context::Mode::Release);

    auto out = captureStdout([] {
        XTracer0Scoped s("traceOp");
        XTimer0::sleepFor(5);
    });
    EXPECT_TRUE(contains(out, "traceOp:"));
}

TEST(XTracer0Scoped, NoCrashWithoutTimer)
{
    // XTracer0Scoped should not crash even if timer context is disabled
    auto& ctx = XTracer0Context::global();
    ctx.setEnabled(true);
    ctx.timerContext().setEnabled(false);

    XTracer0Scoped s("trace_only");
    XTimer0::sleepFor(2);
    SUCCEED();
}

TEST(XTracer0Scoped, SubCalls)
{
    auto& ctx = XTracer0Context::global();
    ctx.setEnabled(true);
    ctx.setLevel(-1);
    ctx.timerContext().setEnabled(true);
    ctx.timerContext().setLevel(-1);
    ctx.timerContext().setMode(XTimer0Context::Mode::Debug);

    auto out = captureStdout([] {
        {
            XTracer0Scoped s("root");
            XTimer0::sleepFor(1);
            s.sub("phase1");
            XTimer0::sleepFor(1);
            s.sub("phase2");
            XTimer0::sleepFor(1);
        }
    });
    EXPECT_TRUE(contains(out, "root:"));
    EXPECT_TRUE(contains(out, "phase1:"));
    EXPECT_TRUE(contains(out, "phase2:"));
}

TEST(XTracer0Scoped, Elapsed)
{
    auto& ctx = XTracer0Context::global();
    ctx.setEnabled(true);

    XTracer0Scoped s("elapsed_test");
    XTimer0::sleepFor(10);
    float ms = s.elapsed();
    EXPECT_GT(ms, 5.f);
}

// ============================================================================
// 3. Level independence (xtimer vs xtracer)
// ============================================================================

TEST(XTracer0Scoped, TracerLevelIndependent)
{
    auto& ctx = XTracer0Context::global();
    ctx.setEnabled(true);
    ctx.setLevel(0);  // tracer: show nothing
    ctx.timerContext().setEnabled(true);
    ctx.timerContext().setLevel(-1);  // timer: show all
    ctx.timerContext().setMode(XTimer0Context::Mode::Release);

    // Timer should still print even when tracer is level 0
    // (the scoped tracer won't write trace markers but timer output still works)
    auto out = captureStdout([] {
        XTracer0Scoped s("indepTest");
        XTimer0::sleepFor(2);
    });
    // Since tracer level is 0, the timer inside won't be created either
    // (because XTracer0Scoped checks shouldPrint before creating XTimer0Scoped)
    // This is intentional: tracer level controls everything
    EXPECT_TRUE(out.empty());
}

TEST(XTracer0Scoped, TimerLevelRestrictedButTracerOpen)
{
    auto& ctx = XTracer0Context::global();
    ctx.setEnabled(true);
    ctx.setLevel(-1);  // tracer: show all
    ctx.timerContext().setEnabled(true);
    ctx.timerContext().setLevel(0);  // timer: show nothing
    ctx.timerContext().setMode(XTimer0Context::Mode::Release);

    // Tracer level -1 means XTracer0Scoped creates the timer,
    // but timer level 0 means no timer output
    auto out = captureStdout([] {
        XTracer0Scoped s("traceOpen");
        XTimer0::sleepFor(2);
    });
    EXPECT_TRUE(out.empty());
}

// ============================================================================
// 4. Edge cases
// ============================================================================

TEST(XTracer0Scoped, NullContext)
{
    XTracer0Context::global().setEnabled(true);
    XTracer0Context::global().setLevel(-1);
    XTracer0Context::global().timerContext().setEnabled(true);
    XTracer0Context::global().timerContext().setLevel(-1);
    XTracer0Context::global().timerContext().setMode(XTimer0Context::Mode::Release);

    auto out = captureStdout([] {
        XTracer0Scoped s("null_ctx", nullptr);
        XTimer0::sleepFor(1);
    });
    EXPECT_TRUE(contains(out, "null_ctx:"));
}

TEST(XTracer0Scoped, RapidCreateDestroy)
{
    auto& ctx = XTracer0Context::global();
    ctx.setEnabled(true);
    ctx.setLevel(-1);

    for (int i = 0; i < 50; ++i) {
        XTracer0Scoped s("rapid");
    }
    SUCCEED();
}

TEST(XTracer0Scoped, BareSubThenNamed)
{
    auto& ctx = XTracer0Context::global();
    ctx.setEnabled(true);
    ctx.setLevel(-1);
    ctx.timerContext().setEnabled(true);
    ctx.timerContext().setLevel(-1);
    ctx.timerContext().setMode(XTimer0Context::Mode::Debug);

    auto out = captureStdout([] {
        {
            XTracer0Scoped s("root");
            s.sub();
            s.sub("named");
        }
    });
    EXPECT_TRUE(contains(out, "root:"));
    EXPECT_TRUE(contains(out, "named:"));
}

TEST(XTracer0Scoped, MultiThread)
{
    auto& ctx = XTracer0Context::global();
    ctx.setEnabled(true);
    ctx.setLevel(-1);
    ctx.timerContext().setEnabled(true);
    ctx.timerContext().setLevel(-1);
    ctx.timerContext().setMode(XTimer0Context::Mode::Release);

    constexpr int kThreads = 4;
    std::vector<std::thread> threads;
    threads.reserve(kThreads);
    for (int i = 0; i < kThreads; ++i) {
        threads.emplace_back([&ctx] {
            XTracer0Scoped s("thread_op", &ctx);
            s.sub("sub");
            XTimer0::sleepFor(1);
        });
    }
    for (auto& t : threads)
        t.join();
    SUCCEED();
}

#endif  // ENABLE_TEST_XTRACER0
