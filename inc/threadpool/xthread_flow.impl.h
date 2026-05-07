#ifndef ALGERNON_THREADPOOL_XTHREAD_FLOW_IMPL_H_
#define ALGERNON_THREADPOOL_XTHREAD_FLOW_IMPL_H_

#include "xthread_flow.h"
#include "log/xlogger.h"

namespace algernon { namespace framework {

inline XFlow::XFlow() = default;

inline XFlow::~XFlow() = default;

inline int XFlow::init(size_t workers, size_t pipelines) {
    XASSERT_RET(!mInited, kErrorAlreadyExists);
    mWorkers   = std::make_unique<XThreadpool>(workers);
    mPipelines = std::make_unique<XThreadpool>(pipelines);
    mInited = true;
    return kSuccess;
}

inline int XFlow::parallelizeTiledTasks(size_t range, size_t tile,
                                         std::function<void(size_t, size_t)>&& f) {
    XASSERT_RET(mInited, kErrorNotReady);

    std::vector<std::future<void>> futures;
    for (size_t i = 0; i < range; i += tile) {
        futures.emplace_back(mWorkers->enqueue(f, i, std::min(range - i, tile)));
    }
    for (auto it = futures.rbegin(); it != futures.rend(); ++it) {
        it->get();
    }
    return kSuccess;
}

template <class F, class... Args>
auto XFlow::addPipeline(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>> {
    XASSERT(mInited);
    return mPipelines->enqueue(std::forward<F>(f), std::forward<Args>(args)...);
}

}} // namespace algernon::framework

#endif // ALGERNON_THREADPOOL_XTHREAD_FLOW_IMPL_H_
