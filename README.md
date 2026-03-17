# Algernon

## 0 简介
- C++ 工具仓;

## 1 编译
- windows 平台使用编译器 `GCC 8.1.0 x86_64-w64-mingw32`;
- 使用 cmake 编译:
    ```bash
    mkdir -p build && cd build
    cmake ..
    make
    ./src/test
    ```


## 2 todo
- [x] caddies
    - [x] cv
        - [x] ximage
    - [x] file
        - [x] xfile
    - [x] json
        - [x] xjson
    - [x] log
        - [x] logger
    - [x] memory
        - [x] xbuffer
    - [x] perf
        - [x] timer
        - [x] performance
    - [x] regex
        - [x] xregex
    - [x] threadpool
        - [x] xthreadpool
    - [x] util
        - [x] argument_parser
- [x] gpu_helper
- [x] gtest 