#ifndef __XTHREAD_FLOW_IMPL_H__
#define __XTHREAD_FLOW_IMPL_H__

#include <functional>

#include "xthread_flow.h"
#include "logger.h"


namespace framework {

    inline Flow::Flow()
        : mIsInited(false), mWorkers(nullptr), mPipelines(nullptr)
    {
        ;
    }

    inline Flow::~Flow()
    {
        if (mWorkers != nullptr) {
            delete mWorkers;
            mWorkers = nullptr;
        }

        if (mPipelines != nullptr) {
            delete mPipelines;
            mPipelines = nullptr;
        }

        mIsInited = false;
    }

    inline int Flow::init(size_t workers, size_t pipelines)
    {
        ASSERTER_WITH_INFO(mIsInited == false, ECODE_INVALID_PARAM, "threadpool already inited!");
        ASSERTER_WITH_INFO(mWorkers == nullptr, ECODE_INVALID_PARAM, "threadpool already inited!");
        ASSERTER_WITH_INFO(mPipelines == nullptr, ECODE_INVALID_PARAM, "threadpool already inited!");

        mWorkers = new XThreadpool(workers);
        mPipelines = new XThreadpool(pipelines);

        ASSERTER_WITH_RET(mWorkers != nullptr, ECODE_NO_MEMORY);
        ASSERTER_WITH_RET(mPipelines != nullptr, ECODE_NO_MEMORY);

        mIsInited = true;

        return ECODE_SUCCESS;
    }

    inline int Flow::parallelizeTiledTasks(size_t range, size_t tile, std::function<void(size_t, size_t)>&& f)
    {
        ASSERTER_WITH_RET(mIsInited == true, ECODE_INVALID_PARAM);
        ASSERTER_WITH_RET(mWorkers != nullptr, ECODE_INVALID_PARAM);
        ASSERTER_WITH_RET(mPipelines != nullptr, ECODE_INVALID_PARAM);

        std::vector<std::future<void>> futures;
        for (size_t i = 0; i < range; i += tile) {
            futures.emplace_back(mWorkers->enqueue(f, i, std::min(range - i, tile)));
        }

        for (auto it = futures.rbegin(); it != futures.rend(); ++it) {
            it->get();
        }

        return ECODE_SUCCESS;
    }

    template<class F, class... Args>
    auto Flow::addPipeline(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>
    {
        ASSERTER(mIsInited == true);
        ASSERTER(mWorkers != nullptr);
        ASSERTER(mPipelines != nullptr);

        return mPipelines->enqueue(std::forward<F>(f), std::forward<Args>(args)...);
    }

} // namespace framework

#endif // __XTHREAD_FLOW_IMPL_H__