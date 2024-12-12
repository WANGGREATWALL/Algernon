#ifndef __XTHREADPOOL_IMPL_H__
#define __XTHREADPOOL_IMPL_H__

#include <algorithm>
#include "xthreadpool.h"
#include "logger.h"

#ifdef __unix__
#include "sched.h"
#include "unistd.h"
#endif


namespace framework {

    inline XThreadpool::XThreadpool(size_t threads)
        : mIsStopped(false)
    {
        for (size_t i = 0; i < threads; ++i)
            mWorkers.emplace_back(
                [this] {
                    for (;;) {
                        std::function<void()> task;

                        {
                            std::unique_lock<std::mutex> lock(this->mMutexQueue);
                            this->mCondition.wait(lock, [this]{ return this->mIsStopped || !this->mTasks.empty(); });
                            if (this->mIsStopped && this->mTasks.empty())
                                return;
                            task = std::move(this->mTasks.front());
                            this->mTasks.pop();
                        }
                        task();
                    }
                }
            );
        
        setaffinity(4, 7);
    }

    template<class F, class... Args>
    auto XThreadpool::enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>
    {
        using return_type = typename std::result_of<F(Args...)>::type;

        auto task = std::make_shared< std::packaged_task<return_type()> >(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        std::future<return_type> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(mMutexQueue);

            // don't allow enqueueing after stopping the pool
            if (mIsStopped)
                throw std::runtime_error("enqueue on stopped threadpool");
            mTasks.emplace([task](){ (*task)(); });
        }
        mCondition.notify_one();
        return res;
    }

    inline int XThreadpool::setaffinity(size_t cpu0, size_t cpu1)
    {
#ifdef __unix__
        cpu0 = std::max(std::min(cpu0, 7), 0);
        cpu1 = std::max(std::min(cpu1, 7), 0);

        cpu_set_t cpuset;

        // check current affinity, use CPU_ISSET(i, &cpuset) to check cpuId
        // sched_getaffinity(0, sizeof(cpuset), &cpuset);

        CPU_ZERO(&cpuset); // clear cpuset
        for (size_t i = cpu0; i <= cpu1; ++i) {
            CPU_SET(i, &cpuset);
        }

        for (auto& t : mWorkers) {
            pthread_t native_thread = t.native_handle();
            int retSetAffinity = sched_setaffinity(pthread_gettid_np(native_thread), sizeof(cpuset), &cpuset);
            ASSERTER_WITH_RET(retSetAffinity == 0, EXIT_FAILURE);
        }
#endif

        return EXIT_SUCCESS;
    }

    inline XThreadpool::~XThreadpool()
    {
        {
            std::unique_lock<std::mutex> lock(mMutexQueue);
            mIsStopped = true;
        }
        mCondition.notify_all();

        for (std::thread& worker : mWorkers)
            worker.join();
    }
    

} // namespace framework

#endif // __XTHREADPOOL_IMPL_H__