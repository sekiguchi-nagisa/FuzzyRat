//
// Created by skgchxngsxyz-carbon on 16/09/15.
//

#include "parser.h"
#include "error.h"
#include "logger.h"

#define EACH_LA_production(E) \
    E(TERM) \
    E(NTERM)

#define EACH_LA_primary(E) \
    E(POPEN) \
    E(TERM) \
    E(NTERM) \
    E(STRING)

#define EACH_LA_regexPrimary(E) \
    E(POPEN) \
    E(TERM) \
    E(DOT) \
    E(CHARSET) \
    E(STRING)


namespace fuzzyrat {

// ####################
// ##     Parser     ##
// ####################

void Parser::operator()(GrammarState &state) {
    this->lexer = &state.lexer();
    this->fetchNext();

    while(this->curKind != EOS) {
        auto pair = this->parse_production();
        std::string name = this->lexer->toTokenText(pair.first);

        if(state.startSymbol().empty()) {
            state.setStartSymbol(name);
        }

        if(!state.map().insert(std::make_pair(std::move(name), std::move(pair.second))).second) {
            throw SemanticError(SemanticError::DefinedProduction, pair.first);
        }
    }
    this->expect(EOS);
}

std::pair<Token, NodePtr> Parser::parse_production() {
    switch(this->curKind) {
    case TERM:
        return this->parse_terminalProduction();
    case NTERM:
        return this->parse_nonTerminalProduction();
    default:
        TokenKind kinds[] = {
#define GEN_ALTER(E) E,
                EACH_LA_production(GEN_ALTER)
#undef GEN_ALTER
        };
        this->alternativeError(ydsh::arraySize(kinds), kinds);
    }

}

std::pair<Token, NodePtr> Parser::parse_nonTerminalProduction() {
    FuncTracer<LogLevel::debug> tracer(__func__);

    Token token = this->expect(NTERM);
    this->expect(DEF);
    auto node = this->parse_alternative();
    this->expect(SEMI_COLON);

    return std::make_pair(token, std::move(node));
}

NodePtr Parser::parse_alternative() {
    FuncTracer<LogLevel::debug> tracer(__func__);

    auto leftNode = this->parse_sequence();
    if(this->curKind == ALT) {
        this->expect(ALT);
        auto rightNode = this->parse_alternative();
        leftNode = shared<AlternativeNode>(std::move(leftNode), std::move(rightNode));
    }
    return leftNode;
}

NodePtr Parser::parse_sequence() {
    FuncTracer<LogLevel::debug> tracer(__func__);

    auto leftNode = this->parse_suffix();
    switch(this->curKind) {
#define GEN_CASE(E) case E:
    EACH_LA_primary(GEN_CASE) {
        auto rightNode = this->parse_sequence();
        leftNode = shared<SequenceNode>(std::move(leftNode), std::move(rightNode));
        break;
    }
#undef GEN_CASE
    default:
        break;
    }
    return leftNode;
}

NodePtr Parser::parse_suffix() {
    FuncTracer<LogLevel::debug> tracer(__func__);

    auto node = this->parse_primary();
    for(bool next = true; next;) {
        switch(this->curKind) {
        case ZERO:
            node = shared<ZeroOrMoreNode>(std::move(node), this->expect(ZERO));
            break;
        case ONE:
            node = shared<OneOrMoreNode>(std::move(node), this->expect(ONE));
            break;
        case OPT:
            node = shared<OptionNode>(std::move(node), this->expect(OPT));
            break;
        default:
            next = false;
            break;
        }
    }
    return node;
}

NodePtr Parser::parse_primary() {
    FuncTracer<LogLevel::debug> tracer(__func__);

    switch(this->curKind) {
    case POPEN: {
        this->expect(POPEN);
        auto node = this->parse_alternative();
        this->expect(PCLOSE);
        return node;
    }
    case TERM: {
    case NTERM:
        Token token = this->curToken;
        this->consume();
        return shared<NonTerminalNode>(token, this->lexer->toTokenText(token));
    }
    case STRING: {
        Token token = this->expect(STRING);
        return shared<StringNode>(token, this->lexer->toTokenText(token));  //FIXME: unquote
    }
    default:
        TokenKind kinds[] = {
#define GEN_ALTER(E) E,
                EACH_LA_primary(GEN_ALTER)
#undef GEN_ALTER
        };
        this->alternativeError(ydsh::arraySize(kinds), kinds);
    }
}

std::pair<Token, NodePtr> Parser::parse_terminalProduction() {
    FuncTracer<LogLevel::debug> tracer(__func__);

    Token token = this->expect(TERM);
    this->expect(DEF);
    auto node = this->parse_regexAlt();
    this->expect(SEMI_COLON);

    return std::make_pair(token, std::move(node));
}

NodePtr Parser::parse_regexAlt() {
    FuncTracer<LogLevel::debug> tracer(__func__);

    auto leftNode = this->parse_regexSeq();
    if(this->curKind == ALT) {
        this->expect(ALT);
        auto rightNode = this->parse_regexAlt();
        leftNode = shared<AlternativeNode>(std::move(leftNode), std::move(rightNode));
    }
    return leftNode;
}

NodePtr Parser::parse_regexSeq() {
    FuncTracer<LogLevel::debug> tracer(__func__);

    auto leftNode = this->parse_regexSuffix();
    switch(this->curKind) {
#define GEN_CASE(E) case E:
    EACH_LA_regexPrimary(GEN_CASE) {
        auto rightNode = this->parse_regexSeq();
        leftNode = shared<SequenceNode>(std::move(leftNode), std::move(rightNode));
        break;
    }
#undef GEN_CASE
    default:
        break;
    }
    return leftNode;
}

NodePtr Parser::parse_regexSuffix() {
    FuncTracer<LogLevel::debug> tracer(__func__);

    auto node = this->parse_regexPrimary();
    for(bool next = true; next;) {
        switch(this->curKind) {
        case ZERO:
            node = shared<ZeroOrMoreNode>(std::move(node), this->expect(ZERO));
            break;
        case ONE:
            node = shared<OneOrMoreNode>(std::move(node), this->expect(ONE));
            break;
        case OPT:
            node = shared<OptionNode>(std::move(node), this->expect(OPT));
            break;
        default:
            next = false;
            break;
        }
    }
    return node;
}

NodePtr Parser::parse_regexPrimary() {
    FuncTracer<LogLevel::debug> tracer(__func__);

    switch(this->curKind) {
    case POPEN: {
        this->expect(POPEN);
        auto node = this->parse_alternative();
        this->expect(PCLOSE);
        return node;
    }
    case TERM: {
        Token token = this->expect(TERM);
        return shared<NonTerminalNode>(token, this->lexer->toTokenText(token));
    }
    case DOT:
        return shared<AnyNode>(this->expect(DOT));
    case CHARSET: {
        Token token = this->expect(CHARSET);
        return shared<CharSetNode>(token, this->lexer->toTokenText(token));
    }
    case STRING: {
        Token token = this->expect(STRING);
        return shared<StringNode>(token, this->lexer->toTokenText(token));
    }
    default:
        TokenKind kinds[] = {
#define GEN_ALTER(E) E,
                EACH_LA_regexPrimary(GEN_ALTER)
#undef GEN_ALTER
        };
        this->alternativeError(ydsh::arraySize(kinds), kinds);
    }
}

} // namespace fuzzyrat