#ifndef __XTHREAD_FLOW_H__
#define __XTHREAD_FLOW_H__

#include "xthreadpool.h"


#define XTHREAD_PARALLELIZE_TILED_TASK_QUOTE(range, tile) \
    framework::Flow::get().parallelizeTiledTasks(range, tile, [&] (size_t _i, size_t _ti) {

#define XTHREAD_PARALLELIZE_TILED_TASK_ASSIGN(range, tile) \
    framework::Flow::get().parallelizeTiledTasks(range, tile, [=] (size_t _i, size_t _ti) {

#define XTHREAD_PARALLELIZE_END });


namespace framework {

    class Flow {
    public:
        static Flow& get() {
            static Flow instance;
            return instance;
        }

        ~Flow();

        Flow(const Flow&) = delete;
        Flow& operator=(const Flow&) = delete;

        int init(size_t workers, size_t pipelines);

        int parallelizeTiledTasks(size_t range, size_t tile, std::function<void(size_t, size_t)>&& f);

        template<class F, class... Args>
        auto addPipeline(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>;
    
    private:
        Flow();

        bool mIsInited;

        XThreadpool* mWorkers;
        XThreadpool* mPipelines;
    };

} // namespace framework

#include "xthread_flow.impl.h"

#endif // __XTHREAD_FLOW_H__