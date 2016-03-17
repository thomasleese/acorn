cmake_minimum_required(VERSION 3.3)

project(Jet)

# Boost
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost REQUIRED COMPONENTS regex)

# LLVM
execute_process(COMMAND /usr/local/opt/llvm/bin/llvm-config --cxxflags OUTPUT_VARIABLE LLVM_CXX_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND /usr/local/opt/llvm/bin/llvm-config --ldflags OUTPUT_VARIABLE LLVM_LD_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND /usr/local/opt/llvm/bin/llvm-config --libs OUTPUT_VARIABLE LLVM_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)

# Source
include_directories(${Boost_INCLUDE_DIRS} /usr/local/Cellar/icu4c/56.1/include)
link_directories(/usr/local/Cellar/icu4c/56.1/lib/)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LLVM_CXX_FLAGS} -std=c++11 -Wall -pedantic -Werror")
set(CMAKE_EXE_LINKER_FLAGS "${LLVM_LD_FLAGS}")

set(SOURCE_FILES compiler/main.cpp compiler/Lexer.cpp compiler/AbstractSyntaxTree.cpp compiler/Parser.cpp compiler/Compiler.cpp compiler/PrettyPrinter.cpp compiler/Errors.cpp compiler/SymbolTable.cpp compiler/CodeGenerator.cpp compiler/Token.cpp)
set(LIBRARIES ${Boost_LIBRARIES} icuuc icudata ncurses ${LLVM_LIBS})

add_executable(jet ${SOURCE_FILES})
target_link_libraries(jet ${LIBRARIES})
