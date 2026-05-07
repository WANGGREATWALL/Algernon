#include "gtest/gtest.h"
#include "util/xargs.h"

using algernon::util::XArgs;

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(*arr))

TEST(XArgs, NoOption) {
    const char* argv[] = {"test"};
    XArgs parser([](char, const std::string&, XArgs::Value&) { return true; });
    EXPECT_TRUE(parser.parse(ARRAY_SIZE(argv), argv));
}

TEST(XArgs, ShortOption) {
    const char* argv[] = {"test", "-x", "5"};
    int val = 0;
    XArgs parser([&](char s, const std::string&, XArgs::Value& v) {
        if (s == 'x') val = std::stoi(v.get());
        return true;
    });
    EXPECT_TRUE(parser.parse(ARRAY_SIZE(argv), argv));
    EXPECT_EQ(val, 5);
}

TEST(XArgs, LongOption) {
    const char* argv[] = {"test", "--level", "3"};
    int level = 0;
    XArgs parser([&](char, const std::string& l, XArgs::Value& v) {
        if (l == "level") level = std::stoi(v.get());
        return true;
    });
    EXPECT_TRUE(parser.parse(ARRAY_SIZE(argv), argv));
    EXPECT_EQ(level, 3);
}

TEST(XArgs, LongOptionEquals) {
    const char* argv[] = {"test", "--key=value"};
    std::string result;
    XArgs parser([&](char, const std::string& l, XArgs::Value& v) {
        if (l == "key") result = v.get();
        return true;
    });
    EXPECT_TRUE(parser.parse(ARRAY_SIZE(argv), argv));
    EXPECT_EQ(result, "value");
}

TEST(XArgs, QuotedValue) {
    const char* argv[] = {"test", R"(--name='Chen Wang')"};
    std::string name;
    XArgs parser([&](char, const std::string& l, XArgs::Value& v) {
        if (l == "name") name = v.get();
        return true;
    });
    EXPECT_TRUE(parser.parse(ARRAY_SIZE(argv), argv));
    EXPECT_EQ(name, "Chen Wang");
}

TEST(XArgs, FailHandler) {
    const char* argv[] = {"test", "-x"};
    XArgs parser([](char, const std::string&, XArgs::Value&) { return false; });
    EXPECT_FALSE(parser.parse(ARRAY_SIZE(argv), argv));
}
