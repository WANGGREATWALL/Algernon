#ifndef ALGERNON_FILE_XFILE_H_
#define ALGERNON_FILE_XFILE_H_

/**
 * @file xfile.h
 * @brief File I/O utilities: read/write file contents, directory listing.
 *
 * @example
 *   std::string content;
 *   algernon::file::XFile::loadToString("config.json", content);
 *   algernon::file::XFile::saveFromString(content, "config_bak.json");
 */

#include <string>
#include <vector>
#include "algernon/memory/xbuffer.h"

namespace algernon { namespace file {

bool exists(const std::string& filename);
bool isDirectory(const std::string& dir);
int  createDirectory(const std::string& dir);

class XFile {
public:
    static int loadToBuffer(const std::string& filename, algernon::memory::XBuffer<char>& buffer);
    static int loadToString(const std::string& filename, std::string& buffer);
    static int saveFromBuffer(const algernon::memory::XBuffer<char>& buffer, const std::string& filename);
    static int saveFromString(const std::string& content, const std::string& filename);
};

class XFileList {
public:
    /** @brief Get all entries (full path) in a directory. */
    static std::vector<std::string> getFullListIn(const std::string& folder);

    /** @brief Get entries filtered by regex pattern. */
    static std::vector<std::string> getFilteredListIn(const std::string& folder, const std::string& regex);
};

class XFileName {
public:
    static std::string getFolder(const std::string& filename);
    static std::string stripPath(const std::string& filename);
    static std::string stripPathAndExt(const std::string& filename);

    struct ImageSize { uint32_t width; uint32_t height; };
    static ImageSize getFirstFoundImageSize(const std::string& filename);
    static ImageSize getLastFoundImageSize(const std::string& filename);

    static std::string getFirstMatchByRegex(const std::string& filename, const std::string& regex);
    static std::string getLastMatchByRegex(const std::string& filename, const std::string& regex);
};

}} // namespace algernon::file

#endif // ALGERNON_FILE_XFILE_H_
