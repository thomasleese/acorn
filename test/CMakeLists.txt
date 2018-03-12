add_executable(acorntest
  acorntest.cpp
  ast/nodes_test.cpp
  parser/keywords_test.cpp
  parser/token_test.cpp
)

target_link_libraries(acorntest catch acorn)
