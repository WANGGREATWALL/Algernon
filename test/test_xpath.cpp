#include "gtest/gtest.h"
#include "file/xpath.h"

using au::file::XPath;

TEST(XPath, Construction) {
    XPath p("/tmp/test/file.txt");
    EXPECT_EQ(p.filename(), "file.txt");
    EXPECT_EQ(p.stem(), "file");
    EXPECT_EQ(p.extension(), ".txt");
    EXPECT_EQ(p.parent().str(), "/tmp/test");
}

TEST(XPath, Concatenation) {
    XPath p("/home/user");
    XPath full = p / "data" / "file.txt";
    EXPECT_EQ(full.str(), "/home/user/data/file.txt");
}

TEST(XPath, Queries) {
    XPath p("/tmp");
    EXPECT_TRUE(p.exists());
    EXPECT_TRUE(p.isDirectory());
    EXPECT_FALSE(p.isFile());
}

TEST(XPath, StringConversion) {
    XPath p("/tmp/test");
    std::string s = p.str();
    EXPECT_EQ(s, "/tmp/test");

    XPath p2(s);
    EXPECT_EQ(p, p2);
}

TEST(XPath, EmptyPath) {
    XPath p;
    EXPECT_TRUE(p.isEmpty());
}
