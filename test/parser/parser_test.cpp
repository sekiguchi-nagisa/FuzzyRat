//
// Created by skgchxngsxyz-carbon on 17/04/06.
//

#include <sstream>

#include <gtest/gtest.h>

#include "parser.h"
#include "../test_common.hpp"

using namespace fuzzyrat;

class ParserTest : public ::testing::Test {
public:
    void test(const char *text, const char *expected) {
        ASSERT_TRUE(text != nullptr);
        ASSERT_TRUE(expected != nullptr);

        Lexer lexer("(string)", text, strlen(text));
        Parser parser(lexer);
        auto pair = parser();

        std::stringstream stream;
        NodePrinter printer(stream);

        printer(lexer.toTokenText(pair.token), pair.node);

        ASSERT_STREQ(expected, stream.str().c_str());
    }
};

TEST_F(ParserTest, regex) {
    ASSERT_(this->test("A = .;", "A = .;\n"));
    ASSERT_(this->test("A = ...;", "A = . . .;\n"));
    ASSERT_(this->test("A = '23';", "A = '23';\n"));
    ASSERT_(this->test("A = [ab-d4];", "A = [ab-d4];\n"));
    ASSERT_(this->test("A = [\\^\\]\\\\];", "A = [\\^\\]\\\\];\n"));
    ASSERT_(this->test("A = 'D'  ;", "A = 'D';\n"));
    ASSERT_(this->test("A = B;", "A = B;\n"));
    ASSERT_(this->test("A = '2'|'3';", "A = '2' | '3';\n"));
    ASSERT_(this->test("A = [a-zA-Z]*;", "A = [a-zA-Z]*;\n"));
    ASSERT_(this->test("A = [_0-9]+;", "A = [_0-9]+;\n"));
    ASSERT_(this->test("A = [^_0-9]+;", "A = [^_0-9]+;\n"));
    ASSERT_(this->test("A = .?;", "A = .?;\n"));
    ASSERT_(this->test("A = ( .. )+;", "A = (. .)+;\n"));
}

TEST_F(ParserTest, base) {
    ASSERT_(this->test("a = A* B+ C?;", "a = A* B+ C?;\n"));
    ASSERT_(this->test("a = (A | a);", "a = A | a;\n"));
    ASSERT_(this->test("a = (b* | a+)?;", "a = (b* | a+)?;\n"));
    ASSERT_(this->test("a = 'hello';", "a = 'hello';\n"));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
