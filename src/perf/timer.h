#ifndef __TIMER_H__
#define __TIMER_H__

#include <string>
#include <vector>
#include <thread>
#include <chrono>

#define SIZE_INDENT 2


namespace perf {

    void setTimerRootName(std::string name);

    /**
     * @brief set perf visible level
     * @param level if set to (-1), all level will be visible
     */
    void setPerfVisibleLevel(size_t level = -1);


    class Timer {
        using TimePoint = std::chrono::steady_clock::time_point;
        using Duration = std::chrono::duration<float, std::milli>;

    public:
        static void sleepfor(long long ms);
        static std::string getTimeFormatted(std::string fmt = "%Y-%m-%d-%H-%M-%S");

        Timer();

        void restart();

        // return duration in ms
        float count();

    private:
        TimePoint   mBegin;
        TimePoint   mEnd;
        Duration    mDuration;
    };


    class TimerScoped {
    public:
        TimerScoped(std::string name);
        ~TimerScoped();

        // finish the last sub node and start a new one
        void sub(std::string name);

        // just finish a sub node
        void sub();

        size_t getLevel() const;
    
    private:
        size_t      mLevelBegin;
    };

} // namespace perf


#endif // __TIMER_H__