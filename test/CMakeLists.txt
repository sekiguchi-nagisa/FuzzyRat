#++++++++++++++++++++++++++++++++++++#
#     setup test in subdirectory     #
#++++++++++++++++++++++++++++++++++++#

include_directories(${GTEST_DIR}/googletest/include)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/base)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/logger)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/parser)
