#include "gtest/gtest.h"

#include "acorn/parser/keywords.h"

using acorn::parser::is_keyword;

TEST(parser_keywords_test, is_keyword_valid)
{
    EXPECT_TRUE(is_keyword("let"));
    EXPECT_TRUE(is_keyword("def"));
    EXPECT_TRUE(is_keyword("if"));
    EXPECT_TRUE(is_keyword("unless"));
}

TEST(parser_keywords_test, is_keyword_empty)
{
    EXPECT_FALSE(is_keyword(""));
}

TEST(parser_keywords_test, is_keyword_invalid)
{
    EXPECT_FALSE(is_keyword("not a keyword"));
    EXPECT_FALSE(is_keyword("23891h91"));
}
