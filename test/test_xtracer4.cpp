#if ENABLE_TEST_XTRACER4

#include <atomic>
#include <chrono>
#include <cstring>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "gtest/gtest.h"
#include "log/xlogger.h"
#include "perf/xperf4_macros.h"
#include "perf/xtimer4.h"
#include "perf/xtracer4.h"


namespace {

// ---------------------------------------------------------------------------
//  CapturingWriter4 鈥?sink for the XTimer4-side companion checks.
//
//  XTracer4 writes to the kernel trace_marker (Android) or no-ops
//  (host). There is no userspace IPerfWriter equivalent, so most
//  assertions verify state machine behaviour and Timer/Tracer co-operation.
// ---------------------------------------------------------------------------

class CapturingWriter4 : public au::perf::IPerfWriter4
{
public:
    void write(const char* data, std::size_t size) noexcept override
    {
        std::lock_guard<std::mutex> lk(mMutex);
        mBuffer.append(data, size);
    }

    std::string drain()
    {
        std::lock_guard<std::mutex> lk(mMutex);
        std::string                 s = std::move(mBuffer);
        mBuffer.clear();
        return s;
    }

    void reset()
    {
        std::lock_guard<std::mutex> lk(mMutex);
        mBuffer.clear();
    }

private:
    std::mutex  mMutex;
    std::string mBuffer;
};

}  // anonymous namespace


class XTracer4Test : public ::testing::Test
{
protected:
    void SetUp() override
    {
        au::log::Config::get().setTag("XTracer4Test");
        au::log::Config::get().setLevel(au::log::Level::Verbose);
        au::log::Config::get().setColorEnabled(false);
#if AU_OS_ANDROID
        au::log::Config::get().setShellPrintEnabled(true);
#endif

        auto& cfg = au::perf::XPerfContext4::defaultContext();
        cfg.setEnabled(true);
        cfg.setMode(au::perf::Mode4::Release);
        cfg.setTimerLevel(au::perf::kPerfLevelAll4);
        cfg.setTracerLevel(au::perf::kPerfLevelAll4);
        cfg.setMaxTreeDepth(64);
        cfg.setAggregateMode(false);
        cfg.setRootName("perf");
        cfg.setWriter(&mCapture);
        mCapture.reset();
    }

    void TearDown() override
    {
        au::perf::XPerfContext4::defaultContext().setWriter(nullptr);
    }

    CapturingWriter4 mCapture;
};


// ===========================================================================
//  1. TracerLevel atomic accessor
// ===========================================================================

TEST_F(XTracer4Test, ContextTracerLevelAccessor)
{
    auto& cfg = au::perf::XPerfContext4::defaultContext();

    cfg.setTracerLevel(0);
    EXPECT_EQ(cfg.getTracerLevel(), 0);

    cfg.setTracerLevel(5);
    EXPECT_EQ(cfg.getTracerLevel(), 5);

    cfg.setTracerLevel(au::perf::kPerfLevelOff4);
    EXPECT_EQ(cfg.getTracerLevel(), au::perf::kPerfLevelOff4);

    cfg.setTracerLevel(au::perf::kPerfLevelAll4);
    EXPECT_EQ(cfg.getTracerLevel(), au::perf::kPerfLevelAll4);
}


// ===========================================================================
//  2. Construct / destruct does not crash on default ctx
// ===========================================================================

TEST_F(XTracer4Test, BasicConstructDestruct)
{
    {
        au::perf::XTracer4Scoped t("tracer4.basic");
        au::perf::XTimer4::sleepFor(1);
    }
    SUCCEED();
}


// ===========================================================================
//  3. Size-aware constructor avoids strlen on caller side
// ===========================================================================

TEST_F(XTracer4Test, SizeAwareConstructor)
{
    const char        text[] = "tracer4.sized.label";
    const std::size_t len    = sizeof(text) - 1;
    {
        au::perf::XTracer4Scoped t(text, len);
        au::perf::XTimer4::sleepFor(1);
    }
    SUCCEED();
}


// ===========================================================================
//  4. nullptr name is tolerated
// ===========================================================================

TEST_F(XTracer4Test, NullNameTolerated)
{
    {
        au::perf::XTracer4Scoped t(static_cast<const char*>(nullptr));
    }
    {
        au::perf::XTracer4Scoped t(nullptr, static_cast<std::size_t>(0));
    }
    SUCCEED();
}


// ===========================================================================
//  5. Long name (>128) is silently truncated, never overruns
// ===========================================================================

TEST_F(XTracer4Test, LongNameTruncatedNoOverflow)
{
    const std::string huge(8 * 1024, 'Z');
    {
        au::perf::XTracer4Scoped t(huge.c_str(), huge.size());
    }
    SUCCEED();
}


// ===========================================================================
//  6. Per-context tracer-level gate
// ===========================================================================

TEST_F(XTracer4Test, LevelGate)
{
    au::perf::XPerfContext4 ctx;
    ctx.setEnabled(true);
    ctx.setTracerLevel(2);

    {
        au::perf::XTracer4Scoped allow(ctx, "allow", 2);
    }
    {
        au::perf::XTracer4Scoped deny(ctx, "deny", 3);
    }

    ctx.setTracerLevel(au::perf::kPerfLevelOff4);
    {
        au::perf::XTracer4Scoped never(ctx, "never", 0);
    }
    SUCCEED();
}


// ===========================================================================
//  7. setEnabled(false) hard-off
// ===========================================================================

TEST_F(XTracer4Test, DisabledHardOff)
{
    au::perf::XPerfContext4 ctx;
    ctx.setEnabled(false);
    ctx.setTracerLevel(au::perf::kPerfLevelAll4);

    {
        au::perf::XTracer4Scoped t(ctx, "off.tracer4");
        t.sub("off.sub");
        t.sub();
    }
    SUCCEED();
}


// ===========================================================================
//  8. sub(name) / sub() state machine
// ===========================================================================

TEST_F(XTracer4Test, SubStateMachine)
{
    {
        au::perf::XTracer4Scoped t("phase.root4");
        t.sub("phase.A");
        au::perf::XTimer4::sleepFor(1);
        t.sub("phase.B");
        au::perf::XTimer4::sleepFor(1);
        t.sub("phase.C");
        t.sub();    // close C
        t.sub();    // already closed 鈫?no-op
    }
    SUCCEED();
}


// ===========================================================================
//  9. Auto-close in-flight sub on destruction
// ===========================================================================

TEST_F(XTracer4Test, AutoCloseSubOnDestruction)
{
    {
        au::perf::XTracer4Scoped t("auto.close.root4");
        t.sub("inflight");
        au::perf::XTimer4::sleepFor(1);
        // NO explicit sub() 鈥?destructor must close in-flight slice itself.
    }
    SUCCEED();
}


// ===========================================================================
//  10. Multi-thread parallel slices
// ===========================================================================

TEST_F(XTracer4Test, MultiThreadParallel)
{
    constexpr int            kThreads = 4;
    std::vector<std::thread> ths;
    ths.reserve(kThreads);
    for (int i = 0; i < kThreads; ++i) {
        ths.emplace_back([i] {
            const std::string label = "mt4.tracer.t" + std::to_string(i);
            au::perf::XTracer4Scoped t(label.c_str(), label.size());
            for (int j = 0; j < 3; ++j) {
                const std::string sub = "sub." + std::to_string(j);
                t.sub(sub.c_str(), sub.size());
                au::perf::XTimer4::sleepFor(1);
            }
        });
    }
    for (auto& th : ths) {
        th.join();
    }
    SUCCEED();
}


// ===========================================================================
//  11. Exception-safe destruction
// ===========================================================================

TEST_F(XTracer4Test, ExceptionSafe)
{
    bool caught = false;
    try {
        au::perf::XTracer4Scoped outer("ex.outer4");
        outer.sub("ex.A");
        au::perf::XTracer4Scoped inner("ex.inner4");
        throw std::runtime_error("propagate");
    } catch (const std::runtime_error& e) {
        caught = true;
        EXPECT_STREQ(e.what(), "propagate");
    }
    EXPECT_TRUE(caught);
}


// ===========================================================================
//  12. Composite AU_PERF4_SCOPE: timer log + tracer slice in lockstep
// ===========================================================================

TEST_F(XTracer4Test, CompositeMacroAlignment)
{
    auto& cfg = au::perf::XPerfContext4::defaultContext();
    cfg.setMode(au::perf::Mode4::Debug);
    cfg.setTimerLevel(au::perf::kPerfLevelAll4);
    cfg.setTracerLevel(au::perf::kPerfLevelAll4);

    {
        AU_PERF4_SCOPE("composite.outer4");
        au::perf::XTimer4::sleepFor(1);
        {
            AU_PERF4_SCOPE("composite.inner4");
            au::perf::XTimer4::sleepFor(1);
        }
    }

    const std::string out = mCapture.drain();
    EXPECT_NE(out.find("composite.outer4"), std::string::npos) << out;
    EXPECT_NE(out.find("composite.inner4"), std::string::npos);
}


// ===========================================================================
//  13. Macro AU_TRACE4 / AU_TRACE4_L: __COUNTER__ disambiguation
// ===========================================================================

TEST_F(XTracer4Test, ConvenienceMacros)
{
    {
        AU_TRACE4("trace4.basic");
        au::perf::XTimer4::sleepFor(1);
    }
    {
        AU_TRACE4_L("trace4.lvl", 1);
    }
    // Two macros on the same source line 鈥?must compile and not collide.
    { AU_TRACE4("trace4.sameA"); AU_TRACE4("trace4.sameB"); }
    SUCCEED();
}


// ===========================================================================
//  14. Temporary std::string label is safely captured
// ===========================================================================

TEST_F(XTracer4Test, TemporaryStringLabelNoDangle)
{
    {
        au::perf::XTracer4Scoped t((std::string("trace4.tmp.") + std::to_string(7)).c_str());
        au::perf::XTimer4::sleepFor(1);
    }
    SUCCEED();
}


#endif  // ENABLE_TEST_XTRACER4