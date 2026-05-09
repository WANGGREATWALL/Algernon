#include "log/xlogger.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <mutex>

#if AURA_OS_ANDROID
#include <android/log.h>
#endif

namespace au {
namespace log {
namespace {

std::mutex        gOutputMutex;
char              gTag[64] = "unknown";
std::atomic<bool> gTagWarned{false};

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
        default:                    return "?";
    }
    // clang-format on
}

inline const char* levelColor(Level level) noexcept
{
    // All badges use reverse video (SGR 7) so the level letter sits on a
    // coloured block — much more visible than a single coloured glyph.
    // Severity escalates: V (dim) < D < I < W < E < F (bold + blink).
    // clang-format off
    switch (level) {
        case Level::Verbose: return "\033[2;7m";      // dim + reverse — lowest priority
        case Level::Debug:   return "\033[7;36m";     // reverse cyan
        case Level::Info:    return "\033[7;32m";     // reverse green
        case Level::Warn:    return "\033[7;33m";     // reverse yellow
        case Level::Error:   return "\033[7;31m";     // reverse red
        case Level::Fatal:   return "\033[1;5;41;97m";// bold + blink + red bg + bright white fg
        default:                    return nullptr;
    }
    // clang-format on
}

#if AURA_OS_ANDROID
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
        default:                    return ANDROID_LOG_DEFAULT;
    }
    // clang-format on
}
#endif

struct FmtResult
{
    char buf[896];
    int  hdrLen;
    int  outLen;
};

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
    if (Config::get().tryConsumeTagWarning()) {
        std::fprintf(stderr,
                     "[unknown][W] log tag not set — "
                     "call Config::get().setTag(\"YourTag\") at startup\n");
    }

    const bool needFlush = (level >= Level::Warn);

#if AURA_OS_ANDROID
    const int  prio         = toAndroidPriority(level);
    const bool shellEnabled = Config::get().isShellPrintEnabled();

    if (!shellEnabled) {
        if (file == nullptr) {
            __android_log_vprint(prio, Config::get().getTag(), fmt, args);
        } else {
            char bodyBuf[820];
            std::vsnprintf(bodyBuf, sizeof(bodyBuf), fmt, args);
            __android_log_print(prio, Config::get().getTag(), "(%s:%d) %s", file, line, bodyBuf);
        }
        return;
    }
#endif

    FmtResult r;
    formatOutBuf(r, Config::get().getTag(), level, Config::get().isColorEnabled(), file, line, fmt, args);

#if AURA_OS_ANDROID
    __android_log_write(prio, Config::get().getTag(), r.buf + r.hdrLen);
#endif

    std::lock_guard<std::mutex> lk(gOutputMutex);
    std::fwrite(r.buf, 1, (size_t)r.outLen, stdout);
    if (needFlush)
        std::fflush(stdout);
}

}  // anonymous namespace

void Config::setTag(const char* tag) noexcept
{
    std::lock_guard<std::mutex> lock(gOutputMutex);
    std::strncpy(gTag, tag ? tag : "unknown", sizeof(gTag) - 1);
    gTag[sizeof(gTag) - 1] = '\0';
    gTagWarned.store(true, std::memory_order_relaxed);
}

const char* Config::getTag() const noexcept { return gTag; }

bool Config::tryConsumeTagWarning() noexcept
{
    if (gTagWarned.load(std::memory_order_relaxed))
        return false;
    bool expected = false;
    return gTagWarned.compare_exchange_strong(expected, true, std::memory_order_relaxed);
}

void detail::logPrintF(Level level, const char* fmt, ...) noexcept
{
    va_list args;
    va_start(args, fmt);
    logPrint(level, nullptr, 0, fmt, args);
    va_end(args);
}

void detail::logPrintFLoc(Level level, const char* file, int line, const char* fmt, ...) noexcept
{
    va_list args;
    va_start(args, fmt);
    logPrint(level, file, line, fmt, args);
    va_end(args);
}

}  // namespace log
}  // namespace au
