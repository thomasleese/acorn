add_executable(acorntest
  acorntest.cpp
  ast/nodes.cpp
  parser/keywords.cpp
  parser/token.cpp
  examples/examples.cpp
)

target_link_libraries(acorntest catch acorn)
