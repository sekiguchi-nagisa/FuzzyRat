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

void Compiler::visit(EmptyNode &) {
    this->generate<EmptyOp>();
}

void Compiler::visit(AnyNode &) {   //FIXME: unicode
    this->generate<AnyOp>();
}

void Compiler::visit(CharSetNode &) {   //FIXME: unicode
    fatal("unimplemented\n");
}

void Compiler::visit(StringNode &node) {    //FIXME: unicode
    auto iter = node.value().begin() + 1;
    auto end = node.value().cend() - 1;

    for(int code; (code = unescapeStr(iter, end)) != -1;) {
        this->generate<CharOp>(code);
    }
}

void Compiler::visit(ZeroOrMoreNode &) {
    fatal("unsupported\n");
}

void Compiler::visit(OneOrMoreNode &) {
    fatal("unsupported\n");
}

void Compiler::visit(OptionNode &) {
    fatal("unsupported\n");
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

CompiledUnit Compiler::operator()(const GrammarState &state) {

    // register production name to name map
    for(auto &e : state.map()) {
        auto pair = this->name2IdMap.insert(std::make_pair(e.first, this->idCount++));
        if(!pair.second) {
            fatal("already found production: %s\n", e.first.c_str());
        }
    }

    // generate code
    std::vector<OpCodePtr> codes(state.map().size());
    for(auto &e : state.map()) {
        const unsigned int id = this->getProductionId(e.first);
        e.second->accept(*this);
        this->generate<RetOp>();
        codes[id] = this->extract();
    }

    // generate compiled unit
    unsigned int startId = this->getProductionId(state.startSymbol());
    return CompiledUnit(startId, std::move(codes));
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

unsigned int Compiler::getProductionId(const std::string &name) {
    auto iter = this->name2IdMap.find(name);
    if(iter == this->name2IdMap.end()) {
        fatal("not found production: %s\n", name.c_str());
    }
    return iter->second;
}

} // namespace fuzzyrat