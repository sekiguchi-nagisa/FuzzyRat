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

int unescapeStr(std::string::const_iterator &iter, std::string::const_iterator end) {
    int code = -1;
    if(iter != end) {
        code = *(iter++);
        if(code == '\\' && iter + 1 != end) {
            char next = *(iter++);
            switch(next) {
            case 't': code = '\t'; break;
            case 'r': code = '\r'; break;
            case 'n': code = '\n'; break;
            case '\\':
            case '"':
            case '\'':
                code = next;
                break;
            case 'x':
                assert(std::distance(end, iter) > 0);
                assert(ydsh::isHex(*iter));
                code = ydsh::toHex(*(iter++));
                if(iter != end && ydsh::isHex(*iter)) {
                    code *= 16;
                    code += ydsh::toHex(*(iter++));
                }
                break;
            }
        }
    }
    return code;
}

} // namespace fuzzyrat
