cmake_minimum_required(VERSION 3.14)

# ============================================================================
# Version (update here; triggers re-configure)
# ============================================================================
set(AURA_VERSION_MAJOR 1)
set(AURA_VERSION_MINOR 2)
set(AURA_VERSION_PATCH 0)
set(AURA_VERSION "${AURA_VERSION_MAJOR}.${AURA_VERSION_MINOR}.${AURA_VERSION_PATCH}")

project(Aura VERSION ${AURA_VERSION} LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "***********************************************************")
message(STATUS "*")
message(STATUS "* Aura v${AURA_VERSION} (${CMAKE_BUILD_TYPE})")
message(STATUS "* Copyright (c) 2020-2026 WANGGREATWALL")
message(STATUS "*")
message(STATUS "***********************************************************")

# ============================================================================
# Generate version.h
# ============================================================================
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/inc/version.h
    @ONLY
)

# ============================================================================
# Include directories
# ============================================================================
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${CMAKE_CURRENT_BINARY_DIR}/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/gtest/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/cJSON
)

# ============================================================================
# Source files
# ============================================================================
set(AURA_SOURCES
    src/sys/xdlib.cpp
    src/sys/xplatform.cpp
    src/file/xpath.cpp
    src/file/xfile.cpp
    src/regex/xregex.cpp
    src/json/xjson.cpp
    src/log/xlogger.cpp
    src/cv/ximage.cpp
    src/perf/xtimer.cpp
    src/perf/xtimer2.cpp
    src/perf/xtracer.cpp
    src/perf/xtracer2.cpp
    src/util/xargs.cpp
    third_party/cJSON/cJSON.c
)

# ============================================================================
# Library
# ============================================================================
add_library(aura STATIC ${AURA_SOURCES})
target_include_directories(aura PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${CMAKE_CURRENT_BINARY_DIR}/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/cJSON
)

# ============================================================================
# Tests
# ============================================================================
set(GTEST_SOURCES
    third_party/gtest/src/gtest-assertion-result.cpp
    third_party/gtest/src/gtest-death-test.cpp
    third_party/gtest/src/gtest-filepath.cpp
    third_party/gtest/src/gtest-matchers.cpp
    third_party/gtest/src/gtest-port.cpp
    third_party/gtest/src/gtest-printers.cpp
    third_party/gtest/src/gtest-test-part.cpp
    third_party/gtest/src/gtest-typed-test.cpp
    third_party/gtest/src/gtest.cpp
)

add_executable(aura_test
    test/main.cpp
    test/test_xmath.cpp
    test/test_xbuffer.cpp
    test/test_xregex.cpp
    test/test_xargs.cpp
    test/test_xpath.cpp
    test/test_xtimer.cpp
    test/test_xtimer2.cpp
    test/test_xtracer2.cpp
    test/test_xlogger.cpp
    test/test_xplatform.cpp
    ${GTEST_SOURCES}
)

target_include_directories(aura_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/gtest
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/gtest/include
)

target_link_libraries(aura_test PRIVATE aura)

# Platform-specific link libraries
if(UNIX AND NOT APPLE)
    target_link_libraries(aura PRIVATE dl pthread)
    target_link_libraries(aura_test PRIVATE pthread)
endif()
if(APPLE)
    target_link_libraries(aura PRIVATE dl)
endif()
