#if ENABLE_TEST_XPATH

#include <filesystem>
#include <fstream>

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

// ============================================================================
// String conversion
// ============================================================================

TEST(XPath, CStr) {
    XPath p("/tmp/test");
    EXPECT_STREQ(p.c_str(), "/tmp/test");
}

TEST(XPath, OperatorString) {
    XPath p("/a/b");
    std::string s = p;
    EXPECT_EQ(s, "/a/b");
}

// ============================================================================
// Operator /=
// ============================================================================

TEST(XPath, OperatorSlashEquals) {
    XPath p("/tmp");
    p /= "sub";
    EXPECT_EQ(p.str(), "/tmp/sub");
    p /= "deep";
    EXPECT_EQ(p.str(), "/tmp/sub/deep");
}

// ============================================================================
// fileSize
// ============================================================================

TEST(XPath, FileSize) {
    // /etc/hosts should exist and have a positive size
    XPath p("/etc/hosts");
    if (p.exists()) {
        EXPECT_GT(p.fileSize(), 0u);
    }
}

// ============================================================================
// createDirs
// ============================================================================

TEST(XPath, CreateDirsSuccess) {
    XPath p("/tmp/aura_xpath_test/sub/deep");
    EXPECT_TRUE(p.createDirs());
    EXPECT_TRUE(p.exists());
    // cleanup
    std::filesystem::remove_all("/tmp/aura_xpath_test");
}

TEST(XPath, CreateDirsAlreadyExists) {
    XPath p("/tmp");
    // Already-existing directory returns false (no directories created)
    EXPECT_FALSE(p.createDirs());
}

// ============================================================================
// listFiles / listDirs / listAll
// ============================================================================

TEST(XPath, ListOperations) {
    XPath dir("/tmp/aura_xpath_list");
    dir.createDirs();
    // Create a file and a subdirectory
    std::ofstream((dir / "test.txt").str()) << "data";
    (dir / "subdir").createDirs();

    auto files = dir.listFiles();
    auto dirs  = dir.listDirs();
    auto all   = dir.listAll();

    EXPECT_GE(files.size(), 1u);
    EXPECT_GE(dirs.size(), 1u);
    EXPECT_GE(all.size(), 2u);

    // cleanup
    std::filesystem::remove_all(dir.str());
}

// ============================================================================
// glob
// ============================================================================

TEST(XPath, GlobWithMatchingPattern) {
    XPath dir("/tmp/aura_xpath_glob");
    dir.createDirs();
    std::ofstream((dir / "a.txt").str()) << "a";
    std::ofstream((dir / "b.txt").str()) << "b";
    std::ofstream((dir / "c.log").str()) << "c";

    auto matches = dir.glob(R"(.*\.txt)");
    EXPECT_EQ(matches.size(), 2u);

    std::filesystem::remove_all(dir.str());
}

TEST(XPath, GlobNoMatch) {
    XPath dir("/tmp/aura_xpath_glob2");
    dir.createDirs();
    auto matches = dir.glob(R"(no_such_pattern)");
    EXPECT_TRUE(matches.empty());
    std::filesystem::remove_all(dir.str());
}

// ============================================================================
// remove / rename / copy
// ============================================================================

TEST(XPath, RemoveFile) {
    XPath p("/tmp/aura_xpath_remove_test.txt");
    std::ofstream(p.str()) << "data";
    EXPECT_TRUE(p.exists());
    EXPECT_TRUE(p.remove());
    EXPECT_FALSE(p.exists());
}

TEST(XPath, RemoveNonExistent) {
    XPath p("/tmp/aura_xpath_no_such_file_xyz");
    EXPECT_FALSE(p.remove());
}

TEST(XPath, Rename) {
    XPath src("/tmp/aura_xpath_rename_src.txt");
    XPath dst("/tmp/aura_xpath_rename_dst.txt");
    std::ofstream(src.str()) << "data";
    EXPECT_TRUE(src.rename(dst));
    EXPECT_FALSE(src.exists());
    EXPECT_TRUE(dst.exists());
    dst.remove();
}

TEST(XPath, Copy) {
    XPath src("/tmp/aura_xpath_copy_src.txt");
    XPath dst("/tmp/aura_xpath_copy_dst.txt");
    std::ofstream(src.str()) << "copy_data";
    EXPECT_TRUE(src.copy(dst));
    EXPECT_TRUE(src.exists());
    EXPECT_TRUE(dst.exists());
    // verify content
    std::ifstream ifs(dst.str());
    std::string content;
    std::getline(ifs, content);
    EXPECT_EQ(content, "copy_data");
    src.remove();
    dst.remove();
}

// ============================================================================
// absolute / relative
// ============================================================================

TEST(XPath, Absolute) {
    XPath p(".");
    XPath abs = p.absolute();
    EXPECT_FALSE(abs.str().empty());
    EXPECT_NE(abs.str().find("/"), std::string::npos);
}

TEST(XPath, Relative) {
    XPath a("/tmp/a/b");
    XPath base("/tmp");
    XPath rel = a.relative(base);
    EXPECT_EQ(rel.str(), "a/b");
}

// ============================================================================
// Comparison operators
// ============================================================================

TEST(XPath, OperatorNotEqual) {
    XPath a("/a"), b("/b");
    EXPECT_TRUE(a != b);
    EXPECT_FALSE(a != a);
}

TEST(XPath, OperatorLess) {
    XPath a("/a"), b("/b");
    // Lexicographic ordering
    EXPECT_TRUE(a < b || b < a);  // at least one is true (strict weak ordering)
}

// ============================================================================
// native
// ============================================================================

TEST(XPath, Native) {
    XPath p("/tmp/test");
    const auto& native = p.native();
    EXPECT_EQ(native.string(), "/tmp/test");
}

#endif  // ENABLE_TEST_XPATH
