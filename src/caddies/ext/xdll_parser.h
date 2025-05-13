#ifndef __XDLL_PARSER_H__
#define __XDLL_PARSER_H__

#include <string>
#include <map>
#include <functional>
#include <initializer_list>
#include <stdexcept>

#ifdef __unix__
#include <dlfcn.h>
#else
#include <Windows.h>
#endif

#include "log/logger.h"

class XDLLParser {
#ifdef __unix__
    using HandleLib = void*;
#else
    using HandleLib = HMODULE;
#endif

public:
    XDLLParser() : mHandle(nullptr) {}
    ~XDLLParser() {
        unload();
    }

    XDLLParser(const XDLLParser& parser) = delete;
    XDLLParser& operator=(const XDLLParser& parser) = delete;

    int load(const std::string& pathLib) {
#ifdef __unix__
        mHandle = dlopen(pathLib.c_str(), RTLD_NOW|RTLD_LOCAL);
#else
        mHandle = LoadLibraryA(pathLib.data());
#endif
        ASSERTER_WITH_INFO(mHandle != nullptr, ERROR_OPEN_FAILED, "failed to open pathLib(%s)!", pathLib.c_str());
        LOGGER_I("success to open pathLib(%s)!\n", pathLib.c_str());
        return NO_ERROR;
    }

    int load(const std::initializer_list<std::string>& pathsLib) {
        for (const auto& path : pathsLib) {
            if (load(path) == NO_ERROR) {
                return NO_ERROR;
            }
        }
        return ERROR_OPEN_FAILED;
    }

    int load(const std::vector<std::string>& pathsLib) {
        for (const auto& path : pathsLib) {
            if (load(path) == NO_ERROR) {
                return NO_ERROR;
            }
        }
        return ERROR_OPEN_FAILED;
    }

    int unload() {
        if (mHandle != nullptr) {
#ifdef __unix__
            auto retDlclose = dlclose(mHandle);
            ASSERTER_WITH_RET(retDlclose == 0, ERROR_INVALID_HANDLE);
#else
            auto retFreeLib = FreeLibrary(mHandle);
            ASSERTER_WITH_RET(retFreeLib == true, ERROR_INVALID_HANDLE);
#endif
            mHandle = nullptr;
        }
        return NO_ERROR;
    }

    /**
     * @example auto result = impl.call<decltype(Max)>("Max", 5, 8);
     */
    template <typename T, typename... Args>
    typename std::result_of<std::function<T>(Args...)>::type
    call(const std::string& funcName, Args&&... args) {
        auto f = getFunction<T>(funcName);
        if (f == nullptr) {
            std::string s = "failed to find symbol: " + funcName;
            throw std::runtime_error(s);
        }
        return f(std::forward<Args>(args)...);
    }

private:
    template <typename T>
    std::function<T> getFunction(const std::string& funcName) {
        auto it = mMapFunc.find(funcName);
        if (it == mMapFunc.end()) {
#ifdef __unix__
            auto addr = dlsym(mHandle, funcName.c_str());
#else
            auto addr = GetProcAddress(mHandle, funcName.c_str());
#endif
            ASSERTER_WITH_INFO(addr != nullptr, nullptr, "failed to get symbol(%s)!", funcName.c_str());

            mMapFunc.insert(std::make_pair(funcName, (void*)addr));
            it = mMapFunc.find(funcName);
        }
        return std::function<T>((T*)(it->second));
    }

private:
    HandleLib mHandle;
    std::map<std::string, void*> mMapFunc;
};

#endif // __XDLL_PARSER_H__