#include <iostream>

#include "Lexer.h"
#include "Parser.h"
#include "AbstractSyntaxTree.h"

using namespace std;

void pprint(AST::CodeBlock block) {
    /*for (AST::Statement statement : block.statements) {
        //pprint(statement);
    }*/
}

int main() {
    Lexer lexer;

    std::vector<Lexer::Token> tokens = lexer.tokenise("example.quark");
    for (Lexer::Token token : tokens) {
        cout << token.lexeme << endl;
    }

    Parser parser(tokens);
    AST::CodeBlock block = parser.parse();

    pprint(block);

    return 0;
}
