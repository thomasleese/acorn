cmake_minimum_required(VERSION 3.3)

project(Acorn)

# Boost
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost REQUIRED COMPONENTS regex)

# LLVM
execute_process(COMMAND llvm-config-3.8 --cxxflags OUTPUT_VARIABLE LLVM_CXX_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND llvm-config-3.8 --ldflags OUTPUT_VARIABLE LLVM_LD_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND llvm-config-3.8 --libs OUTPUT_VARIABLE LLVM_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)

# Source
include_directories(${Boost_INCLUDE_DIRS} /usr/local/Cellar/icu4c/57.1/include)
link_directories(/usr/local/Cellar/icu4c/57.1/lib/)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LLVM_CXX_FLAGS} -std=c++11 -Wall -Werror -pedantic -fexceptions")
set(CMAKE_EXE_LINKER_FLAGS "${LLVM_LD_FLAGS} -lz -fexceptions")

set(SOURCE_FILES compiler/ast/nodes.cpp compiler/ast/visitor.cpp compiler/codegen/builtins.cpp compiler/codegen/types.cpp compiler/codegen/module.cpp compiler/parsing/lexer.cpp compiler/parsing/parser.cpp compiler/parsing/token.cpp compiler/typing/checker.cpp compiler/typing/inferrer.cpp compiler/typing/types.cpp compiler/typing/visitor.cpp compiler/compiler.cpp compiler/pass.cpp compiler/errors.cpp compiler/main.cpp compiler/prettyprinter.cpp compiler/symboltable.cpp)
set(LIBRARIES ${Boost_LIBRARIES} icuuc icudata ncurses ${LLVM_LIBS})

add_executable(acorn ${SOURCE_FILES})
target_link_libraries(acorn ${LIBRARIES})
