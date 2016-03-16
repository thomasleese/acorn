cmake_minimum_required(VERSION 3.3)
project(Quark)

set(SOURCE_FILES compiler/main.cpp compiler/Lexer.cpp compiler/AbstractSyntaxTree.cpp compiler/Parser.cpp compiler/Compiler.cpp compiler/Compiler.h compiler/PrettyPrinter.cpp compiler/PrettyPrinter.h compiler/Errors.cpp compiler/Errors.h compiler/SymbolTable.cpp compiler/SymbolTable.h compiler/CodeGenerator.cpp compiler/CodeGenerator.h)

execute_process(
        COMMAND /usr/local/opt/llvm/bin/llvm-config --cxxflags
        OUTPUT_VARIABLE LLVM_CXX_FLAGS
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
        COMMAND /usr/local/opt/llvm/bin/llvm-config --ldflags
        OUTPUT_VARIABLE LLVM_LD_FLAGS
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
        COMMAND /usr/local/opt/llvm/bin/llvm-config --libs
        OUTPUT_VARIABLE LLVM_LIBS
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LLVM_CXX_FLAGS} -std=c++11 -Wall -pedantic -Werror")
SET(CMAKE_EXE_LINKER_FLAGS "${LLVM_LD_FLAGS}")

add_executable(Quark ${SOURCE_FILES})
target_link_libraries(Quark ${Boost_LIBRARIES} ncurses ${LLVM_LIBS})
