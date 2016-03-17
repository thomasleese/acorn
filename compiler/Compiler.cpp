//
// Created by Thomas Leese on 14/03/2016.
//

#include <cassert>
#include <iostream>

#include "LexicalAnalysis.h"
#include "Parser.h"
#include "AbstractSyntaxTree.h"
#include "PrettyPrinter.h"
#include "SymbolTable.h"
#include "CodeGenerator.h"
#include "Errors.h"

#include "Compiler.h"

void Compiler::compile(std::string filename) {
    LexicalAnalysis::Lexer lexer;

    std::vector<LexicalAnalysis::Token *> tokens = lexer.tokenise(filename);

    for (LexicalAnalysis::Token *token : tokens) {
        std::cout << LexicalAnalysis::rule_string(token->rule) << ": " << token->lexeme << std::endl;
    }

    std::string moduleName = filename.substr(0, filename.find("."));

    Parser parser(tokens);
    AST::Module *module = parser.parse(moduleName);

    SymbolTable::Builder *symbolTableBuilder = new SymbolTable::Builder();
    module->accept(symbolTableBuilder);
    assert(symbolTableBuilder->isAtRoot());

    CodeGenerator *generator = new CodeGenerator(symbolTableBuilder->rootNamespace());
    module->accept(generator);

    PrettyPrinter *printer = new PrettyPrinter();

    module->accept(printer);

    printer->print();
    delete printer;

    delete module;
}
