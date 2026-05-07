#include "gtest/gtest.h"
#include "log/xlogger.h"
#include "log/xerror.h"

using namespace algernon::log;

// ── Level Filtering ──

TEST(XLogger, FilteredLevelProducesNoOutput)
{
    Config::get().setTag("Test");
    Config::get().setLevel(Level::Error);

    // V/D/I/W should be silently discarded (no crash, no output).
    XLOG_V("verbose msg %d\n", 1);
    XLOG_D("debug msg %d\n", 2);
    XLOG_I("info msg %d\n", 3);
    XLOG_W("warn msg %d\n", 4);

    // Restore default.
    Config::get().setLevel(Level::Info);
}

TEST(XLogger, SilentLevelSuppressesAll)
{
    Config::get().setLevel(Level::Silent);
    XLOG_F("this fatal should be suppressed\n");
    Config::get().setLevel(Level::Info);
}

// ── Tag Management ──

TEST(XLogger, SetAndGetTag)
{
    Config::get().setTag("MyApp");
    EXPECT_STREQ(Config::get().getTag(), "MyApp");

    Config::get().setTag(nullptr);
    EXPECT_STREQ(Config::get().getTag(), "unknown");
}

TEST(XLogger, TagTruncation)
{
    // Tag longer than 63 chars should be truncated, not crash.
    std::string longTag(128, 'A');
    Config::get().setTag(longTag.c_str());

    const char* tag = Config::get().getTag();
    EXPECT_EQ(std::strlen(tag), 63u);
}

// ── All Levels Output Without Crash ──

TEST(XLogger, AllLevelsDoNotCrash)
{
    Config::get().setTag("CrashTest");
    Config::get().setLevel(Level::Verbose);

    XLOG_V("verbose %s\n", "ok");
    XLOG_D("debug %d\n", 42);
    XLOG_I("info %.2f\n", 3.14);
    XLOG_W("warn %s\n", "attention");
    XLOG_E("error code=%d\n", -1);
    XLOG_F("fatal %s\n", "critical");

    Config::get().setLevel(Level::Info);
}

// ── Color Toggle ──

TEST(XLogger, ColorToggle)
{
    EXPECT_FALSE(Config::get().isColorEnabled());
    Config::get().setColorEnabled(true);
    EXPECT_TRUE(Config::get().isColorEnabled());

    XLOG_I("this line should have color\n");

    Config::get().setColorEnabled(false);
    EXPECT_FALSE(Config::get().isColorEnabled());
}

// ── XCHECK Macros ──

static int checkHelper(bool pass)
{
    XCHECK_WITH_RET(pass, -1);
    return 0;
}

TEST(XLogger, XCheckWithRet)
{
    EXPECT_EQ(checkHelper(true), 0);
    EXPECT_EQ(checkHelper(false), -1);
}

static int checkMsgHelper(int val)
{
    XCHECK_WITH_MSG(val > 0, -99, "val must be positive, got %d\n", val);
    return val;
}

TEST(XLogger, XCheckWithMsg)
{
    EXPECT_EQ(checkMsgHelper(10), 10);
    EXPECT_EQ(checkMsgHelper(-5), -99);
}
