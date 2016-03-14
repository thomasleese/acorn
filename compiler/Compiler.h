//
// Created by Thomas Leese on 14/03/2016.
//

#ifndef QUARK_COMPILER_H
#define QUARK_COMPILER_H

#include <string>

class Lexer;
class Parser;

class Compiler {

public:
    void compile(std::string filename);

};

#endif //QUARK_COMPILER_H
