#ifndef AURA_PERF_XTRACER3_H_
#define AURA_PERF_XTRACER3_H_

/**
 * @file xtracer3.h
 * @brief Android-only perfetto / ftrace trace_marker helper (v3).
 *
 * On non-Android platforms every function is compiled into a no-op and every
 * class member is elided, so zero runtime cost and no .text footprint.
 *
 * On Android:
 *  - A single process-wide file descriptor is opened lazily on first use and
 *    closed on program exit.
 *  - Each scope writes one "B|pid|name" slice-begin line on construction and
 *    one "E|pid" slice-end line on destruction. Writes to trace_marker are
 *    atomic and reentrant on the kernel side (ftrace guarantees per-write
 *    record boundaries), so no userspace lock is needed.
 *  - Scopes support inline sibling splits via sub(name) / sub().
 *
 * This module is intentionally decoupled from xtimer3.h. Users wanting both
 * a hierarchical log AND a perfetto slice at the same point should either
 * declare both scopes or use the composite AU_PERF3_SCOPE macro defined below.
 */

#include <array>
#include <cstdint>
#include <string_view>

#include "perf/xtimer3.h"
#include "sys/xplatform.h"

namespace au::perf {

/// RAII scoped perfetto tracer. Active only on Android (or tracing-enabled
/// targets that expose /sys/kernel[/debug]/tracing/trace_marker).
class XTracer3Scoped
{
public:
    /// Use the default process-wide XPerfContext3.
    explicit XTracer3Scoped(std::string_view name, int32_t level = 0) noexcept;

    /// Use a caller-specific XPerfContext3 instance.
    XTracer3Scoped(XPerfContext3& cfg, std::string_view name, int32_t level = 0) noexcept;

    ~XTracer3Scoped() noexcept;

    XTracer3Scoped(const XTracer3Scoped&)            = delete;
    XTracer3Scoped& operator=(const XTracer3Scoped&) = delete;

    /// End the previously-open sub slice (if any) and open a new one named @p name.
    /// No-op when this scope is inactive.
    void sub(std::string_view name) noexcept;

    /// End the previously-open sub slice (if any). No-op otherwise.
    void sub() noexcept;

private:
    /// Shared constructor helper.
    void begin(std::string_view name, int32_t level) noexcept;

    XPerfContext3* mCfg{nullptr};
    bool           mActive{false};
    bool           mSubOpen{false};

    // Fixed-size name buffers; avoid heap allocations on the hot path.
    // Anything longer than kMaxName-1 is silently truncated.
    static constexpr size_t    kMaxName = 128;
    std::array<char, kMaxName> mName{};
    uint8_t                    mNameLen{0};
};

}  // namespace au::perf

// ---------------------------------------------------------------------------
// Public macros (complementing those in xtimer3.h).
// ---------------------------------------------------------------------------

#define AU_TRACE3(name) ::au::perf::XTracer3Scoped AU_PERF_UNIQUE(_auTracer3_)((name))
#define AU_TRACE3_L(name, lv) ::au::perf::XTracer3Scoped AU_PERF_UNIQUE(_auTracer3_)((name), (lv))
#define AU_TRACE3_CFG(cfg, name, lv) ::au::perf::XTracer3Scoped AU_PERF_UNIQUE(_auTracer3_)((cfg), (name), (lv))

// Composite: declare both a tree timer and a perfetto slice with the same name
// in one line, ensuring perfetto slices and hierarchical log entries align.
#define AU_PERF3_SCOPE(name) \
    AU_TIMER3(name);         \
    AU_TRACE3(name)

#define AU_PERF3_SCOPE_L(name, lv) \
    AU_TIMER3_L(name, lv);         \
    AU_TRACE3_L(name, lv)

#define AU_PERF3_SCOPE_CFG(cfg, name, lv) \
    AU_TIMER3_CFG(cfg, name, lv);         \
    AU_TRACE3_CFG(cfg, name, lv)

#endif  // AURA_PERF_XTRACER3_H_