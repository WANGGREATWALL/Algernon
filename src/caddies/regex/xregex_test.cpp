#include "xregex.h"
#include "gtest/gtest.h"


TEST(Regex, Match)
{
    std::string content = "Hello, my name is Vincent. Please visit my website at http://www.vincent.com/, send my email with vincent@vivo.com, and my phone number is (123)-456-7890.";

    std::string reNoMatch = "Wang";
    ASSERT_FALSE(re::matchRegexInString(content, reNoMatch));

    std::string rePlainText = "Vincent";
    ASSERT_TRUE(re::matchRegexInString(content, rePlainText));

    std::string reWebsite = R"(https?://[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}(/[a-zA-Z0-9._%+-/?&=]*)?)";
    ASSERT_TRUE(re::matchRegexInString(content, reWebsite));

    std::string reEmail = "\\w+@\\w+\\.\\w+";
    ASSERT_TRUE(re::matchRegexInString(content, reEmail));

    std::string rePhoneNumber = R"(\(\d{3}\)-\d{3}-\d{4})";
    ASSERT_TRUE(re::matchRegexInString(content, rePhoneNumber));
}

TEST(Regex, Fetch)
{
    std::string content = "Hello, my name is Vincent. Please visit my website at http://www.vincent.com/, send my email with vincent@vivo.com, and my phone number is (123)-456-7890.";

    std::string reNoMatch = "Wang";
    ASSERT_EQ(re::getAllMatchesInString(content, reNoMatch).size(), 0);

    std::string reOptional = "[Vv]incent.";
    ASSERT_EQ(re::getAllMatchesInString(content, reOptional).size(), 3);
    ASSERT_EQ(re::getFirstMatchInString(content, reOptional), "Vincent.");
    ASSERT_EQ(re::getLastMatchInString(content, reOptional), "vincent@");
}

TEST(Regex, Replace)
{
    std::string content = "Hello, hello, hello, hello!";

    std::string regex = "[Hh]ello";
    std::string replace = "Hi";

    ASSERT_EQ(re::getAllMatchesInString(content, regex).size(), 4);
    ASSERT_EQ(re::replaceAllMatchesInString(content, regex, replace), "Hi, Hi, Hi, Hi!");
    ASSERT_EQ(re::replaceSpecificMatchInString(content, regex, replace, 0), "Hi, hello, hello, hello!");
    ASSERT_EQ(re::replaceSpecificMatchInString(content, regex, replace, 1), "Hello, Hi, hello, hello!");
    ASSERT_EQ(re::replaceSpecificMatchInString(content, regex, replace, 4), "Hello, hello, hello, hello!");
}