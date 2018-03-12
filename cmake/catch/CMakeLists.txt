set(CATCH_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library(catch INTERFACE)
target_include_directories(catch INTERFACE ${CATCH_INCLUDE_DIR})
