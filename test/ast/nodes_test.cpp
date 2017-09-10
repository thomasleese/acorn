#include "gtest/gtest.h"

#include "acorn/ast/nodes.h"

using std::make_unique, std::unique_ptr;

using namespace acorn;
using namespace acorn::ast;

static Token token;

TEST(ast_nodes_test, block_clone)
{
    auto block1 = new Block(token, make_unique<Name>(token, "name"));
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

TEST(ast_nodes_test, type_name_clone)
{
    auto type_name1 = new TypeName(token, make_unique<Name>(token, "name"));
    auto type_name2 = type_name1->clone();

    EXPECT_FALSE(type_name1 == type_name2);
    EXPECT_FALSE(type_name1->name() == type_name2->name());
}

TEST(ast_nodes_test, decl_name_clone)
{
    auto decl_name1 = new DeclName(token, make_unique<Name>(token, "name"));
    auto decl_name2 = decl_name1->clone();

    EXPECT_FALSE(decl_name1 == decl_name2);
    EXPECT_FALSE(decl_name1->name() == decl_name2->name());
}

TEST(ast_nodes_test, param_name_clone)
{
    auto param_name1 = new DeclName(token, make_unique<Name>(token, "name"));
    auto param_name2 = param_name1->clone();

    EXPECT_FALSE(param_name1 == param_name2);
    EXPECT_FALSE(param_name1->name() == param_name2->name());
}
