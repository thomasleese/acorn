add_executable(acornc acornc.cpp)

target_include_directories(acornc PUBLIC ../include)
target_link_libraries(acornc acorn ${LIBS})
target_compile_features(acornc PUBLIC cxx_std_17)
