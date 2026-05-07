#ifndef CL_SYMBOLS_H_
#define CL_SYMBOLS_H_

#include <vector>
#include <string>

#include "log/xlogger.h"
#include "sys/xdlib.h"

using algernon::sys::XDLib;

#ifdef __APPLE__
#undef CL_TARGET_OPENCL_VERSION
#define CL_TARGET_OPENCL_VERSION 120
#undef CL_HPP_TARGET_OPENCL_VERSION
#define CL_HPP_TARGET_OPENCL_VERSION 120
#undef CL_HPP_MINIMUM_OPENCL_VERSION
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#else
#ifndef CL_TARGET_OPENCL_VERSION
#define CL_TARGET_OPENCL_VERSION 200
#endif
#ifndef CL_HPP_TARGET_OPENCL_VERSION
#define CL_HPP_TARGET_OPENCL_VERSION 200
#endif
#ifndef CL_HPP_MINIMUM_OPENCL_VERSION
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#endif
#endif
#include "CL/opencl.hpp"


namespace gpu {

/**
 * @brief Singleton that holds the dynamically loaded OpenCL library.
 *
 * Usage in trampoline functions (cl_symbols.cpp):
 *   return XDLIB_GET(gpu::CLSymbols::lib(), clSomeFunc)(args...);
 */
class CLSymbols {
public:
    static algernon::sys::XDLib& lib() {
        static CLSymbols instance;
        return instance.mLib;
    }

    ~CLSymbols() = default;

private:
    CLSymbols() {
        int ret = mLib.load(mLibPaths);
        XASSERT(ret == err::kSuccess);
    }

    algernon::sys::XDLib mLib;

    std::vector<std::string> mLibPaths = {
        "/vendor/lib64/libOpenCL.so",

        // __aarch64__
        "/system/vendor/lib64/libOpenCL.so",
        "/system/lib64/libOpenCL.so",

        // __aarch32__
        "/system/vendor/lib/libOpenCL.so",
        "/system/lib/libOpenCL.so",

        // windows default
        "C:/Windows/System32/opencl.dll",
        
        // NVIDIA CUDA
        "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v11.6/bin/OpenCL.dll",
        "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v11.4/bin/OpenCL.dll",
        
        // AMD APP SDK
        "C:/Program Files (x86)/AMD APP/bin/x86_64/OpenCL.dll",
        "C:/Program Files (x86)/AMD APP/bin/x86/OpenCL.dll",
        
        // Intel SDK
        "C:/Program Files (x86)/Intel/OpenCL SDK/6.3/bin/x64/OpenCL.dll",
        "C:/Program Files (x86)/Intel/OpenCL SDK/6.3/bin/x86/OpenCL.dll"
    };
};

} // namespace gpu

#endif // CL_SYMBOLS_H_