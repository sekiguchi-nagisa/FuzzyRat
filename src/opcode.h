//
// Created by skgchxngsxyz-carbon on 16/09/22.
//

#ifndef FUZZYRAT_OPCODE_H
#define FUZZYRAT_OPCODE_H

#include <vector>
#include <memory>

#include "misc/noncopyable.h"

namespace fuzzyrat {

#define EACH_OP_KIND(E) \
    E(Empty) \
    E(Any) \
    E(Char) \
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

//FIXME: charset

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

} // namespace fuzzyrat

#endif //FUZZYRAT_OPCODE_H
