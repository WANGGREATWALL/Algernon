#include "algernon/util/xargs.h"

namespace algernon { namespace util {

bool XArgs::parse(int argc, const char* const* argv, int start) {
    std::vector<std::string> args;
    for (int i = 0; i < argc; ++i) {
        args.emplace_back(argv[i]);
    }
    return parse(args, start);
}

bool XArgs::parse(const std::vector<std::string>& args, int start) {
    for (size_t i = static_cast<size_t>(start); i < args.size(); ++i) {
        const auto& arg = args[i];

        if (arg.size() >= 2 && arg[0] == '-' && arg[1] == '-') {
            // Long option
            std::string opt = arg.substr(2);
            std::string longName;
            Value val;

            auto eqPos = opt.find('=');
            if (eqPos != std::string::npos) {
                longName = opt.substr(0, eqPos);
                std::string rawVal = opt.substr(eqPos + 1);
                // Strip quotes
                if (rawVal.size() >= 2 &&
                    ((rawVal.front() == '\'' && rawVal.back() == '\'') ||
                     (rawVal.front() == '"' && rawVal.back() == '"'))) {
                    rawVal = rawVal.substr(1, rawVal.size() - 2);
                }
                val.set(rawVal);
            } else {
                longName = opt;
                if (i + 1 < args.size() && !args[i + 1].empty() && args[i + 1][0] != '-') {
                    val.set(args[++i]);
                } else {
                    val.set();
                }
            }

            if (!mHandler(0, longName, val)) return false;

        } else if (arg.size() >= 2 && arg[0] == '-') {
            // Short option(s)
            for (size_t j = 1; j < arg.size(); ++j) {
                char c = arg[j];
                Value val;

                if (j == arg.size() - 1) {
                    // Last char: may have value
                    if (i + 1 < args.size() && !args[i + 1].empty() && args[i + 1][0] != '-') {
                        val.set(args[++i]);
                    } else {
                        val.set();
                    }
                } else {
                    val.set();
                }

                if (!mHandler(c, std::string(), val)) return false;
            }
        }
    }
    return true;
}

}} // namespace algernon::util
