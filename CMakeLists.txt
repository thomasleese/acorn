cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

project(acorn VERSION 0.1.0 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

add_subdirectory(cmake/gtest)

find_package(PkgConfig REQUIRED)

pkg_search_module(ICU REQUIRED icu-uc)
pkg_search_module(SPDLOG REQUIRED spdlog)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED OFF)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost REQUIRED COMPONENTS regex)

find_package(LLVM REQUIRED)

include_directories(${Boost_INCLUDE_DIRS} ${ICU_INCLUDE_DIRS} ${SPDLOG_INCLUDE_DIRS})
link_directories(${ICU_LIBRARY_DIRS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LLVM_CXX_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LLVM_LD_FLAGS}")

set(LIBS ${LLVM_LIBS} ${Boost_LIBRARIES} ${ICU_LIBRARIES} z ncurses)

add_subdirectory(lib)
add_subdirectory(src)
add_subdirectory(test)
