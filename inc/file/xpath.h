#ifndef ALGERNON_FILE_XPATH_H_
#define ALGERNON_FILE_XPATH_H_

/**
 * @file xpath.h
 * @brief Pythonic path utility wrapping std::filesystem::path.
 *
 * @example
 *   using algernon::file::XPath;
 *   XPath p("/home/user/data");
 *   XPath file = p / "images" / "test.png";
 *   if (file.exists()) printf("size: %llu\n", file.fileSize());
 *
 *   for (auto& f : p.glob(R"(.*\.png)")) {
 *       printf("%s\n", f.str().c_str());
 *   }
 */

#include <string>
#include <vector>
#include <cstdint>
#include <filesystem>

namespace algernon { namespace file {

class XPath {
public:
    XPath() = default;
    XPath(const std::string& path) : mPath(path) {}
    XPath(const char* path) : mPath(path) {}
    XPath(const std::filesystem::path& path) : mPath(path) {}

    // -- String conversion --

    /** @brief Get path as std::string. */
    std::string str() const { return mPath.string(); }
    operator std::string() const { return str(); }
    const char* c_str() const { return mPath.c_str(); }

    // -- Concatenation --

    XPath operator/(const char* sub) const { return XPath(mPath / sub); }
    XPath operator/(const std::string& sub) const { return XPath(mPath / sub); }
    XPath operator/(const XPath& sub) const { return XPath(mPath / sub.mPath); }
    XPath& operator/=(const std::string& sub) { mPath /= sub; return *this; }

    // -- Decomposition --

    /** @brief Parent directory path. */
    XPath parent() const { return XPath(mPath.parent_path()); }

    /** @brief Filename with extension, e.g. "image.png". */
    std::string filename() const { return mPath.filename().string(); }

    /** @brief Filename without extension, e.g. "image". */
    std::string stem() const { return mPath.stem().string(); }

    /** @brief File extension including dot, e.g. ".png". */
    std::string extension() const { return mPath.extension().string(); }

    // -- Queries --

    bool exists() const { return std::filesystem::exists(mPath); }
    bool isFile() const { return std::filesystem::is_regular_file(mPath); }
    bool isDirectory() const { return std::filesystem::is_directory(mPath); }
    bool isEmpty() const { return mPath.empty(); }
    uint64_t fileSize() const { return std::filesystem::file_size(mPath); }

    // -- Directory operations --

    /** @brief Recursively create directories. Returns true on success. */
    bool createDirs() const { return std::filesystem::create_directories(mPath); }

    /** @brief List regular files in this directory (non-recursive). */
    std::vector<XPath> listFiles() const;

    /** @brief List subdirectories in this directory (non-recursive). */
    std::vector<XPath> listDirs() const;

    /** @brief List all entries in this directory (non-recursive). */
    std::vector<XPath> listAll() const;

    /**
     * @brief Filter directory entries by regex pattern.
     * @param pattern Regex applied to filename (not full path).
     * @example dir.glob(R"(.*\.png)") -- all .png files
     */
    std::vector<XPath> glob(const std::string& pattern) const;

    // -- File operations --

    bool remove() const;
    bool rename(const XPath& newPath) const;
    bool copy(const XPath& dest) const;

    // -- Path transformations --

    XPath absolute() const { return XPath(std::filesystem::absolute(mPath)); }
    XPath relative(const XPath& base) const {
        return XPath(std::filesystem::relative(mPath, base.mPath));
    }

    // -- Comparison --

    bool operator==(const XPath& other) const { return mPath == other.mPath; }
    bool operator!=(const XPath& other) const { return mPath != other.mPath; }
    bool operator<(const XPath& other) const  { return mPath < other.mPath; }

    /** @brief Access underlying std::filesystem::path. */
    const std::filesystem::path& native() const { return mPath; }

private:
    std::filesystem::path mPath;
};

}} // namespace algernon::file

#endif // ALGERNON_FILE_XPATH_H_
