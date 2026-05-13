#if ENABLE_TEST_XREGEX

#include "gtest/gtest.h"
#include "regex/xregex.h"

using namespace au::regex;

TEST(XRegex, Match) {
    std::string text = "Hello, my name is Vincent. Email: vincent@example.com";
    EXPECT_TRUE(match(text, "Vincent"));
    EXPECT_FALSE(match(text, "Wang"));
    EXPECT_TRUE(match(text, R"(\w+@\w+\.\w+)"));
}

TEST(XRegex, FullMatch) {
    EXPECT_TRUE(fullMatch("12345", R"(\d+)"));
    EXPECT_FALSE(fullMatch("123ab", R"(\d+)"));
    EXPECT_TRUE(fullMatch("hello", R"([a-z]+)"));
}

TEST(XRegex, GetMatches) {
    std::string text = "cat bat hat";
    auto all = getAllMatches(text, R"(\w+at)");
    EXPECT_EQ(all.size(), 3u);
    EXPECT_EQ(getFirstMatch(text, R"(\w+at)"), "cat");
    EXPECT_EQ(getLastMatch(text, R"(\w+at)"), "hat");
}

TEST(XRegex, ExtractGroups) {
    auto groups = extractGroups("2026-04-30", R"((\d{4})-(\d{2})-(\d{2}))");
    ASSERT_EQ(groups.size(), 3u);
    EXPECT_EQ(groups[0], "2026");
    EXPECT_EQ(groups[1], "04");
    EXPECT_EQ(groups[2], "30");
}

TEST(XRegex, ReplaceAll) {
    EXPECT_EQ(replaceAll("hello hello", "hello", "hi"), "hi hi");
}

TEST(XRegex, ReplaceNth) {
    EXPECT_EQ(replaceNth("aa bb aa", "aa", "cc", 0), "cc bb aa");
    EXPECT_EQ(replaceNth("aa bb aa", "aa", "cc", 1), "aa bb cc");
    EXPECT_EQ(replaceNth("aa bb aa", "aa", "cc", 5), "aa bb aa");
}

TEST(XRegex, Split) {
    auto parts = split("a,b,,c", ",");
    ASSERT_EQ(parts.size(), 4u);
    EXPECT_EQ(parts[0], "a");
    EXPECT_EQ(parts[1], "b");
    EXPECT_EQ(parts[2], "");
    EXPECT_EQ(parts[3], "c");
}

// ============================================================================
// Edge cases
// ============================================================================

TEST(XRegex, EmptyInput) {
    EXPECT_TRUE(match("", ".*"));
    EXPECT_TRUE(fullMatch("", ".*"));
    auto matches = getAllMatches("", R"(\w+)");
    EXPECT_TRUE(matches.empty());
}

TEST(XRegex, NoMatchReturnsEmpty) {
    EXPECT_EQ(getFirstMatch("abc", R"(\d+)"), "");
    EXPECT_EQ(getLastMatch("abc", R"(\d+)"), "");
    EXPECT_TRUE(getAllMatches("abc", R"(\d+)").empty());
    auto groups = extractGroups("abc", R"((\d+))");
    EXPECT_TRUE(groups.empty());
}

TEST(XRegex, ReplaceAllNoMatchReturnsOriginal) {
    EXPECT_EQ(replaceAll("hello world", "xyz", "hi"), "hello world");
}

TEST(XRegex, SplitLeadingTrailingDelimiter) {
    auto parts = split(",a,b", ",");
    ASSERT_EQ(parts.size(), 3u);
    EXPECT_EQ(parts[0], "");
    EXPECT_EQ(parts[1], "a");
    EXPECT_EQ(parts[2], "b");

    // Trailing empty tokens are skipped by std::regex_token_iterator
    auto parts2 = split("a,b,", ",");
    ASSERT_EQ(parts2.size(), 2u);
    EXPECT_EQ(parts2[0], "a");
    EXPECT_EQ(parts2[1], "b");
}

TEST(XRegex, InvalidRegexThrows) {
    EXPECT_THROW(match("abc", R"([)"), std::regex_error);
}

TEST(XRegex, ReplaceNthOutOfRange) {
    EXPECT_EQ(replaceNth("aa bb aa", "aa", "cc", 999), "aa bb aa");
    EXPECT_EQ(replaceNth("aa bb aa", "aa", "cc", 0), "cc bb aa");
}

TEST(XRegex, ExtractGroupsSingleGroup) {
    auto groups = extractGroups("2026!", R"((\d{4})!)");
    ASSERT_EQ(groups.size(), 1u);
    EXPECT_EQ(groups[0], "2026");
}

#endif  // ENABLE_TEST_XREGEX
