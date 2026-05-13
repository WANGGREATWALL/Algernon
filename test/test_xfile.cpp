#if ENABLE_TEST_XFILE

#include <cstdio>
#include <fstream>

#include "gtest/gtest.h"
#include "file/xfile.h"
#include "memory/xbuffer.h"

using au::file::XFile;
using au::file::XFileList;
using au::file::XFileName;

// ============================================================================
// Helpers
// ============================================================================

namespace {

std::string tmpPath(const std::string& name)
{
    return "/tmp/aura_xfile_test_" + name;
}

void writeFile(const std::string& path, const std::string& content)
{
    std::ofstream ofs(path, std::ios::binary);
    ofs << content;
}

}  // namespace

// ============================================================================
// exists / isDirectory / createDirectory
// ============================================================================

TEST(XFile, exists_true_for_existing_path)
{
    EXPECT_TRUE(au::file::exists("/tmp"));
}

TEST(XFile, exists_false_for_non_existent)
{
    EXPECT_FALSE(au::file::exists("/tmp/__aura_nonexistent_xyz__"));
}

TEST(XFile, isDirectory_true_for_directory)
{
    EXPECT_TRUE(au::file::isDirectory("/tmp"));
}

TEST(XFile, isDirectory_false_for_file)
{
    // Use a known file
    EXPECT_FALSE(au::file::isDirectory("/etc/hosts"));
}

TEST(XFile, createDirectory_success)
{
    std::string p = tmpPath("createdir");
    std::string sub = p + "/sub";
    int ret = au::file::createDirectory(sub);
    EXPECT_EQ(ret, 0);
    EXPECT_TRUE(au::file::isDirectory(sub));
    // cleanup
    std::remove(sub.c_str());
    std::remove(p.c_str());
}

TEST(XFile, createDirectory_already_exists_succeeds)
{
    int ret = au::file::createDirectory("/tmp");
    EXPECT_EQ(ret, 0);
}

// ============================================================================
// XFile load / save
// ============================================================================

TEST(XFile, loadToString_normal_file)
{
    std::string p = tmpPath("loadstr");
    writeFile(p, "hello world");
    std::string out;
    int ret = XFile::loadToString(p, out);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(out, "hello world");
    std::remove(p.c_str());
}

TEST(XFile, loadToString_empty_file)
{
    std::string p = tmpPath("loadstr_empty");
    writeFile(p, "");
    std::string out = "not_empty";
    int ret = XFile::loadToString(p, out);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(out, "");
    std::remove(p.c_str());
}

TEST(XFile, loadToString_non_existent)
{
    std::string out;
    int ret = XFile::loadToString("/tmp/__aura_no_such_file__", out);
    EXPECT_NE(ret, 0);
}

TEST(XFile, loadToBuffer_normal_file)
{
    std::string p = tmpPath("loadbuf");
    std::string content("binary\0data", 11);
    writeFile(p, content);
    au::memory::XBuffer<char> buf;
    int ret = XFile::loadToBuffer(p, buf);
    EXPECT_EQ(ret, 0);
    // XFile appends a null terminator, so buffer size = file_size + 1
    EXPECT_EQ(buf.size(), content.size() + 1);
    EXPECT_EQ(std::memcmp(buf.data(), content.data(), content.size()), 0);
    EXPECT_EQ(buf.data()[content.size()], '\0');
    std::remove(p.c_str());
}

TEST(XFile, loadToBuffer_empty_file)
{
    std::string p = tmpPath("loadbuf_empty");
    writeFile(p, "");
    au::memory::XBuffer<char> buf;
    int ret = XFile::loadToBuffer(p, buf);
    EXPECT_EQ(ret, 0);
    // Empty file → 1-byte buffer (null terminator only)
    EXPECT_EQ(buf.size(), 1u);
    EXPECT_EQ(buf.data()[0], '\0');
    std::remove(p.c_str());
}

TEST(XFile, loadToBuffer_non_existent)
{
    au::memory::XBuffer<char> buf;
    int ret = XFile::loadToBuffer("/tmp/__aura_no_such_file__", buf);
    EXPECT_NE(ret, 0);
}

TEST(XFile, saveFromString_and_loadToString_roundtrip)
{
    std::string p = tmpPath("roundtrip_str");
    std::string content = "line1\nline2\n";
    int ret = XFile::saveFromString(content, p);
    EXPECT_EQ(ret, 0);
    std::string out;
    ret = XFile::loadToString(p, out);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(out, content);
    std::remove(p.c_str());
}

TEST(XFile, saveFromString_empty_content)
{
    std::string p = tmpPath("save_empty");
    int ret = XFile::saveFromString("", p);
    EXPECT_EQ(ret, 0);
    std::string out;
    XFile::loadToString(p, out);
    EXPECT_EQ(out, "");
    std::remove(p.c_str());
}

TEST(XFile, saveFromString_multiline)
{
    std::string p = tmpPath("save_multiline");
    std::string content = "line1\nline2\nline3\n";
    XFile::saveFromString(content, p);
    std::string out;
    XFile::loadToString(p, out);
    EXPECT_EQ(out, content);
    std::remove(p.c_str());
}

TEST(XFile, saveFromBuffer_and_loadToBuffer_roundtrip)
{
    std::string p = tmpPath("roundtrip_buf");
    std::string raw("binary\0payload\xff", 16);
    au::memory::XBuffer<char> buf(raw.size());
    std::memcpy(buf.data(), raw.data(), raw.size());
    int ret = XFile::saveFromBuffer(buf, p);
    EXPECT_EQ(ret, 0);
    au::memory::XBuffer<char> out;
    ret = XFile::loadToBuffer(p, out);
    EXPECT_EQ(ret, 0);
    // loadToBuffer adds null terminator
    EXPECT_EQ(out.size(), raw.size() + 1);
    EXPECT_EQ(std::memcmp(out.data(), raw.data(), raw.size()), 0);
    EXPECT_EQ(out.data()[raw.size()], '\0');
    std::remove(p.c_str());
}

TEST(XFile, saveFromBuffer_empty_buffer)
{
    std::string p = tmpPath("save_buf_empty");
    au::memory::XBuffer<char> buf;
    int ret = XFile::saveFromBuffer(buf, p);
    EXPECT_EQ(ret, 0);
    au::memory::XBuffer<char> out;
    XFile::loadToBuffer(p, out);
    // loadToBuffer null-terminates empty file → 1 byte
    EXPECT_EQ(out.size(), 1u);
    EXPECT_EQ(out.data()[0], '\0');
    std::remove(p.c_str());
}

// ============================================================================
// XFileList
// ============================================================================

TEST(XFileList, getFullListIn_valid_directory)
{
    auto list = XFileList::getFullListIn("/tmp");
    // /tmp should have entries
    EXPECT_FALSE(list.empty());
}

TEST(XFileList, getFullListIn_non_directory_returns_empty)
{
    auto list = XFileList::getFullListIn("/tmp/__aura_nonexistent_dir__");
    EXPECT_TRUE(list.empty());
}

TEST(XFileList, getFilteredListIn_with_matching_pattern)
{
    // Create temp files to filter
    std::string dir = tmpPath("filterdir");
    au::file::createDirectory(dir);
    writeFile(dir + "/test_a.txt", "a");
    writeFile(dir + "/test_b.txt", "b");
    writeFile(dir + "/other.log", "log");

    auto list = XFileList::getFilteredListIn(dir, R"(test_.*\.txt)");
    EXPECT_EQ(list.size(), 2u);
    // cleanup
    std::remove((dir + "/test_a.txt").c_str());
    std::remove((dir + "/test_b.txt").c_str());
    std::remove((dir + "/other.log").c_str());
    std::remove(dir.c_str());
}

TEST(XFileList, getFilteredListIn_no_match_returns_empty)
{
    std::string dir = tmpPath("filterdir_nomatch");
    au::file::createDirectory(dir);
    writeFile(dir + "/data.txt", "x");

    auto list = XFileList::getFilteredListIn(dir, R"(no_such_pattern)");
    EXPECT_TRUE(list.empty());
    std::remove((dir + "/data.txt").c_str());
    std::remove(dir.c_str());
}

// ============================================================================
// XFileName
// ============================================================================

TEST(XFileName, getFolder)
{
    EXPECT_EQ(XFileName::getFolder("/a/b/c.txt"), "/a/b");
    EXPECT_EQ(XFileName::getFolder("c.txt"), "");
    EXPECT_EQ(XFileName::getFolder("/root_file"), "/");
    EXPECT_EQ(XFileName::getFolder("/"), "/");
}

TEST(XFileName, stripPath)
{
    EXPECT_EQ(XFileName::stripPath("/a/b/c.txt"), "c.txt");
    EXPECT_EQ(XFileName::stripPath("c.txt"), "c.txt");
    EXPECT_EQ(XFileName::stripPath("/a/"), "");
}

TEST(XFileName, stripPathAndExt)
{
    EXPECT_EQ(XFileName::stripPathAndExt("/a/b/c.txt"), "c");
    EXPECT_EQ(XFileName::stripPathAndExt("file"), "file");
    EXPECT_EQ(XFileName::stripPathAndExt("/a/b/file.tar.gz"), "file.tar");
    EXPECT_EQ(XFileName::stripPathAndExt(".hidden"), ".hidden");
}

TEST(XFileName, getFirstFoundImageSize_match)
{
    auto sz = XFileName::getFirstFoundImageSize("image_1920x1080.png");
    EXPECT_EQ(sz.width, 1920u);
    EXPECT_EQ(sz.height, 1080u);
}

TEST(XFileName, getFirstFoundImageSize_multiple_matches)
{
    // First match should be "800x600", not "1920x1080"
    auto sz = XFileName::getFirstFoundImageSize("res_800x600_1920x1080.jpg");
    EXPECT_EQ(sz.width, 800u);
    EXPECT_EQ(sz.height, 600u);
}

TEST(XFileName, getFirstFoundImageSize_no_match)
{
    auto sz = XFileName::getFirstFoundImageSize("no_dimensions_here.txt");
    EXPECT_EQ(sz.width, 0u);
    EXPECT_EQ(sz.height, 0u);
}

TEST(XFileName, getLastFoundImageSize_multiple_matches)
{
    auto sz = XFileName::getLastFoundImageSize("res_800x600_1920x1080.jpg");
    EXPECT_EQ(sz.width, 1920u);
    EXPECT_EQ(sz.height, 1080u);
}

TEST(XFileName, getLastFoundImageSize_no_match)
{
    auto sz = XFileName::getLastFoundImageSize("no_dimensions_here.txt");
    EXPECT_EQ(sz.width, 0u);
    EXPECT_EQ(sz.height, 0u);
}

TEST(XFileName, getFirstMatchByRegex_match)
{
    auto s = XFileName::getFirstMatchByRegex("abc123def", R"(\d+)");
    EXPECT_EQ(s, "123");
}

TEST(XFileName, getFirstMatchByRegex_no_match)
{
    auto s = XFileName::getFirstMatchByRegex("abcdef", R"(\d+)");
    EXPECT_EQ(s, "");
}

TEST(XFileName, getLastMatchByRegex_multiple_matches)
{
    auto s = XFileName::getLastMatchByRegex("a1b2c3", R"(\d)");
    EXPECT_EQ(s, "3");
}

TEST(XFileName, getLastMatchByRegex_no_match)
{
    auto s = XFileName::getLastMatchByRegex("abcdef", R"(\d+)");
    EXPECT_EQ(s, "");
}

#endif  // ENABLE_TEST_XFILE
