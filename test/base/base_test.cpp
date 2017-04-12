//
// Created by skgchxngsxyz-carbon on 17/01/22.
//

#include <gtest/gtest.h>
#include <fuzzyrat.h>

#include "misc/size.hpp"
#include "misc/resource.hpp"

#include "../test_common.hpp"

using namespace ydsh;

int FuzzyRat_execImpl(const FuzzyRatCode *code, FuzzyRatResult *result, fuzzyrat::RandFactory &factory);

static std::string format(const ByteBuffer &expect, const FuzzyRatResult &actual) {
    std::string str = "expect:\n";
    for(unsigned int i = 0; i < expect.size(); i++) {
        char buf[8];
        snprintf(buf, arraySize(buf), "%x", static_cast<unsigned int>(expect[i]));
        str += buf;
    }

    str += "\nactual:\n";
    for(unsigned int i = 0; i < actual.size; i++) {
        char buf[8];
        snprintf(buf, arraySize(buf), "%x", static_cast<unsigned int>(actual.data[i]));
        str += buf;
    }
    return str;
}

struct ResultDeleter {
    void operator()(FuzzyRatResult &result) const {
        FuzzyRat_releaseResult(&result);
    }
};

using ScopedResult = ScopedResource<FuzzyRatResult, ResultDeleter>;

class BaseTest : public ::testing::Test {
private:
    ControlledRandFactory randFactory;

    std::string space;

    std::string start;

public:
    BaseTest() = default;
    virtual ~BaseTest() = default;

    virtual void SetUp() {
        this->setSpace("' '");
    }

    void test(const char *code, ByteBuffer &&expected) {
        ASSERT_TRUE(code != nullptr);
        auto *input = FuzzyRat_newContext(nullptr, code, strlen(code));
        ASSERT_TRUE(input != nullptr);

        FuzzyRat_setSpacePattern(input, this->space.c_str());
        if(this->start.size()) {
            FuzzyRat_setStartProduction(input, this->start.c_str());
        }

        auto *cc = FuzzyRat_compile(input);
        FuzzyRat_deleteContext(&input);

        FuzzyRatResult result;
        int r = FuzzyRat_execImpl(cc, &result, this->randFactory);
        FuzzyRat_deleteCode(&cc);
        auto scopedResult = makeScopedResource(std::move(result), ResultDeleter());
        ASSERT_EQ(0, r);

        ASSERT_EQ(expected.size(), result.size);
        if(memcmp(expected.get(), result.data, expected.size()) != 0) {
            EXPECT_TRUE(false) << format(expected, result);
        }
    }

protected:
    void setSequence(std::vector<unsigned int> &&v) {
        this->randFactory.setSequence(std::move(v));
    }

    void setSpace(const char *space) {
        this->space = space;
    }

    void setStartProduction(const char *p) {
        this->start = p;
    }
};

ByteBuffer operator ""_buf(const char *str, std::size_t N) {
    ByteBuffer buf(N);
    buf.append(str, N);
    return buf;
}

TEST_F(BaseTest, any) {
    this->setSequence({'a', 'A', '@', '7'});
    ASSERT_NO_FATAL_FAILURE(ASSERT_(this->test("A = .... ;", "aA@7"_buf)));
}

TEST_F(BaseTest, charset1) {
    this->setSequence({1, 2, 0, 3});
    ASSERT_NO_FATAL_FAILURE(ASSERT_(this->test("A = [abc] [abc] [abc] ;", "bca"_buf)));

    ASSERT_NO_FATAL_FAILURE(ASSERT_(this->test("A = [a-c_] [a-c_] [a-c_] [a-c_];", "cab_"_buf)));
}

TEST_F(BaseTest, charset2) {
    this->setSequence({1, 2, 0, 3});
    const char *src = "A = [^\\x00-\\x1F\\x7F];";

    ASSERT_NO_FATAL_FAILURE(ASSERT_(this->test(src, "!"_buf)));
    ASSERT_NO_FATAL_FAILURE(ASSERT_(this->test(src, "\""_buf)));
    ASSERT_NO_FATAL_FAILURE(ASSERT_(this->test(src, " "_buf)));
    ASSERT_NO_FATAL_FAILURE(ASSERT_(this->test(src, "#"_buf)));
}

TEST_F(BaseTest, charset3) {
    this->setSequence({1, 2, 0, 3});
    const char *src = "A = [\\^\\]x\\\\];"; //       \ ] ^ x

    ASSERT_NO_FATAL_FAILURE(ASSERT_(this->test(src, "]"_buf)));
    ASSERT_NO_FATAL_FAILURE(ASSERT_(this->test(src, "^"_buf)));
    ASSERT_NO_FATAL_FAILURE(ASSERT_(this->test(src, "\\"_buf)));
    ASSERT_NO_FATAL_FAILURE(ASSERT_(this->test(src, "x"_buf)));
}

TEST_F(BaseTest, string) {
    ASSERT_NO_FATAL_FAILURE(ASSERT_(this->test("A = 'abcdefg' ;", "abcdefg"_buf)));

    ASSERT_NO_FATAL_FAILURE(ASSERT_(this->test("A = '\\x53\\n\\t\\\\' ;", "S\n\t\\"_buf)));
}

TEST_F(BaseTest, alter1) {
    this->setSequence({2, 0, 1});
    const char *src = "A = 'a' | 'b' | 'c';";
    ASSERT_NO_FATAL_FAILURE(ASSERT_(this->test(src, "c"_buf)));
    ASSERT_NO_FATAL_FAILURE(ASSERT_(this->test(src, "a"_buf)));
    ASSERT_NO_FATAL_FAILURE(ASSERT_(this->test(src, "b"_buf)));

    this->setSequence({0, 1});
    src = "A = 'a' ('b' | 'c') 'd';";
    ASSERT_NO_FATAL_FAILURE(ASSERT_(this->test(src, "abd"_buf)));
    ASSERT_NO_FATAL_FAILURE(ASSERT_(this->test(src, "acd"_buf)));
}

TEST_F(BaseTest, alter2) {
    const char *src = "a = 'a' ('b' | 'c');";
    this->setSequence({0, 1});

    ASSERT_NO_FATAL_FAILURE(ASSERT_(this->test(src, " a b "_buf)));
    ASSERT_NO_FATAL_FAILURE(ASSERT_(this->test(src, " a c "_buf)));
}

TEST_F(BaseTest, option1) {
    this->setSequence({1, 0, 1});
    const char *src = "A = 'a'?;";
    ASSERT_NO_FATAL_FAILURE(ASSERT_(this->test(src, ""_buf)));
    ASSERT_NO_FATAL_FAILURE(ASSERT_(this->test(src, "a"_buf)));
    ASSERT_NO_FATAL_FAILURE(ASSERT_(this->test(src, ""_buf)));
}

TEST_F(BaseTest, option2) {
    const char *src = "a = 'a'?;";
    this->setSequence({1, 0, 1});
    ASSERT_NO_FATAL_FAILURE(ASSERT_(this->test(src, "  "_buf)));
    ASSERT_NO_FATAL_FAILURE(ASSERT_(this->test(src, " a "_buf)));
    ASSERT_NO_FATAL_FAILURE(ASSERT_(this->test(src, "  "_buf)));
}

TEST_F(BaseTest, zero1) {
    this->setSequence({1});
    const char *src = "A = 'a'*;";
    ASSERT_NO_FATAL_FAILURE(ASSERT_(this->test(src, ""_buf)));

    this->setSequence({0, 0, 1});
    ASSERT_NO_FATAL_FAILURE(ASSERT_(this->test(src, "aa"_buf)));
}

TEST_F(BaseTest, zero2) {
    this->setSequence({1});
    const char *src = "a = 'a'*;";
    ASSERT_NO_FATAL_FAILURE(ASSERT_(this->test(src, "  "_buf)));

    this->setSequence({0, 0, 1});
    ASSERT_NO_FATAL_FAILURE(ASSERT_(this->test(src, "  a a "_buf)));
}

TEST_F(BaseTest, one1) {
    this->setSequence({1});
    const char *src = "A = 'a'+;";
    ASSERT_NO_FATAL_FAILURE(ASSERT_(this->test(src, "a"_buf)));

    this->setSequence({0, 0, 1});
    ASSERT_NO_FATAL_FAILURE(ASSERT_(this->test(src, "aaa"_buf)));
}

TEST_F(BaseTest, one2) {
    this->setSequence({1});
    const char *src = "a = 'a'+;";
    ASSERT_NO_FATAL_FAILURE(ASSERT_(this->test(src, "  a "_buf)));

    this->setSequence({0, 0, 1});
    ASSERT_NO_FATAL_FAILURE(ASSERT_(this->test(src, "  a a a "_buf)));
}

TEST_F(BaseTest, nterm1) {
    const char *src = "B = A; A = 'b' | 'a' A;";
    this->setSequence({1, 0});
    ASSERT_NO_FATAL_FAILURE(ASSERT_(this->test(src, "ab"_buf)));
}

TEST_F(BaseTest, error1) {
    const char *src = "A = 'a'";
    const char *expect = R"(\[error\] \(<null>\):1: mismatched token: EOS, expected: SEMI_COLON)";
    ASSERT_NO_FATAL_FAILURE(EXPECT_EXIT({this->test(src, ""_buf); exit(0);}, ::testing::ExitedWithCode(1), expect));

    src = ".";
    expect = R"(\[error\] \(<null>\):1: no viable alternative: DOT, expected: TERM, NTERM)";
    ASSERT_NO_FATAL_FAILURE(EXPECT_EXIT({this->test(src, ""_buf); exit(0);}, ::testing::ExitedWithCode(1), expect));

    src = "1";
    expect = R"(\[error\] \(<null>\):1: invalid token, expected: TERM, NTERM)";
    ASSERT_NO_FATAL_FAILURE(EXPECT_EXIT({this->test(src, ""_buf); exit(0);}, ::testing::ExitedWithCode(1), expect));


    src = "A = ";
    expect = R"(\[error\] \(<null>\):1: no viable alternative: EOS, expected: POPEN, TERM, DOT, CHARSET, STRING)";
    ASSERT_NO_FATAL_FAILURE(EXPECT_EXIT({this->test(src, ""_buf); exit(0);}, ::testing::ExitedWithCode(1), expect));

    src = "A = 1";
    expect = R"(\[error\] \(<null>\):1: invalid token, expected: POPEN, TERM, DOT, CHARSET, STRING)";
    ASSERT_NO_FATAL_FAILURE(EXPECT_EXIT({this->test(src, ""_buf); exit(0);}, ::testing::ExitedWithCode(1), expect));

    src = "a = ";
    expect = R"(\[error\] \(<null>\):1: no viable alternative: EOS, expected: POPEN, TERM, NTERM, STRING)";
    ASSERT_NO_FATAL_FAILURE(EXPECT_EXIT({this->test(src, ""_buf); exit(0);}, ::testing::ExitedWithCode(1), expect));

    src = "a = 1";
    expect = R"(\[error\] \(<null>\):1: invalid token, expected: POPEN, TERM, NTERM, STRING)";
    ASSERT_NO_FATAL_FAILURE(EXPECT_EXIT({this->test(src, ""_buf); exit(0);}, ::testing::ExitedWithCode(1), expect));
}

TEST_F(BaseTest, error2) {
    const char *expect = "\\[error\\] start production not found";
    const char *src = "   ";
    ASSERT_NO_FATAL_FAILURE(EXPECT_EXIT({this->test(src, ""_buf); exit(0);}, ::testing::ExitedWithCode(1), expect));

    expect = "\\[error\\] undefined start production: C";
    src = "A = B;";
    this->setStartProduction("C");
    ASSERT_NO_FATAL_FAILURE(EXPECT_EXIT({this->test(src, ""_buf); exit(0);}, ::testing::ExitedWithCode(1), expect));

    src = "A = .; A = .;";
    expect = R"(\[error\] \(<null>\):1: already defined production)";
    this->setStartProduction("A");
    ASSERT_NO_FATAL_FAILURE(EXPECT_EXIT({this->test(src, ""_buf); exit(0);}, ::testing::ExitedWithCode(1), expect));

    src = "A = B;";
    expect = R"(\[error\] \(<null>\):1: undefined non-terminal)";
    ASSERT_NO_FATAL_FAILURE(EXPECT_EXIT({this->test(src, ""_buf); exit(0);}, ::testing::ExitedWithCode(1), expect));
}

TEST_F(BaseTest, error3) {
    const char *src = "a = 'a' a;";
    const char *expect = "reach stack size limit";
    ASSERT_NO_FATAL_FAILURE(EXPECT_EXIT({this->test(src, ""_buf); exit(0);}, ::testing::KilledBySignal(SIGABRT), expect));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
