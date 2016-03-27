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

private:
    std::string mangle(const std::string &name);
    llvm::orc::JITSymbol findMangledSymbol(const std::string &name);

private:
    std::unique_ptr<llvm::TargetMachine> m_targetMachine;
    const llvm::DataLayout m_dataLayout;

    llvm::orc::ObjectLinkingLayer<> m_object_layer;
    llvm::orc::IRCompileLayer<llvm::orc::ObjectLinkingLayer<> > m_compiler_layer;

    std::vector<llvm::orc::IRCompileLayer<llvm::orc::ObjectLinkingLayer<> >::ModuleSetHandleT > m_modules;

};

#endif // JET_COMPILER_H
