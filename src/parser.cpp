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

#include "parser.h"

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

#define TRY(E) \
({auto v = E; if(this->hasError()) { return nullptr; } std::forward<decltype(v)>(v); })

Production Parser::operator()() {
    return this->parse_production();
}

NodePtr Parser::parsePattern(const std::string &pattern) {
    Lexer lexer(nullptr, pattern.c_str(), pattern.size());
    Parser parser(lexer);
    return parser.parse_regexAlt();
}

Production Parser::parse_production() {
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
        this->raiseNoViableAlterError(kinds);
        return nullptr;
    }
}

Production Parser::parse_nonTerminalProduction() {
    Token token = TRY(this->expect(NTERM));
    TRY(this->expect(DEF));
    auto node = TRY(this->parse_alternative());
    TRY(this->expect(SEMI_COLON));

    return {token, std::move(node)};
}

NodePtr Parser::parse_alternative() {
    auto leftNode = TRY(this->parse_sequence());
    if(this->curKind == ALT) {
        this->expect(ALT); // always success
        auto rightNode = TRY(this->parse_alternative());
        leftNode = shared<AlternativeNode>(std::move(leftNode), std::move(rightNode));
    }
    return leftNode;
}

NodePtr Parser::parse_sequence() {
    auto leftNode = TRY(this->parse_suffix());
    switch(this->curKind) {
#define GEN_CASE(E) case E:
    EACH_LA_primary(GEN_CASE) {
        auto rightNode = TRY(this->parse_sequence());
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
    auto node = TRY(this->parse_primary());
    for(bool next = true; next;) {
        switch(this->curKind) {
        case ZERO:
            node = shared<ZeroOrMoreNode>(std::move(node), this->expect(ZERO)); // always success
            break;
        case ONE:
            node = shared<OneOrMoreNode>(std::move(node), this->expect(ONE));   // always success
            break;
        case OPT:
            node = shared<OptionNode>(std::move(node), this->expect(OPT));  // always success
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
        this->expect(POPEN);    // always success
        auto node = TRY(this->parse_alternative());
        TRY(this->expect(PCLOSE));
        return node;
    }
    case TERM: {
    case NTERM:
        Token token = this->curToken;
        this->consume();
        return shared<NonTerminalNode>(token, this->lexer->toTokenText(token));
    }
    case STRING: {
        Token token = this->expect(STRING); // always success
        return shared<StringNode>(token, this->lexer->toTokenText(token));
    }
    default:
        TokenKind kinds[] = {
#define GEN_ALTER(E) E,
                EACH_LA_primary(GEN_ALTER)
#undef GEN_ALTER
        };
        this->raiseNoViableAlterError(kinds);
        return nullptr;
    }
}

Production Parser::parse_terminalProduction() {
    Token token = TRY(this->expect(TERM));
    TRY(this->expect(DEF));
    auto node = TRY(this->parse_regexAlt());
    TRY(this->expect(SEMI_COLON));

    return {token, std::move(node)};
}

NodePtr Parser::parse_regexAlt() {
    auto leftNode = TRY(this->parse_regexSeq());
    if(this->curKind == ALT) {
        this->expect(ALT);  // always success
        auto rightNode = TRY(this->parse_regexAlt());
        leftNode = shared<AlternativeNode>(std::move(leftNode), std::move(rightNode));
    }
    return leftNode;
}

NodePtr Parser::parse_regexSeq() {
    auto leftNode = TRY(this->parse_regexSuffix());
    switch(this->curKind) {
#define GEN_CASE(E) case E:
    EACH_LA_regexPrimary(GEN_CASE) {
        auto rightNode = TRY(this->parse_regexSeq());
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
    auto node = TRY(this->parse_regexPrimary());
    for(bool next = true; next;) {
        switch(this->curKind) {
        case ZERO:
            node = shared<ZeroOrMoreNode>(std::move(node), this->expect(ZERO)); // always success
            break;
        case ONE:
            node = shared<OneOrMoreNode>(std::move(node), this->expect(ONE));   // always success
            break;
        case OPT:
            node = shared<OptionNode>(std::move(node), this->expect(OPT));  // always success
            break;
        default:
            next = false;
            break;
        }
    }
    return node;
}

NodePtr Parser::parse_regexPrimary() {
    switch(this->curKind) {
    case POPEN: {
        this->expect(POPEN);    // always success
        auto node = TRY(this->parse_regexAlt());
        TRY(this->expect(PCLOSE));
        return node;
    }
    case TERM: {
        Token token = this->expect(TERM);   // always success
        return shared<NonTerminalNode>(token, this->lexer->toTokenText(token));
    }
    case DOT:
        return shared<AnyNode>(this->expect(DOT));  // always success
    case CHARSET: {
        Token token = this->expect(CHARSET);    // always success
        return shared<CharSetNode>(token, this->lexer->toTokenText(token));
    }
    case STRING: {
        Token token = this->expect(STRING); // always success
        return shared<StringNode>(token, this->lexer->toTokenText(token));
    }
    default:
        TokenKind kinds[] = {
#define GEN_ALTER(E) E,
                EACH_LA_regexPrimary(GEN_ALTER)
#undef GEN_ALTER
        };
        this->raiseNoViableAlterError(kinds);
        return nullptr;
    }
}

} // namespace fuzzyrat