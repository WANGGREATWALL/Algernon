<p align="center">
<code>
       ___    __                                     
      /   |  / /___  ___  _________  ____  ____      
     / /| | / / __ `/ _ \/ ___/ __ \/ __ \/ __ \     
    / ___ |/ / /_/ /  __/ /  / / / / /_/ / / / /     
   /_/  |_/_/\__, /\___/_/  /_/ /_/\____/_/ /_/      
            /____/                                   
</code>
</p>

# *𝔸𝕝𝕘𝕖𝕣𝕟𝕠𝕟*

A modern, high-performance, and lightweight C++17 infrastructure library for building efficient and scalable applications. Algernon provides a curated set of cross-platform components focusing on memory management, multithreading, filesystem operations, and high-performance logging.

## Features

- **Core Infrastructure**: Threadpool, task scheduling (`xthread_flow`), memory buffer (`xbuffer`).
- **Filesystem & Path**: Pythonic path operations (`xpath`) and generic file IO (`xfile`).
- **Cross-Platform**: Robust platform wrappers (`xplatform`, `xdlib`) supporting macOS, Linux, Windows, and Android.
- **Data & Text**: High-performance JSON parser (`xjson`), generic argument parsing (`xargs`), regex wrappers (`xregex`).
- **Diagnostics**: Powerful singleton-free logging (`xlogger`), high-res timers (`xtimer`), and systrace integration (`xtracer`).
- **Math & CV**: Advanced math utilities (`xmath`) and robust image containers (`ximage`).

## Integration

Algernon is designed to be easily embedded into larger projects via CMake.

```cmake
# Add Algernon as a subdirectory
add_subdirectory(third_party/Algernon)

# Link against the static library
target_link_libraries(my_app PRIVATE algernon)
```

Include the umbrella header to access all functionalities:
```cpp
#include "algernon/algernon.h"
```

## Building & Testing

Requires CMake 3.14+ and a C++17 compliant compiler.

```bash
mkdir build && cd build
cmake ..
make -j$(sysctl -n hw.ncpu)
./algernon_test
```

## License

Copyright (c) 2020-2026 WANGGREATWALL. All rights reserved.