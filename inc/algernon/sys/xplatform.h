#ifndef ALGERNON_SYS_XPLATFORM_H_
#define ALGERNON_SYS_XPLATFORM_H_

/**
 * @file xplatform.h
 * @brief Cross-platform system information: OS, CPU, memory, GPU, SoC, env vars.
 *
 * @example
 *   auto cpu = algernon::sys::getCpuInfo();
 *   printf("cores: %d, model: %s\n", cpu.coreCount, cpu.modelName.c_str());
 *
 *   algernon::sys::setEnv("MY_VAR", "hello");
 *   auto val = algernon::sys::getEnv("MY_VAR"); // "hello"
 */

#include <string>
#include <vector>
#include <cstdint>

// ============================================================================
// Compile-time platform macros
// ============================================================================

#if defined(_WIN32) || defined(_WIN64)
    #define ALGERNON_OS_WINDOWS 1
#elif defined(__APPLE__)
    #include <TargetConditionals.h>
    #if TARGET_OS_IPHONE
        #define ALGERNON_OS_IOS 1
    #else
        #define ALGERNON_OS_MACOS 1
    #endif
    #define ALGERNON_OS_APPLE 1
#elif defined(__ANDROID__)
    #define ALGERNON_OS_ANDROID 1
    #define ALGERNON_OS_LINUX   1
#elif defined(__linux__)
    #define ALGERNON_OS_LINUX 1
#else
    #define ALGERNON_OS_UNKNOWN 1
#endif

// Architecture
#if defined(__x86_64__) || defined(_M_X64)
    #define ALGERNON_ARCH_X86_64 1
#elif defined(__i386__) || defined(_M_IX86)
    #define ALGERNON_ARCH_X86 1
#elif defined(__aarch64__) || defined(_M_ARM64)
    #define ALGERNON_ARCH_ARM64 1
#elif defined(__arm__) || defined(_M_ARM)
    #define ALGERNON_ARCH_ARM 1
#endif

namespace algernon { namespace sys {

// ============================================================================
// Enums
// ============================================================================

enum class Platform { Windows, Linux, macOS, iOS, Android, Unknown };
enum class Arch     { x86, x86_64, ARM, ARM64, Unknown };
enum class SocVendor{ Qualcomm, MediaTek, Samsung, HiSilicon, Apple, Unknown };

// Compile-time platform
#if defined(ALGERNON_OS_WINDOWS)
inline constexpr Platform kBuildPlatform = Platform::Windows;
#elif defined(ALGERNON_OS_MACOS)
inline constexpr Platform kBuildPlatform = Platform::macOS;
#elif defined(ALGERNON_OS_IOS)
inline constexpr Platform kBuildPlatform = Platform::iOS;
#elif defined(ALGERNON_OS_ANDROID)
inline constexpr Platform kBuildPlatform = Platform::Android;
#elif defined(ALGERNON_OS_LINUX)
inline constexpr Platform kBuildPlatform = Platform::Linux;
#else
inline constexpr Platform kBuildPlatform = Platform::Unknown;
#endif

#if defined(ALGERNON_ARCH_X86_64)
inline constexpr Arch kBuildArch = Arch::x86_64;
#elif defined(ALGERNON_ARCH_X86)
inline constexpr Arch kBuildArch = Arch::x86;
#elif defined(ALGERNON_ARCH_ARM64)
inline constexpr Arch kBuildArch = Arch::ARM64;
#elif defined(ALGERNON_ARCH_ARM)
inline constexpr Arch kBuildArch = Arch::ARM;
#else
inline constexpr Arch kBuildArch = Arch::Unknown;
#endif

// ============================================================================
// Runtime queries
// ============================================================================

/** @brief Get runtime platform (same as compile-time on most systems). */
Platform runtimePlatform();

/** @brief Identify the SoC vendor (primarily useful on Android). */
SocVendor getSocVendor();

/** @brief Get a human-readable platform name string. */
const char* platformName(Platform p);

/** @brief Get a human-readable architecture name string. */
const char* archName(Arch a);

// ── CPU ──

struct CpuInfo {
    int         coreCount       = 0;
    int         onlineCoreCount = 0;
    std::string modelName;
    Arch        arch            = Arch::Unknown;
};

CpuInfo getCpuInfo();

// ── Memory ──

struct MemoryInfo {
    uint64_t totalBytes     = 0;
    uint64_t availableBytes = 0;
};

MemoryInfo getMemoryInfo();

// ── GPU ──

struct GpuInfo {
    std::string name;
    std::string vendor;
    std::string driverVersion;
};

std::vector<GpuInfo> getGpuInfo();

// ── Environment variables ──

/**
 * @brief Set an environment variable.
 * @return true on success.
 */
bool setEnv(const char* name, const char* value);

/**
 * @brief Get an environment variable value.
 * @return The value string, or empty string if not found.
 */
std::string getEnv(const char* name);

/** @brief Check if an environment variable exists. */
bool hasEnv(const char* name);

// ── Misc ──

/** @brief Get the system hostname. */
std::string getHostName();

/** @brief Get the number of hardware threads. */
int getHardwareConcurrency();

}} // namespace algernon::sys

#endif // ALGERNON_SYS_XPLATFORM_H_
