#include "argument_parser.h"
#include "log/logger.h"
#include "gtest/gtest.h"


// get number of elements
#define NUMBER_OF_ARRAY(arr) (sizeof(arr) / sizeof(*arr))

TEST(ArgumentParser, Fail)
{
    const char* argv[] = {"test_fail", "-x"};
    util::ArgumentParser parser(
        [&](char optionShort, const std::string& optionLong, util::ArgumentParser::ValueOption& valueOption) {
            (void)optionShort;
            (void)optionLong;
            (void)valueOption;
            return false;
        }
    );

    ASSERT_FALSE(parser.parse(NUMBER_OF_ARRAY(argv), argv));
}

TEST(ArgumentParser, NoOption)
{
    const char* argv[] = {"test_no_option"};
    util::ArgumentParser parser(
        [&](char optionShort, const std::string& optionLong, util::ArgumentParser::ValueOption& valueOption) {
            (void)optionShort;
            (void)optionLong;
            (void)valueOption;
            return true;
        }
    );

    ASSERT_TRUE(parser.parse(NUMBER_OF_ARRAY(argv), argv));
}

TEST(ArgumentParser, ShortOptionSingleWithoutValue)
{
    const char* argv[] = {"test_short_option_single_without_value", "-x"};
    util::ArgumentParser parser(
        [&](char optionShort, const std::string& optionLong, util::ArgumentParser::ValueOption& valueOption) {
            (void)optionLong;
            if (optionShort == 'x') {
                EXPECT_TRUE(valueOption.isValid());
                EXPECT_TRUE(valueOption.get().empty());
            }
            return true;
        }
    );

    ASSERT_TRUE(parser.parse(NUMBER_OF_ARRAY(argv), argv));
}

TEST(ArgumentParser, LongOptionSingleWithoutValue)
{
    const char* argv[] = {"test_long_option_single_without_value", "--enable"};
    util::ArgumentParser parser(
        [&](char optionShort, const std::string& optionLong, util::ArgumentParser::ValueOption& valueOption) {
            (void)optionShort;
            if (optionLong == "enable") {
                EXPECT_TRUE(valueOption.isValid());
                EXPECT_TRUE(valueOption.get().empty());
            }
            return true;
        }
    );

    ASSERT_TRUE(parser.parse(NUMBER_OF_ARRAY(argv), argv));
}

TEST(ArgumentParser, ShortOptionSingleWithValue)
{
    const char* argv[] = {"test_short_option_single_with_value", "-x", "5"};
    util::ArgumentParser parser(
        [&](char optionShort, const std::string& optionLong, util::ArgumentParser::ValueOption& valueOption) {
            (void)optionLong;
            if (optionShort == 'x') {
                EXPECT_TRUE(valueOption.isValid());
                EXPECT_EQ(std::stoi(valueOption.get()), 5);
            }
            return true;
        }
    );

    ASSERT_TRUE(parser.parse(NUMBER_OF_ARRAY(argv), argv));
}

TEST(ArgumentParser, LongOptionSingleWithValue)
{
    const char* argv[] = {"test_long_option_single_with_value", "--enable", "true"};
    util::ArgumentParser parser(
        [&](char optionShort, const std::string& optionLong, util::ArgumentParser::ValueOption& valueOption) {
            (void)optionShort;
            if (optionLong == "enable") {
                EXPECT_TRUE(valueOption.isValid());
                EXPECT_EQ(valueOption.get(), "true");
            }
            return true;
        }
    );

    ASSERT_TRUE(parser.parse(NUMBER_OF_ARRAY(argv), argv));
}

TEST(ArgumentParser, LongOptionKeyAndValue)
{
    const char* argv[] = {"test_long_option_key_and_value", "--key=123", "-n"};
    util::ArgumentParser parser(
        [&](char optionShort, const std::string& optionLong, util::ArgumentParser::ValueOption& valueOption) {
            if (optionLong == "key") {
                EXPECT_TRUE(valueOption.isValid());
                EXPECT_EQ(valueOption.get(), "123");
            } else if (optionShort == 'n') {
                EXPECT_TRUE(valueOption.isValid());
                EXPECT_TRUE(valueOption.get().empty());
            }
            return true;
        }
    );
    
    ASSERT_TRUE(parser.parse(NUMBER_OF_ARRAY(argv), argv));
}


TEST(ArgumentParser, LongOptionKeyAndValueWithQuot)
{
    bool has_name = false;
    bool has_address = false;

    const char* argv[] = {"test_any", R"(--name='Chen Wang')", R"(--address="Hangzhou China")"};
    util::ArgumentParser parser(
        [&](char optionShort, const std::string& optionLong, util::ArgumentParser::ValueOption& valueOption) {
            (void)optionShort;
            if (optionLong == "name") {
                EXPECT_TRUE(valueOption.isValid());
                EXPECT_EQ(valueOption.get(), "Chen Wang");
                has_name = valueOption.isValid();
            } else if (optionLong == "address") {
                EXPECT_TRUE(valueOption.isValid());
                EXPECT_EQ(valueOption.get(), "Hangzhou China");
                has_address = valueOption.isValid();
            }
            return true;
        }
    );

    ASSERT_TRUE(parser.parse(NUMBER_OF_ARRAY(argv), argv));
    ASSERT_TRUE(has_name);
    ASSERT_TRUE(has_address);
}


TEST(ArgumentParser, All)
{
    const char* argv[] = {"test_all", "-ab", "123", "-x", "--set", "xyz", "--run", "--key=value", "--enable"};
    util::ArgumentParser parser(
        [&](char optionShort, const std::string& optionLong, util::ArgumentParser::ValueOption& valueOption) {
            if (optionShort == 'a') {
                EXPECT_TRUE(valueOption.isValid());
                EXPECT_TRUE(valueOption.get().empty());
            } else if (optionShort == 'b') {
                EXPECT_TRUE(valueOption.isValid());
                EXPECT_EQ(valueOption.get(), "123");
            } else if (optionShort == 'x') {
                EXPECT_TRUE(valueOption.isValid());
                EXPECT_TRUE(valueOption.get().empty());
            } else if (optionLong == "set") {
                EXPECT_TRUE(valueOption.isValid());
                EXPECT_EQ(valueOption.get(), "xyz");
            } else if (optionLong == "run") {
                EXPECT_TRUE(valueOption.isValid());
                EXPECT_TRUE(valueOption.get().empty());
            } else if (optionLong == "key") {
                EXPECT_TRUE(valueOption.isValid());
                EXPECT_EQ(valueOption.get(), "value");
            } else if (optionLong == "enable") {
                EXPECT_TRUE(valueOption.isValid());
                EXPECT_TRUE(valueOption.get().empty());
            } else {
                return false;
            }
            return true;
        }
    );
    
    ASSERT_TRUE(parser.parse(NUMBER_OF_ARRAY(argv), argv));
}