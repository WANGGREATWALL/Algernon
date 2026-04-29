#ifndef ALGERNON_PERF_XTIMER_H_
#define ALGERNON_PERF_XTIMER_H_

/**
 * @file xtimer.h
 * @brief High-resolution timer and scoped timer tree for performance profiling.
 *
 * @example
 *   algernon::perf::XTimer t;
 *   // ... do work ...
 *   float ms = t.elapsed();
 *
 *   {
 *       algernon::perf::XTimerScoped s("myFunction");
 *       // ... measured block ...
 *       s.sub("phase1");
 *       // ... phase1 work ...
 *       s.sub("phase2");
 *       // ... phase2 work ...
 *   }  // prints timing tree on destruction
 */

#include <string>
#include <vector>
#include <chrono>
#include <cstddef>

namespace algernon { namespace perf {

/** @brief Set the root name of the global timer tree. */
void setTimerRootName(const std::string& name);

/** @brief Set max visible depth level for timer output (-1 = show all). */
void setPerfVisibleLevel(size_t level);

/** @brief Get the current visible depth level. */
size_t getPerfVisibleLevel();

class XTimer {
    using Clock    = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;
    using Duration  = std::chrono::duration<float, std::milli>;

public:
    /** @brief Sleep for given milliseconds. */
    static void sleepFor(long long ms);

    /** @brief Get current time as formatted string (default: "%Y-%m-%d-%H-%M-%S"). */
    static std::string getTimeFormatted(const std::string& fmt = "%Y-%m-%d-%H-%M-%S");

    XTimer();

    /** @brief Restart the timer. */
    void restart();

    /** @brief Get elapsed time in milliseconds since last restart/construction. */
    float elapsed();

private:
    TimePoint mBegin;
};

class XTimerScoped {
public:
    explicit XTimerScoped(const std::string& name);
    ~XTimerScoped();

    /** @brief End current sub-node and start a new one. */
    void sub(const std::string& name);

    /** @brief End current sub-node without starting a new one. */
    void sub();

    size_t getLevel() const;

private:
    size_t mLevelBegin;
};

}} // namespace algernon::perf

#endif // ALGERNON_PERF_XTIMER_H_
