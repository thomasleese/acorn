enable_testing()

set(TEST_FILES parser/keywords_test.cpp
               parser/token_test.cpp)

include_directories(${GTEST_INCLUDE_DIRS})

set(GTEST_LIBS ${GTEST_LIBS_DIR}/libgtest.a ${GTEST_LIBS_DIR}/libgtest_main.a)

add_executable(acorntest ${TEST_FILES})
target_link_libraries(acorntest acorn ${LIBS} ${GTEST_LIBS})

add_test(acorntest acorntest)
