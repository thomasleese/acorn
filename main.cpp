#include <iostream>

#include "lexer.h"

using namespace std;

int main() {
    cout << "Hello, World!" << endl;

    Lexer lexer;

    std::vector<Lexer::Token> tokens = lexer.tokenise("example.quark");
    for (Lexer::Token token : tokens) {
        cout << token.lexeme << endl;
    }

    return 0;
}
