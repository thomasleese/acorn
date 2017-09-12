#include <iostream>

#include <spdlog/spdlog.h>

#include "acorn/ast/nodes.h"
#include "acorn/codegen/generator.h"
#include "acorn/compiler.h"
#include "acorn/parser/scanner.h"
#include "acorn/parser/parser.h"
#include "acorn/prettyprinter.h"
#include "acorn/symboltable/builder.h"
#include "acorn/symboltable/namespace.h"
#include "acorn/typesystem/checker.h"
#include "acorn/utils.h"

using namespace acorn;
using namespace acorn::parser;

static auto logger = spdlog::stdout_color_mt("acorn");

void pretty_print(std::unique_ptr<ast::SourceFile> &source_file) {
    PrettyPrinter pp;
    pp.visit_source_file(source_file.get());
    pp.print();
}

ast::SourceFile *parse(const std::string filename, symboltable::Namespace *root_namespace) {
    Scanner scanner(filename);

    Parser parser(scanner);
    auto source_file = parser.parse(filename);

    if (scanner.has_errors() || parser.has_errors() || !source_file) {
        return nullptr;
    }

    logger->info("Building symbol table...");

    symboltable::Builder symbol_table_builder(root_namespace);
    symbol_table_builder.visit_source_file(source_file.get());
    assert(symbolTableBuilder.is_at_root());

    return_null_if_has_errors(symbol_table_builder);

    pretty_print(source_file);

    logger->info("Running type checker...");

    typesystem::TypeChecker type_checker(root_namespace);
    type_checker.visit_source_file(source_file.get());

    return_null_if_has_errors(type_checker);

    pretty_print(source_file);

    //std::cout << root_namespace->to_string() << std::endl;

    return source_file.release();
}

int main(int argc, char *argv[]) {
    logger->set_level(spdlog::level::debug);
    logger->set_pattern("[%H:%M:%S] %v");

    std::string filename = argv[1];

    auto root_namespace = std::make_unique<symboltable::Namespace>(nullptr);
    auto source_file = parse(filename, root_namespace.get());

    if (source_file == nullptr) {
        return 1;
    }

    compiler::Compiler compiler;
    if (!compiler.compile(source_file, root_namespace.get(), filename)) {
        return 2;
    }

    return 0;
}
