<p align="center">
<code>
      ------    ----    ---- -----------     ------    
     ********   ****    **** ***********    ********   
    ----------  ----    ---- ----    ---   ----------  
   ****    **** ****    **** *********    ****    **** 
   ------------ ----    ---- ---------    ------------ 
   ************ ************ ****  ****   ************ 
   ----    ---- ------------ ----   ----  ----    ---- 
   ****    **** ************ ****    **** ****    **** 
</code>
</p>

<h1 align="center">𝔸𝕦𝕣𝕒</h1>

<p align="center">
  <strong>A Modern, Lightweight, and High-Performance C++17 Infrastructure Library</strong>
</p>

<hr />

Aura is a curated collection of high-performance C++17 components designed to streamline the development of efficient, cross-platform applications. It focuses on zero-overhead abstractions, robust diagnostics, and simplified system-level operations.

### Core Modules

| Category | Modules | Key Features |
| :--- | :--- | :--- |
| **Diagnostics** | `xlogger`, `xtimer2`, `xtracer2` | Hierarchical performance trees, multithreaded logging, and Systrace integration. |
| **System** | `xplatform`, `xdlib` | Hardware topology discovery (CPU/Memory) and unified dynamic library loading. |
| **Filesystem** | `xpath`, `xfile` | Pythonic path manipulation and high-performance file I/O wrappers. |
| **Concurrency** | `xthreadpool`, `xthread_flow` | Low-latency task scheduling and parallel flow management. |
| **Data & Text** | `xjson`, `xargs`, `xregex` | Rapid JSON parsing (cJSON based), CLI argument handling, and regex utilities. |
| **Mathematics** | `xmath`, `ximage` | Optimized constants, memory-aligned math, and lightweight image containers. |
| **Memory** | `xbuffer`, `xerror` | Resource-safe memory buffers and advanced error checking/assertion macros. |

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