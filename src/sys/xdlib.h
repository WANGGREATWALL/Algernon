#ifndef AURA_SYS_XDLIB_H_
#define AURA_SYS_XDLIB_H_

/**
 * @file xdlib.h
 * @brief Cross-platform dynamic library loader with thread-safe symbol caching.
 *
 * Supports Windows (LoadLibrary), Linux/macOS/Android (dlopen).
 *
 * @example
 *   au::sys::XDLib lib;
 *   lib.load("/usr/lib/libfoo.so");
 *
 *   // Resolve a symbol by type:
 *   auto fp = lib.get<decltype(someFunc)>("someFunc");
 *   if (fp) fp(arg1, arg2);
 *
 *   // Or use the convenience macro:
 *   auto fp2 = XDLIB_GET(lib, someFunc);
 */

#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>

#include "log/xlogger.h"

#if defined(__unix__) || defined(__APPLE__)
#include <dlfcn.h>
#define AURA_USE_DLOPEN 1
#else
#include <Windows.h>
#endif

namespace au { namespace sys {

class XDLib {
#ifdef AURA_USE_DLOPEN
    using NativeHandle = void*;
#else
    using NativeHandle = HMODULE;
#endif

public:
    XDLib() = default;
    ~XDLib();

    XDLib(XDLib&& other) noexcept;
    XDLib& operator=(XDLib&& other) noexcept;

    XDLib(const XDLib&) = delete;
    XDLib& operator=(const XDLib&) = delete;

    /**
     * @brief Load a dynamic library from the given path.
     * @param path Absolute or relative path to the shared library.
     * @return kSuccess on success, kErrorOpenFailed on failure.
     */
    int load(const std::string& path);

    /**
     * @brief Try loading from multiple candidate paths, stop on first success.
     * @param paths List of candidate library paths.
     * @return kSuccess on success, kErrorOpenFailed if all paths fail.
     *
     * @example
     *   lib.load({"/usr/lib/libfoo.so", "/opt/lib/libfoo.so"});
     */
    int load(const std::vector<std::string>& paths);

    /**
     * @brief Unload the library and clear the symbol cache.
     * @return kSuccess on success.
     */
    int unload();

    /** @brief Check if a library is currently loaded. */
    bool isLoaded() const;

    /**
     * @brief Resolve a symbol from the loaded library (thread-safe, cached).
     * @tparam Func Function signature type, e.g. decltype(clGetPlatformIDs).
     * @param name The symbol name to look up.
     * @return Typed function pointer, or nullptr if not found.
     *
     * @example
     *   auto fp = lib.get<decltype(clGetPlatformIDs)>("clGetPlatformIDs");
     *   if (fp) fp(num_entries, platforms, num_platforms);
     */
    template <typename Func>
    Func* get(const char* name) {
        std::lock_guard<std::mutex> lock(mMutex);
        auto [it, inserted] = mSymbolCache.try_emplace(name, nullptr);
        if (inserted) {
#ifdef AURA_USE_DLOPEN
            it->second = dlsym(mHandle, name);
#else
            it->second = reinterpret_cast<void*>(GetProcAddress(mHandle, name));
#endif
            if (it->second == nullptr) {
                XLOG_E("XDLib: failed to resolve symbol(%s)!\n", name);
                mSymbolCache.erase(it);
                return nullptr;
            }
        }
        return reinterpret_cast<Func*>(it->second);
    }

private:
    NativeHandle mHandle = nullptr;
    std::unordered_map<std::string, void*> mSymbolCache;
    std::mutex mMutex;
};

}}  // namespace au::sys

/**
 * @brief Convenience macro: resolve a symbol with automatic name stringification.
 * @example auto fp = XDLIB_GET(lib, clGetPlatformIDs);
 */
#define XDLIB_GET(lib, func) (lib).get<decltype(func)>(#func)

#endif // AURA_SYS_XDLIB_H_
