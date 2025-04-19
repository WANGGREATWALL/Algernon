#ifndef _XREGEX_H_
#define _XREGEX_H_

#include <regex>
#include <string>

namespace re
{

/**
 * @brief check whether regular can match string
 *
 * @param content string to be check
 * @param regex regex
 *
 * @return if matchable return true, else false
 */
bool matchRegexInString(const std::string& content, const std::string& regex);

/**
 * @brief get first match in string
 *
 * @param content string to be check
 * @param regex regex
 *
 * @return if success return string, else empty string
 */
std::string getFirstMatchInString(const std::string& content, const std::string& regex);

/**
 * @brief get last match in string
 *
 * @param content string to be check
 * @param regex regex
 *
 * @return if success return string, else empty string
 */
std::string getLastMatchInString(const std::string& content, const std::string& regex);

/**
 * @brief get all matches in string
 *
 * @param content string to be check
 * @param regex regex
 *
 * @return if success return strings, else empty vector
 */
std::vector<std::string> getAllMatchesInString(const std::string& content, const std::string& regex);

/**
 * @brief replace all matches in string
 *
 * @param content string to be replaced
 * @param regex regex
 * @param replacement string to replace
 *
 * @return if success return string replaced, else original content
 */
std::string replaceAllMatchesInString(const std::string& content, const std::string& regex, const std::string& replacement);

/**
 * @brief replace specific match in string
 *
 * @param content string to be replaced
 * @param regex regex
 * @param replacement string to replace
 * @param idx specific index
 *
 * @return if success return string replaced, else original content
 */
std::string replaceSpecificMatchInString(const std::string& content, const std::string& regex, const std::string& replacement, int idx);

} // namespace re

#endif // _XREGEX_H_