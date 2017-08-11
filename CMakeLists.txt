cmake_minimum_required(VERSION 3.2)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

project(Acorn)

add_subdirectory(cmake/gtest)
add_subdirectory(compiler)
