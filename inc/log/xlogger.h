#ifndef ALGERNON_LOG_XLOGGER_H_
#define ALGERNON_LOG_XLOGGER_H_

/**
 * @file xlogger.h
 * @brief Lightweight, cross-platform logging framework.
 *
 * Features:
 *  - Macro-side level check (zero overhead for filtered-out messages)
 *  - ANSI color output (optional)
 *  - Android logcat + optional shell output
 *  - Thread-safe output serialisation
 *  - File/line info only for Warn/Error/Fatal (reduces V/D/I overhead)
 *
 * Quick start:
 *   algernon::log::Config::get().setTag("MyApp");
 *   algernon::log::Config::get().setLevel(algernon::log::Level::Debug);
 *   XLOG_I("initialised, version=%d\n", 1);
 *   XLOG_E("open failed: %s\n", path);
 */

#include <atomic>
#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <mutex>

#include "sys/xplatform.h"
#if defined(ALGERNON_OS_ANDROID)
#include <android/log.h>
#endif

namespace algernon {
namespace log {

enum class Level : int
{
    Verbose = 0,
    Debug   = 1,
    Info    = 2,  ///< default threshold
    Warn    = 3,
    Error   = 4,
    Fatal   = 5,
    Silent  = 6,
};

/// Singleton holding logger configuration.
class Config
{
public:
    static Config& get() noexcept
    {
        static Config instance;
        return instance;
    }

    void setTag(const char* tag) noexcept
    {
        std::lock_guard<std::mutex> lock(mOutputMutex);
        std::strncpy(mTag, tag ? tag : "unknown", sizeof(mTag) - 1);
        mTag[sizeof(mTag) - 1] = '\0';
        mTagWarned.store(true, std::memory_order_relaxed);
    }

    const char* getTag() const noexcept { return mTag; }

    /// Returns true exactly once if setTag() was never called.
    bool tryConsumeTagWarning() noexcept
    {
        if (mTagWarned.load(std::memory_order_relaxed))
            return false;
        bool expected = false;
        return mTagWarned.compare_exchange_strong(expected, true, std::memory_order_relaxed);
    }

    void  setLevel(Level level) noexcept { mLevel.store(level, std::memory_order_relaxed); }
    Level getLevel() const noexcept { return mLevel.load(std::memory_order_relaxed); }

    void setColorEnabled(bool on) noexcept { mColorEnabled.store(on, std::memory_order_relaxed); }
    bool isColorEnabled() const noexcept { return mColorEnabled.load(std::memory_order_relaxed); }
#if defined(ALGERNON_OS_ANDROID)
    void setShellPrintEnabled(bool on) noexcept
    {
        mShellPrint.store(on, std::memory_order_relaxed);
        mShellPrintReady.store(true, std::memory_order_release);
    }

    bool isShellPrintEnabled() noexcept
    {
        if (!mShellPrintReady.load(std::memory_order_acquire)) {
            const bool val = sys::getSystemPropertyValue("vendor.base.debug.enable_shell_log", 0) != 0;
            mShellPrint.store(val, std::memory_order_relaxed);
            mShellPrintReady.store(true, std::memory_order_release);
        }
        return mShellPrint.load(std::memory_order_relaxed);
    }
#endif

    std::mutex& outputMutex() noexcept { return mOutputMutex; }

private:
    Config() { std::memcpy(mTag, "unknown", 8); }
    Config(const Config&)            = delete;
    Config& operator=(const Config&) = delete;

    char               mTag[64]{};
    std::atomic<bool>  mTagWarned{false};
    std::atomic<Level> mLevel{Level::Info};
    std::atomic<bool>  mColorEnabled{false};
    std::mutex         mOutputMutex;
#if defined(ALGERNON_OS_ANDROID)
    std::atomic<bool> mShellPrint{false};
    std::atomic<bool> mShellPrintReady{false};
#endif
};

namespace detail {

/// Clamp snprintf return value to [0, capacity-1].
inline int clampLen(int len, int capacity) noexcept
{
    if (len < 0)
        return 0;
    if (len >= capacity)
        return capacity - 1;
    return len;
}

inline const char* levelTag(Level level) noexcept
{
    // clang-format off
    switch (level) {
        case Level::Verbose: return "V";
        case Level::Debug:   return "D";
        case Level::Info:    return "I";
        case Level::Warn:    return "W";
        case Level::Error:   return "E";
        case Level::Fatal:   return "F";
        default:             return "?";
    }
    // clang-format on
}

inline const char* levelColor(Level level) noexcept
{
    // clang-format off
    switch (level) {
        case Level::Verbose: return "\033[37m";      // gray
        case Level::Debug:   return "\033[36m";      // cyan
        case Level::Info:    return "\033[32m";      // green
        case Level::Warn:    return "\033[33m";      // yellow
        case Level::Error:   return "\033[31m";      // red
        case Level::Fatal:   return "\033[1;41;37m"; // bold, red bg, white text
        default:             return "\033[0m";
    }
    // clang-format on
}
#if defined(ALGERNON_OS_ANDROID)
inline int toAndroidPriority(Level level) noexcept
{
    // clang-format off
    switch (level) {
        case Level::Verbose: return ANDROID_LOG_VERBOSE;
        case Level::Debug:   return ANDROID_LOG_DEBUG;
        case Level::Info:    return ANDROID_LOG_INFO;
        case Level::Warn:    return ANDROID_LOG_WARN;
        case Level::Error:   return ANDROID_LOG_ERROR;
        case Level::Fatal:   return ANDROID_LOG_FATAL;
        default:             return ANDROID_LOG_DEFAULT;
    }
    // clang-format on
}
#endif

constexpr const char* basename(const char* path) noexcept
{
    const char* base = path;
    for (const char* p = path; *p != '\0'; ++p) {
        if (*p == '/' || *p == '\\')
            base = p + 1;
    }
    return base;
}

inline void logPrint(Level level, const char* file, int line, const char* fmt, va_list args) noexcept
{
    auto&       cfg = Config::get();
    const char* tag = cfg.getTag();

    // One-time warning if setTag() was never called.
    if (cfg.tryConsumeTagWarning()) {
        std::fprintf(stderr,
                     "[unknown][W] log tag not set — "
                     "call algernon::log::Config::get().setTag(\"YourTag\") at startup\n");
    }

    const bool needFlush = (level >= Level::Warn);
#if defined(ALGERNON_OS_ANDROID)
    const int  prio         = toAndroidPriority(level);
    const bool shellEnabled = cfg.isShellPrintEnabled();

    if (file == nullptr) {
        // --- V / D / I ---
        if (!shellEnabled) {
            __android_log_vprint(prio, tag, fmt, args);
            return;
        }
        char outBuf[896];
        int  hdrLen =
            clampLen(std::snprintf(outBuf, sizeof(outBuf), "[%s][%s] ", tag, levelTag(level)), (int)sizeof(outBuf));

        va_list argsCopy;
        va_copy(argsCopy, args);

        int bodyLen =
            clampLen(std::vsnprintf(outBuf + hdrLen, sizeof(outBuf) - hdrLen, fmt, args), (int)sizeof(outBuf) - hdrLen);

        __android_log_vprint(prio, tag, fmt, argsCopy);
        va_end(argsCopy);

        {
            std::lock_guard<std::mutex> lk(cfg.outputMutex());
            std::fwrite(outBuf, 1, (size_t)(hdrLen + bodyLen), stdout);
        }
    } else {
        // --- W / E / F ---
        if (!shellEnabled) {
            char bodyBuf[820];
            std::vsnprintf(bodyBuf, sizeof(bodyBuf), fmt, args);
            __android_log_print(prio, tag, "(%s:%d) %s", file, line, bodyBuf);
            return;
        }
        char outBuf[896];
        int  hdrLen =
            clampLen(std::snprintf(outBuf, sizeof(outBuf), "[%s][%s] ", tag, levelTag(level)), (int)sizeof(outBuf));
        int locLen    = clampLen(std::snprintf(outBuf + hdrLen, sizeof(outBuf) - hdrLen, "(%s:%d) ", file, line),
                                 (int)sizeof(outBuf) - hdrLen);
        int bodyStart = hdrLen + locLen;
        int bodyLen   = clampLen(std::vsnprintf(outBuf + bodyStart, sizeof(outBuf) - bodyStart, fmt, args),
                                 (int)sizeof(outBuf) - bodyStart);

        __android_log_write(prio, tag, outBuf + hdrLen);

        {
            std::lock_guard<std::mutex> lk(cfg.outputMutex());
            std::fwrite(outBuf, 1, (size_t)(bodyStart + bodyLen), stdout);
            if (needFlush)
                std::fflush(stdout);
        }
    }

#else  // Desktop: macOS / Linux / Windows
    char outBuf[896];
    int  prefixLen;
    if (file != nullptr) {
        prefixLen =
            clampLen(std::snprintf(outBuf, sizeof(outBuf), "[%s][%s] (%s:%d) ", tag, levelTag(level), file, line),
                     (int)sizeof(outBuf));
    } else {
        prefixLen =
            clampLen(std::snprintf(outBuf, sizeof(outBuf), "[%s][%s] ", tag, levelTag(level)), (int)sizeof(outBuf));
    }
    int bodyLen = clampLen(std::vsnprintf(outBuf + prefixLen, sizeof(outBuf) - prefixLen, fmt, args),
                           (int)sizeof(outBuf) - prefixLen);
    int outLen  = prefixLen + bodyLen;

    {
        std::lock_guard<std::mutex> lk(cfg.outputMutex());
        if (cfg.isColorEnabled()) {
            std::fputs(levelColor(level), stdout);
            std::fwrite(outBuf, 1, (size_t)outLen, stdout);
            std::fputs("\033[0m", stdout);
        } else {
            std::fwrite(outBuf, 1, (size_t)outLen, stdout);
        }
        if (needFlush)
            std::fflush(stdout);
    }
#endif
}

#if defined(__GNUC__) || defined(__clang__)
__attribute__((format(printf, 2, 3)))
#endif
inline void
logPrintF(Level level, const char* fmt, ...) noexcept
{
    va_list args;
    va_start(args, fmt);
    logPrint(level, nullptr, 0, fmt, args);
    va_end(args);
}

#if defined(__GNUC__) || defined(__clang__)
__attribute__((format(printf, 4, 5)))
#endif
inline void
logPrintFLoc(Level level, const char* file, int line, const char* fmt, ...) noexcept
{
    va_list args;
    va_start(args, fmt);
    logPrint(level, file, line, fmt, args);
    va_end(args);
}

}  // namespace detail
}  // namespace log
}  // namespace algernon

// ── Public logging macros ──

#define XLOG_IMPL(lvl, fmt, ...)                                         \
    do {                                                                 \
        if ((lvl) >= algernon::log::Config::get().getLevel()) {          \
            algernon::log::detail::logPrintF((lvl), fmt, ##__VA_ARGS__); \
        }                                                                \
    } while (0)

#define XLOG_IMPL_LOC(lvl, fmt, ...)                                                                             \
    do {                                                                                                         \
        if ((lvl) >= algernon::log::Config::get().getLevel()) {                                                  \
            algernon::log::detail::logPrintFLoc((lvl), algernon::log::detail::basename(__FILE__), __LINE__, fmt, \
                                                ##__VA_ARGS__);                                                  \
        }                                                                                                        \
    } while (0)

// clang-format off
#define XLOG_V(fmt, ...) XLOG_IMPL(algernon::log::Level::Verbose, fmt, ##__VA_ARGS__)
#define XLOG_D(fmt, ...) XLOG_IMPL(algernon::log::Level::Debug,   fmt, ##__VA_ARGS__)
#define XLOG_I(fmt, ...) XLOG_IMPL(algernon::log::Level::Info,    fmt, ##__VA_ARGS__)
#define XLOG_W(fmt, ...) XLOG_IMPL_LOC(algernon::log::Level::Warn,  fmt, ##__VA_ARGS__)
#define XLOG_E(fmt, ...) XLOG_IMPL_LOC(algernon::log::Level::Error, fmt, ##__VA_ARGS__)
#define XLOG_F(fmt, ...) XLOG_IMPL_LOC(algernon::log::Level::Fatal, fmt, ##__VA_ARGS__)
// clang-format on

// ── Assertion / check macros ──

#define XCHECK(expr) assert(expr)

#define XCHECK_WITH_RET(expr, ret)                                                                   \
    do {                                                                                             \
        if (!(expr)) {                                                                               \
            algernon::log::detail::logPrintFLoc(algernon::log::Level::Error,                         \
                                                algernon::log::detail::basename(__FILE__), __LINE__, \
                                                "check failed: '%s'\n", #expr);                      \
            return (ret);                                                                            \
        }                                                                                            \
    } while (0)

#define XCHECK_WITH_MSG(expr, ret, fmt, ...)                                                                           \
    do {                                                                                                               \
        if (!(expr)) {                                                                                                 \
            algernon::log::detail::logPrintFLoc(                                                                       \
                algernon::log::Level::Error, algernon::log::detail::basename(__FILE__), __LINE__, fmt, ##__VA_ARGS__); \
            return (ret);                                                                                              \
        }                                                                                                              \
    } while (0)

// Backward-compatible aliases
#define XASSERT(expr) XCHECK(expr)
#define XASSERT_RET(expr, ret) XCHECK_WITH_RET(expr, ret)
#define XASSERT_INFO(expr, ret, fmt, ...) XCHECK_WITH_MSG(expr, ret, fmt, ##__VA_ARGS__)

#endif  // ALGERNON_LOG_XLOGGER_H_
