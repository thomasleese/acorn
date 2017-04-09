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

    if (lexer.has_errors() || parser.has_errors() || module == nullptr) {
        return nullptr;
    }

    PrettyPrinter pp;

    symboltable::Builder symbol_table_builder;
    module->accept(&symbol_table_builder);
    assert(symbolTableBuilder.is_at_root());

    if (symbol_table_builder.has_errors()) {
        return nullptr;
    }

    auto root_namespace = symbol_table_builder.root_namespace();

    typing::Inferrer inferrer(root_namespace);
    module->accept(&inferrer);

    //std::cout << root_namespace->to_string() << std::endl;
    //module->accept(&pp);
    //pp.print();

    if (inferrer.has_errors()) {
        return nullptr;
    }

    typing::Checker type_checker(root_namespace);
    module->accept(&type_checker);

    if (type_checker.has_errors()) {
        return nullptr;
    }

    generics::Expander expander(root_namespace);
    module->accept(&expander);

    if (expander.has_errors()) {
        return nullptr;
    }

    *name_space = root_namespace;
    return module;
}

int main(int argc, char *argv[]) {
    std::string filename = argv[1];

    symboltable::Namespace *root_namespace;
    auto module = parse(filename, &root_namespace);
    if (module == nullptr) {
        return 1;
    }

    compiler::Compiler compiler;
    if (!compiler.compile(module, root_namespace, filename)) {
        return 2;
    }

    return 0;
}
