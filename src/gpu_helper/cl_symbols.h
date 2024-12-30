#ifndef __CL_SYMBOLS_H__
#define __CL_SYMBOLS_H__

#include <vector>
#include <string>
#include <memory>

#include "log/logger.h"
#include "ext/xdll_parser.h"

#define CL_TARGET_OPENCL_VERSION 200
#define CL_HPP_TARGET_OPENCL_VERSION 200
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#include "CL/cl2.hpp"

#if __unix__
#include <dlfcn.h>
#endif

namespace gpu {

    class CLSymbols {
    public:
        static CLSymbols& get() {
            static CLSymbols instance;
            return instance;
        }
        ~CLSymbols() = default;

        // platform
        // ==============================
        cl_int clGetPlatformIDs(cl_uint num_entries, cl_platform_id* platforms, cl_uint* num_platforms) {
            return mParser->call<decltype(clGetPlatformIDs)>("clGetPlatformIDs", num_entries, platforms, num_platforms);
        }
    
    private:
        CLSymbols() {
            mParser.reset(new XDLLParser);
            int retLoadCLLib = mParser->load(mListLibPath);
            ASSERTER(retLoadCLLib == VDKResultSuccess);
        }

    private:
        std::unique_ptr<XDLLParser> mParser;

        std::vector<std::string> mListLibPath = {
            "/vendor/lib64/libOpenCL.so",

            // __aarch64__
            "/system/vendor/lib64/libOpenCL.so",
            "/system/lib64/libOpenCL.so",

            // __aarch32__
            "/system/vendor/lib/libOpenCL.so",
            "/system/lib/libOpenCL.so"
        };
    };

} // namespace gpu

#endif // __CL_SYMBOLS_H__