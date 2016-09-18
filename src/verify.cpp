//
// Created by skgchxngsxyz-carbon on 16/09/18.
//

#include "verify.h"
#include "state.h"
#include "node.h"

namespace fuzzyrat {

struct NodeVerifier : protected NodeVisitor {
    virtual ~NodeVerifier() = default;
    virtual  void verify(ProductionMap &map) throw(SemanticError) = 0;
};

class NonTerminalVerifier : public NodeVerifier {
private:
    ProductionMap *map;

public:
    NonTerminalVerifier() : map(nullptr) {}
    ~NonTerminalVerifier() = default;

    void verify(ProductionMap &map) throw(SemanticError) override;

private:
#define GEN_VISIT(E) void visit(E ## Node &node) override;
    EACH_NODE_KIND(GEN_VISIT)
#undef GEN_VISIT
};


// #################################
// ##     NonTerminalVerifier     ##
// #################################

void NonTerminalVerifier::verify(ProductionMap &map) throw(SemanticError) {
    this->map = &map;

    for(auto &e : map) {
        e.second->accept(*this);
    }
}

void NonTerminalVerifier::visit(AndPredicateNode &node) {
    node.exprNode()->accept(*this);
}

void NonTerminalVerifier::visit(AnyNode &) {}

void NonTerminalVerifier::visit(CharSetNode &) {}

void NonTerminalVerifier::visit(ChoiceNode &node) {
    node.leftNode()->accept(*this);
    node.rightNode()->accept(*this);
}

void NonTerminalVerifier::visit(NonTerminalNode &node) {
    auto iter = this->map->find(node.name());
    if(iter == this->map->end()) {
        throw SemanticError(SemanticError::UndefinedNonTerminal, node.token());
    }
}

void NonTerminalVerifier::visit(NotPredicateNode &node) {
    node.exprNode()->accept(*this);
}

void NonTerminalVerifier::visit(OneOrMoreNode &node) {
    node.exprNode()->accept(*this);
}

void NonTerminalVerifier::visit(OptionNode &node) {
    node.exprNode()->accept(*this);
}

void NonTerminalVerifier::visit(SequenceNode &node) {
    node.leftNode()->accept(*this);
    node.rightNode()->accept(*this);
}

void NonTerminalVerifier::visit(StringNode &) {}

void NonTerminalVerifier::visit(ZeroOrMoreNode &node) {
    node.exprNode()->accept(*this);
}



void verify(GrammarState &state) throw(SemanticError) {
    NonTerminalVerifier().verify(state.map());
}

} // namespace fuzzyrat
