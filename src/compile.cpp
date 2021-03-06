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

#include <array>

#include "misc/fatal.h"
#include "misc/num.h"
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

static int parseHex(std::string::const_iterator &iter, std::string::const_iterator end) {
    assert(iter != end);
    assert(ydsh::isHex(*iter));
    int code = ydsh::toHex(*(iter++));
    if(iter != end && ydsh::isHex(*iter)) {
        code *= 16;
        code += ydsh::toHex(*(iter++));
    }
    return code;
}

static int parseCharSet(std::string::const_iterator &iter, std::string::const_iterator end) {
    int code = -1;
    if(iter != end) {
        code = *(iter++);
        if(code == '\\' && iter != end) {
            char next = *(iter++);
            switch(next) {
            case 't': code = '\t'; break;
            case 'r': code = '\r'; break;
            case 'n': code = '\n'; break;
            case '\\':
            case ']':
            case '^':
                code = next;
                break;
            case 'x':
                code = parseHex(iter, end);
                break;
            }
        }
    }
    return code;
}

static void setRange(char start, char stop, AsciiMap &map) {
    char begin = start < stop ? start : stop;
    char end = start < stop ? stop : start;
    for(; begin <= end; begin++) {
        map |= begin;
    }
}

void Compiler::visit(CharSetNode &node) {   //FIXME: unicode
    AsciiMap map;

    auto iter = node.value().cbegin() + 1;
    const auto end = node.value().cend() - 1;
    bool negate = *iter == '^';
    if(negate) {
       iter++;
    }
    while(iter != end) {
        int code = parseCharSet(iter, end);
        if(iter != end && *iter == '-' && iter + 1 != end) {
            iter++;
            int next = parseCharSet(iter, end);
            setRange(code, next, map);
        } else {
            map |= code;
        }
    }

    if(negate) {
        map = ~map;
    }
    this->generate<CharSetOp>(std::move(map));
}

static int parseStr(std::string::const_iterator &iter, std::string::const_iterator end) {
    int code = -1;
    if(iter != end) {
        code = *(iter++);
        if(code == '\\' && iter != end) {
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
                code = parseHex(iter, end);
                break;
            }
        }
    }
    return code;
}

void Compiler::visit(StringNode &node) {    //FIXME: unicode
    auto iter = node.value().cbegin() + 1;
    auto end = node.value().cend() - 1;

    for(int code; (code = parseStr(iter, end)) != -1;) {
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

static void fillFlattenView(std::vector<Node *> &values, bool &hasEmpty, std::array<Node *, 2> &&nodes) {
    for(auto *node : nodes) {
        if (node->is(NodeKind::Alternative)) {
            auto *altNode = static_cast<AlternativeNode *>(node);
            fillFlattenView(values, hasEmpty, {{altNode->leftNode().get(), altNode->rightNode().get()}});
        } else if(node->is(NodeKind::Empty)) {
            if(!hasEmpty) {
                hasEmpty = true;
                values.push_back(node);
            }
        } else {
            values.push_back(node);
        }
    }
}

static std::vector<Node *> getFlattenView(Node &leftNode, Node &rightNode) {
    std::vector<Node *> values;
    bool hasEmpty = false;
    fillFlattenView(values, hasEmpty, {{&leftNode, &rightNode}});
    return values;
}

void Compiler::visit(AlternativeNode &node) {
    // backup current head/tail
    auto oldTail = this->tail;
    auto oldHead = this->extract();

    auto empty = std::make_shared<EmptyOp>();
    auto view = getFlattenView(*node.leftNode(), *node.rightNode());
    const unsigned int size = view.size();

    std::vector<OpCodePtr> values(view.size());
    for(unsigned int i = 0; i < size; i++) {
        view[i]->accept(*this);
        this->tail->setNext(empty);
        values[i] = this->extract();
    }

    // restore head/tail
    this->head = std::move(oldHead);
    this->tail = std::move(oldTail);

    this->generate<AltOp>(std::move(values));
    this->append(std::move(empty));
}

void Compiler::visit(NonTerminalNode &node) {
    this->generate<CallOp>(this->getProductionId(node.name()));
}

CompiledUnit Compiler::operator()(const GrammarState &state) {

    // register production name to name map
    for(auto &e : state.map()) {
        auto pair = this->name2IdMap.insert(std::make_pair(e.first, this->idCount++));
        assert(pair.second);
        (void) pair;
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

unsigned int Compiler::getProductionId(const std::string &name) {
    auto iter = this->name2IdMap.find(name);
    assert(iter != this->name2IdMap.end());
    return iter->second;
}

} // namespace fuzzyrat