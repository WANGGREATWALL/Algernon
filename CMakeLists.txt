cmake_minimum_required(VERSION 3.10)

#
# Building in-tree is not allowed (we take care of your craziness).
#
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "Prevented in-tree built. Please create a build directory outside of the source code and call cmake from there. Thank you.")
endif()

#
# Project configuration
#
file(STRINGS "version.mk" VERSION_CONTENTS)
set(REGEX_MAJOR "ALGERNON_VERSION_MAJOR *:= *([0-9]+)")
set(REGEX_MINOR "ALGERNON_VERSION_MINOR *:= *([0-9]+)")
set(REGEX_REVISION "ALGERNON_VERSION_REVISION *:= *([0-9]+)")
string(REGEX MATCH "${REGEX_MAJOR}" _ ${VERSION_CONTENTS})
set(ALGERNON_VERSION_MAJOR ${CMAKE_MATCH_1})
string(REGEX MATCH "${REGEX_MINOR}" _ ${VERSION_CONTENTS})
set(ALGERNON_VERSION_MINOR ${CMAKE_MATCH_1})
string(REGEX MATCH "${REGEX_REVISION}" _ ${VERSION_CONTENTS})
set(ALGERNON_VERSION_REVISION ${CMAKE_MATCH_1})

set(ALGERNON_VERSION ${ALGERNON_VERSION_MAJOR}.${ALGERNON_VERSION_MINOR}.${ALGERNON_VERSION_REVISION})


#
# Set install prefix
#
#set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/install" CACHE PATH "")

project(
    Algernon
    VERSION ${ALGERNON_VERSION}
    LANGUAGES C CXX
)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(ALGERNON_BUILD_LIB_TYPE STATIC)

message("***********************************************************")
message("*")
message("* ${PROJECT_NAME} v${PROJECT_VERSION} (${CMAKE_BUILD_TYPE})")
message("* Copyright (c) 2020-2025 WANGGREATWALL")
message("*")
message("***********************************************************")

add_subdirectory(src)
