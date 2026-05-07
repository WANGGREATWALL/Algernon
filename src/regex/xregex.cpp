#include "regex/xregex.h"

namespace algernon { namespace regex {

bool match(const std::string& content, const std::string& pattern) {
    return std::regex_search(content, std::regex(pattern));
}

bool fullMatch(const std::string& content, const std::string& pattern) {
    return std::regex_match(content, std::regex(pattern));
}

std::string getFirstMatch(const std::string& content, const std::string& pattern) {
    auto all = getAllMatches(content, pattern);
    return all.empty() ? std::string() : all.front();
}

std::string getLastMatch(const std::string& content, const std::string& pattern) {
    auto all = getAllMatches(content, pattern);
    return all.empty() ? std::string() : all.back();
}

std::vector<std::string> getAllMatches(const std::string& content, const std::string& pattern) {
    std::regex re(pattern);
    std::sregex_iterator it(content.begin(), content.end(), re);
    std::sregex_iterator end;

    std::vector<std::string> matches;
    for (; it != end; ++it) {
        matches.push_back(it->str());
    }
    return matches;
}

std::vector<std::string> extractGroups(const std::string& content, const std::string& pattern) {
    std::regex re(pattern);
    std::smatch m;
    std::vector<std::string> groups;
    if (std::regex_search(content, m, re)) {
        for (size_t i = 1; i < m.size(); ++i) {
            groups.push_back(m[i].str());
        }
    }
    return groups;
}

std::string replaceAll(const std::string& content, const std::string& pattern,
                       const std::string& replacement) {
    return std::regex_replace(content, std::regex(pattern), replacement);
}

std::string replaceNth(const std::string& content, const std::string& pattern,
                       const std::string& replacement, int index) {
    std::string result = content;
    std::regex re(pattern);
    std::sregex_iterator it(content.begin(), content.end(), re);
    std::sregex_iterator end;

    int current = 0;
    for (; it != end; ++it, ++current) {
        if (current == index) {
            auto match = *it;
            result.replace(match.position(), match.length(), replacement);
            break;
        }
    }
    return result;
}

std::vector<std::string> split(const std::string& content, const std::string& pattern) {
    std::regex re(pattern);
    std::sregex_token_iterator it(content.begin(), content.end(), re, -1);
    std::sregex_token_iterator end;
    return std::vector<std::string>(it, end);
}

}} // namespace algernon::regex
