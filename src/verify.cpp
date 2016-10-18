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

#include "verify.h"
#include "state.h"
#include "node.h"
#include "logger.h"

namespace fuzzyrat {

class SymbolVerifier : protected NodeVisitor {
private:
    ProductionMap *map;

public:
    SymbolVerifier() : map(nullptr) {}
    ~SymbolVerifier() = default;

    void verify(ProductionMap &map) throw(SemanticError);

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

void SymbolVerifier::visit(EmptyNode &) {}

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

class NodeTranslator : protected NodeVisitor {
private:
    NodePtr retNode;

public:
    NodeTranslator() = default;
    virtual ~NodeTranslator() = default;

protected:
    void setRetNode(Node &node) {
        this->retNode = NodePtr(&node);
    }

    void setRetNode(NodePtr &&node) {
        this->retNode = std::move(node);
    }

    NodePtr translate(NodePtr &node) {
        node->accept(*this);
        return std::move(this->retNode);
    }

    void replace(NodePtr &nodePtr) {
        nodePtr = this->translate(nodePtr);
    }
};


class NodeSimplifier : public NodeTranslator {
private:
    ProductionMap *map;

    unsigned int repeatIndex;

public:
    NodeSimplifier() : map(nullptr), repeatIndex(0) {}
    ~NodeSimplifier() = default;

    void operator()(ProductionMap &map);

private:
#define GEN_VISIT(E) void visit(E ## Node &node) override;
    EACH_NODE_KIND(GEN_VISIT)
#undef GEN_VISIT

    std::string genRepeatName();
};

// ############################
// ##     NodeSimplifier     ##
// ############################

void NodeSimplifier::visit(EmptyNode &node) {
    this->setRetNode(node);
}

void NodeSimplifier::visit(AnyNode &node) {
    this->setRetNode(node);
}

void NodeSimplifier::visit(CharSetNode &node) {
    this->setRetNode(node);
}

void NodeSimplifier::visit(StringNode &node) {
    this->setRetNode(node);
}

/**
 * A? => (A | )
 *
 */
void NodeSimplifier::visit(OptionNode &node) {
    auto exprNode = this->translate(node.exprNode());
    auto alt = shared<AlternativeNode>(std::move(exprNode), shared<EmptyNode>());
    this->setRetNode(std::move(alt));
}

/**
 * A* => A'
 *       A' =  | A A'
 *
 */
void NodeSimplifier::visit(ZeroOrMoreNode &node) {
    auto name = this->genRepeatName();

    auto alt = shared<AlternativeNode>(
            shared<EmptyNode>(),
            shared<SequenceNode>(this->translate(node.exprNode()),
                                 shared<NonTerminalNode>(node.token(), std::string(name))));

    this->map->insert(std::make_pair(name, std::move(alt)));

    this->setRetNode(shared<NonTerminalNode>(node.token(), std::move(name)));
}

/**
 * A+ => A A*
 *
 */
void NodeSimplifier::visit(OneOrMoreNode &node) {
    auto exprNode = this->translate(node.exprNode());
    auto zero = shared<ZeroOrMoreNode>(NodePtr(exprNode), node.token());
    auto seq = shared<SequenceNode>(std::move(exprNode), std::move(zero));

    this->replace(seq);
    this->setRetNode(std::move(seq));
}

void NodeSimplifier::visit(SequenceNode &node) {
    this->replace(node.leftNode());
    this->replace(node.rightNode());

    this->setRetNode(node);
}

void NodeSimplifier::visit(AlternativeNode &node) {
    this->replace(node.leftNode());
    this->replace(node.rightNode());

    this->setRetNode(node);
}

void NodeSimplifier::visit(NonTerminalNode &node) {
    this->setRetNode(node);
}

void NodeSimplifier::operator()(ProductionMap &map) {
    this->map = &map;

    std::vector<std::string> names(map.size());
    unsigned int index = 0;
    for(auto &e : map) {
        names[index++] = e.first;
    }

    // translate production
    for(auto &s : names) {
        auto iter = this->map->find(s);
        assert(iter != this->map->end());
        auto node = iter->second;
        this->replace(node);
        (*this->map)[s] = std::move(node);
    }
}

std::string NodeSimplifier::genRepeatName() {
    std::string name;
    name += std::to_string(this->repeatIndex++);
    name += "_repeat";
    return name;
}

void desugar(GrammarState &state) {
    NodeSimplifier()(state.map());
}

} // namespace fuzzyrat
