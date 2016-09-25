//
// Created by skgchxngsxyz-carbon on 16/09/18.
//

#include "verify.h"
#include "state.h"
#include "node.h"
#include "logger.h"

namespace fuzzyrat {

struct NodeVerifier : protected NodeVisitor {
    virtual ~NodeVerifier() = default;
    virtual  void verify(ProductionMap &map) throw(SemanticError) = 0;
};

class SymbolVerifier : public NodeVerifier {
private:
    ProductionMap *map;

public:
    SymbolVerifier() : map(nullptr) {}
    ~SymbolVerifier() = default;

    void verify(ProductionMap &map) throw(SemanticError) override;

private:
#define GEN_VISIT(E) void visit(E ## Node &node) override;
    EACH_NODE_KIND(GEN_VISIT)
#undef GEN_VISIT
};


// ############################
// ##     SymbolVerifier     ##
// ############################

void SymbolVerifier::verify(ProductionMap &map) throw(SemanticError) {
    this->map = &map;

    for(auto &e : map) {
        e.second->accept(*this);
    }
}

void SymbolVerifier::visit(AnyNode &) {}

void SymbolVerifier::visit(CharSetNode &) {}

void SymbolVerifier::visit(AlternativeNode &node) {
    node.leftNode()->accept(*this);
    node.rightNode()->accept(*this);
}

void SymbolVerifier::visit(NonTerminalNode &node) {
    auto iter = this->map->find(node.name());
    if(iter == this->map->end()) {
        throw SemanticError(SemanticError::UndefinedNonTerminal, node.token());
    }
}

void SymbolVerifier::visit(OneOrMoreNode &node) {
    node.exprNode()->accept(*this);
}

void SymbolVerifier::visit(OptionNode &node) {
    node.exprNode()->accept(*this);
}

void SymbolVerifier::visit(TerminalNode &node) {
    auto iter = this->map->find(node.name());
    if(iter == this->map->end()) {
        throw SemanticError(SemanticError::UndefinedTerminal, node.token());
    }
}

void SymbolVerifier::visit(SequenceNode &node) {
    node.leftNode()->accept(*this);
    node.rightNode()->accept(*this);
}

void SymbolVerifier::visit(StringNode &) {}

void SymbolVerifier::visit(ZeroOrMoreNode &node) {
    node.exprNode()->accept(*this);
}



void verify(GrammarState &state) throw(SemanticError) {
    LOG_DEBUG("start verification");
    SymbolVerifier().verify(state.map());
}

} // namespace fuzzyrat
