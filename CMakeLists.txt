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
    src/perf/xtimer1.cpp
    src/perf/xtracer.cpp
    src/perf/xtracer1.cpp
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
# Test options (set to OFF to disable individual unit tests)
# ============================================================================
option(ENABLE_TEST_XLOGGER   "Enable xlogger unit test"   ON)
option(ENABLE_TEST_XMATH     "Enable xmath unit test"     ON)
option(ENABLE_TEST_XBUFFER   "Enable xbuffer unit test"   ON)
option(ENABLE_TEST_XREGEX    "Enable xregex unit test"    ON)
option(ENABLE_TEST_XARGS     "Enable xargs unit test"     ON)
option(ENABLE_TEST_XPATH     "Enable xpath unit test"     ON)
option(ENABLE_TEST_XTIMER    "Enable xtimer unit test"    ON)
option(ENABLE_TEST_XTIMER1   "Enable xtimer1 unit test"   ON)
option(ENABLE_TEST_XTRACER1  "Enable xtracer1 unit test"  ON)
option(ENABLE_TEST_XPLATFORM "Enable xplatform unit test" ON)

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

set(AURA_TEST_SOURCES test/main.cpp ${GTEST_SOURCES})
set(AURA_TEST_DEFINITIONS "")

# Conditionally add test sources and record compile definitions
macro(aura_add_test NAME)
    string(TOUPPER ${NAME} NAME_UPPER)
    if(ENABLE_TEST_${NAME_UPPER})
        list(APPEND AURA_TEST_SOURCES test/test_${NAME}.cpp)
        list(APPEND AURA_TEST_DEFINITIONS ENABLE_TEST_${NAME_UPPER}=1)
    endif()
endmacro()

aura_add_test(xlogger)
aura_add_test(xmath)
aura_add_test(xbuffer)
aura_add_test(xregex)
aura_add_test(xargs)
aura_add_test(xpath)
aura_add_test(xtimer)
aura_add_test(xtimer1)
aura_add_test(xtracer1)
aura_add_test(xplatform)

add_executable(aura_test ${AURA_TEST_SOURCES})
target_compile_definitions(aura_test PRIVATE ${AURA_TEST_DEFINITIONS})

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
