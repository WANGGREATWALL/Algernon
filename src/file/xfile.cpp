#include "algernon/file/xfile.h"
#include "algernon/log/xlogger.h"
#include "algernon/file/xpath.h"

#include <fstream>
#include <sstream>
#include <filesystem>
#include <regex>

namespace algernon { namespace file {

bool exists(const std::string& filename) {
    return std::filesystem::exists(filename);
}

bool isDirectory(const std::string& dir) {
    return std::filesystem::is_directory(dir);
}

int createDirectory(const std::string& dir) {
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    XASSERT_RET(!ec, kErrorOpenFailed);
    return kSuccess;
}

// -- XFile --

int XFile::loadToBuffer(const std::string& filename, algernon::memory::XBuffer<char>& buffer) {
    XASSERT_RET(exists(filename), kErrorFileNotFound);

    std::ifstream ifs(filename, std::ios::binary | std::ios::ate);
    XASSERT_RET(ifs.is_open(), kErrorOpenFailed);

    auto size = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    buffer = algernon::memory::XBuffer<char>(static_cast<size_t>(size) + 1);
    ifs.read(buffer.data(), size);
    buffer.data()[size] = '\0';

    return kSuccess;
}

int XFile::loadToString(const std::string& filename, std::string& buffer) {
    XASSERT_RET(exists(filename), kErrorFileNotFound);

    std::ifstream ifs(filename, std::ios::binary);
    XASSERT_RET(ifs.is_open(), kErrorOpenFailed);

    std::ostringstream ss;
    ss << ifs.rdbuf();
    buffer = ss.str();

    return kSuccess;
}

int XFile::saveFromBuffer(const algernon::memory::XBuffer<char>& buffer, const std::string& filename) {
    std::ofstream ofs(filename, std::ios::binary);
    XASSERT_RET(ofs.is_open(), kErrorOpenFailed);
    ofs.write(buffer.data(), static_cast<std::streamsize>(buffer.size()));
    return kSuccess;
}

int XFile::saveFromString(const std::string& content, const std::string& filename) {
    std::ofstream ofs(filename, std::ios::binary);
    XASSERT_RET(ofs.is_open(), kErrorOpenFailed);
    ofs << content;
    return kSuccess;
}

// -- XFileList --

std::vector<std::string> XFileList::getFullListIn(const std::string& folder) {
    std::vector<std::string> result;
    if (!std::filesystem::is_directory(folder)) return result;
    for (auto& entry : std::filesystem::directory_iterator(folder)) {
        result.push_back(entry.path().string());
    }
    return result;
}

std::vector<std::string> XFileList::getFilteredListIn(const std::string& folder,
                                                       const std::string& regex) {
    std::vector<std::string> result;
    if (!std::filesystem::is_directory(folder)) return result;
    std::regex re(regex);
    for (auto& entry : std::filesystem::directory_iterator(folder)) {
        if (std::regex_match(entry.path().filename().string(), re)) {
            result.push_back(entry.path().string());
        }
    }
    return result;
}

// -- XFileName --

std::string XFileName::getFolder(const std::string& filename) {
    return std::filesystem::path(filename).parent_path().string();
}

std::string XFileName::stripPath(const std::string& filename) {
    return std::filesystem::path(filename).filename().string();
}

std::string XFileName::stripPathAndExt(const std::string& filename) {
    return std::filesystem::path(filename).stem().string();
}

XFileName::ImageSize XFileName::getFirstFoundImageSize(const std::string& filename) {
    std::regex re(R"((\d+)x(\d+))");
    std::smatch m;
    if (std::regex_search(filename, m, re)) {
        return {static_cast<uint32_t>(std::stoul(m[1].str())),
                static_cast<uint32_t>(std::stoul(m[2].str()))};
    }
    return {0, 0};
}

XFileName::ImageSize XFileName::getLastFoundImageSize(const std::string& filename) {
    std::regex re(R"((\d+)x(\d+))");
    std::sregex_iterator it(filename.begin(), filename.end(), re);
    std::sregex_iterator end;

    ImageSize size{0, 0};
    for (; it != end; ++it) {
        size = {static_cast<uint32_t>(std::stoul((*it)[1].str())),
                static_cast<uint32_t>(std::stoul((*it)[2].str()))};
    }
    return size;
}

std::string XFileName::getFirstMatchByRegex(const std::string& filename, const std::string& regex) {
    std::regex re(regex);
    std::smatch m;
    if (std::regex_search(filename, m, re)) {
        return m.str();
    }
    return {};
}

std::string XFileName::getLastMatchByRegex(const std::string& filename, const std::string& regex) {
    std::regex re(regex);
    std::sregex_iterator it(filename.begin(), filename.end(), re);
    std::sregex_iterator end;
    std::string last;
    for (; it != end; ++it) {
        last = it->str();
    }
    return last;
}

}} // namespace algernon::file
