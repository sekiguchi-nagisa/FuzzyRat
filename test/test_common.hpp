//
// Created by skgchxngsxyz-carbon on 17/01/22.
//

#ifndef FUZZYRAT_TEST_COMMON_HPP
#define FUZZYRAT_TEST_COMMON_HPP

#include <cassert>

#include "opcode.h"

#define ASSERT_(E) do { SCOPED_TRACE(""); (E); } while(false)

class ControlledRandFactory : public fuzzyrat::RandFactory {
private:
    std::vector<unsigned int> sequence;
    unsigned int count;

public:
    ControlledRandFactory() : sequence({0}), count(0) {}
    ~ControlledRandFactory() = default;

    void setSequence(std::vector<unsigned int> &&sequence) {
        this->sequence = std::move(sequence);
        this->count = 0;
    }

    unsigned int generate(unsigned int start, unsigned int stop) override {
        assert(start <= stop);
        unsigned int v = this->sequence.empty() ? 0 : this->sequence[this->count++ % this->sequence.size()];
        if(v < start) {
            v = start;
        }
        if(v > stop) {
            v = stop;
        }
        return v;
    }
};

#endif //FUZZYRAT_TEST_COMMON_HPP
