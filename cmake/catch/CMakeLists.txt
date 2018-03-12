set(CATCH_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library(catch INTERFACE)
target_include_directories(catch INTERFACE ${CATCH_INCLUDE_DIR})

# FIXME check when the following line can be removed
target_compile_definitions(catch INTERFACE CATCH_CONFIG_NO_CPP17_UNCAUGHT_EXCEPTIONS)
