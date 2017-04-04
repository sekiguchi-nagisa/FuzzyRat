//
// Created by skgchxngsxyz-carbon on 17/04/04.
//

#include <gtest/gtest.h>

#include "logger.h"

using namespace fuzzyrat;

class LoggerTest : public ::testing::Test {
public:
    virtual ~LoggerTest() = default;

    virtual void TearDown() {
        unsetenv("FRAT_APPENDER");
        unsetenv("FRAT_LEVEL");
    }
};

TEST_F(LoggerTest, case1) {
    setenv("FRAT_LEVEL", "info", 1);
    ASSERT_NO_FATAL_FAILURE(EXPECT_EXIT({LOG_INFO("hello"); exit(0); }, ::testing::ExitedWithCode(0), "\\[info\\] hello"));

    setenv("FRAT_LEVEL", "error", 1);
    ASSERT_NO_FATAL_FAILURE(EXPECT_EXIT({LOG_WARN("hello"); exit(0); }, ::testing::ExitedWithCode(0), ""));

    setenv("FRAT_LEVEL", "error", 1);
    ASSERT_NO_FATAL_FAILURE(EXPECT_EXIT({LOG_ERROR("hello"); exit(0); }, ::testing::ExitedWithCode(1), "\\[error\\] hello"));

    setenv("FRAT_LEVEL", "debug", 1);
    ASSERT_NO_FATAL_FAILURE(EXPECT_EXIT({LOG_ERROR("hello"); exit(0); }, ::testing::ExitedWithCode(1), "\\[error\\] hello"));

    setenv("FRAT_APPENDER", "/dev/stderr", 1);
    ASSERT_NO_FATAL_FAILURE(EXPECT_EXIT({LOG_DEBUG("hello"); exit(0); }, ::testing::ExitedWithCode(0), "hello"));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}