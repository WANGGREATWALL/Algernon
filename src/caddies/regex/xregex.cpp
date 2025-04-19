#include "regex/xregex.h"
#include "log/logger.h"


namespace re
{


bool matchRegexInString(const std::string& content, const std::string& regex)
{
    std::regex pattern{regex};
    return std::regex_search(content, pattern);
}

std::string getFirstMatchInString(const std::string& content, const std::string& regex)
{
    std::vector<std::string> strings = getAllMatchesInString(content, regex);
    if (strings.empty()) {
        return std::string();
    }
    return strings.front();
}

std::string getLastMatchInString(const std::string& content, const std::string& regex)
{
    std::vector<std::string> strings = getAllMatchesInString(content, regex);
    if (strings.empty()) {
        return std::string();
    }
    return strings.back();
}

std::vector<std::string> getAllMatchesInString(const std::string& content, const std::string& regex)
{
    std::regex pattern{regex};
    std::sregex_iterator it(content.begin(), content.end(), pattern);
    std::sregex_iterator end;

    std::vector<std::string> strings;
    for (; it != end; ++it) {
        strings.push_back(it->str());
    }
    return strings;
}

std::string replaceAllMatchesInString(const std::string& content, const std::string& regex, const std::string& replacement)
{
    std::regex pattern{regex};
    return std::regex_replace(content, pattern, replacement);
}

std::string replaceSpecificMatchInString(const std::string& content, const std::string& regex, const std::string& replacement, int idx)
{
    std::string result = content;

    std::regex pattern{regex};
    std::sregex_iterator it(content.begin(), content.end(), pattern);
    std::sregex_iterator end;

    size_t idxCurrent = 0;

    for (; it != end; ++it) {
        if (idxCurrent == idx) {
            auto match = *it;
            result.replace(match.position(), match.length(), replacement);
            break;
        }
        ++idxCurrent;
    }

    return result;
}


} // namespace re