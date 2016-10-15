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
    const CompiledUnit &unit;
    std::stack<OpCode *, std::vector<OpCode *>> retStack;
    std::default_random_engine randomEngine;

    EvalState(const CompiledUnit &unit) :
            buffer(16), unit(unit), retStack(std::vector<OpCode *>(INIT_STACK_SIZE)),
            randomEngine(createRandomEngine()) {}

    ~EvalState() = default;
};

// eval function
static OpCode *evalEmpty(const EmptyOp *code, EvalState &) {
    return code->next().get();
}

static OpCode *evalAny(const AnyOp *code, EvalState &st) {    //FIXME: control character
    char ch = std::uniform_int_distribution<unsigned int>(32, 126)(st.randomEngine);
    st.buffer += ch;
    return code->next().get();
}

static OpCode *evalChar(const CharOp *code, EvalState &st) {
    st.buffer += code->code();
    return code->next().get();
}

static OpCode *evalCharSet(const CharSetOp *code, EvalState &st) {
    unsigned int index = std::uniform_int_distribution<unsigned int>(0, code->map().population() - 1)(st.randomEngine);
    st.buffer += code->map().lookup(index);
    return code->next().get();
}

static OpCode *evalAlt(const AltOp *code, EvalState &st) {
    unsigned int size = code->opcodes().size();

    unsigned int index = std::uniform_int_distribution<unsigned int>(0, size - 1)(st.randomEngine);
    return code->opcodes()[index].get();
}

static OpCode *evalCall(const CallOp *code, EvalState &st) {
    auto *next = code->next().get();
    if(st.retStack.size() == MAX_STACK_SIZE) {
        fatal("reach stack size limit\n");
    }
    st.retStack.push(next);
    return st.unit.codes()[code->productionId()].get();
}

static OpCode *evalRet(const RetOp *, EvalState &st) {
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

static OpCode *eval(const OpCode *code, EvalState &st) {
#define GEN_CASE(E) case OpKind::E: PRINT_OP(OpKind::E); return eval ## E (static_cast<const E ## Op *>(code), st);
    switch(code->kind()) {
    EACH_OP_KIND(GEN_CASE)
    }
#undef GEN_CASE
    return nullptr;
}

ydsh::ByteBuffer eval(const CompiledUnit &unit) {
    EvalState state(unit);

    auto entryPoint = std::make_shared<CallOp>(unit.startId());
    for(const OpCode *code = entryPoint.get(); (code = eval(code, state)) != nullptr;);

    return std::move(state.buffer);
}

} // namespace fuzzyrat