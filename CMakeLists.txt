cmake_minimum_required(VERSION 3.3)
project(Quark)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -pedantic -Werror")

set(SOURCE_FILES main.cpp Lexer.cpp AbstractSyntaxTree.cpp Parser.cpp)

add_executable(Quark ${SOURCE_FILES})
target_link_libraries(Quark ${Boost_LIBRARIES})
