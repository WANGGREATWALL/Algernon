#ifndef AURA_PERF_XTIMER1_H_
#define AURA_PERF_XTIMER1_H_

/**
 * @file xtimer1.h
 * @brief Cross-platform hierarchical performance timer.
 *
 * Features:
 *  - Cross-platform (Windows / Linux / Android / macOS / iOS).
 *  - Per-scope RAII timer with integer level gating.
 *  - Release mode (default): instant one-liner log at scope end.
 *  - Debug mode: thread-local hierarchical tree, flushed on outermost scope exit.
 *  - Aggregate mode: per-thread trees collected for grouped atexit report.
 *  - Thread-safe: each thread owns its tree (lock-free hot path).
 *  - Crash-safe: all destructors noexcept; flush via std::atexit.
 *  - Multi-instance config: callers own private XPerfContext1 for isolation.
 *
 * Quick start:
 * @code
 *   auto& cfg = au::perf::XPerfContext1::defaultConfig();
 *   cfg.setMode(au::perf::Mode::Debug);
 *   cfg.setTimerLevel(3);
 *
 *   void myFunc() {
 *       AU_PERF1_SCOPE("myFunc");           // timer + tracer
 *       AU_TIMER1_L("sub_phase", 2);       // timer only, level 2
 *   }
 * @endcode
 *
 * Level semantics:
 *  - Scopes carry an integer level. Smaller = more important.
 *  - A scope activates iff level <= XPerfContext1 threshold.
 *  - kPerfLevelOff = -1       : never activate
 *  - kPerfLevelAll = INT32_MAX : always activate
 */

#include <atomic>
#include <chrono>
#include <cstdint>
#include <string>
#include <string_view>

#include "sys/xplatform.h"

namespace au::perf {

// ---------------------------------------------------------------------------
// Level sentinels
// ---------------------------------------------------------------------------

constexpr int32_t kPerfLevelOff = -1;
constexpr int32_t kPerfLevelAll = INT32_MAX;

// ---------------------------------------------------------------------------
// Mode
// ---------------------------------------------------------------------------

enum class Mode : int32_t
{
    Release = 0,  ///< No tree; one-liner log on scope exit.
    Debug   = 1,  ///< Thread-local tree; flush on outermost scope exit.
};

// ---------------------------------------------------------------------------
// XPerfContext1 — per-caller configuration
// ---------------------------------------------------------------------------

/// Thread-safe configuration bundle for the perf subsystem.
///
/// All fields are std::atomic for lock-free reads on the hot path.
/// Writes are expected during an init phase. Callers shipping a dynamic
/// library should own a private instance so multiple libraries in the same
/// process don't share a singleton.
class XPerfContext1
{
public:
    static XPerfContext1& defaultConfig() noexcept;

    XPerfContext1() noexcept = default;
    ~XPerfContext1()         = default;

    XPerfContext1(const XPerfContext1&)            = delete;
    XPerfContext1& operator=(const XPerfContext1&) = delete;

    // ── master switch ──

    void setEnabled(bool on) noexcept { mEnabled.store(on, std::memory_order_relaxed); }
    bool isEnabled() const noexcept { return mEnabled.load(std::memory_order_relaxed); }

    // ── mode ──

    void setMode(Mode mode) noexcept { mMode.store(mode, std::memory_order_relaxed); }
    Mode getMode() const noexcept { return mMode.load(std::memory_order_relaxed); }

    // ── level thresholds ──
    // A scope with `level` activates iff level <= threshold.

    void    setTimerLevel(int32_t t) noexcept { mTimerLevel.store(t, std::memory_order_relaxed); }
    int32_t getTimerLevel() const noexcept { return mTimerLevel.load(std::memory_order_relaxed); }

    void    setTracerLevel(int32_t t) noexcept { mTracerLevel.store(t, std::memory_order_relaxed); }
    int32_t getTracerLevel() const noexcept { return mTracerLevel.load(std::memory_order_relaxed); }

    // ── depth guard ──

    /// Nodes deeper than this are not inserted into the tree (degraded to
    /// one-liner). Default 64.
    void     setMaxTreeDepth(uint32_t d) noexcept { mMaxDepth.store(d, std::memory_order_relaxed); }
    uint32_t getMaxTreeDepth() const noexcept { return mMaxDepth.load(std::memory_order_relaxed); }

    // ── root name (Debug header) ──

    void setRootName(std::string_view name) noexcept;
    void getRootName(char* outBuf, size_t bufSize) const noexcept;

    // ── aggregate mode ──

    /// When enabled, per-thread Debug trees are appended to a process-global
    /// collector instead of being flushed immediately. Call flushAggregated()
    /// (or rely on std::atexit) to emit grouped output.
    void setAggregateMode(bool on) noexcept { mAggregate.store(on, std::memory_order_relaxed); }
    bool isAggregateMode() const noexcept { return mAggregate.load(std::memory_order_relaxed); }

    /// Flush the process-global collector now. Idempotent.
    static void flushAggregated() noexcept;

    // ── system-property loader ──
    //
    // Pass nullptr to skip an entry. On non-Android the call is a no-op.
    //   propEnabled     : "0" / "1"
    //   propMode        : "0" (Release) / "1" (Debug)
    //   propTimerLevel  : signed int string
    //   propTracerLevel : signed int string

    void loadFromSystemProperty(const char* propEnabled,
                                const char* propMode,
                                const char* propTimerLevel,
                                const char* propTracerLevel) noexcept;

private:
    std::atomic<bool>     mEnabled{true};
    std::atomic<Mode>     mMode{Mode::Release};
    std::atomic<int32_t>  mTimerLevel{3};
    std::atomic<int32_t>  mTracerLevel{kPerfLevelAll};
    std::atomic<uint32_t> mMaxDepth{64};
    std::atomic<bool>     mAggregate{false};

    // Fixed-size buffer avoids std::string allocations that could cause
    // static-destruction ordering issues.
    mutable std::atomic<uint32_t> mRootNameLen{4};
    char                          mRootName[64]{'p', 'e', 'r', 'f', '\0'};
};

// ---------------------------------------------------------------------------
// XTimer1 — lightweight stopwatch
// ---------------------------------------------------------------------------

class XTimer1
{
public:
    using Clock     = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;

    /// Sleep for @p ms milliseconds. Negative or zero returns immediately.
    static void sleepFor(std::chrono::milliseconds ms) noexcept;

    /// Thread-safe formatted time string "<fmt>_<ms>".
    static std::string getTimeFormatted(std::string_view fmt = "%Y-%m-%d-%H-%M-%S") noexcept;

    XTimer1() noexcept : mBegin(Clock::now()) {}

    void  restart() noexcept { mBegin = Clock::now(); }
    float elapsedMs() const noexcept
    {
        return std::chrono::duration<float, std::milli>(Clock::now() - mBegin).count();
    }

private:
    TimePoint mBegin;
};

// ---------------------------------------------------------------------------
// XTimer1Scoped — RAII scoped timer
// ---------------------------------------------------------------------------

/// Activation rules (evaluated once at construction):
///  1. cfg.isEnabled() must be true
///  2. level <= cfg.getTimerLevel() must hold
/// If inactive, all member functions become no-ops with zero heap allocation.
///
/// Thread-local: each thread owns its node pool; no locks on the hot path.
/// The outermost active scope on a thread flushes the thread's subtree on
/// destruction (Release: nothing to flush; Debug: DFS-print or aggregate).
class XTimer1Scoped
{
public:
    /// Use the default process-wide config.
    explicit XTimer1Scoped(std::string_view name, int32_t level = 0) noexcept;

    /// Use a caller-specific config.
    XTimer1Scoped(XPerfContext1& cfg, std::string_view name, int32_t level = 0) noexcept;

    ~XTimer1Scoped() noexcept;

    XTimer1Scoped(const XTimer1Scoped&)            = delete;
    XTimer1Scoped& operator=(const XTimer1Scoped&) = delete;

    /// End the last sub-node (if any) and open a new sibling with @p name.
    /// No-op when this scope is inactive or not in Debug mode.
    void sub(std::string_view name) noexcept;

    /// End the last sub-node (if any). No-op when none is open.
    void sub() noexcept;

private:
    void begin(std::string_view name, int32_t level) noexcept;

    XPerfContext1*                          mCfg{nullptr};
    int32_t                                 mNodeIdx{-1};
    int32_t                                 mSubNodeIdx{-1};
    uint32_t                                mDepth{0};
    bool                                    mIsRoot{false};
    std::chrono::steady_clock::time_point   mBegin;
    // Owned copy for the degraded (one-liner) path — fixes v3's string_view
    // dangling bug where a temporary name outlives the scope.
    char                                    mNameBuf[256]{};
    uint32_t                                mNameLen{0};
};

}  // namespace au::perf

// ---------------------------------------------------------------------------
// Macros
// ---------------------------------------------------------------------------

#define AU_PERF_CONCAT1_(a, b) a##b
#define AU_PERF_CONCAT1(a, b)  AU_PERF_CONCAT1_(a, b)
#define AU_PERF_UNIQUE1(p)     AU_PERF_CONCAT1(p, __COUNTER__)

#define AU_TIMER1(name) \
    ::au::perf::XTimer1Scoped AU_PERF_UNIQUE1(_auTmr1_)((name))

#define AU_TIMER1_L(name, lv) \
    ::au::perf::XTimer1Scoped AU_PERF_UNIQUE1(_auTmr1_)((name), (lv))

#define AU_TIMER1_CFG(cfg, name, lv) \
    ::au::perf::XTimer1Scoped AU_PERF_UNIQUE1(_auTmr1_)((cfg), (name), (lv))

// Composite macros (AU_PERF1_SCOPE etc.) are defined in xtracer1.h.

#endif  // AURA_PERF_XTIMER1_H_
