#include "argument_parser.h"
#include "log/logger.h"

namespace util
{

/**
 * @brief remove quotation marks from string
 */
std::string stripQuot(const std::string& str)
{
    auto charFirst = str.front();
    auto charLast = str.back();

    if (charFirst == charLast && (charFirst == '\'' || charFirst == '\"')) {
        return str.substr(1, str.length() - 2);
    } else {
        return str;
    }
}

/**
 * @brief pick key and value from '--key=value'
 */
bool pickKeyAndValue(const std::string& str, std::string& key, std::string& value)
{
    auto pos = str.find_first_of('=');
    if (pos == std::string::npos) {
        return false;
    }

    key = str.substr(0, pos);
    value = stripQuot(str.substr(pos + 1));
    return true;
}

bool ArgumentParser::parse(int argc, const char* const* const argv, int start)
{
    ASSERTER(argc >= 1);
    ASSERTER(argv != nullptr);
    ASSERTER(start >= 0);

    std::vector<std::string> args;
    for (int i = start; i < argc; ++i) {
        const char* curr = argv[i];
        if (curr == nullptr) return false;
        args.push_back(curr);
    }
    return parse(args, 0);
}

bool ArgumentParser::parse(const std::vector<std::string>& args, int start)
{
    for (size_t i = start; i < args.size(); ++i) {
        const std::string& curr = args.at(i);

        if (curr[0] == '-') {
            ValueOption valueOption;
            if (i != (args.size() - 1) && args.at(i + 1).front() != '-') { // skip the last or empty option 
                valueOption.set(args.at(i + 1));
            } else {
                valueOption.set(); // set to valid but empty value
            }

            if (curr[1] == '-') { // long_option --xxxx
                const std::string opt = curr.substr(2);
                std::string key, value;
                if (pickKeyAndValue(opt, key, value)) {
                    // handle '--key=value'
                    ValueOption tmp;
                    tmp.set(value);
                    if (!mHandler(0, key, tmp)) return false;
                } else {
                    // handle '--key value'
                    if (!mHandler(0, opt, valueOption)) return false;
                }
            } else { // short_option -xyz pattern
                for (size_t j = 1; j < curr.size(); ++j) {
                    char opt = curr[j];
                    if (j != (curr.size() - 1)) {
                        ValueOption tmp;
                        tmp.set();
                        if (!mHandler(opt, "", tmp)) return false;
                    } else {
                        // only the last char ('z') possess value
                        if (!mHandler(opt, "", valueOption)) return false;
                    }
                }
            }
        }
    }

    return true;
}

} // namespace util