//
// Created by Thomas Leese on 14/03/2016.
//

#include <cassert>
#include <iostream>

#include "Lexer.h"
#include "Parser.h"
#include "AbstractSyntaxTree.h"
#include "PrettyPrinter.h"
#include "SymbolTable.h"
#include "CodeGenerator.h"
#include "Errors.h"
#include "Typing.h"

#include "Compiler.h"

void Compiler::compile(std::string filename) {
    Lexer lexer;

    std::vector<Token *> tokens = lexer.tokenise(filename);

    /*for (Token *token : tokens) {
        std::cout << Token::rule_string(token->rule) << ": " << token->lexeme << std::endl;
    }*/

    std::string moduleName = filename.substr(0, filename.find("."));

    Parser parser(tokens);
    AST::Module *module = parser.parse(moduleName);

    AST::Simplifier *simplifier = new AST::Simplifier();
    module->accept(simplifier);

    SymbolTable::Builder *symbolTableBuilder = new SymbolTable::Builder();
    module->accept(symbolTableBuilder);
    assert(symbolTableBuilder->isAtRoot());

    Typing::Inferrer *typeInferrer = new Typing::Inferrer(symbolTableBuilder->rootNamespace());
    module->accept(typeInferrer);

    Typing::Checker *typeChecker = new Typing::Checker(symbolTableBuilder->rootNamespace());
    module->accept(typeChecker);

    CodeGenerator *generator = new CodeGenerator(symbolTableBuilder->rootNamespace());
    module->accept(generator);

    PrettyPrinter *printer = new PrettyPrinter();
    module->accept(printer);

    printer->print();
    delete printer;

    delete module;
}
