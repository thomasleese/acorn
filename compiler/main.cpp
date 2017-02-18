#include <iostream>

#include "ast.h"
#include "codegen.h"
#include "compiler.h"
#include "diagnostics.h"
#include "lexer.h"
#include "parser.h"
#include "prettyprinter.h"
#include "symboltable.h"
#include "typing.h"

using namespace acorn;

ast::SourceFile *parse(std::string filename, symboltable::Namespace **name_space) {
    Lexer lexer(filename);

    Parser parser(lexer);
    auto module = parser.parse(filename);

    if (lexer.has_errors() || parser.has_errors()) {
        return nullptr;
    }

    symboltable::Builder symbol_table_builder;
    module->accept(&symbol_table_builder);
    assert(symbolTableBuilder.isAtRoot());

    if (symbol_table_builder.has_errors()) {
        return nullptr;
    }

    auto root_namespace = symbol_table_builder.rootNamespace();

    typing::Inferrer inferrer(root_namespace);
    module->accept(&inferrer);

    if (inferrer.has_errors()) {
        return nullptr;
    }

    std::cout << root_namespace->to_string() << std::endl;

    PrettyPrinter pp;
    module->accept(&pp);
    pp.print();

    typing::Checker type_checker(root_namespace);
    module->accept(&type_checker);

    if (type_checker.has_errors()) {
        return nullptr;
    }

    *name_space = root_namespace;
    return module;
}

int main(int argc, char *argv[]) {
    std::string filename = "../test.acorn";
    symboltable::Namespace *root_namespace;
    auto module = parse(filename, &root_namespace);

    compiler::Compiler compiler;
    if (compiler.compile(module, root_namespace, filename)) {
        return 0;
    } else {
        return 1;
    }
}
