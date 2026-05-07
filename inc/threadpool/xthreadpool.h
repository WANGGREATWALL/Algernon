#ifndef ALGERNON_THREADPOOL_XTHREADPOOL_H_
#define ALGERNON_THREADPOOL_XTHREADPOOL_H_

/**
 * @file xthreadpool.h
 * @brief Thread pool for concurrent task execution.
 *
 * @example
 *   algernon::framework::XThreadpool pool(4);
 *   auto f1 = pool.enqueue([](int a) { return a * 2; }, 21);
 *   auto f2 = pool.enqueue([]() { return std::string("hello"); });
 *   printf("%d %s\n", f1.get(), f2.get().c_str());
 */

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

namespace algernon { namespace framework {

class XThreadpool {
public:
    explicit XThreadpool(size_t threads);
    ~XThreadpool();

    XThreadpool(const XThreadpool&) = delete;
    XThreadpool& operator=(const XThreadpool&) = delete;

    /**
     * @brief Enqueue a task and get a future for its result.
     * @tparam F Callable type.
     * @tparam Args Argument types.
     * @return std::future holding the return value.
     */
    template <class F, class... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>>;

private:
    std::vector<std::thread>           mWorkers;
    std::queue<std::function<void()>>  mTasks;
    std::mutex                         mMutex;
    std::condition_variable            mCondition;
    bool                               mStopped = false;
};

}} // namespace algernon::framework

#include "xthreadpool.impl.h"

#endif // ALGERNON_THREADPOOL_XTHREADPOOL_H_
