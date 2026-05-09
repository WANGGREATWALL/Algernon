#ifndef AURA_PERF_XTRACER2_H_
#define AURA_PERF_XTRACER2_H_

#include "perf/xtimer2.h"

namespace au {
namespace perf {

/**
 * @brief Scoped tracer for Android SysTrace / Perfetto.
 * On Android, writes to the kernel trace_marker. On other OS, does nothing.
 */
class XTracer2Scoped
{
public:
    /**
     * @brief Create a scoped tracer.
     * @param ctx Perf context. If nullptr, falls back to global context.
     * @param name Tag name (must be a static string / string literal).
     * @param level Visibility level.
     */
    XTracer2Scoped(XPerfContext* ctx, const char* name, int level = 0);
    ~XTracer2Scoped();

    /** @brief Starts a new sub-phase, implicitly closing the previous sub-phase. */
    void sub(const char* name);

    /** @brief Closes the current sub-phase without starting a new one. */
    void sub();

private:
    XPerfContext* mCtx;
    const char*   mName;
    const char*   mLastSubName;
    bool          mActive;
    bool          mHasSub;
};


/**
 * @brief Combined scoped performance monitor.
 * Automatically utilizes both XTimer2Scoped and XTracer2Scoped based on context configuration.
 */
class XPerfScoped
{
public:
    XPerfScoped(XPerfContext* ctx, const char* name, int timerLevel = 0, int tracerLevel = 0)
        : mTimer(ctx, name, timerLevel), mTracer(ctx, name, tracerLevel)
    {
    }

    void sub(const char* name)
    {
        mTimer.sub(name);
        mTracer.sub(name);
    }

    void sub()
    {
        mTimer.sub();
        mTracer.sub();
    }

private:
    XTimer2Scoped  mTimer;
    XTracer2Scoped mTracer;
};

}  // namespace perf
}  // namespace au

#endif  // AURA_PERF_XTRACER2_H_
