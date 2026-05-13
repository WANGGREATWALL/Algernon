#include "perf/xtracer1.h"

#include <cstdio>
#include <cstring>

#if AU_OS_ANDROID
#include <fcntl.h>
#include <unistd.h>
#endif

namespace au::perf {

namespace {

#if AU_OS_ANDROID

/// Lazily-opened process-wide trace_marker fd. C++11 static init is
/// thread-safe. The fd is intentionally never closed — the kernel reclaims
/// it on process exit.
static int getTraceFd() noexcept
{
    static int fd = []() {
        int f = open("/sys/kernel/debug/tracing/trace_marker", O_WRONLY | O_CLOEXEC);
        if (f < 0) {
            f = open("/sys/kernel/tracing/trace_marker", O_WRONLY | O_CLOEXEC);
        }
        return f;
    }();
    return fd;
}

/// Write a single trace record. Names are truncated to fit the stack buffer.
static void writeTraceMarker(char mode, int pid, const char* name, size_t nameLen) noexcept
{
    int fd = getTraceFd();
    if (fd < 0) {
        return;
    }

    char   buf[256];
    size_t n = 0;

    if (mode == 'B') {
        n = static_cast<size_t>(
            std::snprintf(buf, sizeof(buf), "B|%d|%.*s", pid,
                          static_cast<int>(nameLen), name));
    } else {
        n = static_cast<size_t>(
            std::snprintf(buf, sizeof(buf), "E|%d", pid));
    }

    if (n > 0) {
        (void)write(fd, buf, std::min(n, sizeof(buf) - 1));
    }
}

#endif  // AU_OS_ANDROID

}  // namespace

XTracer1Scoped::XTracer1Scoped(std::string_view name, int32_t level) noexcept
{
    mCfg = &XPerfContext1::defaultConfig();
    begin(name, level);
}

XTracer1Scoped::XTracer1Scoped(XPerfContext1& cfg, std::string_view name, int32_t level) noexcept
{
    mCfg = &cfg;
    begin(name, level);
}

void XTracer1Scoped::begin(std::string_view name, int32_t level) noexcept
{
#if AU_OS_ANDROID
    if (mCfg == nullptr || !mCfg->isEnabled() || level > mCfg->getTracerLevel()) {
        return;
    }

    mActive  = true;
    mNameLen = static_cast<uint8_t>(std::min(name.size(), kMaxName - 1));
    std::memcpy(mName.data(), name.data(), mNameLen);
    mName[mNameLen] = '\0';

    writeTraceMarker('B', getpid(), mName.data(), mNameLen);
#else
    (void)name;
    (void)level;
#endif
}

XTracer1Scoped::~XTracer1Scoped() noexcept
{
#if AU_OS_ANDROID
    if (!mActive) {
        return;
    }

    if (mSubOpen) {
        sub();
    }

    writeTraceMarker('E', getpid(), nullptr, 0);
#endif
}

void XTracer1Scoped::sub(std::string_view name) noexcept
{
#if AU_OS_ANDROID
    if (!mActive) {
        return;
    }

    if (mSubOpen) {
        writeTraceMarker('E', getpid(), nullptr, 0);
    }

    mSubOpen = true;

    // Truncate name to match begin() — fixes v3 bug where sub() wrote
    // arbitrarily long names to trace_marker.
    const size_t len = std::min(name.size(), kMaxName - 1);
    char         buf[kMaxName];
    std::memcpy(buf, name.data(), len);
    buf[len] = '\0';

    writeTraceMarker('B', getpid(), buf, len);
#else
    (void)name;
#endif
}

void XTracer1Scoped::sub() noexcept
{
#if AU_OS_ANDROID
    if (!mActive || !mSubOpen) {
        return;
    }

    writeTraceMarker('E', getpid(), nullptr, 0);
    mSubOpen = false;
#endif
}

}  // namespace au::perf
