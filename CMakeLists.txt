cmake_minimum_required(VERSION 3.3)
project(Quark)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp lexer.cpp)

add_executable(Quark ${SOURCE_FILES})
target_link_libraries(Quark ${Boost_LIBRARIES})
