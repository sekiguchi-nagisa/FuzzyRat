//
// Created by skgchxngsxyz-carbon on 16/09/15.
//

#include "parser.h"

#include "misc/size.hpp"
#include "misc/fatal.h"

#define EACH_LA_primary(E) \
    E(POPEN) \
    E(IDENTIFIER) \
    E(DOT) \
    E(STRING) \
    E(CHARSET)

#define EACH_LA_prefix(E) \
    E(AND) \
    E(NOT) \
    EACH_LA_primary(E)


namespace fuzzyrat {

// ####################
// ##     Parser     ##
// ####################

ProductionMap Parser::operator()(Lexer &lexer) {
    this->lexer = &lexer;

    ProductionMap map;
    while(this->curKind != EOS) {
        auto pair = map.insert(this->parse_production());
        if(!pair.second) {
            fatal("already defined production\n");
        }
    }
    this->expect(EOS);
    return map;
}

std::pair<std::string, NodePtr> Parser::parse_production() {
    Token token = this->expect(IDENTIFIER);
    this->expect(DEF);
    auto node = this->parse_choice();
    this->expect(SEMI_COLON);

    return std::make_pair(this->lexer->toTokenText(token), std::move(node));
}

NodePtr Parser::parse_choice() {
    auto leftNode = this->parse_sequence();
    if(this->curKind == ALT) {
        auto rightNode = this->parse_choice();
        leftNode = unique<ChoiceNode>(std::move(leftNode), std::move(rightNode));
    }
    return leftNode;
}

NodePtr Parser::parse_sequence() {
    auto leftNode = this->parse_prefix();
    switch(this->curKind) {
#define GEN_CASE(E) case E:
    EACH_LA_prefix(GEN_CASE) {
        auto rightNode = this->parse_sequence();
        leftNode = unique<SequenceNode>(std::move(leftNode), std::move(rightNode));
        break;
    }
#undef GEN_CASE
    default:
        break;
    }
    return leftNode;
}

NodePtr Parser::parse_prefix() {
    switch(this->curKind) {
    case AND: {
        Token token = this->expect(AND);
        return unique<AndPredicateNode>(token, this->parse_prefix());
    }
    case NOT: {
        Token token = this->expect(NOT);
        return unique<NotPredicateNode>(token, this->parse_prefix());
    }
    default:
        return this->parse_suffix();
    }
}

NodePtr Parser::parse_suffix() {
    auto node = this->parse_primary();
    for(bool next = true; next;) {
        switch(this->curKind) {
        case ZERO:
            node = unique<ZeroOrMoreNode>(std::move(node), this->expect(ZERO));
            break;
        case ONE:
            node = unique<OneOrMoreNode>(std::move(node), this->expect(ONE));
            break;
        case OPT:
            node = unique<OptionNode>(std::move(node), this->expect(OPT));
            break;
        default:
            next = false;
            break;
        }
    }
    return node;
}

NodePtr Parser::parse_primary() {
    switch(this->curKind) {
    case POPEN: {
        this->expect(POPEN);
        auto node = this->parse_choice();
        this->expect(PCLOSE);
        return node;
    }
    case IDENTIFIER: {
        Token token = this->expect(IDENTIFIER);
        return unique<NonTerminalNode>(token, this->lexer->toTokenText(token));
    }
    case DOT:
        return unique<AnyNode>(this->expect(DOT));
    case STRING: {
        Token token = this->expect(STRING);
        return unique<StringNode>(token, this->lexer->toTokenText(token));  //FIXME: unquote
    }
    case CHARSET: {
        Token token = this->expect(CHARSET);
        return unique<StringNode>(token, this->lexer->toTokenText(token));  //FIXME: unquote
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

} // namespace fuzzyrat