#ifndef ALGERNON_PERF_XTRACER_H_
#define ALGERNON_PERF_XTRACER_H_

/**
 * @file xtracer.h
 * @brief Scoped performance tracer with Android systrace integration.
 *
 * On Android, writes to the kernel trace_marker file for systrace/perfetto.
 * On other platforms, only produces timer tree output.
 *
 * @example
 *   {
 *       algernon::perf::XTracerScoped t("processFrame");
 *       t.sub("preprocess");
 *       // ... work ...
 *       t.sub("inference");
 *       // ... work ...
 *   }
 */

#include <memory>
#include <string>

#include "perf/xtimer.h"

namespace algernon {
namespace perf {

class XTracerScoped
{
public:
    explicit XTracerScoped(const std::string& name);
    ~XTracerScoped();

    /** @brief End current sub-node and start a new one. */
    void sub(const std::string& name);

    /** @brief End current sub-node without starting a new one. */
    void sub();

private:
    std::unique_ptr<XTimerScoped> mTimer;
    int                           mFdTrace = -1;
    std::string                   mNameMain;
    std::string                   mNameNode;
};

}  // namespace perf
}  // namespace algernon

#endif  // ALGERNON_PERF_XTRACER_H_
