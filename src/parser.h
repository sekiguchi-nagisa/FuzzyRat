//
// Created by skgchxngsxyz-carbon on 16/09/15.
//

#ifndef FUZZYRAT_PARSER_H
#define FUZZYRAT_PARSER_H

#include "misc/parser_base.hpp"
#include "state.h"
#include "node.h"

namespace fuzzyrat {

using ParseError = ydsh::parser_base::ParseError<TokenKind>;

class Parser : ydsh::parser_base::ParserBase<TokenKind, Lexer> {
public:
    Parser() = default;
    ~Parser() = default;

    void operator()(GrammarState &state);

private:
    std::pair<Token, NodePtr> parse_production();
    std::pair<Token, NodePtr> parse_nonTerminalProduction();
    NodePtr parse_alternative();
    NodePtr parse_sequence();
    NodePtr parse_suffix();
    NodePtr parse_primary();

    std::pair<Token, NodePtr> parse_terminalProduction();
    NodePtr parse_regexAlt();
    NodePtr parse_regexSeq();
    NodePtr parse_regexSuffix();
    NodePtr parse_regexPrimary();
};

} // namespace fuzzyrat

#endif //FUZZYRAT_PARSER_H
