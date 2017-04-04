#=====================#
#     logger_test     #
#=====================#

set(TEST_NAME logger_test)

add_executable(${TEST_NAME}
    logger_test.cpp
)
target_link_libraries(${TEST_NAME} gtest gtest_main FuzzyRat_static)
add_test(${TEST_NAME} ${TEST_NAME})