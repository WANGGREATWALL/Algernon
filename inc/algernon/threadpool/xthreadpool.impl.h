#ifndef ALGERNON_THREADPOOL_XTHREADPOOL_IMPL_H_
#define ALGERNON_THREADPOOL_XTHREADPOOL_IMPL_H_

#include "xthreadpool.h"

namespace algernon { namespace framework {

inline XThreadpool::XThreadpool(size_t threads) {
    for (size_t i = 0; i < threads; ++i) {
        mWorkers.emplace_back([this] {
            for (;;) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(mMutex);
                    mCondition.wait(lock, [this] { return mStopped || !mTasks.empty(); });
                    if (mStopped && mTasks.empty()) return;
                    task = std::move(mTasks.front());
                    mTasks.pop();
                }
                task();
            }
        });
    }
}

template <class F, class... Args>
auto XThreadpool::enqueue(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>> {
    using return_type = std::invoke_result_t<F, Args...>;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(mMutex);
        if (mStopped)
            throw std::runtime_error("enqueue on stopped threadpool");
        mTasks.emplace([task]() { (*task)(); });
    }
    mCondition.notify_one();
    return res;
}

inline XThreadpool::~XThreadpool() {
    {
        std::unique_lock<std::mutex> lock(mMutex);
        mStopped = true;
    }
    mCondition.notify_all();
    for (auto& worker : mWorkers) {
        worker.join();
    }
}

}} // namespace algernon::framework

#endif // ALGERNON_THREADPOOL_XTHREADPOOL_IMPL_H_
