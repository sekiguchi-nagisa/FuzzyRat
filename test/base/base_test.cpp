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

public:
    BaseTest() = default;
    virtual ~BaseTest() = default;

    void doTest(const char *code, ByteBuffer &&expected) {
        ASSERT_TRUE(code != nullptr);
        auto *input = FuzzyRat_newContext("<dummy>", code, strlen(code));
        ASSERT_TRUE(input != nullptr);

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
};

ByteBuffer operator ""_buf(const char *str, std::size_t N) {
    ByteBuffer buf(N);
    buf.append(str, N);
    return buf;
}

TEST_F(BaseTest, any) {
    this->setSequence({'a', 'A', '@', '7'});
    ASSERT_(this->doTest("A = .... ;", "aA@7"_buf));
}

TEST_F(BaseTest, charset) {
    this->setSequence({1, 2, 0, 3});
    ASSERT_(this->doTest("A = [abc] [abc] [abc] ;", "bca"_buf));

    ASSERT_(this->doTest("A = [a-c_] [a-c_] [a-c_] [a-c_];", "cab_"_buf));
}

TEST_F(BaseTest, string) {
    ASSERT_(this->doTest("A = 'abcdefg' ;", "abcdefg"_buf));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
