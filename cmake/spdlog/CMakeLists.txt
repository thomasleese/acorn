set(SPDLOG_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library(spdlog INTERFACE)
target_include_directories(spdlog INTERFACE ${SPDLOG_INCLUDE_DIR})
