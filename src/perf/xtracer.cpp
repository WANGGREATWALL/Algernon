#include "perf/xtracer.h"

#include "log/xlogger.h"

#ifdef ALGERNON_OS_ANDROID
#include <fcntl.h>
#include <unistd.h>
#endif

namespace algernon {
namespace perf {

namespace {

enum TraceMode
{
    kBegin = 0,
    kEnd
};

int openTraceFile()
{
#ifdef ALGERNON_OS_ANDROID
    int fd = open("/sys/kernel/debug/tracing/trace_marker", O_WRONLY);
    if (fd == -1)
        fd = open("/sys/kernel/tracing/trace_marker", O_WRONLY);
    return fd;
#else
    return -1;
#endif
}

void closeTraceFile(int fd)
{
#ifdef ALGERNON_OS_ANDROID
    if (fd >= 0)
        close(fd);
#else
    (void)fd;
#endif
}

void writeTraceMessage([[maybe_unused]] int fd, [[maybe_unused]] const std::string& name,
                       [[maybe_unused]] TraceMode mode)
{
#ifdef ALGERNON_OS_ANDROID
    if (fd < 0)
        return;
    std::string msg = (mode == kBegin ? "B|" : "E|") + std::to_string(getpid()) + "|" + name;
    write(fd, msg.c_str(), msg.size());
#endif
}

}  // anonymous namespace

XTracerScoped::XTracerScoped(const std::string& name)
{
    mTimer = std::make_unique<XTimerScoped>(name);
    if (mTimer->getLevel() <= getPerfVisibleLevel()) {
        mNameMain = name;
        mFdTrace  = openTraceFile();
        writeTraceMessage(mFdTrace, name, kBegin);
    }
}

XTracerScoped::~XTracerScoped()
{
    if (mTimer && mTimer->getLevel() <= getPerfVisibleLevel()) {
        if (!mNameNode.empty()) {
            writeTraceMessage(mFdTrace, mNameNode, kEnd);
        }
        writeTraceMessage(mFdTrace, mNameMain, kEnd);
        closeTraceFile(mFdTrace);
    }
}

void XTracerScoped::sub(const std::string& name)
{
    if (mTimer && mTimer->getLevel() < getPerfVisibleLevel()) {
        mTimer->sub(name);
        if (!mNameNode.empty())
            writeTraceMessage(mFdTrace, mNameNode, kEnd);
        mNameNode = name;
        writeTraceMessage(mFdTrace, name, kBegin);
    }
}

void XTracerScoped::sub()
{
    if (mTimer && mTimer->getLevel() < getPerfVisibleLevel()) {
        mTimer->sub();
        if (!mNameNode.empty()) {
            writeTraceMessage(mFdTrace, mNameNode, kEnd);
            mNameNode.clear();
        }
    }
}

}  // namespace perf
}  // namespace algernon
