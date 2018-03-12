enable_testing()

set(GTEST_LIBS ${GTEST_LIBS_DIR}/libgtest.a ${GTEST_LIBS_DIR}/libgtest_main.a)

add_executable(acorntest
  ast/nodes_test.cpp
  parser/keywords_test.cpp
  parser/token_test.cpp
)

target_include_directories(acorntest
  PUBLIC ../include
  PRIVATE ${GTEST_INCLUDE_DIRS}
)

target_link_libraries(acorntest acorn ${LIBS} ${GTEST_LIBS})

add_test(acorntest acorntest)
