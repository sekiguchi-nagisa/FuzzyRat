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

#ifndef FUZZYRAT_COMPILE_H
#define FUZZYRAT_COMPILE_H

#include "misc/fatal.h"
#include "node.h"
#include "opcode.h"
#include "state.h"

namespace fuzzyrat {

class Compiler : public NodeVisitor {
private:
    OpCodePtr head;
    OpCodePtr tail;

    unsigned int idCount{0};
    std::unordered_map<std::string, unsigned int> name2IdMap;

public:
    Compiler() : head(nullptr), tail(nullptr) {}
    ~Compiler() = default;

#define GEN_VISIT(E) void visit(E ## Node &node) override;
    EACH_NODE_KIND(GEN_VISIT)
#undef GEN_VISIT

    CompiledUnit operator()(const GrammarState &state);

private:
    void append(OpCodePtr &&code);

    OpCodePtr extract() {
        this->tail = nullptr;
        return std::move(this->head);
    }

    template <typename T, typename ... Arg>
    void generate(Arg && ... arg) {
        this->append(std::make_shared<T>(std::forward<Arg>(arg)...));
    }

    unsigned int getProductionId(const std::string &name);
};

} // namespace fuzzyrat

#endif //FUZZYRAT_COMPILE_H
