#include "gtest/gtest.h"

#include "acorn/ast/nodes.h"

using namespace acorn;
using namespace acorn::ast;

static Token token;

TEST(ast_nodes_test, block_clone)
{
    auto block1 = new Block(token, std::make_unique<Name>(token, "name"));
    auto block2 = block1->clone();

    EXPECT_FALSE(block1 == block2);

    auto name1 = dynamic_cast<Name *>(block1->expressions()[0].get());
    auto name2 = dynamic_cast<Name *>(block2->expressions()[0].get());

    EXPECT_FALSE(name1 == name2);
    EXPECT_TRUE(name1->value() == name2->value());
}

TEST(ast_nodes_test, name_clone)
{
    auto name1 = new Name(token, "name");
    auto name2 = name1->clone();

    EXPECT_FALSE(name1 == name2);
    EXPECT_TRUE(name1->value() == name2->value());
}
