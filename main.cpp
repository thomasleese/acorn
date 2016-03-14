#include <iostream>

#include "Lexer.h"
#include "Parser.h"
#include "AbstractSyntaxTree.h"

using namespace std;

int main() {
    Lexer lexer;

    std::vector<Lexer::Token> tokens = lexer.tokenise("example.quark");
    /*for (Lexer::Token token : tokens) {
        cout << token.lexeme << endl;
    }*/

    Parser parser(tokens);
    AST::Module *module = parser.parse("example");

    cout << module->pprint() << endl;

    delete module;

    return 0;
}
