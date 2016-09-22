//
// Created by skgchxngsxyz-carbon on 16/09/12.
//

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
