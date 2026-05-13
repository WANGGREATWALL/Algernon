#ifndef AURA_PERF_XTIMER0_H_
#define AURA_PERF_XTIMER0_H_

/**
 * @file xtimer0.h
 * @brief High-resolution scoped timer with per-caller context and tree output.
 *
 * Overview:
 *   XTimer0Context holds per-caller configuration (enable, level, mode).
 *   XTimer0 is a bare stopwatch.
 *   XTimer0Scoped is an RAII guard that records timing and optionally builds
 *   a tree in Debug mode.
 *
 * Modes:
 *   Release (default): prints "[name]: X.XX ms" on destruction.
 *   Debug:            builds a thread_local tree and prints hierarchically.
 *
 * Thread safety:
 *   All config reads are lock-free (std::atomic). Output is serialised via a
 *   global mutex. In Debug mode each thread owns a separate tree; trees are
 *   printed atomically per thread.
 *
 * Quick start:
 *   au::perf::XTimer0Context::global().setEnabled(true);
 *   {
 *       au::perf::XTimer0Scoped s("myOp");
 *       // ... work ...
 *       s.sub("phase1");
 *       // ... work ...
 *       s.sub("phase2");
 *       // ... work ...
 *   }  // prints timing on destruction
 *
 * @example Per-caller configuration:
 *   auto& ctx = au::perf::XTimer0Context::get("myLib");
 *   ctx.configureFrom("vendor.mylib.xtimer");
 *   au::perf::XTimer0Scoped s("op", &ctx);
 */

#include <atomic>
#include <chrono>
#include <cstddef>
#include <string>

namespace au {
namespace perf {

// ============================================================================
// XTimer0Context — per-caller configuration
// ============================================================================

class XTimer0Context
{
public:
    enum class Mode : int
    {
        Release = 0,  ///< instant flat output per scope
        Debug   = 1   ///< tree-structured output
    };

    /** @brief Get or create a named context (max 32 contexts). */
    static XTimer0Context& get(const char* name);

    /** @brief Global default context. */
    static XTimer0Context& global();

    XTimer0Context()                         = default;

    // ── configuration ──

    void setEnabled(bool on) noexcept { mEnabled.store(on, std::memory_order_relaxed); }
    bool isEnabled()     const noexcept { return mEnabled.load(std::memory_order_relaxed); }

    void setLevel(int level) noexcept { mLevel.store(level, std::memory_order_relaxed); }
    int  getLevel()     const noexcept { return mLevel.load(std::memory_order_relaxed); }

    void setMode(Mode mode) noexcept { mMode.store(static_cast<int>(mode), std::memory_order_relaxed); }
    Mode getMode()       const noexcept { return static_cast<Mode>(mMode.load(std::memory_order_relaxed)); }

    /** @brief Read configuration from system properties or environment variables.
     *  Reads keys: "<prefix>.enabled", "<prefix>.level", "<prefix>.mode" */
    void configureFrom(const char* propertyPrefix) noexcept;

    /** @brief Write formatted output through xlogger (default) or a custom writer. */
    using Writer = void (*)(const char* msg, int len, void* userData);
    void setWriter(Writer w, void* userData) noexcept
    {
        mWriter     = w;
        mWriterData = userData;
    }

    // ── internal (called by XTimer0Scoped) ──

    bool shouldPrint(int level) const noexcept
    {
        if (!isEnabled())
            return false;
        int lim = getLevel();
        return lim < 0 || level <= lim;
    }

    void write(const char* msg, int len) const noexcept;

private:
    friend class XTimer0Scoped;

    std::atomic<bool> mEnabled{false};
    std::atomic<int>  mLevel{-1};
    std::atomic<int>  mMode{0};

    Writer mWriter     = nullptr;
    void*  mWriterData = nullptr;

    XTimer0Context(const XTimer0Context&) = delete;
    XTimer0Context& operator=(const XTimer0Context&) = delete;
};

// ============================================================================
// XTimer0 — bare stopwatch
// ============================================================================

class XTimer0
{
    using Clock     = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;

public:
    /** @brief Sleep for given milliseconds (cross-platform). */
    static void sleepFor(long long ms);

    /** @brief Get current time as formatted string (thread-safe). */
    static std::string getTimeFormatted(const std::string& fmt = "%Y-%m-%d-%H-%M-%S");

    XTimer0() { restart(); }

    void  restart() noexcept { mBegin = Clock::now(); }
    float elapsed() const noexcept;

private:
    TimePoint mBegin;
};

// ============================================================================
// XTimer0Scoped — RAII scoped timer
// ============================================================================

class XTimer0Scoped
{
public:
    /**
     * @brief Start a named timing scope.
     * @param name  Human-readable label (must outlive this object).
     * @param ctx   Context pointer (nullptr → global context).
     */
    explicit XTimer0Scoped(const char* name, XTimer0Context* ctx = nullptr) noexcept;
    ~XTimer0Scoped() noexcept;

    XTimer0Scoped(const XTimer0Scoped&) = delete;
    XTimer0Scoped& operator=(const XTimer0Scoped&) = delete;

    /** @brief End current sub-node and start a new named one. */
    void sub(const char* name) noexcept;

    /** @brief End current sub-node without starting a new one. */
    void sub() noexcept;

    /** @brief Elapsed milliseconds since this scope started. */
    float elapsed() const noexcept;

    /** @brief Depth of this scope relative to tree root (1-based). */
    int getLevel() const noexcept { return mLevel; }

private:
    // ── Release-mode path ──
    void releasePrint() noexcept;
    void releasePrintSub() noexcept;

    // ── Debug-mode path ──
    struct TreeNode;
    void debugEnter(const char* name) noexcept;
    void debugLeave() noexcept;

    XTimer0Context* mCtx;
    XTimer0         mTimer;
    const char*    mName;
    const char*    mSegName;   // current segment name (Release mode)
    int            mLevel;
    int            mLevelBegin;
    bool           mActive;   // true if this scope is recording
};

}  // namespace perf
}  // namespace au

// ============================================================================
// Convenience macros
// ============================================================================

/**
 * @def XTIMER0_SCOPED(name)
 * Create a scoped timer using the global context.
 * Usage: { XTIMER0_SCOPED("myOp"); ... XTIMER0_SUB("phase1"); ... }
 */
#define XTIMER0_SCOPED(name) \
    ::au::perf::XTimer0Scoped __xtimer_scope_##__LINE__(name)

/**
 * @def XTIMER0_SCOPED_CTX(name, ctx)
 * Create a scoped timer with an explicit context.
 */
#define XTIMER0_SCOPED_CTX(name, ctx) \
    ::au::perf::XTimer0Scoped __xtimer_scope_##__LINE__(name, ctx)

/**
 * @def XTIMER0_SUB(name)
 * End current sub-node and start a new one.
 * Must be called within an XTIMER0_SCOPED block.
 */
#define XTIMER0_SUB(name) \
    __xtimer_scope_##__LINE__.sub(name)

#endif  // AURA_PERF_XTIMER0_H_
