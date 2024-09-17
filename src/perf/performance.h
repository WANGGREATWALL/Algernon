#ifndef __PERFORMANCE_H__
#define __PERFORMANCE_H__

#include <string>
#include "timer.h"


namespace perf {

    class TracerScoped {
    public:
        TracerScoped(std::string name);
        ~TracerScoped();

        // finish the last sub node and start a new one
        void sub(std::string name);

        // just finish a sub node
        void sub();
    
    private:
        TimerScoped*    mTimer;
        int             mFdTrace;

        std::string     mNameMain;
        std::string     mNameNode;
    };

} // namespace perf

#endif // __PERFORMANCE_H__