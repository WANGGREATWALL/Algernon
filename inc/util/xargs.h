#ifndef AURA_UTIL_XARGS_H_
#define AURA_UTIL_XARGS_H_

/**
 * @file xargs.h
 * @brief Command-line argument parser supporting short and long options.
 *
 * Supported argument formats:
 *   -x               short option without value
 *   -x value         short option with value
 *   -abc value       combined short options (last gets the value)
 *   --option         long option without value
 *   --option value   long option with value
 *   --option=value   long option with = syntax
 *
 * @example
 *   int level = 0;
 *   bool help = false;
 *   au::util::XArgs parser([&](char s, const std::string& l,
 *                                     au::util::XArgs::Value& v) {
 *       if (s == 'h' || l == "help") help = true;
 *       else if (s == 'l' || l == "level") level = std::stoi(v.get());
 *       else return false;
 *       return true;
 *   });
 *   parser.parse(argc, argv);
 */

#include <string>
#include <vector>
#include <functional>

namespace au { namespace util {

class XArgs {
public:
    class Value {
    public:
        bool isValid() const { return mValid; }
        void set(const std::string& val = "") { mValue = val; mValid = true; }
        std::string get() const { return mValue; }
    private:
        bool mValid = false;
        std::string mValue;
    };

    /**
     * @brief Handler callback type.
     * @param shortOpt Single char option (0 if long option).
     * @param longOpt  Long option name (empty if short option).
     * @param value    The option's value (check isValid() first).
     * @return true to continue parsing, false to stop.
     */
    using Handler = std::function<bool(char, const std::string&, Value&)>;

    explicit XArgs(const Handler& handler) : mHandler(handler) {}

    bool parse(int argc, const char* const* argv, int start = 1);
    bool parse(const std::vector<std::string>& args, int start = 1);

private:
    Handler mHandler;
};

}}  // namespace au::util

#endif // AURA_UTIL_XARGS_H_
