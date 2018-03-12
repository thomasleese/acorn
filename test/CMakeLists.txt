enable_testing()

add_executable(acorntest
  acorntest.cpp
  ast/nodes_test.cpp
  parser/keywords_test.cpp
  parser/token_test.cpp
)

target_include_directories(acorntest
  PUBLIC ../include
)

target_link_libraries(acorntest catch acorn ${LIBS} ${GTEST_LIBS})

add_test(acorntest acorntest)
