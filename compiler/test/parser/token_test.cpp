#include "gtest/gtest.h"

#include "acorn/parser/token.h"

using namespace acorn::parser;

TEST(parser_token_test, kind_to_string_valid)
{
    EXPECT_EQ(Token::kind_to_string(Token::Keyword), "keyword");
    EXPECT_EQ(Token::kind_to_string(Token::Name), "name");
}

TEST(parser_token_test, token_to_string_for_keyword)
{
    Token token;
    token.kind = Token::Keyword;
    token.lexeme = "test";
    EXPECT_EQ(token.to_string(), "test");
}

TEST(parser_token_test, token_to_string_for_other)
{
    Token token;
    token.kind = Token::Name;
    token.lexeme = "test";
    EXPECT_EQ(token.to_string(), "name");
}
