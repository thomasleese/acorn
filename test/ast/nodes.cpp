#include "catch.hpp"

#include "acorn/ast/nodes.h"

using std::make_unique;
using std::unique_ptr;

using namespace acorn;
using namespace acorn::ast;

static Token token;

SCENARIO("cloning nodes") {
    GIVEN("a block") {
        auto block1 = new Block(token, make_unique<Name>(token, "name"));
        auto block2 = block1->clone();

        REQUIRE(block1 != block2);

        auto name1 = static_cast<Name *>(block1->expressions()[0]);
        auto name2 = static_cast<Name *>(block2->expressions()[0]);

        REQUIRE(name1 != name2);
        REQUIRE(name1->value() == name2->value());
    }

    GIVEN("a name") {
        auto name1 = new Name(token, "name");
        auto name2 = name1->clone();

        REQUIRE(name1 != name2);
        REQUIRE(name1->value() == name2->value());
    }

    GIVEN("a type name") {
        auto type_name1 = new TypeName(token, make_unique<Name>(token, "name"));
        auto type_name2 = type_name1->clone();

        REQUIRE(type_name1 != type_name2);
        REQUIRE(type_name1->name() != type_name2->name());
    }

    GIVEN("a decl name") {
        auto decl_name1 = new DeclName(token, make_unique<Name>(token, "name"));
        auto decl_name2 = decl_name1->clone();

        REQUIRE(decl_name1 != decl_name2);
        REQUIRE(decl_name1->name() != decl_name2->name());
    }

    GIVEN("a param name") {
        auto param_name1 = new ParamName(token, make_unique<Name>(token, "name"));
        auto param_name2 = param_name1->clone();

        REQUIRE(param_name1 != param_name2);
        REQUIRE(param_name1->name() != param_name2->name());
    }
}
