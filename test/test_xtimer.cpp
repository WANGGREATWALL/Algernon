#if ENABLE_TEST_XTIMER

#include <atomic>
#include <chrono>
#include <functional>
#include <stdexcept>
#include <string>
#include <thread>

#include "gtest/gtest.h"
#include "log/xlogger.h"
#include "perf/xtimer.h"
#include "test_helpers.h"

using namespace au::perf;

// ============================================================================
//  Fixture
// ============================================================================

class XTimerScopedTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        au::log::Config::get().setTag("XTimer");
        au::log::Config::get().setLevel(au::log::Level::Info);
        au::log::Config::get().setColorEnabled(false);
#if AU_OS_ANDROID
        au::log::Config::get().setShellPrintEnabled(true);
#endif
        au::perf::setPerfVisibleLevel(static_cast<size_t>(-1));
        au::perf::setTimerRootName("test");
    }

    void TearDown() override
    {
        // Defensive cleanup: if any test leaked nodes (e.g. tests that
        // exercise scope lifecycle without captureStdout), silently drain
        // the tree so subsequent tests start from a clean slate.
        au::log::Config::get().setLevel(au::log::Level::Silent);
        {
            XTimerScoped drain("_cleanup_");
        }  // dtor -> joinLevelTo(0) -> showByDFS suppressed (Silent) -> tree cleared
        au::log::Config::get().setLevel(au::log::Level::Info);
    }
};

// ============================================================================
//  Case 1: XTimer_BasicAPI  (no fixture)
// ============================================================================

TEST(XTimer, BasicAPI)
{
    // --- Stopwatch ---
    XTimer t;
    XTimer::sleepFor(10);
    float ms = t.elapsed();
    EXPECT_GT(ms, 5.0f);
    EXPECT_LT(ms, 100.0f);

    // --- Restart ---
    t.restart();
    EXPECT_LT(t.elapsed(), 5.0f);
    XTimer::sleepFor(10);
    t.restart();
    EXPECT_LT(t.elapsed(), 5.0f);

    // --- SleepFor edge cases ---
    // Negative / zero duration: std::this_thread::sleep_for returns
    // immediately per the C++ standard -- safe no-ops.
    XTimer::sleepFor(0);
    XTimer::sleepFor(-1);
    XTimer::sleepFor(-100);

    // --- getTimeFormatted ---
    auto s1 = XTimer::getTimeFormatted();
    EXPECT_FALSE(s1.empty());
    EXPECT_GE(s1.size(), 10u);

    auto s2 = XTimer::getTimeFormatted("%H:%M:%S");
    EXPECT_TRUE(aura::test::contains(s2, ":"));
}

// ============================================================================
//  Case 2: XTimerScoped_GlobalConfig
// ============================================================================

TEST_F(XTimerScopedTest, GlobalConfig)
{
    // Default: show all levels
    EXPECT_EQ(au::perf::getPerfVisibleLevel(), static_cast<size_t>(-1));

    // Set / get round-trip
    au::perf::setPerfVisibleLevel(0);
    EXPECT_EQ(au::perf::getPerfVisibleLevel(), 0u);
    au::perf::setPerfVisibleLevel(5);
    EXPECT_EQ(au::perf::getPerfVisibleLevel(), 5u);
    au::perf::setPerfVisibleLevel(static_cast<size_t>(-1));
    EXPECT_EQ(au::perf::getPerfVisibleLevel(), static_cast<size_t>(-1));

    // setTimerRootName does not crash
    au::perf::setTimerRootName("MyRoot");
    SUCCEED();
}

// ============================================================================
//  Case 3: XTimerScoped_BasicBehavior
// ============================================================================

TEST_F(XTimerScopedTest, BasicBehavior)
{
    // --- Construct / destruct ---
    {
        XTimerScoped s("ctor_dtor");
    }

    // --- getLevel ---
    {
        XTimerScoped s("root");
        EXPECT_EQ(s.getLevel(), 1u);
    }

    // --- Mixed named / bare sub calls ---
    {
        XTimerScoped s("mixed");
        s.sub("A");
        s.sub("B");
        s.sub();
        s.sub("C");
        EXPECT_EQ(s.getLevel(), 1u);
    }

    // --- Consecutive bare subs (safe no-ops) ---
    {
        XTimerScoped s("bare");
        s.sub();
        s.sub();
    }

    // --- Bare sub immediately after construction ---
    {
        XTimerScoped s("bare_first");
        s.sub();  // no open sub -> safe no-op
    }

    // --- Empty name ---
    {
        XTimerScoped s("");
        s.sub("");
    }

    // --- Sequential scopes ---
    for (int i = 0; i < 5; ++i) {
        XTimerScoped s("seq");
        s.sub("phase");
        s.sub();
    }

    SUCCEED();
}

// ============================================================================
//  Case 4: XTimerScoped_TreeOutput
// ============================================================================

TEST_F(XTimerScopedTest, TreeOutput)
{
    // --- Single scope output format ---
    {
        auto out = aura::test::captureStdout([] {
            XTimerScoped root("root");
            XTimer::sleepFor(1);
        });
        EXPECT_TRUE(aura::test::contains(out, "[XTimer][I]"));
        EXPECT_TRUE(aura::test::contains(out, "test:"));
        EXPECT_TRUE(aura::test::contains(out, "|--root:"));
        EXPECT_TRUE(aura::test::contains(out, " ms"));
    }

    // --- Two child nodes: indentation ---
    {
        auto out = aura::test::captureStdout([] {
            XTimerScoped root("root");
            root.sub("A");
            XTimer::sleepFor(1);
            root.sub("B");
            XTimer::sleepFor(1);
        });
        EXPECT_TRUE(aura::test::contains(out, "|--root:"));
        EXPECT_TRUE(aura::test::contains(out, "|  |--A:"));
        EXPECT_TRUE(aura::test::contains(out, "|  |--B:"));
        EXPECT_LT(out.find("A:"), out.find("B:"));
    }

    // --- Nested scopes: indentation ---
    {
        auto out = aura::test::captureStdout([] {
            XTimerScoped outer("outer");
            XTimer::sleepFor(1);
            {
                XTimerScoped inner("inner");
                XTimer::sleepFor(1);
            }
        });
        EXPECT_TRUE(aura::test::contains(out, "|--outer:"));
        EXPECT_TRUE(aura::test::contains(out, "|  |--inner:"));
        EXPECT_LT(out.find("outer:"), out.find("inner:"));
    }

    // --- 10-level deep nesting ---
    {
        auto out = aura::test::captureStdout([] {
            std::function<void(int)> nest = [&](int d) {
                if (d >= 10) return;
                XTimerScoped s(("L" + std::to_string(d)).c_str());
                XTimer::sleepFor(0);
                nest(d + 1);
            };
            XTimerScoped root("deep10_root");
            nest(0);
        });
        for (int i = 0; i < 10; ++i)
            EXPECT_TRUE(aura::test::contains(out, "L" + std::to_string(i) + ":"));
        EXPECT_TRUE(aura::test::contains(out,
            "|  |  |  |  |  |  |  |  |  |--L9:"));
    }

    // --- 50-level deep nesting (DFS recursion stress) ---
    {
        auto out = aura::test::captureStdout([] {
            std::function<void(int)> nest = [&](int d) {
                if (d >= 50) return;
                XTimerScoped s(("N" + std::to_string(d)).c_str());
                XTimer::sleepFor(0);
                nest(d + 1);
            };
            XTimerScoped root("deep_root");
            nest(0);
        });
        EXPECT_TRUE(aura::test::contains(out, "deep_root"));
        EXPECT_TRUE(aura::test::contains(out, "N49:"));
        EXPECT_TRUE(aura::test::contains(out, "|--"));
    }

    // --- Wide tree (many sibling subs) ---
    {
        auto out = aura::test::captureStdout([] {
            XTimerScoped root("wide");
            for (int i = 0; i < 20; ++i) {
                root.sub(("sub" + std::to_string(i)).c_str());
                XTimer::sleepFor(0);
            }
        });
        for (int i = 0; i < 20; ++i)
            EXPECT_TRUE(aura::test::contains(out,
                "|  |--sub" + std::to_string(i) + ":"));
    }
}

// ============================================================================
//  Case 5: XTimerScoped_ColorOutput
// ============================================================================

TEST_F(XTimerScopedTest, ColorOutput)
{
    // --- Color enabled: ANSI escape codes present ---
    au::log::Config::get().setColorEnabled(true);
    {
        auto out = aura::test::captureStdout([] {
            XTimerScoped root("color_test");
            root.sub("phase");
            XTimer::sleepFor(1);
        });
        EXPECT_TRUE(aura::test::contains(out, "\033[7;32m[I]\033[0m"));
        int lines  = aura::test::countOccurrences(out, "\n");
        int resets = aura::test::countOccurrences(out, "\033[0m");
        EXPECT_GE(resets, lines);
        EXPECT_TRUE(aura::test::contains(out, "|--color_test:"));
    }

    // --- Color disabled: no ANSI escape sequences ---
    au::log::Config::get().setColorEnabled(false);
    {
        auto out = aura::test::captureStdout([] {
            XTimerScoped root("nocolor");
            XTimer::sleepFor(1);
        });
        EXPECT_FALSE(aura::test::contains(out, "\033["));
        EXPECT_TRUE(aura::test::contains(out, "[XTimer][I]"));
        EXPECT_TRUE(aura::test::contains(out, "nocolor:"));
    }
}

// ============================================================================
//  Case 6: XTimerScoped_ExceptionSafety
// ============================================================================

TEST_F(XTimerScopedTest, ExceptionSafety)
{
    // --- Exception thrown inside scope: RAII dtor still runs ---
    {
        auto out = aura::test::captureStdout([] {
            try {
                XTimerScoped root("try_block");
                root.sub("before_throw");
                XTimer::sleepFor(1);
                throw std::runtime_error("test");
            } catch (const std::exception&) {
            }
        });
        EXPECT_TRUE(aura::test::contains(out, "try_block:"));
    }

    // --- Exception between sub calls: tree still usable after catch ---
    {
        auto out = aura::test::captureStdout([] {
            try {
                XTimerScoped root("mid_sub");
                root.sub("A");
                throw std::runtime_error("mid");
            } catch (const std::exception&) {
            }
        });
        EXPECT_TRUE(aura::test::contains(out, "mid_sub:"));
    }

    // --- Unbalanced sub calls are safe ---
    {
        auto out = aura::test::captureStdout([] {
            XTimerScoped root("unbalanced");
            root.sub("A");
            root.sub();    // closes A
            root.sub();    // extra bare sub: safe no-op
            root.sub("B"); // new sub works normally
            XTimer::sleepFor(1);
        });
        EXPECT_TRUE(aura::test::contains(out, "A:"));
        EXPECT_TRUE(aura::test::contains(out, "B:"));
    }

    // --- Bare sub with no prior named sub: safe no-op ---
    {
        XTimerScoped root("empty_subs");
        root.sub();
        root.sub();
        root.sub("after_empty");
        SUCCEED();
    }
}

// ============================================================================
//  Case 7: XTimerScoped_CorruptionDefense
// ============================================================================

TEST_F(XTimerScopedTest, CorruptionDefense)
{
    // --- Name with embedded newline ---
    {
        auto out = aura::test::captureStdout([] {
            XTimerScoped root("line1\nline2");
            XTimer::sleepFor(1);
        });
        EXPECT_TRUE(aura::test::contains(out, "[XTimer][I]"));
    }

    // --- Name with printf format specifiers ---
    {
        auto out = aura::test::captureStdout([] {
            XTimerScoped root("%s%d%p_test");
            XTimer::sleepFor(1);
        });
        // Name rendered literally -- no format-string vulnerability.
        EXPECT_TRUE(aura::test::contains(out, "%s%d%p_test"));
    }

    // --- Name with ANSI escape codes ---
    {
        XTimerScoped root("\033[31mRED\033[0m");
        root.sub("\033[34mBLUE\033[0m");
        SUCCEED();
    }

    // --- Very long name (2000 chars) ---
    {
        std::string longName(2000, 'x');
        auto        out = aura::test::captureStdout([&] {
            XTimerScoped root(longName);
            XTimer::sleepFor(1);
        });
        EXPECT_TRUE(aura::test::contains(out, "xxx"));
    }

    // --- Special characters: tab, backslash ---
    {
        XTimerScoped root("tab\there\\path");
        root.sub("back\\slash\tagain");
        SUCCEED();
    }

    // --- Unicode name (UTF-8) ---
    {
        auto out = aura::test::captureStdout([] {
            XTimerScoped root("test_utf8_\xc3\xa4\xc3\xb6");  // "test_utf8_äö"
            XTimer::sleepFor(1);
        });
        EXPECT_TRUE(aura::test::contains(out, "test_utf8_"));
    }

    SUCCEED();
}

// ============================================================================
//  Case 8: XTimerScoped_Stress
// ============================================================================

TEST_F(XTimerScopedTest, Stress)
{
    // Suppress log output during stress loops so XLOG_I inside each scope
    // destruction does not flood stdout or cause O(N) slowdown.
    // The scope lifecycle (addNode / joinLevelTo / tree clear) is still
    // exercised -- only the final fwrite in XLOG_I is skipped.

    // --- 10k rapid create/destroy ---
    {
        au::log::Config::get().setLevel(au::log::Level::Silent);
        auto begin = std::chrono::steady_clock::now();
        for (int i = 0; i < 10000; ++i) {
            XTimerScoped s("iter");
        }
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - begin).count();
        au::log::Config::get().setLevel(au::log::Level::Info);
        EXPECT_LT(elapsed, 500);
    }

    // --- 100k create/destroy (memory pressure) ---
    // ASAN: cmake -DENABLE_ASAN=ON to verify no leaks.
    // TSAN: cmake -DENABLE_TSAN=ON to verify no data races.
    {
        au::log::Config::get().setLevel(au::log::Level::Silent);
        for (int i = 0; i < 100000; ++i) {
            XTimerScoped s("memtest");
        }
        au::log::Config::get().setLevel(au::log::Level::Info);
        SUCCEED();
    }

    // --- Mixed creation patterns ---
    {
        au::log::Config::get().setLevel(au::log::Level::Silent);
        for (int i = 0; i < 1000; ++i) {
            { XTimerScoped s("flat"); }
            { XTimerScoped s("subs"); s.sub("a"); s.sub(); }
            { XTimerScoped o("outer"); { XTimerScoped in("inner"); } }
        }
        au::log::Config::get().setLevel(au::log::Level::Info);
    }

    // --- Single scope with many subs (wide tree) ---
    {
        au::log::Config::get().setLevel(au::log::Level::Silent);
        XTimerScoped root("many_subs");
        for (int i = 0; i < 1000; ++i) {
            root.sub(("s" + std::to_string(i)).c_str());
        }
        for (int i = 0; i < 1000; ++i) {
            root.sub();
        }
        au::log::Config::get().setLevel(au::log::Level::Info);
    }

    // --- Cross-thread safety ---
    // TimerTree records the constructing thread's id. Scopes on other
    // threads are silently ignored (inThisThread() returns false -> no-op).
    {
        std::atomic<int> t1Started{0};
        std::atomic<int> t1Done{0};

        std::thread t([&] {
            t1Started.store(1, std::memory_order_release);
            for (int i = 0; i < 1000; ++i) {
                XTimerScoped s("other_thread");
                s.sub("sub");
            }
            t1Done.store(1, std::memory_order_release);
        });

        while (t1Started.load(std::memory_order_acquire) == 0) {
            std::this_thread::yield();
        }

        auto out = aura::test::captureStdout([] {
            XTimerScoped root("main_thread");
            XTimer::sleepFor(1);
        });

        while (t1Done.load(std::memory_order_acquire) == 0) {
            std::this_thread::yield();
        }
        t.join();

        EXPECT_TRUE(aura::test::contains(out, "main_thread:"));
    }

    SUCCEED();
}

#endif  // ENABLE_TEST_XTIMER
