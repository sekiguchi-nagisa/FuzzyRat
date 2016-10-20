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

bool isLexicalProduction(const std::string &name) {
    return !name.empty() && (name.front() == '_'  || isUpperLetter(name.front()));
}

// #########################
// ##     NodePrinter     ##
// #########################

void NodePrinter::printGroup(Node &node, bool exceptSeq) {
    bool paren = node.is(NodeKind::Alternative) || (!exceptSeq && node.is(NodeKind::Sequence));
    if(paren) {
        this->stream << "(";
    }

    node.accept(*this);

    if(paren) {
        this->stream << ")";
    }
}

void NodePrinter::visit(AnyNode &) {
    this->stream << ".";
}

void NodePrinter::visit(EmptyNode &) {}

void NodePrinter::visit(CharSetNode &node) {
    this->stream << node.value();
}

void NodePrinter::visit(StringNode &node) {
    this->stream << node.value();
}

void NodePrinter::visit(OptionNode &node) {
    this->printGroup(*node.exprNode());
    this->stream << "?";
}

void NodePrinter::visit(ZeroOrMoreNode &node) {
    this->printGroup(*node.exprNode());
    this->stream << "*";
}

void NodePrinter::visit(OneOrMoreNode &node) {
    this->printGroup(*node.exprNode());
    this->stream << "+";
}

void NodePrinter::visit(NonTerminalNode &node) {
    this->stream << node.name();
}

void NodePrinter::visit(SequenceNode &node) {
    this->printGroup(*node.leftNode(), true);
    this->stream << " ";
    this->printGroup(*node.rightNode(), true);
}

void NodePrinter::visit(AlternativeNode &node) {
    node.leftNode()->accept(*this);
    this->stream << " | ";
    node.rightNode()->accept(*this);
}

void NodePrinter::operator()(const ProductionMap &map) {
    for(auto &e : map) {
        this->stream << e.first << " = ";
        e.second->accept(*this);
        this->stream << ";" << std::endl << std::endl;
    }
}

} // namespace fuzzyrat
