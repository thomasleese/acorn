//
// Created by Thomas Leese on 14/03/2016.
//

#include <cstdio>
#include <cassert>
#include <iostream>

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/RTDyldMemoryManager.h>
#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include <llvm/ExecutionEngine/Orc/IRCompileLayer.h>
#include <llvm/ExecutionEngine/Orc/LambdaResolver.h>
#include <llvm/ExecutionEngine/Orc/ObjectLinkingLayer.h>
#include <llvm/IR/Mangler.h>
#include <llvm/Support/DynamicLibrary.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/ToolOutputFile.h>

#include "ast.h"
#include "codegen/module.h"
#include "parsing/lexer.h"
#include "parsing/parser.h"
#include "prettyprinter.h"
#include "symboltable.h"
#include "diagnostics.h"
#include "typing/checker.h"
#include "typing/inferrer.h"

#include "compiler.h"

using namespace acorn;
using namespace acorn::compiler;
using namespace acorn::diagnostics;

Compiler::Compiler() : m_diagnostics(new Reporter()) {
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmPrinters();
    llvm::InitializeAllAsmParsers();

    auto registry = llvm::PassRegistry::getPassRegistry();
    llvm::initializeCore(*registry);
    llvm::initializeCodeGen(*registry);
    llvm::initializeLoopStrengthReducePass(*registry);
    llvm::initializeLowerIntrinsicsPass(*registry);
    //llvm::initializeUnreachableBlockElimPass(*registry);

    llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
}

Compiler::~Compiler() {

}

void Compiler::debug(std::string line) const {
    std::cerr << line << std::endl;
}

bool Compiler::compile(std::string filename) {
    std::string outputName = filename + ".o";
    std::string moduleName = filename.substr(0, filename.find_last_of("."));

    Lexer lexer(filename);

    //lexer.debug();

    debug("Parsing...");

    auto parser = new Parser(m_diagnostics, lexer);
    auto module = parser->parse(filename);

    if (m_diagnostics->has_errors()) {
        return false;
    }

    debug("Building the Symbol Table...");

    symboltable::Builder symbol_table_builder;
    module->accept(&symbol_table_builder);
    assert(symbolTableBuilder.isAtRoot());

    if (symbol_table_builder.has_errors()) {
        return false;
    }

    auto rootNamespace = symbol_table_builder.rootNamespace();

    debug("Inferring types...");

    auto typeInferrer = new typing::Inferrer(m_diagnostics, rootNamespace);
    module->accept(typeInferrer);

    std::cout << rootNamespace->to_string() << std::endl;

    auto printer = new PrettyPrinter();
    module->accept(printer);
    printer->print();
    delete printer;

    if (m_diagnostics->has_errors()) {
        return false;
    }

    delete typeInferrer;

    debug("Checking types...");

    auto typeChecker = new typing::Checker(m_diagnostics, rootNamespace);
    module->accept(typeChecker);

    if (m_diagnostics->has_errors()) {
        return false;
    }

    delete typeChecker;

    debug("Generating code...");

    llvm::Triple triple(llvm::sys::getDefaultTargetTriple());

    if (triple.getOS() == llvm::Triple::Darwin &&
        triple.getVendor() == llvm::Triple::Apple) {
        // Rewrite darwinX.Y triples to macosx10.X'.Y ones.
        // It affects code generation on our platform.
        llvm::SmallString<16> osxBuf;
        llvm::raw_svector_ostream osx(osxBuf);
        osx << llvm::Triple::getOSTypeName(llvm::Triple::MacOSX);

        unsigned major, minor, micro;
        triple.getMacOSXVersion(major, minor, micro);
        osx << major << "." << minor;
        if (micro != 0)
            osx << "." << micro;

        triple.setOSName(osx.str());
    }

    std::string error;
    const llvm::Target *target = llvm::TargetRegistry::lookupTarget(triple.str(), error);

    std::string cpu = llvm::sys::getHostCPUName();
    llvm::CodeGenOpt::Level opt_level = llvm::CodeGenOpt::None;
    llvm::TargetOptions target_options;

    llvm::SubtargetFeatures features;

    llvm::StringMap<bool> host_features;
    llvm::sys::getHostCPUFeatures(host_features);
    for (auto &it : host_features) {
        features.AddFeature(it.first(), it.second);
    }
    std::string target_features = features.getString();

    llvm::TargetMachine *target_machine = target->createTargetMachine(triple.str(), cpu, target_features, target_options, llvm::Reloc::PIC_, llvm::CodeModel::Default, opt_level);

    auto data_layout = target_machine->createDataLayout();

    auto generator = new codegen::ModuleGenerator(m_diagnostics, rootNamespace, m_context, &data_layout);
    module->accept(generator);

    if (m_diagnostics->has_errors()) {
        return false;
    }

    llvm::Module *llvmModule = generator->module();
    delete generator;

    delete module;

    debug("Generating object file...");

    llvmModule->setDataLayout(data_layout);
    llvmModule->setTargetTriple(triple.str());

    llvmModule->dump();

    llvm::legacy::PassManager pass_manager;

    std::error_code error_code;
    llvm::tool_output_file output_file(outputName, error_code, llvm::sys::fs::F_None);

    target_machine->addPassesToEmitFile(pass_manager, output_file.os(), llvm::TargetMachine::CGFT_ObjectFile);

    pass_manager.run(*llvmModule);

    output_file.keep();

    output_file.os().close();

    std::string cmd = "clang " + outputName + " -o " + moduleName;
    system(cmd.c_str());

    remove(outputName.c_str());

    return true;
}
