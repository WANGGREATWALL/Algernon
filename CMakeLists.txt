cmake_minimum_required(VERSION 3.14)

# ============================================================================
# Version (update here; triggers re-configure)
# ============================================================================
set(ALGERNON_VERSION_MAJOR 1)
set(ALGERNON_VERSION_MINOR 1)
set(ALGERNON_VERSION_PATCH 0)
set(ALGERNON_VERSION "${ALGERNON_VERSION_MAJOR}.${ALGERNON_VERSION_MINOR}.${ALGERNON_VERSION_PATCH}")

project(Algernon VERSION ${ALGERNON_VERSION} LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "***********************************************************")
message(STATUS "*")
message(STATUS "* Algernon v${ALGERNON_VERSION} (${CMAKE_BUILD_TYPE})")
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
set(ALGERNON_SOURCES
    src/sys/xdlib.cpp
    src/sys/xplatform.cpp
    src/file/xpath.cpp
    src/file/xfile.cpp
    src/regex/xregex.cpp
    src/json/xjson.cpp
    src/cv/ximage.cpp
    src/perf/xtimer.cpp
    src/perf/xtracer.cpp
    src/util/xargs.cpp
    third_party/cJSON/cJSON.c
)

# ============================================================================
# Library
# ============================================================================
add_library(algernon STATIC ${ALGERNON_SOURCES})
target_include_directories(algernon PUBLIC
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

add_executable(algernon_test
    test/main.cpp
    test/test_xmath.cpp
    test/test_xbuffer.cpp
    test/test_xregex.cpp
    test/test_xargs.cpp
    test/test_xpath.cpp
    test/test_xtimer.cpp
    test/test_xlogger.cpp
    test/test_xplatform.cpp
    ${GTEST_SOURCES}
)

target_include_directories(algernon_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/gtest
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/gtest/include
)

target_link_libraries(algernon_test PRIVATE algernon)

# Platform-specific link libraries
if(UNIX AND NOT APPLE)
    target_link_libraries(algernon PRIVATE dl pthread)
    target_link_libraries(algernon_test PRIVATE pthread)
endif()
if(APPLE)
    target_link_libraries(algernon PRIVATE dl)
endif()
