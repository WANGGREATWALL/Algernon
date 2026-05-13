#if ENABLE_TEST_XTRACER

#include <thread>

#include "gtest/gtest.h"
#include "perf/xtracer.h"

using au::perf::XTracerScoped;

// ============================================================================
// Basic lifecycle
// ============================================================================

TEST(XTracer, construct_destruct_no_crash)
{
    {
        XTracerScoped tracer("test_trace");
        // tracer writes "B" on construction, "E" on destruction (no-op on macOS)
    }
    SUCCEED();
}

TEST(XTracer, empty_name_allowed)
{
    {
        XTracerScoped tracer("");
        SUCCEED();
    }
}

TEST(XTracer, long_name_no_overflow)
{
    std::string longName(1024, 'x');
    {
        XTracerScoped tracer(longName);
        SUCCEED();
    }
}

// ============================================================================
// sub() state machine
// ============================================================================

TEST(XTracer, sub_single_transition)
{
    XTracerScoped tracer("root");
    tracer.sub("child");
    // child node should start; root node exists
    SUCCEED();
}

TEST(XTracer, sub_multiple_transitions)
{
    XTracerScoped tracer("root");
    tracer.sub("child_a");
    tracer.sub("child_b");
    tracer.sub("child_c");
    SUCCEED();
}

TEST(XTracer, sub_end_without_starting_new)
{
    XTracerScoped tracer("root");
    tracer.sub("child");
    tracer.sub();  // end current node without starting new one
    SUCCEED();
}

TEST(XTracer, sub_call_before_first_sub)
{
    XTracerScoped tracer("root");
    tracer.sub();  // should be a no-op or handled gracefully
    SUCCEED();
}

TEST(XTracer, sub_empty_name)
{
    XTracerScoped tracer("root");
    tracer.sub("");
    SUCCEED();
}

// ============================================================================
// Multiple tracers
// ============================================================================

TEST(XTracer, multiple_independent_tracers)
{
    {
        XTracerScoped a("tracer_a");
        XTracerScoped b("tracer_b");
        a.sub("a_child");
        b.sub("b_child");
    }
    SUCCEED();
}

TEST(XTracer, nested_tracers)
{
    XTracerScoped outer("outer");
    {
        XTracerScoped inner("inner");
        inner.sub("inner_child");
    }
    outer.sub("outer_child");
    SUCCEED();
}

// ============================================================================
// Concurrent construction
// ============================================================================

TEST(XTracer, concurrent_construction_different_threads)
{
    auto fn = []() {
        XTracerScoped t("thread_trace");
        t.sub("step");
    };
    std::thread t1(fn);
    std::thread t2(fn);
    std::thread t3(fn);
    t1.join();
    t2.join();
    t3.join();
    SUCCEED();
}

// ============================================================================
// sub() pattern variations
// ============================================================================

TEST(XTracer, alternating_sub_with_and_without_name)
{
    XTracerScoped tracer("root");
    tracer.sub("s1");
    tracer.sub();    // close s1
    tracer.sub("s2");
    tracer.sub();    // close s2
    tracer.sub("s3");
    SUCCEED();
}

TEST(XTracer, sub_after_destruction_of_inner_scope)
{
    XTracerScoped tracer("main");
    {
        XTracerScoped inner("inner");
        inner.sub("detail");
    }
    // inner is destroyed; main should still function
    tracer.sub("after_inner");
    SUCCEED();
}

#endif  // ENABLE_TEST_XTRACER
