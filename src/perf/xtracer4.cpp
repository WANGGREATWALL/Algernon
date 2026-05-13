#include "perf/xtracer4.h"

#include <algorithm>
#include <cstdio>
#include <cstring>

#include "sys/xplatform.h"

#if AU_OS_ANDROID
#include <fcntl.h>
#include <unistd.h>
#endif


namespace au {
namespace perf {


// ===========================================================================
//  Process-wide trace_marker fd helper (Android only)
// ===========================================================================

namespace {


#if AU_OS_ANDROID

/// Lazily-opened process-wide trace_marker fd. C++11 static-init is
/// thread-safe. The fd is intentionally never closed 鈥?the kernel reclaims
/// it on process exit, and O_CLOEXEC keeps fork+exec children clean.
int getTraceFd4() noexcept
{
    static int fd = []() {
        int f = ::open("/sys/kernel/debug/tracing/trace_marker", O_WRONLY | O_CLOEXEC);
        if (f < 0) {
            f = ::open("/sys/kernel/tracing/trace_marker", O_WRONLY | O_CLOEXEC);
        }
        return f;
    }();
    return fd;
}


/// One trace_marker write. Format: "B|pid|name" or "E|pid".
/// The kernel guarantees per-write atomicity up to one page, which is
/// already far above our 256-byte cap.
void writeTraceMarker4(char mode, int pid, const char* name, std::size_t nameLen) noexcept
{
    int fd = getTraceFd4();
    if (fd < 0) {
        return;
    }

    char        buf[256];
    std::size_t n = 0;

    if (mode == 'B') {
        const int rc = std::snprintf(buf, sizeof(buf), "B|%d|%.*s", pid, static_cast<int>(nameLen), name);
        if (rc <= 0) {
            return;
        }
        n = (static_cast<std::size_t>(rc) >= sizeof(buf)) ? sizeof(buf) - 1 : static_cast<std::size_t>(rc);
    } else {
        const int rc = std::snprintf(buf, sizeof(buf), "E|%d", pid);
        if (rc <= 0) {
            return;
        }
        n = static_cast<std::size_t>(rc);
    }

    (void)::write(fd, buf, n);
}

#endif  // AU_OS_ANDROID


}  // anonymous namespace


// ===========================================================================
//  XTracer4Scoped
// ===========================================================================

XTracer4Scoped::XTracer4Scoped(const char* name, int32_t level) noexcept
{
    begin(XPerfContext4::defaultContext(), name, name != nullptr ? std::strlen(name) : 0u, level);
}


XTracer4Scoped::XTracer4Scoped(const char* name, std::size_t nameLen, int32_t level) noexcept
{
    begin(XPerfContext4::defaultContext(), name, nameLen, level);
}


XTracer4Scoped::XTracer4Scoped(XPerfContext4& ctx, const char* name, int32_t level) noexcept
{
    begin(ctx, name, name != nullptr ? std::strlen(name) : 0u, level);
}


XTracer4Scoped::XTracer4Scoped(XPerfContext4& ctx, const char* name, std::size_t nameLen, int32_t level) noexcept
{
    begin(ctx, name, nameLen, level);
}


void XTracer4Scoped::begin(XPerfContext4& ctx, const char* name, std::size_t nameLen, int32_t level) noexcept
{
    mCtx     = &ctx;
    mActive  = false;
    mSubOpen = false;
    mNameLen = 0;
    mName[0] = '\0';

    if (!ctx.isEnabled()) {
        return;
    }
    if (level > ctx.getTracerLevel()) {
        return;
    }

    // Copy + truncate the label even on non-Android paths so any future
    // sink (e.g. an in-memory trace ring for desktop) can read it back.
    const std::size_t cp = std::min(nameLen, kMaxName - 1);
    if (name != nullptr && cp > 0) {
        std::memcpy(mName, name, cp);
    }
    mName[cp] = '\0';
    mNameLen  = static_cast<uint8_t>(cp);
    mActive   = true;

#if AU_OS_ANDROID
    writeTraceMarker4('B', getpid(), mName, mNameLen);
#endif
}


XTracer4Scoped::~XTracer4Scoped() noexcept
{
    if (!mActive) {
        return;
    }

    if (mSubOpen) {
        sub();  // close the inflight sub before the outer slice
    }

#if AU_OS_ANDROID
    writeTraceMarker4('E', getpid(), nullptr, 0);
#endif
}


void XTracer4Scoped::sub(const char* name) noexcept
{
    sub(name, name != nullptr ? std::strlen(name) : 0u);
}


void XTracer4Scoped::sub(const char* name, std::size_t nameLen) noexcept
{
    if (!mActive) {
        return;
    }

#if AU_OS_ANDROID
    if (mSubOpen) {
        writeTraceMarker4('E', getpid(), nullptr, 0);
    }
#endif

    mSubOpen = true;

#if AU_OS_ANDROID
    // Truncate the sub name to the same kMaxName ceiling. This is the bug
    // v3 had: it forwarded an unbounded length to snprintf and relied on
    // its truncation; passing the bound up front is both faster and safer.
    char        buf[kMaxName];
    std::size_t cp = std::min(nameLen, kMaxName - 1);
    if (name != nullptr && cp > 0) {
        std::memcpy(buf, name, cp);
    }
    buf[cp] = '\0';
    writeTraceMarker4('B', getpid(), buf, cp);
#else
    (void)name;
    (void)nameLen;
#endif
}


void XTracer4Scoped::sub() noexcept
{
    if (!mActive || !mSubOpen) {
        return;
    }

#if AU_OS_ANDROID
    writeTraceMarker4('E', getpid(), nullptr, 0);
#endif
    mSubOpen = false;
}


}  // namespace perf
}  // namespace au