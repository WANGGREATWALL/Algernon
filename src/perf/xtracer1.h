#ifndef AURA_PERF_XTRACER1_H_
#define AURA_PERF_XTRACER1_H_

/**
 * @file xtracer1.h
 * @brief Android-only Perfetto / ftrace trace_marker helper.
 *
 * On non-Android platforms every function body is compiled away to nothing,
 * so there is zero runtime cost and negligible .text footprint.
 *
 * On Android:
 *  - A single process-wide fd is opened lazily (C++11 static init) with
 *    O_CLOEXEC and never explicitly closed.
 *  - Each scope writes "B|pid|name" on construction and "E|pid" on
 *    destruction. ftrace guarantees per-write atomicity up to PAGE_SIZE.
 *  - Scopes support inline sibling splits via sub(name) / sub().
 *
 * This module is intentionally decoupled from xtimer1.h. Use the composite
 * AU_PERF1_SCOPE macro to declare both a timer and a tracer in one line.
 */

#include <array>
#include <cstdint>
#include <string_view>

#include "perf/xtimer1.h"
#include "sys/xplatform.h"

namespace au::perf {

/// RAII scoped Perfetto tracer. Active only on Android.
class XTracer1Scoped
{
public:
    /// Use the default process-wide config.
    explicit XTracer1Scoped(std::string_view name, int32_t level = 0) noexcept;

    /// Use a caller-specific config.
    XTracer1Scoped(XPerfContext1& cfg, std::string_view name, int32_t level = 0) noexcept;

    ~XTracer1Scoped() noexcept;

    XTracer1Scoped(const XTracer1Scoped&)            = delete;
    XTracer1Scoped& operator=(const XTracer1Scoped&) = delete;

    /// End previous sub-slice (if any) and open a new one named @p name.
    void sub(std::string_view name) noexcept;

    /// End previous sub-slice (if any).
    void sub() noexcept;

private:
    void begin(std::string_view name, int32_t level) noexcept;

    XPerfContext1* mCfg{nullptr};
    bool           mActive{false};
    bool           mSubOpen{false};

    static constexpr size_t    kMaxName = 128;
    std::array<char, kMaxName> mName{};
    uint8_t                    mNameLen{0};
};

}  // namespace au::perf

// ---------------------------------------------------------------------------
// Public macros
// ---------------------------------------------------------------------------

#define AU_TRACE1(name) \
    ::au::perf::XTracer1Scoped AU_PERF_UNIQUE1(_auTrc1_)((name))

#define AU_TRACE1_L(name, lv) \
    ::au::perf::XTracer1Scoped AU_PERF_UNIQUE1(_auTrc1_)((name), (lv))

#define AU_TRACE1_CFG(cfg, name, lv) \
    ::au::perf::XTracer1Scoped AU_PERF_UNIQUE1(_auTrc1_)((cfg), (name), (lv))

// Composite: timer + tracer with the same name in one line.
#define AU_PERF1_SCOPE(name) \
    AU_TIMER1(name);         \
    AU_TRACE1(name)

#define AU_PERF1_SCOPE_L(name, lv) \
    AU_TIMER1_L(name, lv);         \
    AU_TRACE1_L(name, lv)

#define AU_PERF1_SCOPE_CFG(cfg, name, lv) \
    AU_TIMER1_CFG(cfg, name, lv);         \
    AU_TRACE1_CFG(cfg, name, lv)

#endif  // AURA_PERF_XTRACER1_H_
