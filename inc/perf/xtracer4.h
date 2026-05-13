#ifndef AURA_PERF_XTRACER4_H_
#define AURA_PERF_XTRACER4_H_

/**
 * @file xtracer4.h
 * @brief Final-form Android Perfetto / ftrace trace_marker tracer for Aura.
 *
 * On Android:
 *  - A single process-wide trace_marker fd is opened lazily with O_CLOEXEC
 *    on first use; the kernel reclaims it on process exit (no explicit close).
 *  - Each scope writes "B|pid|name" on construction and "E|pid" on
 *    destruction. Per-write atomicity is guaranteed by ftrace up to one
 *    page, so no userspace lock is required.
 *  - sub(name) / sub() emit additional begin/end pairs nested inside the
 *    current scope, allowing inline phase markers without nesting C++
 *    lifetimes.
 *
 * On non-Android targets every body is compiled away to nothing, leaving
 * no .text footprint and zero runtime cost.
 *
 * Decoupling note:
 *  XTracer4 is intentionally independent of XTimer4. The composite macro
 *  @c AU_PERF4_SCOPE declares both with the same label, so the perfetto
 *  slice and the hierarchical log entry stay aligned.
 */

#include <cstddef>
#include <cstdint>

#include "perf/xtimer4.h"

namespace au {
namespace perf {

/**
 * @brief RAII scoped Perfetto / ftrace tracer (Android-only payload).
 *
 * Active iff @c ctx.isEnabled() && level <= ctx.getTracerLevel().
 *
 * Name lifetime: the constructor copies the label bytes into a fixed
 * inline buffer; the caller may safely pass a temporary @c std::string.
 */
class XTracer4Scoped
{
public:
    /// Use the default process context.
    explicit XTracer4Scoped(const char* name, int32_t level = 0) noexcept;
    XTracer4Scoped(const char* name, std::size_t nameLen, int32_t level = 0) noexcept;

    /// Use a caller-specific context.
    XTracer4Scoped(XPerfContext4& ctx, const char* name, int32_t level = 0) noexcept;
    XTracer4Scoped(XPerfContext4& ctx, const char* name, std::size_t nameLen, int32_t level = 0) noexcept;

    ~XTracer4Scoped() noexcept;

    XTracer4Scoped(const XTracer4Scoped&)            = delete;
    XTracer4Scoped& operator=(const XTracer4Scoped&) = delete;

    /// End the previous sub-slice (if any) and open a new one named @p name.
    /// No-op when this scope is inactive.
    void sub(const char* name) noexcept;
    void sub(const char* name, std::size_t nameLen) noexcept;

    /// End the previous sub-slice (if any). No-op otherwise.
    void sub() noexcept;

private:
    void begin(XPerfContext4& ctx, const char* name, std::size_t nameLen, int32_t level) noexcept;

    XPerfContext4* mCtx;
    bool           mActive;
    bool           mSubOpen;

    // Fixed inline buffer; longer names are truncated to fit. Names are
    // never read from this buffer after the kernel write, so the buffer
    // need not survive past begin().
    static constexpr std::size_t kMaxName = 128;
    char                         mName[kMaxName];
    uint8_t                      mNameLen;
};

}  // namespace perf
}  // namespace au

#endif  // AURA_PERF_XTRACER4_H_