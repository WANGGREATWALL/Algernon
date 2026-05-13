#include "perf/xtracer3.h"

#include <cstdio>
#include <cstring>

#if AU_OS_ANDROID
#include <fcntl.h>
#include <unistd.h>
#endif

namespace au::perf {

namespace {

#if AU_OS_ANDROID
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

static void writeTraceMarker(char mode, int pid, const char* name, size_t nameLen) noexcept
{
    int fd = getTraceFd();
    if (fd < 0) {
        return;
    }

    char   buf[256];
    size_t len = 0;

    if (mode == 'B') {
        // "B|pid|name"
        len = static_cast<size_t>(std::snprintf(buf, sizeof(buf), "B|%d|%.*s", pid, static_cast<int>(nameLen), name));
    } else {
        // "E|pid"
        len = static_cast<size_t>(std::snprintf(buf, sizeof(buf), "E|%d", pid));
    }

    if (len > 0) {
        (void)write(fd, buf, std::min(len, sizeof(buf) - 1));
    }
}
#endif

}  // namespace

XTracer3Scoped::XTracer3Scoped(std::string_view name, int32_t level) noexcept
{
    mCfg = &XPerfContext3::defaultConfig();
    begin(name, level);
}

XTracer3Scoped::XTracer3Scoped(XPerfContext3& cfg, std::string_view name, int32_t level) noexcept
{
    mCfg = &cfg;
    begin(name, level);
}

void XTracer3Scoped::begin(std::string_view name, int32_t level) noexcept
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

XTracer3Scoped::~XTracer3Scoped() noexcept
{
#if AU_OS_ANDROID
    if (!mActive) {
        return;
    }

    if (mSubOpen) {
        sub();  // close sub
    }

    writeTraceMarker('E', getpid(), nullptr, 0);
#endif
}

void XTracer3Scoped::sub(std::string_view name) noexcept
{
#if AU_OS_ANDROID
    if (!mActive) {
        return;
    }

    if (mSubOpen) {
        writeTraceMarker('E', getpid(), nullptr, 0);
    }

    mSubOpen = true;
    writeTraceMarker('B', getpid(), name.data(), name.size());
#else
    (void)name;
#endif
}

void XTracer3Scoped::sub() noexcept
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