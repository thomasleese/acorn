//
// Created by Thomas Leese on 14/03/2016.
//

#include <iostream>

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/RTDyldMemoryManager.h>
#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include <llvm/ExecutionEngine/Orc/IRCompileLayer.h>
#include <llvm/ExecutionEngine/Orc/LambdaResolver.h>
#include <llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h>
#include <llvm/Support/DynamicLibrary.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/ToolOutputFile.h>

#include <spdlog/spdlog.h>

#include "acorn/ast/nodes.h"
#include "acorn/codegen/generator.h"

#include "acorn/compiler.h"

using namespace acorn;
using namespace acorn::compiler;
using namespace acorn::diagnostics;

static auto logger = spdlog::get("acorn");

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

    llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
}

Compiler::~Compiler() {

}

bool Compiler::compile(ast::SourceFile *module, symboltable::Namespace *root_namespace, std::string filename) {
    auto output_name = filename + ".o";
    auto module_name = filename.substr(0, filename.find_last_of("."));

    logger->info("Compiling {} to {}.", filename, output_name);

    auto triple = get_triple();
    auto target_machine = get_target_machine(triple);

    auto data_layout = target_machine->createDataLayout();

    codegen::CodeGenerator generator(root_namespace, &data_layout);
    generator.visit_source_file(module);

    if (generator.has_errors()) {
        return false;
    }

    auto llvm_module = generator.module();

    delete module;

    logger->debug("Generating object file...");

    llvm_module->setTargetTriple(triple.str());

    llvm_module->print(llvm::errs(), nullptr);

    llvm::legacy::PassManager pass_manager;

    std::error_code error_code;
    llvm::ToolOutputFile output_file(output_name, error_code, llvm::sys::fs::F_None);

    target_machine->addPassesToEmitFile(pass_manager, output_file.os(), llvm::TargetMachine::CGFT_ObjectFile);

    pass_manager.run(*llvm_module);

    output_file.keep();
    output_file.os().close();

    logger->debug("Compiling object file...");

    std::string cmd = "clang " + output_name + " -o " + module_name;
    system(cmd.c_str());

    remove(output_name.c_str());

    return true;
}

llvm::Triple Compiler::get_triple() const {
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

    return triple;
}

llvm::TargetMachine *Compiler::get_target_machine(llvm::Triple triple) const {
    std::string error;
    const auto target = llvm::TargetRegistry::lookupTarget(triple.str(), error);

    auto cpu = llvm::sys::getHostCPUName();
    auto opt_level = llvm::CodeGenOpt::None;
    llvm::TargetOptions target_options;

    llvm::SubtargetFeatures features;

    llvm::StringMap<bool> host_features;
    llvm::sys::getHostCPUFeatures(host_features);
    for (auto &it : host_features) {
        features.AddFeature(it.first(), it.second);
    }
    auto target_features = features.getString();

    return target->createTargetMachine(
        triple.str(), cpu, target_features, target_options,
        llvm::Reloc::PIC_, llvm::CodeModel::Medium, opt_level
    );
}
