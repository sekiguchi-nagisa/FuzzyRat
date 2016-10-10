//
// Created by skgchxngsxyz-carbon on 16/09/22.
//

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

    OpCode(OpKind kind) : kind_(kind), next_(nullptr) {}
    ~OpCode() = default;

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
    ~EmptyOp() = default;
};

struct AnyOp : public OpCode {
    AnyOp() : OpCode(OpKind::Any) {}
    ~AnyOp() = default;
};

class CharOp : public OpCode {
private:
    const int code_;

public:
    CharOp(int code) : OpCode(OpKind::Char), code_(code) {}
    ~CharOp() = default;

    int code() const {
        return this->code_;
    }
};

class AsciiMap {
private:
    std::uint64_t map_[2];
    unsigned int population_;

    AsciiMap(std::uint64_t upper, std::uint64_t lower, unsigned int population) :
            map_{upper, lower}, population_(population) { }

public:
    AsciiMap() : map_{0, 0}, population_(0) { }

    AsciiMap &operator|=(char ch) {
        if(!this->contains(ch)) {
            this->population_++;
        }

        if(ch >= 0 && ch < 64) {
            this->map_[0] |= (1L << ch);
        } else if(ch >= 64) {
            this->map_[1] |= (1L << (ch - 64));
        } else {
            throw std::logic_error("must be ascii character");
        }
        return *this;
    }

    AsciiMap operator~() const {
        return AsciiMap(~this->map_[0], ~this->map_[1], 128 - this->population_);
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
        for(unsigned int i = 64; i < 128; i++) {
            if(this->map_[1] & (1L << (i - 64)) && bitCount++ == count) {
                return i;
            }
        }
        return -1;
    }
};

class CharSetOp : public OpCode {
private:
    AsciiMap map_;

public:
    CharSetOp(AsciiMap &&map) : OpCode(OpKind::CharSet), map_(std::move(map)) {}
    ~CharSetOp() = default;

    const AsciiMap &map() const {
        return this->map_;
    }
};

class AltOp : public OpCode {
private:
    std::vector<OpCodePtr> opcodes_;

public:
    AltOp(std::vector<OpCodePtr> &&opcodes) : OpCode(OpKind::Alt), opcodes_(std::move(opcodes)) {}
    ~AltOp() = default;

    const std::vector<OpCodePtr> &opcodes() const {
        return this->opcodes_;
    }
};

class CallOp : public OpCode {
private:
    unsigned int productionId_;

public:
    CallOp(unsigned int productionId) : OpCode(OpKind::Call), productionId_(productionId) {}
    ~CallOp() = default;

    unsigned int productionId() const {
        return this->productionId_;
    }
};

struct RetOp : public OpCode {
    RetOp() : OpCode(OpKind::Ret) {}
    ~RetOp() = default;
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

    CompiledUnit(CompiledUnit &&s) : statrtId_(s.statrtId_), codes_(std::move(s.codes_)) {}

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

ydsh::ByteBuffer eval(const CompiledUnit &unit);


} // namespace fuzzyrat

#endif //FUZZYRAT_OPCODE_H
