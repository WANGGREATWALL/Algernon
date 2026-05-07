#ifndef ALGERNON_REGEX_XREGEX_H_
#define ALGERNON_REGEX_XREGEX_H_

/**
 * @file xregex.h
 * @brief Regex utilities: match, search, extract, replace, split.
 *
 * @example
 *   using namespace algernon::regex;
 *
 *   // Check if pattern exists in text
 *   bool ok = match("Hello World", "Hello");
 *
 *   // Extract capture groups
 *   auto groups = extractGroups("2026-04-30", R"((\d{4})-(\d{2})-(\d{2}))");
 *   // groups = {"2026", "04", "30"}
 *
 *   // Split by pattern
 *   auto parts = split("a,b,,c", ",");
 *   // parts = {"a", "b", "", "c"}
 */

#include <regex>
#include <string>
#include <vector>

namespace algernon {
namespace regex {

/**
 * @brief Check if pattern matches anywhere in the string.
 * @example match("hello@world.com", R"(\w+@\w+\.\w+)") => true
 */
bool match(const std::string& content, const std::string& pattern);

/**
 * @brief Check if the entire string matches the pattern.
 * @example fullMatch("12345", R"(\d+)") => true
 *          fullMatch("123ab", R"(\d+)") => false
 */
bool fullMatch(const std::string& content, const std::string& pattern);

/**
 * @brief Get the first match in the string.
 * @return Matched substring, or empty string if no match.
 */
std::string getFirstMatch(const std::string& content, const std::string& pattern);

/**
 * @brief Get the last match in the string.
 * @return Matched substring, or empty string if no match.
 */
std::string getLastMatch(const std::string& content, const std::string& pattern);

/**
 * @brief Get all non-overlapping matches.
 * @example getAllMatches("aaa bbb aaa", "aaa") => {"aaa", "aaa"}
 */
std::vector<std::string> getAllMatches(const std::string& content, const std::string& pattern);

/**
 * @brief Extract capture groups from the first match.
 * @return Vector of captured group strings (excluding group 0 = full match).
 * @example extractGroups("2026-04-30", R"((\d{4})-(\d{2})-(\d{2}))") => {"2026","04","30"}
 */
std::vector<std::string> extractGroups(const std::string& content, const std::string& pattern);

/**
 * @brief Replace all matches with the replacement string.
 * @example replaceAll("hello hello", "hello", "hi") => "hi hi"
 */
std::string replaceAll(const std::string& content, const std::string& pattern, const std::string& replacement);

/**
 * @brief Replace the Nth match (0-indexed).
 * @example replaceNth("aa bb aa", "aa", "cc", 1) => "aa bb cc"
 */
std::string replaceNth(const std::string& content, const std::string& pattern, const std::string& replacement,
                       int index);

/**
 * @brief Split string by regex pattern.
 * @example split("a,b,,c", ",") => {"a", "b", "", "c"}
 */
std::vector<std::string> split(const std::string& content, const std::string& pattern);

}  // namespace regex
}  // namespace algernon

#endif  // ALGERNON_REGEX_XREGEX_H_
