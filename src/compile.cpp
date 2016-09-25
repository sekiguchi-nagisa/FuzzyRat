//
// Created by skgchxngsxyz-carbon on 16/09/22.
//

#include <array>

#include "misc/fatal.h"
#include "compile.h"

namespace fuzzyrat {

// ######################
// ##     Compiler     ##
// ######################

void Compiler::visit(AnyNode &) {   //FIXME: unicode
    this->generate<AnyOp>();
}

void Compiler::visit(CharSetNode &) {   //FIXME: unicode
    fatal("unimplemented\n");
}

void Compiler::visit(StringNode &node) {    //FIXME: unicode
    for(auto &e : node.value()) {
        this->generate<CharOp>(e);
    }
}

void Compiler::visit(ZeroOrMoreNode &node) {
    this->generateRepeat(*node.exprNode());
}

void Compiler::visit(OneOrMoreNode &node) {
    node.exprNode()->accept(*this);
    this->generateRepeat(*node.exprNode());
}


void Compiler::visit(OptionNode &node) {
    this->generateAlternative(nullptr, node.exprNode().get());
}

void Compiler::visit(SequenceNode &node) {
    node.leftNode()->accept(*this);
    node.rightNode()->accept(*this);
}

void Compiler::visit(AlternativeNode &node) {
    this->generateAlternative(node.leftNode().get(), node.rightNode().get());
}

void Compiler::visit(NonTerminalNode &node) {
    this->generate<CallOp>(this->getProductionId(node.name()));
}

void Compiler::visit(TerminalNode &node) {
    this->generate<CallOp>(this->getProductionId(node.name()));
}

CompiledUnit Compiler::operator()(const GrammarState &state) {
    CompiledUnit estate;
    this->estate = &estate;

    // register production name to name map
    for(auto &e : state.map()) {
        this->registerProductionName(e.first);
    }

    for(auto &e : state.map()) {
        this->generateProduction(e.first, *e.second);
    }

    unsigned int startId = this->getProductionId(state.startSymbol());
    estate.setStartId(startId);

    return estate;
}

void Compiler::append(OpCodePtr &&code) {
    if(this->head) {
        this->tail->setNext(std::move(code));
        this->tail = this->tail->next();
    } else {
        this->head = std::move(code);
        this->tail = this->head;
    }
}

static void fillFlattenView(std::vector<Node *> &values, bool &hasEmpty, std::array<Node *, 2> &&nodes) {
    for(auto *node : nodes) {
        if(node == nullptr) {
            hasEmpty = true;
        } else if (node->is(NodeKind::Alternative)){
            auto *altNode = static_cast<AlternativeNode *>(node);
            fillFlattenView(values, hasEmpty, {{altNode->leftNode().get(), altNode->rightNode().get()}});
        } else if(node->is(NodeKind::Option)) {
            fillFlattenView(values, hasEmpty, {{nullptr, static_cast<OptionNode *>(node)->exprNode().get()}});
        } else {
            values.push_back(node);
        }
    }
}

static std::vector<Node *> getFlattenView(Node *leftNode, Node *rightNode) {
    std::vector<Node *> values;
    bool hasEmpty = false;
    fillFlattenView(values, hasEmpty, {{leftNode, rightNode}});
    if(hasEmpty) {
        values.push_back(nullptr);
    }
    return values;
}

void Compiler::generateAlternative(Node *leftNode, Node *rightNode) {
    auto cur = this->extract();
    std::vector<OpCodePtr> values;

    auto empty = std::make_shared<EmptyOp>();
    for(auto &e : getFlattenView(leftNode, rightNode)) {
        if(e != nullptr) {
            e->accept(*this);
        } else {
            this->generate<EmptyOp>();
        }
        this->tail->setNext(empty);
        values.push_back(this->extract());
    }

    auto next = std::make_shared<AltOp>(std::move(values));
    if(cur) {
        cur->setNext(std::move(next));
    } else {
        cur = std::move(next);
    }
    this->head = std::move(cur);
    this->tail = std::move(empty);
}

void Compiler::generateRepeat(Node &) {
    fatal("unimplemented\n");
}

unsigned int Compiler::generateProduction(const std::string &name, Node &node) {
    const unsigned int id = this->getProductionId(name);
    node.accept(*this);
    this->generate<RetOp>();

    auto code = this->extract();
    assert(id == this->estate->codes().size());
    this->estate->codes().push_back(std::move(code));

    return id;
}

void Compiler::registerProductionName(const std::string &name) {
    auto pair = this->name2IdMap.insert(std::make_pair(name, this->idCount++));
    if(!pair.second) {
        fatal("already found production: %s\n", name.c_str());
    }
}

unsigned int Compiler::getProductionId(const std::string &name) {
    auto iter = this->name2IdMap.find(name);
    if(iter == this->name2IdMap.end()) {
        fatal("not found production: %s\n", name.c_str());
    }
    return iter->second;
}

} // namespace fuzzyrat