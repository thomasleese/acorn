cmake_minimum_required(VERSION 3.3)

project(Acorn)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED OFF)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost REQUIRED COMPONENTS regex)

execute_process(COMMAND /usr/local/Cellar/llvm/4.0.0/bin/llvm-config --cxxflags OUTPUT_VARIABLE LLVM_CXX_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND /usr/local/Cellar/llvm/4.0.0/bin/llvm-config --ldflags OUTPUT_VARIABLE LLVM_LD_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND /usr/local/Cellar/llvm/4.0.0/bin/llvm-config --libs OUTPUT_VARIABLE LLVM_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)

include_directories(${Boost_INCLUDE_DIRS} /usr/local/Cellar/icu4c/58.2/include)
link_directories(/usr/local/Cellar/icu4c/58.2/lib/)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LLVM_CXX_FLAGS} -std=c++14 -Wall -pedantic")
set(CMAKE_EXE_LINKER_FLAGS "${LLVM_LD_FLAGS} -fexceptions")

set(SOURCE_FILES compiler/ast.cpp
                 compiler/codegen.cpp
                 compiler/compiler.cpp
                 compiler/diagnostics.cpp
                 compiler/lexer.cpp
                 compiler/main.cpp
                 compiler/parser.cpp
                 compiler/prettyprinter.cpp
                 compiler/symboltable.cpp
                 compiler/token.cpp
                 compiler/typing.cpp
                 compiler/types.cpp)

add_executable(acorn ${SOURCE_FILES})
target_link_libraries(acorn ${LLVM_LIBS} ${Boost_LIBRARIES} z icuuc icudata ncurses)
