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

#ifndef FUZZYRAT_PARSER_H
#define FUZZYRAT_PARSER_H

#include "misc/parser_base.hpp"
#include "state.h"
#include "node.h"

namespace fuzzyrat {

struct Production {
    Token token;
    NodePtr node;

    Production(Token token, NodePtr &&nodePtr) : token(token), node(std::move(nodePtr)) {}

    Production(std::nullptr_t) : token() {}
};

using ParseError = ydsh::parser_base::ParseError<TokenKind>;

class Parser : public ydsh::parser_base::AbstractParser<TokenKind, Lexer> {
public:
    explicit Parser(Lexer &lexer) {
        this->lexer = &lexer;
        this->fetchNext();
    }

    ~Parser() = default;

    explicit operator bool() const {
        return this->curKind != EOS;
    }

    Production operator()();

    static NodePtr parsePattern(const std::string &pattern);

private:
    Production parse_production();
    Production parse_nonTerminalProduction();
    NodePtr parse_alternative();
    NodePtr parse_sequence();
    NodePtr parse_suffix();
    NodePtr parse_primary();

    Production parse_terminalProduction();
    NodePtr parse_regexAlt();
    NodePtr parse_regexSeq();
    NodePtr parse_regexSuffix();
    NodePtr parse_regexPrimary();
};

} // namespace fuzzyrat

#endif //FUZZYRAT_PARSER_H
