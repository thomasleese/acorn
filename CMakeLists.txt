cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

project(acorn VERSION 0.1.0 LANGUAGES CXX)

add_subdirectory(cmake/catch)
add_subdirectory(cmake/spdlog)

find_package(PkgConfig REQUIRED)

pkg_check_modules(ICU REQUIRED icu-uc)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED OFF)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost REQUIRED COMPONENTS regex)

find_package(LLVM REQUIRED CONFIG)

link_directories(${ICU_LIBRARY_DIRS}) # FIXME make this part of 'acorn' target

add_subdirectory(lib)
add_subdirectory(src)
add_subdirectory(test)
