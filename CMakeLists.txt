cmake_minimum_required(VERSION 3.10)
project(Algernon)

add_subdirectory(src)

file(COPY ${CMAKE_SOURCE_DIR}/src/main.json DESTINATION ${CMAKE_BINARY_DIR}/src)