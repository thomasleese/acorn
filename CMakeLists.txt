cmake_minimum_required(VERSION 3.3)

project(Acorn)

execute_process(COMMAND /usr/local/Cellar/llvm/3.9.0/bin/llvm-config --cxxflags OUTPUT_VARIABLE LLVM_CXX_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND /usr/local/Cellar/llvm/3.9.0/bin/llvm-config --ldflags OUTPUT_VARIABLE LLVM_LD_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND /usr/local/Cellar/llvm/3.9.0/bin/llvm-config --libs OUTPUT_VARIABLE LLVM_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LLVM_CXX_FLAGS} -std=c++11 -Wall -pedantic")
set(CMAKE_EXE_LINKER_FLAGS "${LLVM_LD_FLAGS} -lcurses -lz -fexceptions")

set(SOURCE_FILES compiler/ast.cpp
                 compiler/codegen/builtins.cpp
                 compiler/codegen/types.cpp
                 compiler/codegen/module.cpp
                 compiler/lexer.cpp
                 compiler/parser.cpp
                 compiler/typing/checker.cpp
                 compiler/typing/inferrer.cpp
                 compiler/typing/types.cpp
                 compiler/compiler.cpp
                 compiler/diagnostics.cpp
                 compiler/main.cpp
                 compiler/prettyprinter.cpp
                 compiler/symboltable.cpp compiler/token.cpp compiler/token.h)

add_executable(acorn ${SOURCE_FILES})
target_link_libraries(acorn ${LLVM_LIBS})
