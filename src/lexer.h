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

#ifndef FUZZYRAT_LEXER_H
#define FUZZYRAT_LEXER_H

#include "misc/lexer_base.hpp"

#define EACH_TOKEN_KIND(TOK) \
    TOK(INVALID) \
    TOK(EOS) \
    TOK(TERM) \
    TOK(NTERM) \
    TOK(DEF) \
    TOK(SEMI_COLON) \
    TOK(DOT) \
    TOK(STRING) \
    TOK(CHARSET) \
    TOK(AND) \
    TOK(NOT) \
    TOK(ZERO) \
    TOK(ONE) \
    TOK(OPT) \
    TOK(POPEN) \
    TOK(PCLOSE) \
    TOK(ALT)

namespace fuzzyrat {

enum TokenKind : unsigned int {
#define GEN_ENUM(E) E,
    EACH_TOKEN_KIND(GEN_ENUM)
#undef GEN_ENUM
};

using Token = ydsh::parser_base::Token;

const char *toString(TokenKind kind);

class Lexer : public ydsh::parser_base::LexerBase {
private:
    const char *fileName_;

public:
    NON_COPYABLE(Lexer);

    Lexer(const char *fileName, FILE *fp) : LexerBase(fp), fileName_(fileName) {}
    ~Lexer() = default;

    const char *filename() const {
        return this->fileName_;
    }

    unsigned int getLineNum(Token token) const {
        assert(this->withinRange(token));

        unsigned int n = 1;
        for(unsigned int i = 0; i < token.pos; i++) {
            if(this->buf[i] == '\n') {
                n++;
            }
        }
        return n;
    }

    TokenKind nextToken(Token &token);

    static bool isInvalidToken(TokenKind kind) {
        return kind == TokenKind::INVALID;
    }
};


} // namespace

#endif //FUZZYRAT_LEXER_H
