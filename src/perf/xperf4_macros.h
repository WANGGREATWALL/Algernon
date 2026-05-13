#ifndef AURA_PERF_XPERF4_MACROS_H_
#define AURA_PERF_XPERF4_MACROS_H_

/**
 * @file xperf4_macros.h
 * @brief Convenience macros for the v4 perf subsystem.
 *
 * Define @c AU_PERF4_DISABLE_ALL=1 in build flags to strip every scope
 * down to @c ((void)0). This eliminates both the .text and the runtime
 * cost in shipping builds where instrumentation is undesired.
 */

#include "perf/xtimer4.h"
#include "perf/xtracer4.h"

#define AU_PERF4_CONCAT_(a, b) a##b
#define AU_PERF4_CONCAT(a, b) AU_PERF4_CONCAT_(a, b)
#define AU_PERF4_UNIQUE(prefix) AU_PERF4_CONCAT(prefix, __COUNTER__)

#if defined(AU_PERF4_DISABLE_ALL) && AU_PERF4_DISABLE_ALL

#define AU_TIMER4(name) ((void)0)
#define AU_TIMER4_L(name, lv) ((void)0)
#define AU_TIMER4_CFG(cfg, name, lv) ((void)0)
#define AU_TRACE4(name) ((void)0)
#define AU_TRACE4_L(name, lv) ((void)0)
#define AU_TRACE4_CFG(cfg, name, lv) ((void)0)
#define AU_PERF4_SCOPE(name) ((void)0)
#define AU_PERF4_SCOPE_L(name, lv) ((void)0)
#define AU_PERF4_SCOPE_CFG(cfg, name, lv) ((void)0)

#else

#define AU_TIMER4(name) ::au::perf::XTimer4Scoped AU_PERF4_UNIQUE(_auTmr4_)((name))
#define AU_TIMER4_L(name, lv) ::au::perf::XTimer4Scoped AU_PERF4_UNIQUE(_auTmr4_)((name), (lv))
#define AU_TIMER4_CFG(cfg, name, lv) ::au::perf::XTimer4Scoped AU_PERF4_UNIQUE(_auTmr4_)((cfg), (name), (lv))

#define AU_TRACE4(name) ::au::perf::XTracer4Scoped AU_PERF4_UNIQUE(_auTrc4_)((name))
#define AU_TRACE4_L(name, lv) ::au::perf::XTracer4Scoped AU_PERF4_UNIQUE(_auTrc4_)((name), (lv))
#define AU_TRACE4_CFG(cfg, name, lv) ::au::perf::XTracer4Scoped AU_PERF4_UNIQUE(_auTrc4_)((cfg), (name), (lv))

/// Composite: declare both a tree timer and a perfetto slice with the same
/// label in one source line. Identifiers are __COUNTER__-disambiguated so
/// multiple AU_PERF4_SCOPE on the same line are legal.
#define AU_PERF4_SCOPE(name) \
    AU_TIMER4(name);         \
    AU_TRACE4(name)

#define AU_PERF4_SCOPE_L(name, lv) \
    AU_TIMER4_L(name, lv);         \
    AU_TRACE4_L(name, lv)

#define AU_PERF4_SCOPE_CFG(cfg, name, lv) \
    AU_TIMER4_CFG(cfg, name, lv);         \
    AU_TRACE4_CFG(cfg, name, lv)

#endif  // AU_PERF4_DISABLE_ALL

#endif  // AURA_PERF_XPERF4_MACROS_H_