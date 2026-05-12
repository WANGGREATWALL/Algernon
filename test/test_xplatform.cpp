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

#endif  // ENABLE_TEST_XPLATFORM
