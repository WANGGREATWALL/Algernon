#ifndef ALGERNON_LOG_XLOGGER_H_
#define ALGERNON_LOG_XLOGGER_H_

/**
 * @file xlogger.h
 * @brief Lightweight logging framework with level filtering and colored output.
 *
 * @example
 *   algernon::XLogConfig::get().setTag("[MyApp]");
 *   algernon::XLogConfig::get().setLevel(algernon::LogLevel::Debug);
 *   XLOG_I("initialized, version=%s\n", ALGERNON_VERSION);
 *   XLOG_E("failed to open file: %s\n", path);
 */

#include <cstdio>
#include <cstdarg>
#include <atomic>
#include <mutex>
#include <cstring>
#include <cassert>

#ifdef __ANDROID__
#include <android/log.h>
#endif

#include "xerror.h"

namespace algernon {

// ============================================================================
// Log Levels (industry-standard order: small = most verbose)
// ============================================================================

enum class LogLevel : int {
    Verbose = 0,  ///< Finest-grained informational events
    Debug   = 1,  ///< Fine-grained informational events for debugging
    Info    = 2,  ///< Informational messages highlighting progress
    Warn    = 3,  ///< Potentially harmful situations
    Error   = 4,  ///< Error events that might still allow continued operation
    Fatal   = 5,  ///< Severe error events leading to abort
    Silent  = 6,  ///< Suppress all output
};

// ============================================================================
// Log Configuration Singleton
// ============================================================================

class XLogConfig {
public:
    static XLogConfig& get() {
        static XLogConfig instance;
        return instance;
    }

    void setLevel(LogLevel level) { mLevel.store(level, std::memory_order_relaxed); }
    LogLevel getLevel() const { return mLevel.load(std::memory_order_relaxed); }

    void setTag(const char* tag) {
        std::lock_guard<std::mutex> lock(mMutex);
        std::strncpy(mTag, tag, sizeof(mTag) - 1);
        mTag[sizeof(mTag) - 1] = '\0';
    }
    const char* getTag() {
        std::lock_guard<std::mutex> lock(mMutex);
        return mTag;
    }

    void setColorEnabled(bool enabled) { mColorEnabled.store(enabled, std::memory_order_relaxed); }
    bool isColorEnabled() const { return mColorEnabled.load(std::memory_order_relaxed); }

private:
    XLogConfig() = default;
    XLogConfig(const XLogConfig&) = delete;
    XLogConfig& operator=(const XLogConfig&) = delete;

    std::atomic<LogLevel> mLevel{LogLevel::Verbose};
    std::atomic<bool>     mColorEnabled{true};
    char                  mTag[64] = "[algernon]";
    std::mutex            mMutex;
};

// ============================================================================
// Internal print function
// ============================================================================

namespace detail {

inline const char* levelTag(LogLevel level) {
    switch (level) {
        case LogLevel::Verbose: return "[V]";
        case LogLevel::Debug:   return "[D]";
        case LogLevel::Info:    return "[I]";
        case LogLevel::Warn:    return "[W]";
        case LogLevel::Error:   return "[E]";
        case LogLevel::Fatal:   return "[F]";
        default:                return "[?]";
    }
}

inline const char* levelColor(LogLevel level) {
    switch (level) {
        case LogLevel::Verbose: return "\033[37m";    // gray
        case LogLevel::Debug:   return "\033[36m";    // cyan
        case LogLevel::Info:    return "\033[32m";    // green
        case LogLevel::Warn:    return "\033[33m";    // yellow
        case LogLevel::Error:   return "\033[31m";    // red
        case LogLevel::Fatal:   return "\033[41;37m"; // red bg, white text
        default:                return "\033[0m";
    }
}

inline void logPrint(LogLevel level, [[maybe_unused]] const char* file,
                     [[maybe_unused]] int line, const char* fmt, ...) {
    auto& cfg = XLogConfig::get();
    if (level < cfg.getLevel()) return;

    const char* tag   = cfg.getTag();
    const char* ltag  = levelTag(level);

#ifdef __ANDROID__
    int androidLevel = ANDROID_LOG_VERBOSE + static_cast<int>(level);
    va_list args;
    va_start(args, fmt);
    __android_log_vprint(androidLevel, tag, fmt, args);
    va_end(args);
#else
    if (cfg.isColorEnabled()) {
        std::printf("%s%s %s ", levelColor(level), tag, ltag);
    } else {
        std::printf("%s %s ", tag, ltag);
    }

    va_list args;
    va_start(args, fmt);
    std::vprintf(fmt, args);
    va_end(args);

    if (cfg.isColorEnabled()) {
        std::printf("\033[0m");
    }
#endif
}

} // namespace detail
} // namespace algernon

// ============================================================================
// Public Macros
// ============================================================================

#define XLOG_V(fmt, ...) algernon::detail::logPrint(algernon::LogLevel::Verbose, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define XLOG_D(fmt, ...) algernon::detail::logPrint(algernon::LogLevel::Debug,   __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define XLOG_I(fmt, ...) algernon::detail::logPrint(algernon::LogLevel::Info,    __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define XLOG_W(fmt, ...) algernon::detail::logPrint(algernon::LogLevel::Warn,    __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define XLOG_E(fmt, ...) algernon::detail::logPrint(algernon::LogLevel::Error,   __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define XLOG_F(fmt, ...) algernon::detail::logPrint(algernon::LogLevel::Fatal,   __FILE__, __LINE__, fmt, ##__VA_ARGS__)

// ============================================================================
// Assertion Macros
// ============================================================================

#define XASSERT(expr) assert(expr)

#define XASSERT_RET(expr, ret) \
    do { \
        if (!(expr)) { \
            XLOG_E("assertion '%s' failed! (%s:%d)\n", #expr, __FILE__, __LINE__); \
            return (ret); \
        } \
    } while (0)

#define XASSERT_INFO(expr, ret, fmt, ...) \
    do { \
        if (!(expr)) { \
            XLOG_E(fmt " (%s:%d)\n", ##__VA_ARGS__, __FILE__, __LINE__); \
            return (ret); \
        } \
    } while (0)

#endif // ALGERNON_LOG_XLOGGER_H_
