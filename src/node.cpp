//
// Created by skgchxngsxyz-carbon on 16/09/14.
//

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
