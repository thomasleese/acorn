#include <iostream>

#include "acorn/ast/nodes.h"
#include "acorn/codegen.h"
#include "acorn/compiler.h"
#include "acorn/parser/lexer.h"
#include "acorn/parser.h"
#include "acorn/prettyprinter.h"
#include "acorn/typesystem/inferrer.h"
#include "acorn/typesystem/checker.h"

using namespace acorn;

#define return_if_has_errors(thing) if ((thing).has_errors()) { return nullptr; }

ast::SourceFile *parse(const std::string filename, symboltable::Namespace *root_namespace) {
    Lexer lexer(filename);

    Parser parser(lexer);
    auto module = parser.parse(filename);

    if (lexer.has_errors() || parser.has_errors() || !module) {
        return nullptr;
    }

    PrettyPrinter pp;

    symboltable::Builder symbol_table_builder(root_namespace);
    module->accept(&symbol_table_builder);
    assert(symbolTableBuilder.is_at_root());

    return_if_has_errors(symbol_table_builder);

    std::cout << "inferrer" << std::endl;

    typesystem::TypeInferrer inferrer(root_namespace);
    module->accept(&inferrer);

    //std::cout << root_namespace->to_string() << std::endl;
    //module->accept(&pp);
    //pp.print();

    return_if_has_errors(inferrer);

    std::cout << "checker" << std::endl;

    typesystem::TypeChecker type_checker(root_namespace);
    module->accept(&type_checker);

    return_if_has_errors(type_checker);

    return module.release();
}

int main(int argc, char *argv[]) {
    std::string filename = argv[1];

    auto root_namespace = std::make_unique<symboltable::Namespace>(nullptr);
    auto module = parse(filename, root_namespace.get());

    if (module == nullptr) {
        return 1;
    }

    compiler::Compiler compiler;
    if (!compiler.compile(module, root_namespace.get(), filename)) {
        return 2;
    }

    return 0;
}
