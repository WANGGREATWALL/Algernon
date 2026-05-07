#include "file/xpath.h"
#include <regex>

namespace algernon { namespace file {

std::vector<XPath> XPath::listFiles() const {
    std::vector<XPath> result;
    if (!isDirectory()) return result;
    for (auto& entry : std::filesystem::directory_iterator(mPath)) {
        if (entry.is_regular_file()) {
            result.emplace_back(entry.path());
        }
    }
    return result;
}

std::vector<XPath> XPath::listDirs() const {
    std::vector<XPath> result;
    if (!isDirectory()) return result;
    for (auto& entry : std::filesystem::directory_iterator(mPath)) {
        if (entry.is_directory()) {
            result.emplace_back(entry.path());
        }
    }
    return result;
}

std::vector<XPath> XPath::listAll() const {
    std::vector<XPath> result;
    if (!isDirectory()) return result;
    for (auto& entry : std::filesystem::directory_iterator(mPath)) {
        result.emplace_back(entry.path());
    }
    return result;
}

std::vector<XPath> XPath::glob(const std::string& pattern) const {
    std::vector<XPath> result;
    if (!isDirectory()) return result;
    std::regex re(pattern);
    for (auto& entry : std::filesystem::directory_iterator(mPath)) {
        if (std::regex_match(entry.path().filename().string(), re)) {
            result.emplace_back(entry.path());
        }
    }
    return result;
}

bool XPath::remove() const {
    std::error_code ec;
    return std::filesystem::remove(mPath, ec);
}

bool XPath::rename(const XPath& newPath) const {
    std::error_code ec;
    std::filesystem::rename(mPath, newPath.mPath, ec);
    return !ec;
}

bool XPath::copy(const XPath& dest) const {
    std::error_code ec;
    std::filesystem::copy(mPath, dest.mPath, std::filesystem::copy_options::recursive, ec);
    return !ec;
}

}} // namespace algernon::file
