#include "sys/xdlib.h"

#include "log/xerror.h"

namespace algernon {
namespace sys {

XDLib::~XDLib() { unload(); }

XDLib::XDLib(XDLib&& other) noexcept : mHandle(other.mHandle), mSymbolCache(std::move(other.mSymbolCache))
{
    other.mHandle = nullptr;
}

XDLib& XDLib::operator=(XDLib&& other) noexcept
{
    if (this != &other) {
        unload();
        mHandle       = other.mHandle;
        mSymbolCache  = std::move(other.mSymbolCache);
        other.mHandle = nullptr;
    }
    return *this;
}

int XDLib::load(const std::string& path)
{
#ifdef ALGERNON_USE_DLOPEN
    mHandle = dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
#else
    mHandle = LoadLibraryA(path.c_str());
#endif
    XCHECK_WITH_MSG(mHandle != nullptr, err::kErrorOpenFailed, "XDLib: failed to load library(%s)!", path.c_str());
    XLOG_I("XDLib: loaded library(%s)\n", path.c_str());
    return err::kSuccess;
}

int XDLib::load(const std::vector<std::string>& paths)
{
    for (const auto& path : paths) {
#ifdef ALGERNON_USE_DLOPEN
        auto handle = dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
#else
        auto handle = LoadLibraryA(path.c_str());
#endif
        if (handle != nullptr) {
            mHandle = handle;
            XLOG_I("XDLib: loaded library(%s)\n", path.c_str());
            return err::kSuccess;
        }
    }
    XLOG_E("XDLib: failed to load library from %zu candidate paths!\n", paths.size());
    return err::kErrorOpenFailed;
}

int XDLib::unload()
{
    if (mHandle != nullptr) {
#ifdef ALGERNON_USE_DLOPEN
        auto ret = dlclose(mHandle);
        XCHECK_WITH_RET(ret == 0, err::kErrorInvalidHandle);
#else
        auto ret = FreeLibrary(mHandle);
        XCHECK_WITH_RET(ret == true, err::kErrorInvalidHandle);
#endif
        mHandle = nullptr;
        std::lock_guard<std::mutex> lock(mMutex);
        mSymbolCache.clear();
    }
    return err::kSuccess;
}

bool XDLib::isLoaded() const { return mHandle != nullptr; }

}  // namespace sys
}  // namespace algernon
