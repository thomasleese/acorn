find_package(PkgConfig REQUIRED)

pkg_search_module(ICU REQUIRED icu-uc)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED OFF)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost REQUIRED COMPONENTS regex)

find_package(LLVM REQUIRED)

include_directories(${Boost_INCLUDE_DIRS} ${ICU_INCLUDE_DIRS})
link_directories(${ICU_LIBRARY_DIRS})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LLVM_CXX_FLAGS} -Wall -pedantic")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LLVM_LD_FLAGS} -fexceptions")

include_directories(include)

set(SOURCE_FILES src/codegen.cpp
                 src/compiler.cpp
                 src/diagnostics.cpp
                 src/lexer.cpp
                 src/main.cpp
                 src/parser.cpp
                 src/prettyprinter.cpp
                 src/symboltable.cpp
                 src/token.cpp
                 src/typesystem/checker.cpp
                 src/typesystem/types.cpp
                 src/ast/visitor.cpp
                 src/ast/nodes.cpp
                 src/typesystem/visitor.cpp
                 src/typesystem/inferrer.cpp
                 src/parser/keywords.cpp)

add_executable(acorn ${SOURCE_FILES})
target_link_libraries(acorn ${LLVM_LIBS} ${Boost_LIBRARIES} ${ICU_LIBRARIES} z ncurses)
