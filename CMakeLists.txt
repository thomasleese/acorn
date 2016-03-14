cmake_minimum_required(VERSION 3.3)
project(Quark)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -pedantic -Werror")

set(SOURCE_FILES compiler/main.cpp compiler/Lexer.cpp compiler/AbstractSyntaxTree.cpp compiler/Parser.cpp compiler/Compiler.cpp compiler/Compiler.h)

add_executable(Quark ${SOURCE_FILES})
target_link_libraries(Quark ${Boost_LIBRARIES})
