//
// Created by skgchxngsxyz-carbon on 16/09/22.
//

#include <stack>
#include <random>
#include <algorithm>

#include "opcode.h"
#include "misc/fatal.h"

namespace fuzzyrat {

static constexpr unsigned int MAX_STACK_SIZE = 2 * 1024 * 1024;
static constexpr unsigned int INIT_STACK_SIZE = 256;

static std::default_random_engine createRandomEngine() {
    std::vector<int> v(32);
    std::random_device rdev;
    std::generate(v.begin(), v.end(), std::ref(rdev));
    std::seed_seq seed(v.begin(), v.end());
    return std::default_random_engine(seed);
}

struct EvalState {
    ydsh::ByteBuffer buffer;
    CompiledUnit &unit;
    std::stack<OpCode *, std::vector<OpCode *>> retStack;
    std::default_random_engine randomEngine;

    EvalState(CompiledUnit &unit) :
            buffer(16), unit(unit), retStack(std::vector<OpCode *>(INIT_STACK_SIZE)),
            randomEngine(createRandomEngine()) {}

    ~EvalState() = default;
};

// eval function
static OpCode *evalEmpty(EmptyOp *code, EvalState &) {
    return code->next().get();
}

static OpCode *evalAny(AnyOp *code, EvalState &st) {    //FIXME: control character
    char ch = std::uniform_int_distribution<unsigned int>(32, 126)(st.randomEngine);
    st.buffer += ch;
    return code->next().get();
}

static OpCode *evalChar(CharOp *code, EvalState &st) {
    st.buffer += code->code();
    return code->next().get();
}

static OpCode *evalCharSet(CharSetOp *code, EvalState &st) {
    unsigned int index = std::uniform_int_distribution<unsigned int>(0, code->map().population() - 1)(st.randomEngine);
    st.buffer += code->map().lookup(index);
    return code->next().get();
}

static OpCode *evalAlt(AltOp *code, EvalState &st) {
    unsigned int size = code->opcodes().size();

    unsigned int index = std::uniform_int_distribution<unsigned int>(0, size - 1)(st.randomEngine);
    return code->opcodes()[index].get();
}

static OpCode *evalCall(CallOp *code, EvalState &st) {
    auto *next = code->next().get();
    if(st.retStack.size() == MAX_STACK_SIZE) {
        fatal("reach stack size limit\n");
    }
    st.retStack.push(next);
    return st.unit.codes()[code->productionId()].get();
}

static OpCode *evalRet(RetOp *, EvalState &st) {
    auto *next = st.retStack.top();
    st.retStack.pop();
    return next;
}

#if 0

static const char *toString(OpKind kind) {
    const char *v[] = {
#define GEN_STR(E) #E,
            EACH_OP_KIND(GEN_STR)
#undef GEN_STR
    };
    return v[static_cast<unsigned int>(kind)];
}

#define PRINT_OP(OP) fprintf(stderr, "op: %s\n", toString(OP))
#else
#define PRINT_OP(OP)
#endif

static OpCode *eval(OpCode *code, EvalState &st) {
#define GEN_CASE(E) case OpKind::E: PRINT_OP(OpKind::E); return eval ## E (static_cast<E ## Op *>(code), st);
    switch(code->kind()) {
    EACH_OP_KIND(GEN_CASE)
    }
#undef GEN_CASE
}

ydsh::ByteBuffer eval(CompiledUnit &unit) {
    EvalState state(unit);

    auto entryPoint = std::make_shared<CallOp>(unit.startId());
    for(OpCode *code = entryPoint.get(); (code = eval(code, state)) != nullptr;);

    return std::move(state.buffer);
}

} // namespace fuzzyrat
