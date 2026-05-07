#include "gtest/gtest.h"
#include "regex/xregex.h"

using namespace algernon::regex;

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
