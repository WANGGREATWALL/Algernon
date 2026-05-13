#ifndef AURA_PERF_XTIMER4_H_
#define AURA_PERF_XTIMER4_H_

/**
 * @file xtimer4.h
 * @brief Final-form hierarchical performance timer for the Aura SDK.
 *
 * This is the v4 / final design: it consolidates the lessons learned from
 * the four prior iterations (xtimer/xtimer0..3) and the supplementary
 * @c perf_best_design.md whitepaper.
 *
 * Highlights vs prior versions:
 *  - **Pimpl ABI**: the public header carries no @c std::atomic, no STL
 *    container, no implementation detail. Compile firewall + binary
 *    compatibility.
 *  - **Owned name buffers**: every scope copies its label into either an
 *    inline buffer or the thread-local arena. Eliminates the dangling
 *    @c string_view bug present in v3.
 *  - **Size-aware constructors**: a @c (name, len) overload skips the
 *    @c strlen() probe on the hot path.
 *  - **Pluggable writer**: a thin @c IPerfWriter virtual interface lets
 *    unit tests intercept output without relying on @c CaptureStdout, and
 *    lets shipping code redirect to a custom sink (file / network / ring).
 *  - **Compile-time disable**: define @c AU_PERF4_DISABLE_ALL=1 in build
 *    flags to strip every scope down to @c ((void)0).
 *
 * Threading model:
 *  - Every thread owns its own tree (TLS pool + arena + open-stack).
 *    Hot path is fully lock-free.
 *  - All atomic reads use @c std::memory_order_relaxed because the only
 *    ordering requirement is single-config-load consistency, not cross-
 *    thread happens-before.
 *  - All destructors are @c noexcept; OOM degrades a node to a one-liner
 *    instead of propagating an exception.
 *
 * Quick start:
 * @code
 *   auto& ctx = au::perf::XPerfContext4::defaultContext();
 *   ctx.setEnabled(true);
 *   ctx.setMode(au::perf::Mode4::Debug);
 *   ctx.setTimerLevel(3);
 *
 *   void XNet::forward() {
 *       AU_PERF4_SCOPE("XNet::forward");
 *       AU_TIMER4_L("preprocess", 2);
 *       // ... work ...
 *   }
 * @endcode
 */

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <string>

namespace au {
namespace perf {

// ---------------------------------------------------------------------------
// Level sentinels (plain integers; level can be any int32_t).
// ---------------------------------------------------------------------------

/// Hard-off sentinel: never activate any scope on this channel.
constexpr int32_t kPerfLevelOff4 = -1;

/// Always-on sentinel: every scope passes the level gate.
constexpr int32_t kPerfLevelAll4 = INT32_MAX;


// ---------------------------------------------------------------------------
// Mode selector.
// ---------------------------------------------------------------------------

enum class Mode4 : int32_t
{
    Release = 0,  ///< One-liner per scope, no tree work.
    Debug   = 1,  ///< Build a thread-local tree; flush on outermost scope.
};


// ---------------------------------------------------------------------------
// IPerfWriter — pluggable output sink.
// ---------------------------------------------------------------------------

/**
 * @brief Abstract sink for formatted perf lines.
 *
 * Implementations must be thread-safe: @c write() can be called from many
 * threads concurrently. The default sink (used when no custom writer is
 * installed) routes output through @c xlogger (XLOG_I).
 *
 * @note The buffer pointer is only valid for the duration of the call.
 *       Implementations that buffer must copy.
 */
class IPerfWriter4
{
public:
    virtual ~IPerfWriter4() = default;

    /// @param data  Pointer to UTF-8 / ASCII bytes (not null-terminated).
    /// @param size  Number of bytes in @p data.
    virtual void write(const char* data, std::size_t size) noexcept = 0;
};


// ---------------------------------------------------------------------------
// XPerfContext4 — per-caller configuration bundle.
// ---------------------------------------------------------------------------

/// Forward declaration: implementation lives entirely in the .cpp.
class XPerfContext4Impl;

/**
 * @brief Per-caller configuration container (Pimpl ABI).
 *
 * All public read/write accessors are atomic and lock-free; safe to call
 * from any thread at any time.
 *
 * Each shared library can own a private instance to isolate from other
 * libraries in the same process.
 */
class XPerfContext4
{
    // ── friends with internal-impl access (same translation unit boundary) ──
    friend class XTimer4Scoped;
    friend class XTracer4Scoped;

public:
    XPerfContext4() noexcept;
    ~XPerfContext4() noexcept;

    XPerfContext4(const XPerfContext4&)            = delete;
    XPerfContext4& operator=(const XPerfContext4&) = delete;

    /// Process-wide default instance.
    static XPerfContext4& defaultContext() noexcept;

    // ── master switch ──

    void setEnabled(bool on) noexcept;
    bool isEnabled() const noexcept;

    // ── mode ──

    void  setMode(Mode4 mode) noexcept;
    Mode4 getMode() const noexcept;

    // ── level thresholds ──

    void    setTimerLevel(int32_t threshold) noexcept;
    int32_t getTimerLevel() const noexcept;

    void    setTracerLevel(int32_t threshold) noexcept;
    int32_t getTracerLevel() const noexcept;

    // ── depth guard ──

    /// Nodes deeper than this are degraded to a one-liner instead of being
    /// inserted into the tree (default 64).
    void     setMaxTreeDepth(uint32_t depth) noexcept;
    uint32_t getMaxTreeDepth() const noexcept;

    // ── root header label ──

    /// Truncates to 63 chars internally. @p name may be null (treated as empty).
    void setRootName(const char* name) noexcept;
    void setRootName(const char* name, std::size_t len) noexcept;

    /// Copy into @p outBuf (always NUL-terminated).
    void getRootName(char* outBuf, std::size_t bufSize) const noexcept;

    // ── aggregate mode ──

    void setAggregateMode(bool on) noexcept;
    bool isAggregateMode() const noexcept;

    /// Drain the process-global aggregate queue. Idempotent.
    /// Auto-registered via std::atexit on first aggregate scope.
    static void flushAggregated() noexcept;

    // ── pluggable writer ──

    /// Install a custom sink. Pass @c nullptr to restore the default
    /// (xlogger-backed) sink. Lifetime: the writer must outlive every
    /// active scope using this context. Thread-safe to swap, but not
    /// to free a writer while threads are still emitting.
    void          setWriter(IPerfWriter4* writer) noexcept;
    IPerfWriter4* getWriter() const noexcept;

    // ── system property loader ──
    //
    //   propEnabled      : "0" or "1"
    //   propMode         : "0" (Release) or "1" (Debug)
    //   propTimerLevel   : signed int string
    //   propTracerLevel  : signed int string
    // Pass nullptr to skip an entry. On non-Android, falls back to env vars.

    void loadFromSystemProperty(const char* propEnabled, const char* propMode, const char* propTimerLevel,
                                const char* propTracerLevel) noexcept;

private:
    XPerfContext4Impl* mImpl;  // Pimpl: hides std::atomic from the public ABI.
};


// ---------------------------------------------------------------------------
// XTimer4 — bare stopwatch.
// ---------------------------------------------------------------------------

/// Lightweight stopwatch. Trivially copyable; ~ns construction.
class XTimer4
{
public:
    using Clock     = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;

    /// Sleep helper (portable). Non-positive @p ms returns immediately.
    static void sleepFor(int64_t ms) noexcept;

    /// Thread-safe wrapper around localtime. Format defaults to
    /// "%Y-%m-%d-%H-%M-%S". Returns "<formatted>_<ms>" with sub-second tail.
    static std::string getTimeFormatted(const char* fmt = "%Y-%m-%d-%H-%M-%S") noexcept;

    XTimer4() noexcept : mBegin(Clock::now()) {}

    void  restart() noexcept { mBegin = Clock::now(); }
    float elapsedMs() const noexcept;

private:
    TimePoint mBegin;
};


// ---------------------------------------------------------------------------
// XTimer4Scoped — RAII scoped timer.
// ---------------------------------------------------------------------------

/**
 * @brief RAII scoped timer with optional thread-local tree building.
 *
 * Activation rules (evaluated once at construction):
 *   - @c ctx.isEnabled() must be true
 *   - @c level must satisfy @c level <= ctx.getTimerLevel()
 *
 * If inactive, every member is a no-op with zero allocation.
 *
 * Name lifetime: the constructor copies the label bytes into either an
 * inline buffer (Release path) or the TLS arena (Debug path); the caller
 * may safely pass a temporary @c std::string or a @c char[] that goes out
 * of scope before this object is destroyed.
 */
class XTimer4Scoped
{
public:
    /// Use the default process context.
    explicit XTimer4Scoped(const char* name, int32_t level = 0) noexcept;

    /// Size-aware overload: skips @c strlen() on hot path.
    XTimer4Scoped(const char* name, std::size_t nameLen, int32_t level = 0) noexcept;

    /// Use a caller-specific context.
    XTimer4Scoped(XPerfContext4& ctx, const char* name, int32_t level = 0) noexcept;
    XTimer4Scoped(XPerfContext4& ctx, const char* name, std::size_t nameLen, int32_t level = 0) noexcept;

    ~XTimer4Scoped() noexcept;

    XTimer4Scoped(const XTimer4Scoped&)            = delete;
    XTimer4Scoped& operator=(const XTimer4Scoped&) = delete;

    /// End the previous sub-node (if any) and open a new sibling.
    void sub(const char* name) noexcept;
    void sub(const char* name, std::size_t nameLen) noexcept;

    /// End the previous sub-node (if any). No-op when none is open.
    void sub() noexcept;

    /// Milliseconds since this scope began.
    float elapsedMs() const noexcept;

private:
    /// Shared constructor helper.
    void begin(XPerfContext4& ctx, const char* name, std::size_t nameLen, int32_t level) noexcept;

    XPerfContext4*                        mCtx;
    int32_t                               mNodeIdx;     ///< -1 inactive, -2 active-no-tree
    int32_t                               mSubNodeIdx;  ///< -1 = no open sub
    uint32_t                              mDepth;
    bool                                  mIsRoot;
    std::chrono::steady_clock::time_point mBegin;

    // Inline name buffer for the active-no-tree path. Anything longer
    // than this is truncated. The Debug path uses the TLS arena via
    // an offset/len pair stored in the pool node.
    static constexpr std::size_t kInlineNameCap = 96;
    char                         mNameInline[kInlineNameCap];
    uint32_t                     mNameLen;
};

}  // namespace perf
}  // namespace au

#endif  // AURA_PERF_XTIMER4_H_