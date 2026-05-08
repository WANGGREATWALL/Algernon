#ifndef XLOGGER_H
#define XLOGGER_H

/**
 * @file xlogger.h
 * @brief Lightweight, cross-platform logging framework.
 *
 * Features:
 *  - Macro-side level check (zero overhead for filtered-out messages)
 *  - ANSI color output (optional, applies to stdout/shell on all platforms)
 *  - Android logcat + optional shell/stdout output
 *  - Thread-safe output serialisation
 *  - File/line info only for Warn/Error/Fatal (reduces V/D/I overhead)
 *
 * Newline convention:
 *  Always terminate format strings with '\n' (e.g. XLOG_I("msg\n")).
 *  - Desktop/shell: '\n' is written as-is by fwrite for proper line separation.
 *  - Android logcat: logd (API 21+) strips any trailing '\n' from every record
 *    before storage, so no double blank lines appear regardless of level.
 *
 * Android shell output:
 *  By default, logs go to logcat only. To also print to stdout (e.g. adb shell),
 *  call setShellPrintEnabled(true) at startup. xlogger intentionally does NOT read
 *  any system property internally — the property name and read logic are business-
 *  specific and must be handled by the caller. Recommended pattern:
 *
 *    // In your module's init (e.g. MyModule.cpp):
 *    #include "sys/xplatform.h"
 *    const bool shellOn =
 *        sys::getSystemPropertyValue("vendor.algo_module.enable_log_shell", 0) != 0;
 *    log::Config::get().setShellPrintEnabled(shellOn);
 *
 * Quick start:
 *   log::Config::get().setTag("MyApp");
 *   log::Config::get().setLevel(log::Level::Debug);
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
#if ALGERNON_OS_ANDROID
#include <android/log.h>
#endif

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

#if ALGERNON_OS_ANDROID
    void setShellPrintEnabled(bool on) noexcept { mShellPrint.store(on, std::memory_order_relaxed); }

    bool isShellPrintEnabled() const noexcept { return mShellPrint.load(std::memory_order_relaxed); }
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
#if ALGERNON_OS_ANDROID
    std::atomic<bool> mShellPrint{false};
#endif
};

namespace detail {

/// Clamp snprintf return value to [0, capacity-1].
inline int clampLen(int len, int capacity) noexcept { return len < 0 ? 0 : (len >= capacity ? capacity - 1 : len); }

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

/// Returns the ANSI color code for the [L] badge, or nullptr for no color.
inline const char* levelColor(Level level) noexcept
{
    // clang-format off
    switch (level) {
        case Level::Verbose: return "\033[2m";        // dim — low visual priority
        case Level::Debug:   return "\033[36m";       // cyan
        case Level::Info:    return "\033[32m";       // green
        case Level::Warn:    return "\033[33m";       // yellow
        case Level::Error:   return "\033[31m";       // red
        case Level::Fatal:   return "\033[1;41;37m";  // bold, red bg, white text
        default:             return nullptr;
    }
    // clang-format on
}

#if ALGERNON_OS_ANDROID
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

/// Packed output-line result — keeps logPrint free of raw buffer arithmetic.
struct FmtResult
{
    char buf[896];
    int  hdrLen;  ///< length of "[tag][L] "; Android logcat reads buf+hdrLen
    int  outLen;  ///< total valid bytes in buf
};

/// Build the complete output line into @p r (output parameter — guaranteed zero-copy).
/// Format: "[tag][L] (file:line) body"  where (file:line) is omitted for V/D/I.
/// When color=true, [L] is wrapped with ANSI codes inline; hdrLen skips past them.
inline void formatOutBuf(FmtResult& r, const char* tag, Level level, bool color, const char* file, int line,
                         const char* fmt, va_list args) noexcept
{
    const char* lc = color ? levelColor(level) : nullptr;
    if (lc) {
        r.hdrLen = clampLen(std::snprintf(r.buf, sizeof(r.buf), "[%s]%s[%s]\033[0m ", tag, lc, levelTag(level)),
                            (int)sizeof(r.buf));
    } else {
        r.hdrLen = clampLen(std::snprintf(r.buf, sizeof(r.buf), "[%s][%s] ", tag, levelTag(level)), (int)sizeof(r.buf));
    }
    int prefixLen = r.hdrLen;
    if (file != nullptr) {
        prefixLen += clampLen(std::snprintf(r.buf + prefixLen, sizeof(r.buf) - prefixLen, "(%s:%d) ", file, line),
                              (int)sizeof(r.buf) - prefixLen);
    }
    int bodyLen = clampLen(std::vsnprintf(r.buf + prefixLen, sizeof(r.buf) - prefixLen, fmt, args),
                           (int)sizeof(r.buf) - prefixLen);
    r.outLen    = prefixLen + bodyLen;
}

inline void logPrint(Level level, const char* file, int line, const char* fmt, va_list args) noexcept
{
    // One-time warning if setTag() was never called.
    if (Config::get().tryConsumeTagWarning()) {
        std::fprintf(stderr,
                     "[unknown][W] log tag not set — "
                     "call log::Config::get().setTag(\"YourTag\") at startup\n");
    }

    const bool needFlush = (level >= Level::Warn);

#if ALGERNON_OS_ANDROID
    const int  prio         = toAndroidPriority(level);
    const bool shellEnabled = Config::get().isShellPrintEnabled();

    if (!shellEnabled) {
        // Fast path: logcat only — no outBuf, no mutex.
        // logd (API 21+) strips any trailing '\n' automatically.
        if (file == nullptr) {
            __android_log_vprint(prio, Config::get().getTag(), fmt, args);  // V/D/I: zero user-side snprintf
        } else {
            char bodyBuf[820];
            std::vsnprintf(bodyBuf, sizeof(bodyBuf), fmt, args);
            __android_log_print(prio, Config::get().getTag(), "(%s:%d) %s", file, line, bodyBuf);
        }
        return;
    }
#endif

    // stdout path: desktop, or Android with shellEnabled=true.
    // Color codes are baked into the [L] badge inside the buffer — no outer wrapping needed.
    FmtResult r;
    formatOutBuf(r, Config::get().getTag(), level, Config::get().isColorEnabled(), file, line, fmt, args);

#if ALGERNON_OS_ANDROID
    // Also emit to logcat; r.buf+hdrLen skips past the header (incl. any ANSI codes).
    __android_log_write(prio, Config::get().getTag(), r.buf + r.hdrLen);
#endif

    std::lock_guard<std::mutex> lk(Config::get().outputMutex());
    std::fwrite(r.buf, 1, (size_t)r.outLen, stdout);
    if (needFlush)
        std::fflush(stdout);
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

// ── Public logging macros ──

// Internal: level check + dispatch without location (V/D/I).
#define XLOG_IMPL(lvl, fmt, ...)                               \
    do {                                                       \
        if ((lvl) >= log::Config::get().getLevel()) {          \
            log::detail::logPrintF((lvl), fmt, ##__VA_ARGS__); \
        }                                                      \
    } while (0)

// Internal: level check + dispatch with (file:line) (W/E/F).
#define XLOG_IMPL_LOC(lvl, fmt, ...)                                                                         \
    do {                                                                                                     \
        if ((lvl) >= log::Config::get().getLevel()) {                                                        \
            log::detail::logPrintFLoc((lvl), log::detail::basename(__FILE__), __LINE__, fmt, ##__VA_ARGS__); \
        }                                                                                                    \
    } while (0)

// clang-format off
#define XLOG_V(fmt, ...) XLOG_IMPL(log::Level::Verbose, fmt, ##__VA_ARGS__)
#define XLOG_D(fmt, ...) XLOG_IMPL(log::Level::Debug,   fmt, ##__VA_ARGS__)
#define XLOG_I(fmt, ...) XLOG_IMPL(log::Level::Info,    fmt, ##__VA_ARGS__)
#define XLOG_W(fmt, ...) XLOG_IMPL_LOC(log::Level::Warn,  fmt, ##__VA_ARGS__)
#define XLOG_E(fmt, ...) XLOG_IMPL_LOC(log::Level::Error, fmt, ##__VA_ARGS__)
#define XLOG_F(fmt, ...) XLOG_IMPL_LOC(log::Level::Fatal, fmt, ##__VA_ARGS__)
// clang-format on

// ── Assertion / check macros ──

#define XCHECK(expr) assert(expr)

#define XCHECK_WITH_RET(expr, ret)                                                                  \
    do {                                                                                            \
        if (!(expr)) {                                                                              \
            log::detail::logPrintFLoc(log::Level::Error, log::detail::basename(__FILE__), __LINE__, \
                                      "check failed: '%s'\n", #expr);                               \
            return (ret);                                                                           \
        }                                                                                           \
    } while (0)

#define XCHECK_WITH_MSG(expr, ret, fmt, ...)                                                             \
    do {                                                                                                 \
        if (!(expr)) {                                                                                   \
            log::detail::logPrintFLoc(log::Level::Error, log::detail::basename(__FILE__), __LINE__, fmt, \
                                      ##__VA_ARGS__);                                                    \
            return (ret);                                                                                \
        }                                                                                                \
    } while (0)

#endif  // XLOGGER_H