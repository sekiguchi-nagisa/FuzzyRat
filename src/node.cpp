//
// Created by skgchxngsxyz-carbon on 16/09/14.
//

#include "node.h"

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


} // namespace fuzzyrat
