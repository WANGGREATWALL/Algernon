#if ENABLE_TEST_XTIMER0

#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "gtest/gtest.h"
#include "perf/xtimer0.h"

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
// 1. XTimer0
// ============================================================================

TEST(XTimer0, Elapsed)
{
    XTimer0 t;
    XTimer0::sleepFor(10);
    float ms = t.elapsed();
    EXPECT_GT(ms, 5.f);
    EXPECT_LT(ms, 100.f);
}

TEST(XTimer0, Restart)
{
    XTimer0 t;
    XTimer0::sleepFor(10);
    t.restart();
    float ms = t.elapsed();
    EXPECT_LT(ms, 5.f);
}

TEST(XTimer0, TimeFormatted)
{
    auto s = XTimer0::getTimeFormatted();
    EXPECT_FALSE(s.empty());
    EXPECT_GE(s.size(), 10u);
}

// ============================================================================
// 2. XTimer0Context
// ============================================================================

TEST(XTimer0Context, GlobalDefaults)
{
    auto& ctx = XTimer0Context::global();
    EXPECT_FALSE(ctx.isEnabled());
    EXPECT_EQ(ctx.getLevel(), -1);
    EXPECT_EQ(ctx.getMode(), XTimer0Context::Mode::Release);
}

TEST(XTimer0Context, SetEnabled)
{
    auto& ctx = XTimer0Context::global();
    ctx.setEnabled(true);
    EXPECT_TRUE(ctx.isEnabled());
    ctx.setEnabled(false);
    EXPECT_FALSE(ctx.isEnabled());
}

TEST(XTimer0Context, SetLevel)
{
    auto& ctx = XTimer0Context::global();
    ctx.setLevel(3);
    EXPECT_EQ(ctx.getLevel(), 3);
    ctx.setLevel(-1);
    EXPECT_EQ(ctx.getLevel(), -1);
}

TEST(XTimer0Context, SetMode)
{
    auto& ctx = XTimer0Context::global();
    ctx.setMode(XTimer0Context::Mode::Debug);
    EXPECT_EQ(ctx.getMode(), XTimer0Context::Mode::Debug);
    ctx.setMode(XTimer0Context::Mode::Release);
    EXPECT_EQ(ctx.getMode(), XTimer0Context::Mode::Release);
}

TEST(XTimer0Context, ShouldPrint)
{
    auto& ctx = XTimer0Context::global();
    ctx.setEnabled(false);
    EXPECT_FALSE(ctx.shouldPrint(0));
    EXPECT_FALSE(ctx.shouldPrint(5));

    ctx.setEnabled(true);
    ctx.setLevel(2);
    EXPECT_TRUE(ctx.shouldPrint(0));
    EXPECT_TRUE(ctx.shouldPrint(2));
    EXPECT_FALSE(ctx.shouldPrint(3));

    ctx.setLevel(-1);
    EXPECT_TRUE(ctx.shouldPrint(100));
}

TEST(XTimer0Context, NamedContexts)
{
    auto& a = XTimer0Context::get("test_lib_a");
    auto& b = XTimer0Context::get("test_lib_b");
    auto& a2 = XTimer0Context::get("test_lib_a");

    EXPECT_EQ(&a, &a2);
    EXPECT_NE(&a, &b);

    a.setEnabled(true);
    a.setLevel(3);
    b.setEnabled(false);

    EXPECT_TRUE(a.isEnabled());
    EXPECT_FALSE(b.isEnabled());
    EXPECT_EQ(a.getLevel(), 3);
}

TEST(XTimer0Context, ConfigureFrom)
{
    auto& ctx = XTimer0Context::get("configure_test");

    // Reset to known state
    ctx.setEnabled(false);
    ctx.setLevel(-1);
    ctx.setMode(XTimer0Context::Mode::Release);

    // configureFrom with no matching env vars should keep defaults
    ctx.configureFrom("nonexistent.prefix.xyz");
    EXPECT_FALSE(ctx.isEnabled());  // still off
}

// ============================================================================
// 3. XTimer0Scoped — Release mode
// ============================================================================

TEST(XTimer0Scoped, ReleaseDisabled)
{
    auto& ctx = XTimer0Context::global();
    ctx.setEnabled(false);
    ctx.setMode(XTimer0Context::Mode::Release);

    auto out = captureStdout([] {
        XTimer0Scoped s("noop");
        XTimer0::sleepFor(5);
    });
    EXPECT_TRUE(out.empty());
}

TEST(XTimer0Scoped, ReleasePrints)
{
    auto& ctx = XTimer0Context::global();
    ctx.setEnabled(true);
    ctx.setLevel(-1);
    ctx.setMode(XTimer0Context::Mode::Release);

    auto out = captureStdout([] {
        XTimer0Scoped s("myOp");
        XTimer0::sleepFor(5);
    });
    EXPECT_TRUE(contains(out, "myOp:"));
    EXPECT_TRUE(contains(out, "ms"));
}

TEST(XTimer0Scoped, ReleaseLevelFiltered)
{
    auto& ctx = XTimer0Context::global();
    ctx.setEnabled(true);
    ctx.setLevel(0);  // show nothing
    ctx.setMode(XTimer0Context::Mode::Release);

    auto out = captureStdout([] {
        XTimer0Scoped s("filtered");
        XTimer0::sleepFor(5);
    });
    EXPECT_TRUE(out.empty());
}

TEST(XTimer0Scoped, ReleaseWithSubs)
{
    auto& ctx = XTimer0Context::global();
    ctx.setEnabled(true);
    ctx.setLevel(-1);
    ctx.setMode(XTimer0Context::Mode::Release);

    auto out = captureStdout([] {
        XTimer0Scoped s("root");
        XTimer0::sleepFor(2);
        s.sub("phase1");
        XTimer0::sleepFor(2);
        s.sub("phase2");
        XTimer0::sleepFor(2);
    });
    // Release mode: each segment printed as "<name>: X ms"
    EXPECT_TRUE(contains(out, "root:"));
    EXPECT_TRUE(contains(out, "phase1:"));
    EXPECT_TRUE(contains(out, "phase2:"));
}

TEST(XTimer0Scoped, ReleaseBareSub)
{
    auto& ctx = XTimer0Context::global();
    ctx.setEnabled(true);
    ctx.setLevel(-1);
    ctx.setMode(XTimer0Context::Mode::Release);

    auto out = captureStdout([] {
        XTimer0Scoped s("test");
        s.sub();  // bare sub — transitions to named sub
        s.sub("named");
        XTimer0::sleepFor(1);
    });
    EXPECT_TRUE(contains(out, "test:"));
    EXPECT_TRUE(contains(out, "named:"));
}

// ============================================================================
// 4. XTimer0Scoped — Debug mode
// ============================================================================

TEST(XTimer0Scoped, DebugDisabled)
{
    auto& ctx = XTimer0Context::global();
    ctx.setEnabled(false);
    ctx.setMode(XTimer0Context::Mode::Debug);

    auto out = captureStdout([] {
        XTimer0Scoped s("noop");
        XTimer0::sleepFor(5);
    });
    EXPECT_TRUE(out.empty());
}

TEST(XTimer0Scoped, DebugTreeOutput)
{
    auto& ctx = XTimer0Context::global();
    ctx.setEnabled(true);
    ctx.setLevel(-1);
    ctx.setMode(XTimer0Context::Mode::Debug);

    auto out = captureStdout([] {
        {
            XTimer0Scoped s("root");
            XTimer0::sleepFor(2);
            s.sub("phase1");
            XTimer0::sleepFor(2);
            s.sub("phase2");
            XTimer0::sleepFor(2);
        }
    });
    // Tree output: root at top, phase1/phase2 as children with indent
    EXPECT_TRUE(contains(out, "root:"));
    EXPECT_TRUE(contains(out, "phase1:"));
    EXPECT_TRUE(contains(out, "phase2:"));
    // Check tree indentation (|-- prefix for children)
    EXPECT_TRUE(contains(out, "|--"));
}

TEST(XTimer0Scoped, DebugLevelFilter)
{
    auto& ctx = XTimer0Context::global();
    ctx.setEnabled(true);
    ctx.setLevel(1);  // only show level 1 (top-level scopes)
    ctx.setMode(XTimer0Context::Mode::Debug);

    auto out = captureStdout([] {
        {
            XTimer0Scoped s("root");
            s.sub("hidden");
            XTimer0::sleepFor(1);
        }
    });
    EXPECT_TRUE(contains(out, "root:"));
    EXPECT_FALSE(contains(out, "hidden:"));
}

TEST(XTimer0Scoped, DebugNestedScopes)
{
    auto& ctx = XTimer0Context::global();
    ctx.setEnabled(true);
    ctx.setLevel(-1);
    ctx.setMode(XTimer0Context::Mode::Debug);

    auto out = captureStdout([] {
        {
            XTimer0Scoped outer("outer");
            XTimer0::sleepFor(1);
            {
                XTimer0Scoped inner("inner");
                XTimer0::sleepFor(1);
            }
        }
    });
    // inner should be indented under outer
    auto outerPos = out.find("outer:");
    auto innerPos = out.find("inner:");
    EXPECT_NE(outerPos, std::string::npos);
    EXPECT_NE(innerPos, std::string::npos);
    // inner should appear after outer
    EXPECT_LT(outerPos, innerPos);
}

// ============================================================================
// 5. Thread safety
// ============================================================================

TEST(XTimer0Scoped, MultiThreadNoCrash)
{
    auto& ctx = XTimer0Context::global();
    ctx.setEnabled(true);
    ctx.setLevel(-1);
    ctx.setMode(XTimer0Context::Mode::Release);

    constexpr int kThreads = 4;
    constexpr int kIter    = 20;

    std::vector<std::thread> threads;
    threads.reserve(kThreads);
    for (int i = 0; i < kThreads; ++i) {
        threads.emplace_back([i, &ctx] {
            for (int j = 0; j < kIter; ++j) {
                XTimer0Scoped s("thread_op");
                XTimer0::sleepFor(1);
                s.sub("sub_op");
                XTimer0::sleepFor(1);
            }
        });
    }
    for (auto& t : threads)
        t.join();
    // No crash = success
    SUCCEED();
}

TEST(XTimer0Scoped, MultiThreadDebugTrees)
{
    auto& ctx = XTimer0Context::global();
    ctx.setEnabled(true);
    ctx.setLevel(-1);
    ctx.setMode(XTimer0Context::Mode::Debug);

    constexpr int kThreads = 4;
    std::vector<std::thread> threads;
    threads.reserve(kThreads);
    for (int i = 0; i < kThreads; ++i) {
        threads.emplace_back([i, &ctx] {
            XTimer0Scoped s("thread_root");
            XTimer0::sleepFor(1);
            s.sub("child");
            XTimer0::sleepFor(1);
        });
    }
    for (auto& t : threads)
        t.join();
    SUCCEED();
}

// ============================================================================
// 6. Edge cases
// ============================================================================

TEST(XTimer0Scoped, NullContext)
{
    // Passing nullptr should fall back to global context
    XTimer0Context::global().setEnabled(true);
    XTimer0Context::global().setLevel(-1);
    XTimer0Context::global().setMode(XTimer0Context::Mode::Release);

    auto out = captureStdout([] {
        XTimer0Scoped s("using_global", nullptr);
        XTimer0::sleepFor(1);
    });
    EXPECT_TRUE(contains(out, "using_global:"));
}

TEST(XTimer0Scoped, RapidCreateDestroy)
{
    auto& ctx = XTimer0Context::global();
    ctx.setEnabled(true);
    ctx.setLevel(-1);
    ctx.setMode(XTimer0Context::Mode::Debug);

    for (int i = 0; i < 100; ++i) {
        XTimer0Scoped s("rapid");
    }
    SUCCEED();
}

TEST(XTimer0Scoped, EmptyName)
{
    auto& ctx = XTimer0Context::global();
    ctx.setEnabled(true);
    ctx.setLevel(-1);
    ctx.setMode(XTimer0Context::Mode::Release);

    auto out = captureStdout([] {
        XTimer0Scoped s("");
    });
    // Should not crash with empty name
    EXPECT_TRUE(contains(out, ":"));
}

TEST(XTimer0Scoped, ElapsedDuringScope)
{
    auto& ctx = XTimer0Context::global();
    ctx.setEnabled(true);

    XTimer0Scoped s("test");
    XTimer0::sleepFor(10);
    float mid = s.elapsed();
    EXPECT_GT(mid, 5.f);
    XTimer0::sleepFor(10);
    float end = s.elapsed();
    EXPECT_GT(end, mid);
}

TEST(XTimer0Scoped, SubWithoutNameThenNamed)
{
    auto& ctx = XTimer0Context::global();
    ctx.setEnabled(true);
    ctx.setLevel(-1);
    ctx.setMode(XTimer0Context::Mode::Debug);

    auto out = captureStdout([] {
        {
            XTimer0Scoped s("root");
            s.sub();          // bare sub
            s.sub("named");   // named sub after bare
            XTimer0::sleepFor(1);
        }
    });
    EXPECT_TRUE(contains(out, "root:"));
    EXPECT_TRUE(contains(out, "named:"));
}

TEST(XTimer0Scoped, MultipleSubWithoutName)
{
    auto& ctx = XTimer0Context::global();
    ctx.setEnabled(true);
    ctx.setLevel(-1);
    ctx.setMode(XTimer0Context::Mode::Debug);

    // Multiple bare sub() calls should not crash
    auto out = captureStdout([] {
        {
            XTimer0Scoped s("root");
            s.sub();
            s.sub();
            s.sub("final");
        }
    });
    EXPECT_TRUE(contains(out, "root:"));
    EXPECT_TRUE(contains(out, "final:"));
}

#endif  // ENABLE_TEST_XTIMER0
