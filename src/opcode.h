/*
 * Copyright (C) 2016 Nagisa Sekiguchi
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FUZZYRAT_OPCODE_H
#define FUZZYRAT_OPCODE_H

#include <vector>
#include <memory>

#include "misc/noncopyable.h"
#include "misc/buffer.hpp"

namespace fuzzyrat {

#define EACH_OP_KIND(E) \
    E(Empty) \
    E(Any) \
    E(Char) \
    E(CharSet) \
    E(Alt) \
    E(Call) \
    E(Ret)

enum class OpKind : unsigned int {
#define GEN_ENUM(E) E,
    EACH_OP_KIND(GEN_ENUM)
#undef GEN_ENUM
};

class OpCode;

using OpCodePtr = std::shared_ptr<OpCode>;

class OpCode {
protected:
    const OpKind kind_;

    OpCodePtr next_;

public:
    NON_COPYABLE(OpCode);

    explicit OpCode(OpKind kind) : kind_(kind), next_(nullptr) {}
    virtual ~OpCode() = default;

    OpKind kind() const {
        return this->kind_;
    }

    bool is(OpKind kind) const {
        return this->kind() == kind;
    }

    const OpCodePtr &next() const {
        return this->next_;
    }

    void setNext(OpCodePtr &&next) {
        this->next_ = std::move(next);
    }

    void setNext(const OpCodePtr &next) {
        this->next_ = next;
    }
};

struct EmptyOp : public OpCode {
    EmptyOp() : OpCode(OpKind::Empty) {}
    ~EmptyOp() override = default;
};

struct AnyOp : public OpCode {
    AnyOp() : OpCode(OpKind::Any) {}
    ~AnyOp() override = default;
};

class CharOp : public OpCode {
private:
    const int code_;

public:
    explicit CharOp(int code) : OpCode(OpKind::Char), code_(code) {}
    ~CharOp() override = default;

    int code() const {
        return this->code_;
    }
};

class AsciiMap {
private:
    std::uint64_t map_[2];
    unsigned int population_{0};

    AsciiMap(std::uint64_t upper, std::uint64_t lower, unsigned int population) :
            map_{upper, lower}, population_(population) { }

public:
    AsciiMap() : map_{0, 0} { }

    AsciiMap &operator|=(char ch) {
        if(!this->contains(ch)) {
            this->population_++;
        }

        if(ch >= 0 && ch < 64) {
            this->map_[0] |= (1L << ch);
        } else if(ch >= 64) {
            this->map_[1] |= (1L << (ch - 64));
        } else {
            fatal("must be ascii character\n");
        }
        return *this;
    }

    AsciiMap operator~() const {
        return {~this->map_[0], ~this->map_[1], 128 - this->population_};
    }

    bool contains(char ch) const {
        return ch < 0 ? false :
               ch < 64 ? this->map_[0] & (1L << ch) :
               this->map_[1] & (1L << (ch - 64));
    }

    unsigned int population() const {
        return this->population_;
    }

    char lookup(unsigned int count) const {
        unsigned int bitCount = 0;

        // search lower bit
        for(unsigned int i = 0; i < 64; i++) {
            if(this->map_[0] & (1L << i) && bitCount++ == count) {
                return i;
            }
        }

        // search upper bit
        for(unsigned int i = 0; i < 64; i++) {
            if(this->map_[1] & (1L << i) && bitCount++ == count) {
                return i + 64;
            }
        }
        return -1;
    }
};

class CharSetOp : public OpCode {
private:
    AsciiMap map_;

public:
    explicit CharSetOp(AsciiMap &&map) : OpCode(OpKind::CharSet), map_(std::move(map)) {}
    ~CharSetOp() override = default;

    const AsciiMap &map() const {
        return this->map_;
    }
};

class AltOp : public OpCode {
private:
    std::vector<OpCodePtr> opcodes_;

public:
    explicit AltOp(std::vector<OpCodePtr> &&opcodes) : OpCode(OpKind::Alt), opcodes_(std::move(opcodes)) {}
    ~AltOp() override = default;

    const std::vector<OpCodePtr> &opcodes() const {
        return this->opcodes_;
    }
};

class CallOp : public OpCode {
private:
    unsigned int productionId_;

public:
    explicit CallOp(unsigned int productionId) : OpCode(OpKind::Call), productionId_(productionId) {}
    ~CallOp() override = default;

    unsigned int productionId() const {
        return this->productionId_;
    }
};

struct RetOp : public OpCode {
    RetOp() : OpCode(OpKind::Ret) {}
    ~RetOp() override = default;
};

class CompiledUnit {
private:
    /**
     * indicate start production id
     */
    unsigned int statrtId_;

    std::vector<OpCodePtr> codes_;

public:
    NON_COPYABLE(CompiledUnit);

    CompiledUnit(unsigned int startId, std::vector<OpCodePtr> &&codes) :
            statrtId_(startId), codes_(std::move(codes)) {}

    CompiledUnit(CompiledUnit &&s) noexcept : statrtId_(s.statrtId_), codes_(std::move(s.codes_)) {}

    ~CompiledUnit() = default;

    CompiledUnit &operator=(CompiledUnit &&s) noexcept {
        auto tmp(std::move(s));
        this->swap(tmp);
        return *this;
    }

    void swap(CompiledUnit &e) noexcept {
        std::swap(this->statrtId_, e.statrtId_);
        std::swap(this->codes_, e.codes_);
    }

    unsigned int startId() const {
        return this->statrtId_;
    }

    const std::vector<std::shared_ptr<OpCode>> &codes() const {
        return this->codes_;
    }
};

struct RandFactory {
    virtual unsigned int generate(unsigned int start, unsigned int stop) = 0;
    virtual ~RandFactory() = default;
};


ydsh::ByteBuffer eval(const CompiledUnit &unit, RandFactory &randFactory);


} // namespace fuzzyrat

#endif //FUZZYRAT_OPCODE_H
