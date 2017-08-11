cmake_minimum_required(VERSION 3.9)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

project(Acorn)

find_package(PkgConfig REQUIRED)

pkg_search_module(ICU REQUIRED icu-uc)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED OFF)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost REQUIRED COMPONENTS regex)

find_package(LLVM REQUIRED)

include_directories(${Boost_INCLUDE_DIRS} ${ICU_INCLUDE_DIRS})
link_directories(${ICU_LIBRARY_DIRS})

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LLVM_CXX_FLAGS} -Wall -pedantic")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LLVM_LD_FLAGS} -fexceptions")

set(SOURCE_FILES compiler/codegen.cpp
                 compiler/compiler.cpp
                 compiler/diagnostics.cpp
                 compiler/lexer.cpp
                 compiler/main.cpp
                 compiler/parser.cpp
                 compiler/prettyprinter.cpp
                 compiler/symboltable.cpp
                 compiler/token.cpp
                 compiler/typesystem/checker.cpp
                 compiler/typesystem/types.cpp
                 compiler/ast/visitor.cpp
                 compiler/ast/nodes.cpp
                 compiler/typesystem/visitor.cpp
                 compiler/typesystem/inferrer.cpp)

add_executable(acorn ${SOURCE_FILES})
target_link_libraries(acorn ${LLVM_LIBS} ${Boost_LIBRARIES} ${ICU_LIBRARIES} z ncurses)
