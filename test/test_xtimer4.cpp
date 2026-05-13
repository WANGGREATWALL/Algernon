#if ENABLE_TEST_XTIMER4

#include <atomic>
#include <chrono>
#include <cstring>
#include <functional>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "gtest/gtest.h"
#include "log/xlogger.h"
#include "perf/xperf4_macros.h"
#include "perf/xtimer4.h"


namespace {

// ---------------------------------------------------------------------------
//  CapturingWriter 鈥?IPerfWriter4 implementation that buffers everything
//  in memory. Solves the v3 test reliance on stdout capture and lets us
//  assert exact content / call counts.
// ---------------------------------------------------------------------------

class CapturingWriter4 : public au::perf::IPerfWriter4
{
public:
    void write(const char* data, std::size_t size) noexcept override
    {
        std::lock_guard<std::mutex> lk(mMutex);
        mBuffer.append(data, size);
        ++mCallCount;
    }

    std::string drain()
    {
        std::lock_guard<std::mutex> lk(mMutex);
        std::string                 s = std::move(mBuffer);
        mBuffer.clear();
        return s;
    }

    int callCount() const
    {
        std::lock_guard<std::mutex> lk(mMutex);
        return mCallCount;
    }

    void reset()
    {
        std::lock_guard<std::mutex> lk(mMutex);
        mBuffer.clear();
        mCallCount = 0;
    }

private:
    mutable std::mutex mMutex;
    std::string        mBuffer;
    int                mCallCount{0};
};

}  // anonymous namespace


// ---------------------------------------------------------------------------
//  Fixture
// ---------------------------------------------------------------------------

class XTimer4Test : public ::testing::Test
{
protected:
    void SetUp() override
    {
        au::log::Config::get().setTag("XTimer4Test");
        au::log::Config::get().setLevel(au::log::Level::Verbose);
        au::log::Config::get().setColorEnabled(false);
#if AU_OS_ANDROID
        au::log::Config::get().setShellPrintEnabled(true);
#endif
        // Reset the default context to a deterministic baseline.
        auto& cfg = au::perf::XPerfContext4::defaultContext();
        cfg.setEnabled(true);
        cfg.setMode(au::perf::Mode4::Release);
        cfg.setTimerLevel(3);
        cfg.setTracerLevel(au::perf::kPerfLevelAll4);
        cfg.setMaxTreeDepth(64);
        cfg.setAggregateMode(false);
        cfg.setRootName("perf");
        cfg.setWriter(&mCapture);
        mCapture.reset();
    }

    void TearDown() override
    {
        // Restore default writer so a stray scope after teardown doesn't
        // dereference a destroyed CapturingWriter.
        au::perf::XPerfContext4::defaultContext().setWriter(nullptr);
    }

    static int countOccurrences(const std::string& s, const std::string& sub)
    {
        int         cnt = 0;
        std::size_t pos = s.find(sub);
        while (pos != std::string::npos) {
            ++cnt;
            pos = s.find(sub, pos + sub.size());
        }
        return cnt;
    }

    CapturingWriter4 mCapture;
};


// ===========================================================================
//  1. XPerfContext4 鈥?basic atomic accessors
// ===========================================================================

TEST_F(XTimer4Test, ContextBasicAccessors)
{
    auto& cfg = au::perf::XPerfContext4::defaultContext();

    cfg.setEnabled(false);
    EXPECT_FALSE(cfg.isEnabled());
    cfg.setEnabled(true);
    EXPECT_TRUE(cfg.isEnabled());

    cfg.setMode(au::perf::Mode4::Debug);
    EXPECT_EQ(cfg.getMode(), au::perf::Mode4::Debug);

    cfg.setTimerLevel(7);
    EXPECT_EQ(cfg.getTimerLevel(), 7);
    cfg.setTimerLevel(au::perf::kPerfLevelOff4);
    EXPECT_EQ(cfg.getTimerLevel(), au::perf::kPerfLevelOff4);

    cfg.setTracerLevel(0);
    EXPECT_EQ(cfg.getTracerLevel(), 0);

    cfg.setMaxTreeDepth(8);
    EXPECT_EQ(cfg.getMaxTreeDepth(), 8u);

    cfg.setRootName("AlgoRoot");
    char name[64] = {0};
    cfg.getRootName(name, sizeof(name));
    EXPECT_STREQ(name, "AlgoRoot");

    cfg.setAggregateMode(true);
    EXPECT_TRUE(cfg.isAggregateMode());
    cfg.setAggregateMode(false);
    EXPECT_FALSE(cfg.isAggregateMode());
}


// ===========================================================================
//  2. Multiple independent contexts
// ===========================================================================

TEST_F(XTimer4Test, MultipleContextsIndependent)
{
    au::perf::XPerfContext4 a;
    au::perf::XPerfContext4 b;

    a.setTimerLevel(1);
    b.setTimerLevel(99);

    EXPECT_EQ(a.getTimerLevel(), 1);
    EXPECT_EQ(b.getTimerLevel(), 99);

    au::perf::XPerfContext4::defaultContext().setTimerLevel(5);
    EXPECT_EQ(a.getTimerLevel(), 1);
    EXPECT_EQ(b.getTimerLevel(), 99);
    EXPECT_EQ(au::perf::XPerfContext4::defaultContext().getTimerLevel(), 5);
}


// ===========================================================================
//  3. RootName 鈥?size-aware setter + null tolerance
// ===========================================================================

TEST_F(XTimer4Test, RootNameSizeAwareAndNullTolerant)
{
    auto& cfg = au::perf::XPerfContext4::defaultContext();

    // size-aware setter
    const char* literal = "literal-root";
    cfg.setRootName(literal, std::strlen(literal));
    char buf[64] = {0};
    cfg.getRootName(buf, sizeof(buf));
    EXPECT_STREQ(buf, "literal-root");

    // null pointer is tolerated and produces an empty name
    cfg.setRootName(nullptr);
    std::memset(buf, 0xCC, sizeof(buf));
    cfg.getRootName(buf, sizeof(buf));
    EXPECT_STREQ(buf, "");

    // very long name is truncated to fit (63 chars + NUL)
    std::string huge(200, 'x');
    cfg.setRootName(huge.c_str(), huge.size());
    cfg.getRootName(buf, sizeof(buf));
    EXPECT_LE(std::strlen(buf), sizeof(buf) - 1);
    EXPECT_GT(std::strlen(buf), 0u);
}


// ===========================================================================
//  4. XTimer4 鈥?elapsed / restart / sleepFor non-positive
// ===========================================================================

TEST_F(XTimer4Test, TimerElapsed)
{
    au::perf::XTimer4 t;
    au::perf::XTimer4::sleepFor(10);
    const float ms = t.elapsedMs();
    EXPECT_GT(ms, 5.0f);
    EXPECT_LT(ms, 500.0f) << "elapsedMs=" << ms;
}


TEST_F(XTimer4Test, TimerSleepNonPositiveNoOp)
{
    au::perf::XTimer4 t;
    au::perf::XTimer4::sleepFor(0);
    au::perf::XTimer4::sleepFor(-5);
    EXPECT_LT(t.elapsedMs(), 50.0f);
}


TEST_F(XTimer4Test, TimerGetTimeFormatted)
{
    const std::string s = au::perf::XTimer4::getTimeFormatted("%Y-%m-%d-%H-%M-%S");
    EXPECT_GE(s.size(), 21u);  // "YYYY-MM-DD-HH-MM-SS_<ms>"
    EXPECT_NE(s.find('_'), std::string::npos);
}


// ===========================================================================
//  5. Release-mode: one-liner per scope, no tree characters
// ===========================================================================

TEST_F(XTimer4Test, ReleaseModeOneLiner)
{
    auto& cfg = au::perf::XPerfContext4::defaultContext();
    cfg.setMode(au::perf::Mode4::Release);
    cfg.setTimerLevel(au::perf::kPerfLevelAll4);

    {
        au::perf::XTimer4Scoped s("rel.scope");
        au::perf::XTimer4::sleepFor(2);
    }

    const std::string out = mCapture.drain();
    EXPECT_NE(out.find("[perf4]"),    std::string::npos) << out;
    EXPECT_NE(out.find("rel.scope"),  std::string::npos) << out;
    EXPECT_NE(out.find("ms"),         std::string::npos) << out;
    EXPECT_EQ(out.find("|--"),        std::string::npos);
    EXPECT_EQ(out.find("`--"),        std::string::npos);
}


// ===========================================================================
//  6. Debug-mode: hierarchical tree with header + ASCII branches
// ===========================================================================

TEST_F(XTimer4Test, DebugModeTreeHierarchy)
{
    auto& cfg = au::perf::XPerfContext4::defaultContext();
    cfg.setMode(au::perf::Mode4::Debug);
    cfg.setTimerLevel(au::perf::kPerfLevelAll4);

    {
        au::perf::XTimer4Scoped root("root4");
        {
            au::perf::XTimer4Scoped a("childA4");
            {
                au::perf::XTimer4Scoped c("grand4");
                au::perf::XTimer4::sleepFor(1);
            }
        }
        {
            au::perf::XTimer4Scoped b("childB4");
            au::perf::XTimer4::sleepFor(1);
        }
    }

    const std::string out = mCapture.drain();
    EXPECT_NE(out.find("[perf4]"), std::string::npos) << out;
    EXPECT_NE(out.find("tid="),    std::string::npos);
    EXPECT_EQ(countOccurrences(out, "root4"),   1);
    EXPECT_EQ(countOccurrences(out, "childA4"), 1);
    EXPECT_EQ(countOccurrences(out, "childB4"), 1);
    EXPECT_EQ(countOccurrences(out, "grand4"),  1);
    EXPECT_NE(out.find("|--"), std::string::npos);
    EXPECT_NE(out.find("`--"), std::string::npos);
    // ASCII-only invariant: no UTF-8 box-drawing leakage
    EXPECT_EQ(out.find("\xE2"), std::string::npos) << "non-ASCII char detected";
}


// ===========================================================================
//  7. sub(name) / sub() phase transitions in Debug mode
// ===========================================================================

TEST_F(XTimer4Test, SubAndSubNamed)
{
    auto& cfg = au::perf::XPerfContext4::defaultContext();
    cfg.setMode(au::perf::Mode4::Debug);
    cfg.setTimerLevel(au::perf::kPerfLevelAll4);

    {
        au::perf::XTimer4Scoped root("pipeline4");
        root.sub("step1");
        au::perf::XTimer4::sleepFor(1);
        root.sub("step2");
        au::perf::XTimer4::sleepFor(1);
        root.sub();  // explicit close, no new sub
    }

    const std::string out = mCapture.drain();
    EXPECT_EQ(countOccurrences(out, "pipeline4"), 1);
    EXPECT_EQ(countOccurrences(out, "step1"),     1);
    EXPECT_EQ(countOccurrences(out, "step2"),     1);
    EXPECT_EQ(out.find("(open)"), std::string::npos) << "sub() must close open subs";
}


// ===========================================================================
//  8. Level filtering: kPerfLevelOff fully silences; threshold gates above
// ===========================================================================

TEST_F(XTimer4Test, LevelFiltering)
{
    auto& cfg = au::perf::XPerfContext4::defaultContext();
    cfg.setMode(au::perf::Mode4::Release);
    cfg.setTimerLevel(2);

    {
        au::perf::XTimer4Scoped s("visible4", 2);
    }
    EXPECT_NE(mCapture.drain().find("visible4"), std::string::npos);

    {
        au::perf::XTimer4Scoped s("hidden4", 3);
    }
    EXPECT_EQ(mCapture.drain().find("hidden4"), std::string::npos);

    cfg.setTimerLevel(au::perf::kPerfLevelOff4);
    {
        au::perf::XTimer4Scoped s("never4", 0);
    }
    EXPECT_EQ(mCapture.drain().find("never4"), std::string::npos);
}

// ===========================================================================
//  9. setEnabled(false) hard-off
// ===========================================================================

TEST_F(XTimer4Test, DisabledHardOff)
{
    auto& cfg = au::perf::XPerfContext4::defaultContext();
    cfg.setEnabled(false);
    cfg.setMode(au::perf::Mode4::Debug);
    cfg.setTimerLevel(au::perf::kPerfLevelAll4);

    {
        au::perf::XTimer4Scoped a("off4.a");
        au::perf::XTimer4Scoped b("off4.b");
    }
    EXPECT_TRUE(mCapture.drain().empty());
}


// ===========================================================================
//  10. MaxTreeDepth degrades over-deep nodes to one-liner
// ===========================================================================

TEST_F(XTimer4Test, MaxTreeDepthDegradesOverdeep)
{
    auto& cfg = au::perf::XPerfContext4::defaultContext();
    cfg.setMode(au::perf::Mode4::Debug);
    cfg.setTimerLevel(au::perf::kPerfLevelAll4);
    cfg.setMaxTreeDepth(3);

    {
        au::perf::XTimer4Scoped a("d0");
        au::perf::XTimer4Scoped b("d1");
        au::perf::XTimer4Scoped c("d2");
        au::perf::XTimer4Scoped d("d3");  // exceeds cap 鈫?degraded one-liner
        au::perf::XTimer4Scoped e("d4");  // also degraded
    }

    const std::string out = mCapture.drain();
    // All five names must still appear (degraded path still records)
    EXPECT_NE(out.find("d0"), std::string::npos);
    EXPECT_NE(out.find("d1"), std::string::npos);
    EXPECT_NE(out.find("d2"), std::string::npos);
    EXPECT_NE(out.find("d3"), std::string::npos);
    EXPECT_NE(out.find("d4"), std::string::npos);
}


// ===========================================================================
//  11. Multi-thread isolation: each thread one header, no interleave
// ===========================================================================

TEST_F(XTimer4Test, MultiThreadIsolation)
{
    auto& cfg = au::perf::XPerfContext4::defaultContext();
    cfg.setMode(au::perf::Mode4::Debug);
    cfg.setTimerLevel(au::perf::kPerfLevelAll4);
    cfg.setAggregateMode(false);

    constexpr int kThreads = 4;
    {
        std::vector<std::thread> ths;
        ths.reserve(kThreads);
        for (int i = 0; i < kThreads; ++i) {
            ths.emplace_back([] {
                au::perf::XTimer4Scoped root("rootMT4");
                for (int j = 0; j < 3; ++j) {
                    au::perf::XTimer4Scoped child("childMT4");
                    au::perf::XTimer4::sleepFor(1);
                }
            });
        }
        for (auto& t : ths) {
            t.join();
        }
    }

    const std::string out         = mCapture.drain();
    const int         headerCount = countOccurrences(out, "[perf4][tid=");
    EXPECT_EQ(headerCount, kThreads);
    EXPECT_EQ(countOccurrences(out, "rootMT4"), kThreads);
}


// ===========================================================================
//  12. Aggregate mode: silent until flushAggregated()
// ===========================================================================

TEST_F(XTimer4Test, AggregateModeIdempotentFlush)
{
    auto& cfg = au::perf::XPerfContext4::defaultContext();
    cfg.setMode(au::perf::Mode4::Debug);
    cfg.setTimerLevel(au::perf::kPerfLevelAll4);
    cfg.setAggregateMode(true);

    {
        std::thread worker([] {
            au::perf::XTimer4Scoped r("agg4.worker");
            au::perf::XTimer4::sleepFor(1);
        });
        worker.join();
        au::perf::XTimer4Scoped r("agg4.main");
    }

    const std::string outDuring = mCapture.drain();
    EXPECT_EQ(outDuring.find("agg4.worker"), std::string::npos);
    EXPECT_EQ(outDuring.find("agg4.main"),   std::string::npos);

    au::perf::XPerfContext4::flushAggregated();
    const std::string outFlush = mCapture.drain();
    EXPECT_NE(outFlush.find("agg4.worker"),     std::string::npos);
    EXPECT_NE(outFlush.find("agg4.main"),       std::string::npos);
    EXPECT_NE(outFlush.find("aggregate flush"), std::string::npos);

    // Second flush must be a no-op (idempotent).
    au::perf::XPerfContext4::flushAggregated();
    EXPECT_TRUE(mCapture.drain().empty());

    cfg.setAggregateMode(false);
}


// ===========================================================================
//  13. Exception safety: outermost flush still fires when stack unwinds
// ===========================================================================

TEST_F(XTimer4Test, ExceptionSafety)
{
    auto& cfg = au::perf::XPerfContext4::defaultContext();
    cfg.setMode(au::perf::Mode4::Debug);
    cfg.setTimerLevel(au::perf::kPerfLevelAll4);

    bool caught = false;
    try {
        au::perf::XTimer4Scoped root("will.throw4");
        au::perf::XTimer4Scoped inner("inner4");
        throw std::runtime_error("boom");
    } catch (const std::runtime_error& e) {
        caught = true;
        EXPECT_STREQ(e.what(), "boom");
    }
    EXPECT_TRUE(caught);

    const std::string out = mCapture.drain();
    EXPECT_NE(out.find("will.throw4"), std::string::npos);
    EXPECT_NE(out.find("inner4"),      std::string::npos);
}


// ===========================================================================
//  14. Per-library context isolation: private cfg uses its own root name
// ===========================================================================

TEST_F(XTimer4Test, PrivateContextIsolation)
{
    au::perf::XPerfContext4 lib;
    lib.setMode(au::perf::Mode4::Debug);
    lib.setTimerLevel(au::perf::kPerfLevelAll4);
    lib.setRootName("libIso4");

    CapturingWriter4 libCap;
    lib.setWriter(&libCap);

    {
        au::perf::XTimer4Scoped a(lib, "libIso4.work");
        {
            au::perf::XTimer4Scoped inner(lib, "libIso4.inner");
        }
    }

    const std::string outLib = libCap.drain();
    EXPECT_NE(outLib.find("libIso4"),       std::string::npos) << outLib;
    EXPECT_NE(outLib.find("libIso4.inner"), std::string::npos);

    // The default context's writer must NOT have received any output.
    EXPECT_TRUE(mCapture.drain().empty());
}


// ===========================================================================
//  15. Writer swapping: routes to whichever sink is current
// ===========================================================================

TEST_F(XTimer4Test, WriterSwap)
{
    CapturingWriter4 secondary;
    auto&            cfg = au::perf::XPerfContext4::defaultContext();
    cfg.setMode(au::perf::Mode4::Release);
    cfg.setTimerLevel(au::perf::kPerfLevelAll4);

    {
        au::perf::XTimer4Scoped s("first.scope");
    }
    EXPECT_NE(mCapture.drain().find("first.scope"), std::string::npos);

    cfg.setWriter(&secondary);
    {
        au::perf::XTimer4Scoped s("second.scope");
    }
    EXPECT_TRUE(mCapture.drain().empty());
    EXPECT_NE(secondary.drain().find("second.scope"), std::string::npos);

    cfg.setWriter(&mCapture);  // restore for any later tests
}


// ===========================================================================
//  16. Long-name truncation: name >= kMaxNameLen4 must be marked truncated
// ===========================================================================

TEST_F(XTimer4Test, LongNameTruncationMarker)
{
    auto& cfg = au::perf::XPerfContext4::defaultContext();
    cfg.setMode(au::perf::Mode4::Debug);
    cfg.setTimerLevel(au::perf::kPerfLevelAll4);

    // 1500 chars > arena cap of 1023 鈫?arena marks the node truncated.
    // The print-clip layer additionally guarantees the "(truncated)"
    // suffix is not swallowed by emitFormatted4()'s 512-byte cap.
    std::string longName(1500, 'A');
    {
        au::perf::XTimer4Scoped s(longName.c_str(), longName.size(), 0);
    }
    const std::string out = mCapture.drain();
    EXPECT_NE(out.find("(truncated)"), std::string::npos) << out;
    EXPECT_NE(out.find("AAAAAAAA"),    std::string::npos);
}


// ===========================================================================
//  17. Temporary-string name: must NOT dangle (regression vs v3 bug)
// ===========================================================================

TEST_F(XTimer4Test, TemporaryStringNameNoDangle)
{
    auto& cfg = au::perf::XPerfContext4::defaultContext();
    cfg.setMode(au::perf::Mode4::Release);
    cfg.setTimerLevel(au::perf::kPerfLevelAll4);

    // The temporary std::string is destroyed after the constructor returns.
    // v3 stored a string_view 鈫?use-after-free. v4 must copy the bytes.
    {
        au::perf::XTimer4Scoped s((std::string("temp.name.") + std::to_string(42)).c_str());
        au::perf::XTimer4::sleepFor(1);
    }

    const std::string out = mCapture.drain();
    EXPECT_NE(out.find("temp.name.42"), std::string::npos) << out;
}


// ===========================================================================
//  18. Convenience macros AU_TIMER4 / AU_TIMER4_L
// ===========================================================================

TEST_F(XTimer4Test, ConvenienceMacros)
{
    auto& cfg = au::perf::XPerfContext4::defaultContext();
    cfg.setMode(au::perf::Mode4::Release);
    cfg.setTimerLevel(au::perf::kPerfLevelAll4);

    {
        AU_TIMER4("macro4.basic");
        au::perf::XTimer4::sleepFor(1);
    }
    EXPECT_NE(mCapture.drain().find("macro4.basic"), std::string::npos);

    {
        AU_TIMER4_L("macro4.lvl", 1);
    }
    EXPECT_NE(mCapture.drain().find("macro4.lvl"), std::string::npos);

    // __COUNTER__ disambiguation: two macros on the same source line.
    { AU_TIMER4("same.A"); AU_TIMER4("same.B"); }
    const std::string out = mCapture.drain();
    EXPECT_NE(out.find("same.A"), std::string::npos);
    EXPECT_NE(out.find("same.B"), std::string::npos);
}


// ===========================================================================
//  19. Stress: 10k scopes 鈥?performance smoke + leak guard
// ===========================================================================

TEST_F(XTimer4Test, StressTenThousandScopes)
{
    auto& cfg = au::perf::XPerfContext4::defaultContext();
    cfg.setMode(au::perf::Mode4::Release);
    cfg.setTimerLevel(au::perf::kPerfLevelAll4);

    auto begin = std::chrono::steady_clock::now();
    for (int i = 0; i < 10000; ++i) {
        au::perf::XTimer4Scoped s("stress");
    }
    auto end = std::chrono::steady_clock::now();
    const double nsPerOp =
        std::chrono::duration<double, std::nano>(end - begin).count() / 10000.0;
    EXPECT_LT(nsPerOp, 50000.0) << "per scope = " << nsPerOp << " ns";
    mCapture.reset();  // discard output
}


// ===========================================================================
//  20. elapsedMs() can be sampled mid-scope without affecting the tree
// ===========================================================================

TEST_F(XTimer4Test, MidScopeElapsedMs)
{
    auto& cfg = au::perf::XPerfContext4::defaultContext();
    cfg.setMode(au::perf::Mode4::Release);
    cfg.setTimerLevel(au::perf::kPerfLevelAll4);

    au::perf::XTimer4Scoped s("mid.scope");
    au::perf::XTimer4::sleepFor(5);
    const float t1 = s.elapsedMs();
    au::perf::XTimer4::sleepFor(5);
    const float t2 = s.elapsedMs();

    EXPECT_GT(t1, 2.0f);
    EXPECT_GT(t2, t1);
}


#endif  // ENABLE_TEST_XTIMER4