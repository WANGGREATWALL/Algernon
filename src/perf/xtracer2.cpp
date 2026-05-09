#include "perf/xtracer2.h"

#include "sys/xplatform.h"

#ifdef AURA_OS_ANDROID
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <cstdio>
#endif

namespace au {
namespace perf {

namespace {

class TracerMarker
{
public:
    static TracerMarker& get()
    {
        static TracerMarker instance;
        return instance;
    }

    void writeBegin(const char* name)
    {
#ifdef AURA_OS_ANDROID
        if (mFd < 0)
            return;
        char buf[256];
        int  len = snprintf(buf, sizeof(buf), "B|%d|%s", mPid, name);
        if (len > 0) {
            write(mFd, buf, std::min(len, (int)sizeof(buf) - 1));
        }
#else
        (void)name;
#endif
    }

    void writeEnd(const char* name)
    {
#ifdef AURA_OS_ANDROID
        if (mFd < 0)
            return;
        char buf[256];
        int  len = snprintf(buf, sizeof(buf), "E|%d|%s", mPid, name);
        if (len > 0) {
            write(mFd, buf, std::min(len, (int)sizeof(buf) - 1));
        }
#else
        (void)name;
#endif
    }

private:
    TracerMarker()
    {
#ifdef AURA_OS_ANDROID
        mFd = open("/sys/kernel/tracing/trace_marker", O_WRONLY | O_CLOEXEC);
        if (mFd == -1) {
            mFd = open("/sys/kernel/debug/tracing/trace_marker", O_WRONLY | O_CLOEXEC);
        }
        mPid = getpid();
#endif
    }

    ~TracerMarker()
    {
#ifdef AURA_OS_ANDROID
        if (mFd >= 0) {
            close(mFd);
        }
#endif
    }

#ifdef AURA_OS_ANDROID
    int mFd  = -1;
    int mPid = 0;
#endif
};

}  // anonymous namespace

// ── XTracer2Scoped ──

XTracer2Scoped::XTracer2Scoped(XPerfContext* ctx, const char* name, int level)
    : mCtx(ctx ? ctx : &getGlobalPerfContext()), mName(name), mLastSubName(nullptr), mHasSub(false)
{
    mActive = mCtx->isTracerEnabled(level);
    if (mActive) {
        TracerMarker::get().writeBegin(name);
    }
}

void XTracer2Scoped::sub(const char* name)
{
    if (!mActive)
        return;

    if (mHasSub) {
        TracerMarker::get().writeEnd(mLastSubName);
    }
    TracerMarker::get().writeBegin(name);
    mHasSub      = true;
    mLastSubName = name;
}

void XTracer2Scoped::sub()
{
    if (!mActive)
        return;

    if (mHasSub) {
        TracerMarker::get().writeEnd(mLastSubName);
        mHasSub      = false;
        mLastSubName = nullptr;
    }
}

XTracer2Scoped::~XTracer2Scoped()
{
    if (!mActive)
        return;

    if (mHasSub) {
        TracerMarker::get().writeEnd(mLastSubName);
    }
    TracerMarker::get().writeEnd(mName);
}

}  // namespace perf
}  // namespace au
