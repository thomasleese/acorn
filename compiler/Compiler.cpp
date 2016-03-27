//
// Created by Thomas Leese on 14/03/2016.
//

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

#include "Lexer.h"
#include "Parser.h"
#include "AbstractSyntaxTree.h"
#include "PrettyPrinter.h"
#include "SymbolTable.h"
#include "CodeGenerator.h"
#include "Errors.h"
#include "Typing.h"

#include "Compiler.h"

Compiler::Compiler() :
        m_targetMachine(llvm::EngineBuilder().selectTarget()),
        m_dataLayout(m_targetMachine->createDataLayout()),
        m_compiler_layer(m_object_layer, llvm::orc::SimpleCompiler(*m_targetMachine))
{
    llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
}

Compiler::~Compiler() {

}

void Compiler::debug(std::string line) {
    //std::cerr << line << std::endl;
}

template <typename T> std::vector<T> singletonSet(T t) {
    std::vector<T> Vec;
    Vec.push_back(std::move(t));
    return Vec;
}

void Compiler::compile(std::string filename) {
    std::string moduleName = filename.substr(0, filename.find("."));

    debug("Lexing...");

    Lexer lexer;
    std::vector<Token *> tokens = lexer.tokenise(filename);

    debug("Parsing...");

    Parser parser(tokens);
    AST::Module *module = parser.parse(moduleName);

    debug("Simplifying AST...");

    AST::Simplifier *simplifier = new AST::Simplifier();
    module->accept(simplifier);
    delete simplifier;

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

    CodeGenerator *generator = new CodeGenerator(rootNamespace, m_targetMachine.get());
    module->accept(generator);
    llvm::Module *llvmModule = generator->module();
    delete generator;

    llvmModule->dump();

    delete module;

    debug("Running code...");

    auto Resolver = llvm::orc::createLambdaResolver(
            [&](const std::string &Name) {
                if (auto Sym = findMangledSymbol(Name))
                    return llvm::RuntimeDyld::SymbolInfo(Sym.getAddress(), Sym.getFlags());
                return llvm::RuntimeDyld::SymbolInfo(nullptr);
            },
            [](const std::string &S) { return nullptr; });

    auto H = m_compiler_layer.addModuleSet(singletonSet(std::move(llvmModule)),
    llvm::make_unique<llvm::SectionMemoryManager>(),
                                       std::move(Resolver));

    m_modules.push_back(H);

    debug("Finding main function...");

    auto main_fn = findMangledSymbol(mangle("main_0"));
    assert(main_fn && "Function not found");

    debug("Calling main function...");

    int (*fp)() = (int (*)())(intptr_t) main_fn.getAddress();
    std::cout << fp() << std::endl;
}

std::string Compiler::mangle(const std::string &name) {
    std::string MangledName;
    {
        llvm::raw_string_ostream MangledNameStream(MangledName);
        llvm::Mangler::getNameWithPrefix(MangledNameStream, name, m_dataLayout);
    }
    return MangledName;
}

llvm::orc::JITSymbol Compiler::findMangledSymbol(const std::string &name) {
    // Search modules in reverse order: from last added to first added.
    // This is the opposite of the usual search order for dlsym, but makes more
    // sense in a REPL where we want to bind to the newest available definition.
    for (auto H : llvm::make_range(m_modules.rbegin(), m_modules.rend()))
        if (auto Sym = m_compiler_layer.findSymbolIn(H, name, true))
            return Sym;

    // If we can't find the symbol in the JIT, try looking in the host process.
    if (auto SymAddr = llvm::RTDyldMemoryManager::getSymbolAddressInProcess(name))
        return llvm::orc::JITSymbol(SymAddr, llvm::JITSymbolFlags::Exported);

    return nullptr;
}
