#include "xregex.h"
#include "logger.h"

XRegex::XRegex(std::string regex)
{
    mRegex = regex;
}

void XRegex::reset(std::string regex)
{
    mRegex = regex;
}

bool XRegex::matchable(std::string input)
{
    std::smatch result;
    return std::regex_search(input, result, mRegex);
}

std::string XRegex::findFirstOneIn(std::string input)
{
    std::vector<std::string> strings = findAllIn(input);
    if (strings.empty()) {
        return std::string();
    }
    return strings.front();
}

std::string XRegex::findLastOneIn(std::string input)
{
    std::vector<std::string> strings = findAllIn(input);
    if (strings.empty()) {
        return std::string();
    }
    return strings.back();
}

std::vector<std::string> XRegex::findAllIn(std::string input)
{
    std::sregex_iterator it(input.begin(), input.end(), mRegex);
    std::sregex_iterator end;

    std::vector<std::string> strings;
    for (; it != end; ++it) {
        strings.push_back(it->str());
    }
    return strings;
}