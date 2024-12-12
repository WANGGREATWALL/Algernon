#ifndef __XREGEX_H__
#define __XREGEX_H__

#include <regex>
#include <string>

class XRegex {
public:
    XRegex(std::string regex);

    void reset(std::string regex);

    bool matchable(std::string input);

    std::string findFirstOneIn(std::string input);
    std::string findLastOneIn(std::string input);
    std::vector<std::string> findAllIn(std::string input);

private:
    std::regex mRegex;
};

#endif // __XREGEX_H__