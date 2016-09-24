//
// Created by skgchxngsxyz-carbon on 16/09/14.
//

#ifndef FUZZYRAT_NODE_H
#define FUZZYRAT_NODE_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

#include "lexer.h"

namespace fuzzyrat {

struct NodeVisitor;

#define EACH_NODE_KIND(E) \
    E(Any) \
    E(String) \
    E(CharSet) \
    E(ZeroOrMore) \
    E(OneOrMore) \
    E(Option) \
    E(Sequence) \
    E(Alternative) \
    E(Terminal) \
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

    Node(NodeKind kind, Token token) : SourceInfo(token), kind_(kind) {}

public:
    virtual ~Node() = default;

    NodeKind kind() const {
        return this->kind_;
    }

    bool is(NodeKind kind) const {
        return this->kind_ == kind;
    }

    void accept(NodeVisitor &visitor);
};

using NodePtr = std::unique_ptr<Node>;

template <typename T, typename ...Arg>
std::unique_ptr<T> unique(Arg && ... arg) {
    return std::unique_ptr<T>(new T(std::forward<Arg>(arg)...));
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

class TerminalNode : public Node {
private:
    std::string name_;

public:
    TerminalNode(Token token, std::string &&name) :
            Node(NodeKind::Terminal, token), name_(std::move(name)) {}

    ~TerminalNode() = default;

    const std::string &name() const {
        return this->name_;
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

bool isTerminal(const std::string &);
bool isNonTerminal(const std::string &);

using ProductionMap = std::unordered_map<std::string, NodePtr>;

struct NodeVisitor {
    virtual ~NodeVisitor() = default;

#define GEN_VISIT(E) virtual void visit(E ## Node &) = 0;
    EACH_NODE_KIND(GEN_VISIT)
#undef GEN_VISIT
};


} // namespace fuzzyrat

#endif //FUZZYRAT_NODE_H
