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

#include "Lexer.h"
#include "Parser.h"
#include "AbstractSyntaxTree.h"
#include "PrettyPrinter.h"
#include "SymbolTable.h"
#include "CodeGenerator.h"
#include "Errors.h"
#include "Typing.h"

#include "Compiler.h"

Compiler::Compiler()
{
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmPrinters();
    llvm::InitializeAllAsmParsers();

    llvm::PassRegistry *registry = llvm::PassRegistry::getPassRegistry();
    llvm::initializeCore(*registry);
    llvm::initializeCodeGen(*registry);
    llvm::initializeLoopStrengthReducePass(*registry);
    llvm::initializeLowerIntrinsicsPass(*registry);
    llvm::initializeUnreachableBlockElimPass(*registry);

    llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
}

Compiler::~Compiler() {

}

void Compiler::debug(std::string line) {
    //std::cerr << line << std::endl;
}

void Compiler::compile(std::string filename) {
    std::string outputName = filename + ".o";
    std::string moduleName = filename.substr(0, filename.find_last_of("."));

    debug("Lexing...");

    Lexer lexer;
    std::vector<Token *> tokens = lexer.tokenise(filename);

    debug("Parsing...");

    Parser parser(tokens);
    AST::SourceFile *module = parser.parse(filename);

    /*PrettyPrinter *printer = new PrettyPrinter();
    module->accept(printer);
    printer->print();
    delete printer;*/

    debug("Building the Symbol Table...");

    SymbolTable::Builder *symbolTableBuilder = new SymbolTable::Builder();
    module->accept(symbolTableBuilder);
    assert(symbolTableBuilder->isAtRoot());

    SymbolTable::Namespace *rootNamespace = symbolTableBuilder->rootNamespace();
    delete symbolTableBuilder;

    debug("Inferring types...");

    Typing::Inferrer *typeInferrer = new Typing::Inferrer(rootNamespace);
    module->accept(typeInferrer);
    delete typeInferrer;

    debug("Checking types...");

    Typing::Checker *typeChecker = new Typing::Checker(rootNamespace);
    module->accept(typeChecker);
    delete typeChecker;

    debug("Generating code...");

    CodeGenerator *generator = new CodeGenerator(rootNamespace);
    module->accept(generator);
    llvm::Module *llvmModule = generator->module();
    delete generator;

    delete module;

    debug("Generating object file...");

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

    std::string cpu = "x86-64";
    llvm::CodeGenOpt::Level opt_level = llvm::CodeGenOpt::None;
    llvm::TargetOptions target_options;
    std::string target_features;

    llvm::TargetMachine *target_machine = target->createTargetMachine(triple.str(), cpu, target_features, target_options, llvm::Reloc::PIC_, llvm::CodeModel::Default, opt_level);

    llvmModule->setDataLayout(target_machine->createDataLayout());
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
}
