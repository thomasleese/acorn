//
// Created by Thomas Leese on 14/03/2016.
//

#include <iostream>

#include "Lexer.h"
#include "Parser.h"
#include "AbstractSyntaxTree.h"
#include "Compiler.h"

void Compiler::compile(std::string filename) {
    Lexer lexer;

    std::vector<Lexer::Token> tokens = lexer.tokenise(filename);
    /*for (Lexer::Token token : tokens) {
        cout << token.lexeme << endl;
    }*/

    Parser parser(tokens);
    AST::Module *module = parser.parse(filename);

    std::cout << module->pprint() << std::endl;

    delete module;
}
