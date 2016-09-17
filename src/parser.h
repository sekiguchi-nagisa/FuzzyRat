//
// Created by skgchxngsxyz-carbon on 16/09/15.
//

#ifndef FUZZYRAT_PARSER_H
#define FUZZYRAT_PARSER_H

#include "misc/parser_base.hpp"
#include "lexer.h"
#include "node.h"

namespace fuzzyrat {

using ParseError = ydsh::parser_base::ParseError<TokenKind>;

class Parser : ydsh::parser_base::ParserBase<TokenKind, Lexer> {
public:
    Parser() = default;
    ~Parser() = default;

    ProductionMap operator()(Lexer &lexer);

private:
    std::pair<Token, NodePtr> parse_production();
    NodePtr parse_choice();
    NodePtr parse_sequence();
    NodePtr parse_prefix();
    NodePtr parse_suffix();
    NodePtr parse_primary();
};

} // namespace fuzzyrat

#endif //FUZZYRAT_PARSER_H
