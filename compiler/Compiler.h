//
// Created by Thomas Leese on 14/03/2016.
//

#ifndef JET_COMPILER_H
#define JET_COMPILER_H

#include <string>
#include <vector>

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/Orc/IRCompileLayer.h>
#include <llvm/ExecutionEngine/Orc/ObjectLinkingLayer.h>

namespace llvm {
    class TargetMachine;
    class DataLayout;
    class Module;
}

class Compiler {

public:
    Compiler();
    ~Compiler();

    void debug(std::string line);

    void compile(std::string filename);

};

#endif // JET_COMPILER_H
