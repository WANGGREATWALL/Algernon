#ifndef AURA_PERF_XTIMER2_H_
#define AURA_PERF_XTIMER2_H_

#include <chrono>
#include <string>

namespace au {
namespace perf {

/**
 * @brief Performance configuration parameters for a module.
 */
struct PerfConfig
{
    bool timerEnabled  = true;
    int  timerLevel    = 255;
    bool tracerEnabled = true;
    int  tracerLevel   = 255;
    bool treeMode      = false;  // Release by default
};

/**
 * @brief Independent performance context for managing log and tracer states.
 * Supports dynamic configuration via system properties.
 */
class XPerfContext
{
public:
    /**
     * @brief Construct an XPerfContext.
     * @param name Name of the context (e.g. module name).
     * @param timerEnProp System property name for timer enable.
     * @param timerLevelProp System property name for timer level.
     * @param tracerEnProp System property name for tracer enable.
     * @param tracerLevelProp System property name for tracer level.
     * @param treeModeProp System property name for tree mode enable.
     */
    XPerfContext(const std::string& name, const std::string& timerEnProp = "", const std::string& timerLevelProp = "",
                 const std::string& tracerEnProp = "", const std::string& tracerLevelProp = "",
                 const std::string& treeModeProp = "");

    /** @brief Reload properties from the system. */
    void reload();

    const char* getName() const { return mName.c_str(); }

    inline bool isTimerEnabled(int level) const { return mConfig.timerEnabled && level <= mConfig.timerLevel; }

    inline bool isTracerEnabled(int level) const { return mConfig.tracerEnabled && level <= mConfig.tracerLevel; }

    inline bool isTreeMode() const { return mConfig.treeMode; }

    /** @brief Manually override configuration. */
    void       setConfig(const PerfConfig& config) { mConfig = config; }
    PerfConfig getConfig() const { return mConfig; }

private:
    std::string mName;
    std::string mProps[5];
    PerfConfig  mConfig;
};

/** @brief Returns a default, global performance context. */
XPerfContext& getGlobalPerfContext();


/**
 * @brief Lightweight timer wrapping std::chrono.
 */
class XTimer2
{
    using Clock     = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;
    using Duration  = std::chrono::duration<float, std::milli>;

public:
    static void        sleepFor(long long ms);
    static std::string getTimeFormatted(const std::string& fmt = "%Y-%m-%d-%H-%M-%S");

    XTimer2() { restart(); }
    void  restart() { mBegin = Clock::now(); }
    float elapsed() const
    {
        auto end = Clock::now();
        return Duration(end - mBegin).count();
    }

private:
    TimePoint mBegin;
};


/**
 * @brief Scoped timer. In tree mode, outputs a hierarchical tree on destruction.
 * In release mode, outputs linear timeline logs sequentially.
 */
class XTimer2Scoped
{
public:
    /**
     * @brief Create a scoped timer.
     * @param ctx Perf context. If nullptr, falls back to global context.
     * @param name Tag name (must be a static string / string literal for performance).
     * @param level Visibility level.
     */
    XTimer2Scoped(XPerfContext* ctx, const char* name, int level = 0);
    ~XTimer2Scoped();

    /** @brief Starts a new sub-phase, implicitly closing the previous sub-phase. */
    void sub(const char* name);

    /** @brief Closes the current sub-phase without starting a new one. */
    void sub();

private:
    XPerfContext* mCtx;
    const char*   mName;
    const char*   mLastSubName;
    int           mLevel;
    bool          mActive;
    bool          mTreeMode;
    bool          mHasSub;

    XTimer2 mTimer;
    XTimer2 mSubTimer;
};

}  // namespace perf
}  // namespace au

#endif  // AURA_PERF_XTIMER2_H_
