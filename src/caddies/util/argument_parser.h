/**
 * Arguemnt parser.
 *
 * Example:
 *
 * test -h -l 6
 * test --help --level 6
 * test --help --level=6
 *
 * Code:
 *
 * int main(int argc, char **argv)
 * {
 * bool print_help = false;
 * int level = 0;
 * util::ArgumentParser parser(
 * [&](char optionShort, const std::string &optionLong, util::ArgumentParser::ValueOption& valueOption) {
 * if (optionShort == 'h' || optionLong == "help") {
 * print_help = true;
 * } else if (optionShort == 'l' || optionLong == "level") {
 * level = std::stoi(valueOption.get());
 * } else {
 * if (optionShort != 0)
 * cerr << "Error: invalid option `-" << optionShort << "'" << endl;
 * else
 * cerr << "Error: invalid option `--" << optionLong << "'" << endl;
 * return false;
 * }
 * return true;
 * }
 * );
 *
 * if (!parser(argc, argv))
 * return 0;
 *
 * //! ... you app code ...
 * }
 */

 #ifndef __ARGUMENT_PARSER_H__
 #define __ARGUMENT_PARSER_H__
 
 #include <string>
 #include <vector>
 #include <functional>
 
 namespace util {
 
 class ArgumentParser {
 public:
 
     class ValueOption {
     public:
         bool isValid() const { return mIsValid; }
 
         void set(const std::string& value="") {
             mValue = value;
             mIsValid = true;
         }
 
         std::string get() {
             return mValue;
         }
 
     private:
         bool mIsValid{false};
         std::string mValue;
     };
 
     /**
      * @brief Callback of ArgumentParser
      *
      * @param optionShort for example: -h, -v, -pnl
      * @param optionLong for example: --help, --version, --level
      * @param valueOption value of option
      *
      * @return true parse continue
      * @return false parse terminate
      *
      * @note
      * optionShort and optionLong
      * - when parse to short option, the "optionShort" is option you set, and "optionLong" is empty string.
      * - when parse to long option, the "optionShort" is 0, and "optionLong" is option you set.
      *
      * valueOption
      * - when option with a value, valueOption.isValid() return true, valueOption.get() return the value by string.
      * for example: '-s abc -n', when parse to 's', valueOption is valid, and the value is 'abc'. when 'n',
      * valueOption is valid, but the value is empty string.
      * - when short option has multiple char, look like '-abc 123', option 'a', 'b' will be valid without value,
      * but the last option 'c' will be valid and get the value (123).
      */
     using Handler = std::function<bool(
         char, // short option
         const std::string&, // long option
         ValueOption& // value
     )>;
 
     ArgumentParser(const Handler& handler) : mHandler(handler) {}
 
     /**
      * @brief 解析
      *
      * @param argc number of arguments
      * @param argv list of arguments
      * @param start index to begin
      *
      * @return true parse complete
      * @return false parse fail
      *
      * @note
      * 5 types of key-value arguments are supported:
      * (1) -x
      * (2) -x value
      * (3) --xxxx
      * (4) --xxxx value
      * (5) --xxxx=value
      */
     bool parse(int argc, const char* const* const argv, int start=1);
     bool parse(const std::vector<std::string>& args, int start=1);
 
 private:
     Handler mHandler;
 };
 
 } // namespace util
 
 #endif // __ARGUMENT_PARSER_H__