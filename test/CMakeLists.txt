add_executable(acorntest
  acorntest.cpp
  ast/nodes.cpp
  parser/keywords.cpp
  parser/token.cpp
)

target_link_libraries(acorntest catch acorn)
