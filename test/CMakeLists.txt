add_executable(acorntest
  acorntest.cpp
  diagnostics.cpp
  parser/parser.cpp
  parser/scanner.cpp
  parser/token.cpp
)

target_link_libraries(acorntest catch acorn)
