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

#include <cstring>

#include "node.h"
#include "misc/num.h"

namespace fuzzyrat {

// ##################
// ##     Node     ##
// ##################

void Node::accept(NodeVisitor &visitor) {
    switch(this->kind()) {
#define GEN_CASE(E) case NodeKind::E: visitor.visit(static_cast<E ## Node &>(*this)); break;
    EACH_NODE_KIND(GEN_CASE)
#undef GEN_CASE
    }
}

static bool isUpperLetter(char ch) {
    return std::isalpha(ch) && std::isupper(ch);
}

static bool isLowerLetter(char ch) {
    return std::isalpha(ch) && std::islower(ch);
}

bool isTerminal(const std::string &name) {
    return !name.empty() && (name.front() == '_'  || isUpperLetter(name.front()));
}

bool isNonTerminal(const std::string &name) {
    return !name.empty() && isLowerLetter(name.front());
}

} // namespace fuzzyrat
