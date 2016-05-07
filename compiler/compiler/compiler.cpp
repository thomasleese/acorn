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

#include "../ast/nodes.h"
#include "../codegen/module.h"
#include "../lexer.h"
#include "../parser.h"
#include "../prettyprinter.h"
#include "../symboltable.h"
#include "../errors.h"
#include "../preprocessor.h"
#include "../Typing.h"

#include "compiler.h"

using namespace jet;
using namespace jet::compiler;

Compiler::Compiler() {
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmPrinters();
    llvm::InitializeAllAsmParsers();

    auto registry = llvm::PassRegistry::getPassRegistry();
    llvm::initializeCore(*registry);
    llvm::initializeCodeGen(*registry);
    llvm::initializeLoopStrengthReducePass(*registry);
    llvm::initializeLowerIntrinsicsPass(*registry);
    llvm::initializeUnreachableBlockElimPass(*registry);

    llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
}

Compiler::~Compiler() {

}

bool Compiler::check_pass(Pass *pass) const {
    if (pass->has_errors()) {
        while (pass->has_errors()) {
            auto error = pass->next_error();
            error->print();
            delete error;
        }

        return false;
    } else {
        return true;
    }
}

void Compiler::debug(std::string line) {
    std::cerr << line << std::endl;
}

void Compiler::compile(std::string filename) {
    std::string outputName = filename + ".o";
    std::string moduleName = filename.substr(0, filename.find_last_of("."));

    debug("Lexing...");

    auto lexer = new Lexer();
    std::vector<Token *> tokens = lexer->tokenise(filename);
    if (!check_pass(lexer)) {
        return;
    }

    debug("Parsing...");

    auto parser = new Parser(tokens);
    ast::SourceFile *module = parser->parse(filename);
    if (!check_pass(parser)) {
        return;
    }

    debug("Building the Symbol Table...");

    symboltable::Builder *symbolTableBuilder = new symboltable::Builder();
    module->accept(symbolTableBuilder);
    assert(symbolTableBuilder->isAtRoot());

    if (!check_pass(symbolTableBuilder)) {
        return;
    }

    symboltable::Namespace *rootNamespace = symbolTableBuilder->rootNamespace();
    delete symbolTableBuilder;

    debug("Parsing generics...");

    auto generics_pass = new preprocessor::GenericsPass(rootNamespace);
    module->accept(generics_pass);

    if (!check_pass(generics_pass)) {
        return;
    }

    auto printer = new PrettyPrinter();
    module->accept(printer);
    printer->print();
    delete printer;

    debug("Inferring types...");

    auto typeInferrer = new typing::Inferrer(rootNamespace);
    module->accept(typeInferrer);

    if (!check_pass(typeInferrer)) {
        return;
    }

    delete typeInferrer;

    debug("Checking types...");

    auto typeChecker = new typing::Checker(rootNamespace);
    module->accept(typeChecker);

    if (!check_pass(typeChecker)) {
        return;
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

    auto generator = new codegen::ModuleGenerator(rootNamespace, &data_layout);
    module->accept(generator);

    if (!check_pass(generator)) {
        return;
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
}
