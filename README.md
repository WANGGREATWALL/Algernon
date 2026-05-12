<div align="center">
<pre>
      ------    ----    ---- -----------     ------    
     ********   ****    **** ***********    ********   
    ----------  ----    ---- ----    ---   ----------  
   ****    **** ****    **** *********    ****    **** 
   ------------ ----    ---- ---------    ------------ 
   ************ ************ ****  ****   ************ 
   ----    ---- ------------ ----   ----  ----    ---- 
   ****    **** ************ ****    **** ****    **** 
</pre>
</div>

<h1 align="center">𝔸𝕦𝕣𝕒</h1>

<p align="center">
  <strong>A Modern, Lightweight, and High-Performance C++17 Infrastructure Library</strong>
</p>

<hr />

Aura is a curated collection of high-performance C++17 components designed to streamline the development of efficient, cross-platform applications. It focuses on zero-overhead abstractions, robust diagnostics, and simplified system-level operations.

### Core Modules

> **Status legend:** ✅ unit test completed &nbsp;|&nbsp; ☐ unit test completed but disabled &nbsp;|&nbsp; — not yet implemented

| Category | Module | Key Features | Status |
| :--- | :--- | :--- | :---: |
| **Diagnostics** | `xlogger` | Multithreaded hierarchical logging with color output, level filtering, and Android logcat support. | ✅ |
| | `xtimer` | High-resolution wall-clock timer with sleep helpers and formatted timestamps. | ✅ |
| | `xtimer2` | Scoped performance tree with release/debug modes, nesting, and thread-safe output. | ✅ |
| | `xtimer3` | Next-gen scoped timer with aggregate mode, thread isolation, and macro sugar. | ✅ |
| | `xtracer` | Systrace / Perfetto integration for system-level tracing on Android. | — |
| | `xtracer2` | Combined timer+tracer scoped probes with tree output and Systrace ATrace calls. | ✅ |
| | `xtracer3` | Next-gen composite perf scope (timer + tracer) with level filtering and thread safety. | ✅ |
| **System** | `xplatform` | Hardware topology (CPU cores, memory), environment variables, and OS detection. | ✅ |
| | `xdlib` | Unified cross-platform dynamic library loading (dlopen / LoadLibrary). | — |
| **Filesystem** | `xpath` | Pythonic path manipulation (join, split, stem, extension) and filesystem queries. | ✅ |
| | `xfile` | High-performance file I/O with RAII handles and memory-mapped reads. | — |
| **Concurrency** | `xthreadpool` | Low-latency work-stealing thread pool with priority-aware task dispatch. | — |
| | `xthread_flow` | Directed acyclic graph of tasks with parallel scheduling across thread-pool workers. | — |
| **Data & Text** | `xjson` | Rapid JSON parsing and serialization built on cJSON with C++ RAII wrappers. | — |
| | `xargs` | Lightweight CLI argument parser with short/long options and quoted values. | ✅ |
| | `xregex` | std::regex convenience wrappers: match, extract groups, replace, and split. | ✅ |
| **Mathematics** | `xmath` | Constants (π, e), min/max/clamp, power-of-two alignment, and trig helpers. | ✅ |
| | `ximage` | Lightweight multi-channel image container with ROI extraction and pixel iterators. | — |
| **Memory** | `xbuffer` | Shared-memory buffer with zero-copy reference counting and move semantics. | ✅ |
| | `xerror` | Assertion macros (XASSERT, XCHECK) with formatted messages and return values. | — |

### Supported Platforms
* **Desktop**: macOS (Apple Silicon/Intel), Linux (x86_64/AArch64), Windows (MSVC)
* **Mobile**: Android (NDK)

### Quick Start

#### Integration
Aura is designed for seamless integration via CMake:

```cmake
add_subdirectory(third_party/Aura)
target_link_libraries(your_app PRIVATE aura)
```

#### Build & Test
Ensure you have CMake 3.14+ and a C++17 compliant compiler.

```bash
mkdir build && cd build
cmake ..
make -j$(sysctl -n hw.ncpu)
./aura_test
```

### Design Philosophy
* **Minimal Dependencies**: Strictly avoids heavy external frameworks.
* **Performance First**: Prioritizes cache-friendly data structures and zero-copy semantics.
* **Developer Experience**: Clean, consistent API design with extensive test coverage.

---

<p align="center">
  Copyright (c) 2020-2026 WANGGREATWALL. All rights reserved.
</p>