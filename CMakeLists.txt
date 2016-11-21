cmake_minimum_required(VERSION 2.7)
project(FuzzyRat)


#+++++++++++++++++++#
#     build re2c    #
#+++++++++++++++++++#

set(RE2C_V "0.16")
set(RE2C_URL "https://github.com/skvadrik/re2c/releases/download/${RE2C_V}/re2c-${RE2C_V}.tar.gz")
set(RE2C_PATH ${CMAKE_CURRENT_BINARY_DIR}/re2c-${RE2C_V}.tar.gz)
set(RE2C_DIR ${CMAKE_CURRENT_BINARY_DIR}/re2c-${RE2C_V})

if(NOT EXISTS "${RE2C_DIR}")
    file(DOWNLOAD "${RE2C_URL}" "${RE2C_PATH}" SHOW_PROGRESS)
    execute_process(COMMAND ${CMAKE_COMMAND} -E tar xvf ${RE2C_PATH})
endif()

if(NOT EXISTS "${RE2C_DIR}/re2c")
    execute_process(COMMAND ./configure WORKING_DIRECTORY ${RE2C_DIR})
    execute_process(COMMAND make WORKING_DIRECTORY ${RE2C_DIR})
endif()


#========================#
#     generate Lexer     #
#========================#

set(lexer_src ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp)
add_custom_command(OUTPUT ${lexer_src}
    COMMAND ${RE2C_DIR}/re2c -W -Werror -8 -s -o ${lexer_src}
        ${CMAKE_CURRENT_SOURCE_DIR}/src/lexer.re2c.cpp
)
add_custom_target(gen_lexer DEPENDS ${lexer_src})


#########################
##     libFuzzyRat     ##
#########################

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -std=c++11")

set(SOURCE_FILES
        ${lexer_src}
        src/node.cpp
        src/parser.cpp
        src/error.cpp
        src/verify.cpp
        src/logger.cpp
        src/compile.cpp
        src/fuzzyrat.cpp)

include_directories(src)
include_directories(include)
add_library(FuzzyRat_static STATIC ${SOURCE_FILES})


######################
##     FuzzyRat     ##
######################

add_executable(FuzzyRat src/main.cpp)
target_link_libraries(FuzzyRat FuzzyRat_static)


#+++++++++++++++++++++++++#
#     show enviroment     #
#+++++++++++++++++++++++++#

message("")
message("+++++ List of System Configuration +++++")
message(STATUS "CMAKE_BUILD_TYPE                 = ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_SYSTEM                     = ${CMAKE_SYSTEM}")
message(STATUS "CMAKE_CXX_COMPILER_ID            = ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "CMAKE_CXX_COMPILER_VERSION       = ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "CMAKE_CXX_COMPILER               = ${CMAKE_CXX_COMPILER}")
message(STATUS "CMAKE_CXX_FLAGS                  = ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS_DEBUG            = ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CMAKE_CXX_FLAGS_RELEASE          = ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "CMAKE_CXX_FLAGS_RELWITHDEBINFO   = ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
message(STATUS "CMAKE_INSTALL_PREFIX             = ${CMAKE_INSTALL_PREFIX}")
message("")
