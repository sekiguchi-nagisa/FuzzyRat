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

#ifndef FUZZYRAT_NODE_H
#define FUZZYRAT_NODE_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

#include "misc/resource.hpp"
#include "lexer.h"

namespace fuzzyrat {

struct NodeVisitor;

#define EACH_NODE_KIND(E) \
    E(Empty) \
    E(Any) \
    E(String) \
    E(CharSet) \
    E(ZeroOrMore) \
    E(OneOrMore) \
    E(Option) \
    E(Sequence) \
    E(Alternative) \
    E(NonTerminal)


enum class NodeKind {
#define GEN_ENUM(E) E,
    EACH_NODE_KIND(GEN_ENUM)
#undef GEN_ENUM
};

class SourceInfo {
protected:
    Token token_;

    explicit SourceInfo(Token token) : token_(token) {}
    ~SourceInfo() = default;

public:
    Token token() const {
        return this->token_;
    }

    void updateToken(Token token) {
        if(this->token_.pos <= token.pos) {
            this->token_.size = token.pos + token.size - this->token_.pos;
        }
    }
};

class Node : public SourceInfo {
protected:
    const NodeKind kind_;

private:
    unsigned int refcount_;

public:
    Node(NodeKind kind, Token token) : SourceInfo(token), kind_(kind), refcount_(0) {}

    virtual ~Node() = default;

    NodeKind kind() const {
        return this->kind_;
    }

    bool is(NodeKind kind) const {
        return this->kind_ == kind;
    }

    void accept(NodeVisitor &visitor);

    friend void intrusivePtr_addRef(Node *ptr) {
        if(ptr != nullptr) {
            ptr->refcount_++;
        }
    }

    friend void intrusivePtr_release(Node *ptr) {
        if(ptr != nullptr && --ptr->refcount_ == 0) {
            delete ptr;
        }
    }
};

template <typename T>
using SharedPtr = ydsh::IntrusivePtr<T>;

using NodePtr = SharedPtr<Node>;

template <typename T, typename ...Arg>
NodePtr shared(Arg &&... arg) {
    return NodePtr(new T(std::forward<Arg>(arg)...));
}

class EmptyNode : public Node {
public:
    EmptyNode() : Node(NodeKind::Empty, {0, 0}) {}
    ~EmptyNode() = default;
};

class AnyNode : public Node {
public:
    explicit AnyNode(Token token) : Node(NodeKind::Any, token) {}
    ~AnyNode() = default;
};

class StringNode : public Node {
private:
    std::string value_;

public:
    StringNode(Token token, std::string &&value) : Node(NodeKind::String, token), value_(std::move(value)) {}
    ~StringNode() = default;

    const std::string &value() const {
        return this->value_;
    }
};

class CharSetNode : public Node {
private:
    std::string value_;

public:
    CharSetNode(Token token, std::string &&value) : Node(NodeKind::CharSet, token), value_(std::move(value)) {}
    ~CharSetNode() = default;

    const std::string &value() const {
        return this->value_;
    }
};

class ZeroOrMoreNode : public Node {
private:
    NodePtr exprNode_;

public:
    ZeroOrMoreNode(NodePtr &&exprNode, Token token) :
            Node(NodeKind::ZeroOrMore, exprNode->token()), exprNode_(std::move(exprNode)) {
        this->updateToken(token);
    }

    ~ZeroOrMoreNode() = default;

    NodePtr &exprNode() {
        return this->exprNode_;
    }
};

class OneOrMoreNode : public Node {
private:
    NodePtr exprNode_;

public:
    OneOrMoreNode(NodePtr &&exprNode, Token token) :
            Node(NodeKind::OneOrMore, exprNode->token()), exprNode_(std::move(exprNode)) {
        this->updateToken(token);
    }

    ~OneOrMoreNode() = default;

    NodePtr &exprNode() {
        return this->exprNode_;
    }
};

class OptionNode : public Node {
private:
    NodePtr exprNode_;

public:
    OptionNode(NodePtr &&exprNode, Token token) :
            Node(NodeKind::Option, exprNode->token()), exprNode_(std::move(exprNode)) {
        this->updateToken(token);
    }

    ~OptionNode() = default;

    NodePtr &exprNode() {
        return this->exprNode_;
    }
};

class SequenceNode : public Node {
private:
    NodePtr leftNode_;
    NodePtr rightNode_;

public:
    SequenceNode(NodePtr &&leftNode, NodePtr &&rightNode) :
            Node(NodeKind::Sequence, leftNode->token()),
            leftNode_(std::move(leftNode)), rightNode_(std::move(rightNode)) {
        this->updateToken(this->rightNode_->token());
    }

    ~SequenceNode() = default;

    NodePtr &leftNode() {
        return this->leftNode_;
    }

    NodePtr &rightNode() {
        return this->rightNode_;
    }
};

class AlternativeNode : public Node {
private:
    NodePtr leftNode_;
    NodePtr rightNode_;

public:
    AlternativeNode(NodePtr &&leftNode, NodePtr &&rightNode) :
            Node(NodeKind::Alternative, leftNode->token()),
            leftNode_(std::move(leftNode)), rightNode_(std::move(rightNode)) {
        this->updateToken(this->rightNode_->token());
    }

    ~AlternativeNode() = default;

    NodePtr &leftNode() {
        return this->leftNode_;
    }

    NodePtr &rightNode() {
        return this->rightNode_;
    }
};

class NonTerminalNode : public Node {
private:
    std::string name_;

public:
    NonTerminalNode(Token token, std::string &&name) :
            Node(NodeKind::NonTerminal, token), name_(std::move(name)) {}

    ~NonTerminalNode() = default;

    const std::string &name() const {
        return this->name_;
    }
};

bool isLexicalProduction(const std::string &name);

using ProductionMap = std::unordered_map<std::string, NodePtr>;

struct NodeVisitor {
    virtual ~NodeVisitor() = default;

#define GEN_VISIT(E) virtual void visit(E ## Node &) = 0;
    EACH_NODE_KIND(GEN_VISIT)
#undef GEN_VISIT
};

class NodePrinter : protected NodeVisitor {
private:
    std::ostream &stream;

public:
    NodePrinter(std::ostream &stream) : stream(stream) {}
    ~NodePrinter() = default;

private:
#define GEN_VISIT(E) void visit(E ## Node &node) override;
    EACH_NODE_KIND(GEN_VISIT)
#undef GEN_VISIT

    void printGroup(Node &node, bool exceptSeq = false);

public:
    void operator()(const std::string productionName, const NodePtr &node);

    void operator()(const ProductionMap &map);

};


} // namespace fuzzyrat

#endif //FUZZYRAT_NODE_H
