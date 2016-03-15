//
// Created by Thomas Leese on 14/03/2016.
//

#include <iostream>

#include "Lexer.h"
#include "Parser.h"
#include "AbstractSyntaxTree.h"
#include "PrettyPrinter.h"
#include "Compiler.h"

void Compiler::compile(std::string filename) {
    Lexer lexer;

    std::vector<Lexer::Token *> tokens = lexer.tokenise(filename);

    /*for (Lexer::Token token : tokens) {
        std::cout << token.lexeme << std::endl;
    }*/

    std::string moduleName = filename.substr(0, filename.find("."));

    Parser parser(tokens);
    AST::Module *module = parser.parse(moduleName);

    PrettyPrinter *printer = new PrettyPrinter();
    module->accept(printer);
    printer->print();
    delete printer;

    delete module;
}
