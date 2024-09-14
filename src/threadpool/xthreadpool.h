#ifndef __XTHREADPOOL_H__
#define __XTHREADPOOL_H__

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>


namespace framework {

    class XThreadpool {
    public:
        XThreadpool(size_t threads);
        ~XThreadpool();

        template<class F, class... Args>
        auto enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>;
    
    private:
        int setaffinity(size_t cpu0, size_t cpu1);
    
    private:
        std::vector<std::thread> mWorkers;
        std::queue<std::function<void()>> mTasks;
        std::mutex mMutexQueue;
        std::condition_variable mCondition;
        bool mIsStopped;
    };

} // namespace framework

#include "xthreadpool.impl.h"

#endif // __XTHREADPOOL_H__