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

set(LIB_FILES lib/codegen.cpp
              lib/compiler.cpp
              lib/diagnostics.cpp
              lib/lexer.cpp
              lib/parser.cpp
              lib/prettyprinter.cpp
              lib/symboltable.cpp
              lib/token.cpp
              lib/typesystem/checker.cpp
              lib/typesystem/types.cpp
              lib/ast/visitor.cpp
              lib/ast/nodes.cpp
              lib/typesystem/visitor.cpp
              lib/typesystem/inferrer.cpp
              lib/parser/keywords.cpp)

set(TEST_FILES test/parser/keywords_test.cpp)

set(SOURCE_FILES src/main.cpp)

set(LIBS ${LLVM_LIBS} ${Boost_LIBRARIES} ${ICU_LIBRARIES} z ncurses)

add_executable(acorn ${LIB_FILES} ${SOURCE_FILES})
target_link_libraries(acorn ${LIBS})

enable_testing()

include_directories(${GTEST_INCLUDE_DIRS})
add_executable(acorntest ${LIB_FILES} ${TEST_FILES})
target_link_libraries(acorntest ${LIBS} ${GTEST_LIBS_DIR}/libgtest.a ${GTEST_LIBS_DIR}/libgtest_main.a)
add_test(acorntest acorntest)
