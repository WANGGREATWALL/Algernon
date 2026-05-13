#if ENABLE_TEST_XPLATFORM

#include "gtest/gtest.h"
#include "sys/xplatform.h"

using namespace au::sys;

TEST(XPlatform, BuildPlatform) {
#ifdef __APPLE__
    EXPECT_EQ(kBuildPlatform, Platform::macOS);
#elif defined(__linux__)
    EXPECT_EQ(kBuildPlatform, Platform::Linux);
#endif
    EXPECT_NE(kBuildPlatform, Platform::Unknown);
}

TEST(XPlatform, CpuInfo) {
    auto info = getCpuInfo();
    EXPECT_GT(info.coreCount, 0);
    EXPECT_GT(info.onlineCoreCount, 0);
    EXPECT_FALSE(info.modelName.empty());
}

TEST(XPlatform, MemoryInfo) {
    auto info = getMemoryInfo();
    EXPECT_GT(info.totalBytes, 0u);
}

TEST(XPlatform, EnvVars) {
    EXPECT_TRUE(setEnv("AURA_TEST_VAR", "hello"));
    EXPECT_TRUE(hasEnv("AURA_TEST_VAR"));
    EXPECT_EQ(getEnv("AURA_TEST_VAR"), "hello");
}

TEST(XPlatform, HostName) {
    EXPECT_FALSE(getHostName().empty());
}

TEST(XPlatform, HardwareConcurrency) {
    EXPECT_GT(getHardwareConcurrency(), 0);
}

// ============================================================================
// Env var edge cases
// ============================================================================

TEST(XPlatform, EnvVarNonExistent) {
    EXPECT_EQ(getEnv("AURA_NONEXISTENT_VAR_XYZ"), "");
    EXPECT_FALSE(hasEnv("AURA_NONEXISTENT_VAR_XYZ"));
}

TEST(XPlatform, EnvVarOverwrite) {
    EXPECT_TRUE(setEnv("AURA_TEST_OVERWRITE", "first"));
    EXPECT_EQ(getEnv("AURA_TEST_OVERWRITE"), "first");
    EXPECT_TRUE(setEnv("AURA_TEST_OVERWRITE", "second"));
    EXPECT_EQ(getEnv("AURA_TEST_OVERWRITE"), "second");
}

// ============================================================================
// Memory info — available bytes
// ============================================================================

TEST(XPlatform, MemoryInfoAvailableBytes) {
    auto info = getMemoryInfo();
    EXPECT_GT(info.totalBytes, 0u);
    // availableBytes may be zero on some platforms, but total > 0
}

// ============================================================================
// platformName / archName
// ============================================================================

TEST(XPlatform, PlatformName) {
    auto name = platformName(kBuildPlatform);
    EXPECT_STRNE(name, "");
    EXPECT_NE(name, nullptr);
}

TEST(XPlatform, ArchName) {
    auto name = archName(kBuildArch);
    EXPECT_STRNE(name, "");
    EXPECT_NE(name, nullptr);
}

// ============================================================================
// GPU info (may be empty on headless/CI)
// ============================================================================

TEST(XPlatform, GpuInfo) {
    auto gpus = getGpuInfo();
    // May be empty on headless / macOS without GPU detection
    for (const auto& gpu : gpus) {
        EXPECT_FALSE(gpu.name.empty());
    }
}

#endif  // ENABLE_TEST_XPLATFORM
