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

static MaybeError OK() {
    return nullptr;
}

template <typename ... Arg>
static MaybeError ERR(Arg && ...arg) {
    return MaybeError(new SemanticError(std::forward<Arg>(arg)...));
}

#define TRY(E) \
({ auto v = E; if(v) { return v; }})

class SymbolVerifier {
private:
    ProductionMap *map{nullptr};

public:
    SymbolVerifier() = default;
    ~SymbolVerifier() = default;

    MaybeError operator()(ProductionMap &map);

private:
    MaybeError dispatch(Node &node);

#define GEN_VISIT(E) MaybeError visit(E ## Node &node);
    EACH_NODE_KIND(GEN_VISIT)
#undef GEN_VISIT
};


// ############################
// ##     SymbolVerifier     ##
// ############################

MaybeError SymbolVerifier::operator()(ProductionMap &map) {
    this->map = &map;

    for(auto &e : map) {
        TRY(this->dispatch(*e.second));
    }
    return OK();
}

MaybeError SymbolVerifier::dispatch(Node &node) {
    switch(node.kind()) {
#define GEN_CASE(E) case NodeKind::E: return this->visit(static_cast<E ## Node &>(node));
        EACH_NODE_KIND(GEN_CASE)
#undef GEN_CASE
    default:
        return nullptr; // normally unreachable
    }
}


MaybeError SymbolVerifier::visit(EmptyNode &) { return OK(); }

MaybeError SymbolVerifier::visit(AnyNode &) { return OK(); }

MaybeError SymbolVerifier::visit(CharSetNode &) { return OK(); }

MaybeError SymbolVerifier::visit(AlternativeNode &node) {
    TRY(this->dispatch(*node.leftNode()));
    TRY(this->dispatch(*node.rightNode()));
    return OK();
}

MaybeError SymbolVerifier::visit(NonTerminalNode &node) {
    auto iter = this->map->find(node.name());
    if(iter == this->map->end()) {
        return ERR(SemanticError::UndefinedNonTerminal, node.token());
    }
    return OK();
}

MaybeError SymbolVerifier::visit(OneOrMoreNode &node) {
    return this->dispatch(*node.exprNode());
}

MaybeError SymbolVerifier::visit(OptionNode &node) {
    return this->dispatch(*node.exprNode());
}

MaybeError SymbolVerifier::visit(SequenceNode &node) {
    TRY(this->dispatch(*node.leftNode()));
    TRY(this->dispatch(*node.rightNode()));
    return OK();
}

MaybeError SymbolVerifier::visit(StringNode &) { return OK(); }

MaybeError SymbolVerifier::visit(ZeroOrMoreNode &node) {
    return this->dispatch(*node.exprNode());
}

MaybeError verify(GrammarState &state) {
    LOG_DEBUG("start verification");
    return SymbolVerifier()(state.map());
}

class NodeTranslator : protected NodeVisitor {
private:
    NodePtr retNode;

public:
    NodeTranslator() = default;

    ~NodeTranslator() override = default;

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
    ProductionMap *map{nullptr};

    unsigned int repeatIndex{0};

public:
    NodeSimplifier() = default;
    ~NodeSimplifier() override = default;

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
 *       A' =  (A A' | )
 *
 */
void NodeSimplifier::visit(ZeroOrMoreNode &node) {
    auto name = this->genRepeatName();

    auto alt = shared<AlternativeNode>(
            shared<SequenceNode>(this->translate(node.exprNode()),
                                 shared<NonTerminalNode>(node.token(), std::string(name))),
            shared<EmptyNode>());

    this->map->insert(std::make_pair(name, std::move(alt)));

    this->setRetNode(shared<NonTerminalNode>(node.token(), std::move(name)));
}

/**
 * A+ => (A A*)
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

void desugar(ProductionMap &map) {
    NodeSimplifier()(map);
}


class SpaceInserter : public NodeTranslator {
private:
    NodePtr space_;

public:
    SpaceInserter() = default;
    ~SpaceInserter() override = default;

    void operator()(GrammarState &state, NodePtr &&pattern);

private:
#define GEN_VISIT(E) void visit(E ## Node &node) override;
    EACH_NODE_KIND(GEN_VISIT)
#undef GEN_VISIT

    NodePtr space() const {
        return this->space_;
    }
};

// ###########################
// ##     SpaceInserter     ##
// ###########################

void SpaceInserter::visit(AnyNode &node) {
    this->setRetNode(node);
}

void SpaceInserter::visit(EmptyNode &node) {
    this->setRetNode(node);
}

void SpaceInserter::visit(CharSetNode &node) {
    this->setRetNode(node);
}

void SpaceInserter::visit(StringNode &node) {
    this->setRetNode(node);
}

void SpaceInserter::visit(OptionNode &node) {
    this->setRetNode(node);
}

/**
 * A*
 * => (<space> A)*
 *
 */
void SpaceInserter::visit(ZeroOrMoreNode &node) {
    auto seq = shared<SequenceNode>(this->space(), this->translate(node.exprNode()));
    node.exprNode() = std::move(seq);
    this->setRetNode(node);
}

/**
 * A+
 * => (<space> A)+
 *
 */
void SpaceInserter::visit(OneOrMoreNode &node) {
    auto seq = shared<SequenceNode>(this->space(), this->translate(node.exprNode()));
    node.exprNode() = std::move(seq);
    this->setRetNode(node);
}

void SpaceInserter::visit(NonTerminalNode &node) {
    this->setRetNode(node);
}

/**
 * A B
 * => A <Space> B
 *
 */
void SpaceInserter::visit(SequenceNode &node) {
    this->replace(node.leftNode());
    auto seq = shared<SequenceNode>(this->space(), this->translate(node.rightNode()));
    node.rightNode() = std::move(seq);
    this->setRetNode(node);
}

void SpaceInserter::visit(AlternativeNode &node) {
    this->replace(node.leftNode());
    this->replace(node.rightNode());
    this->setRetNode(node);
}

void SpaceInserter::operator()(GrammarState &state, NodePtr &&pattern) {
    {
        Token token = {0, 0};
        state.map().insert(std::make_pair(spaceName, std::move(pattern)));
        this->space_ = shared<NonTerminalNode>(token, spaceName);
    }

    for(auto &e : state.map()) {
        if(!isLexicalProduction(e.first)) {
            this->replace(e.second);
        }
    }

    /**
     * update start production
     *
     * A = B
     * => A' = <space> A <space>
     *
     */
    if(!isLexicalProduction(state.startSymbol())) {
        std::string old = state.startSymbol();
        std::string name = old;
        name += "'";
        state.setStartSymbol(name);
        Token token = {0, 0};
        auto start = shared<SequenceNode>(
                this->space(),
                shared<SequenceNode>(shared<NonTerminalNode>(token, std::move(old)), this->space()));
        state.map().insert(std::make_pair(std::move(name), std::move(start)));
    }
}

void insertSpace(GrammarState &state, NodePtr &&pattern) {
    SpaceInserter()(state, std::move(pattern));
}

} // namespace fuzzyrat
