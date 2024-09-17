#include "performance.h"

#ifdef __ANDROID__
#include <fcntl.h> // for open()
#include <unistd.h> // for getpid()
#endif

#define CHECK_TRACER_FD(fd) if (fd == -1) { return; }

#define END_LAST_SUB_NODE() if (!mNameNode.empty()) { writeMessage(mFdTrace, mNameNode, END); }


namespace perf {

    extern size_t LevelVisible; // define in timer.cpp

    enum Mode {
        BEGIN = 0,
        END
    };

    inline int openTraceFile()
    {
        int fd = -1;
#ifdef __ANDROID__
        fd = open("/sys/kernel/debug/tracing/trace_marker", O_WRONLY);
        if (fd == -1) {
            fd = open("/sys/kernel/tracing/trace_marker", O_WRONLY);
        }
#endif
        return fd;
    }

    inline void closeTraceFile(int fd)
    {
        CHECK_TRACER_FD(fd);
#ifdef __ANDROID__
        close(fd);
#endif
    }

    inline void writeMessage(int fd, std::string name, Mode mode)
    {
        CHECK_TRACER_FD(fd);
        std::string modeStr = mode == BEGIN ? "B|" : "E|";
#ifdef __ANDROID__
        std::string message = modeStr + std::to_string(getpid()) + "|" + name;
        write(fd, message.c_str(), message.size());
#endif
    }

    TracerScoped::TracerScoped(std::string name)
    {
        mTimer = new TimerScoped(name);

        if (mTimer->getLevel() <= LevelVisible) {
            mNameMain = name;
            mFdTrace = openTraceFile();
            writeMessage(mFdTrace, name, BEGIN);
        }
    }

    TracerScoped::~TracerScoped()
    {
        if (mTimer->getLevel() <= LevelVisible) {
            END_LAST_SUB_NODE();
            writeMessage(mFdTrace, mNameMain, END);
            closeTraceFile(mFdTrace);
        }

        delete mTimer;
        mTimer = nullptr;
    }

    void TracerScoped::sub(std::string name)
    {
        if (mTimer->getLevel() < LevelVisible) {
            mTimer->sub(name);
            END_LAST_SUB_NODE();
            writeMessage(mFdTrace, mNameNode = name, BEGIN);
        }
    }

    void TracerScoped::sub()
    {
        if (mTimer->getLevel() < LevelVisible) {
            mTimer->sub();
            END_LAST_SUB_NODE();
        }
    }

} // namespace perf