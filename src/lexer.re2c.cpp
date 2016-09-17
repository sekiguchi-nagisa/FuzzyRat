//
// Created by skgchxngsxyz-carbon on 16/09/12.
//

#include <lexer.h>

namespace fuzzyrat {

const char *toString(TokenKind kind) {
    const char *v[] = {
#define GEN_STR(E) #E,
            EACH_TOKEN_KIND(GEN_STR)
#undef GEN_STR
    };
    return v[static_cast<unsigned int>(kind)];
}

// ###################
// ##     Lexer     ##
// ###################

#define RET(k) do { kind = k; goto END; } while(false)

#define REACH_EOS() do { this->endOfString = true; goto EOS; } while(false)

#define SKIP() goto INIT

#define ERROR() do { RET(INVALID); } while(false)


TokenKind Lexer::nextToken(Token &token) {
    /*!re2c
      re2c:define:YYCTYPE = "unsigned char";
      re2c:define:YYCURSOR = this->cursor;
      re2c:define:YYLIMIT = this->limit;
      re2c:define:YYMARKER = this->marker;
      re2c:define:YYCTXMARKER = this->ctxMarker;
      re2c:define:YYFILL:naked = 1;
      re2c:define:YYFILL@len = #;
      re2c:define:YYFILL = "if(!this->fill(#)) { REACH_EOS(); }";
      re2c:yyfill:enable = 1;
      re2c:indent:top = 1;
      re2c:indent:string = "    ";

      IDENTIFIER = [_a-zA-Z][_a-zA-Z0-9]*;
      SCHAR      = [^\t\r\n'\\\000] | '\\' [trn'\\] | '\\' [uU] [0-9a-fA-F]{4};
      DCHAR      = [^\t\r\n"\\\000] | '\\' [trn"\\] | '\\' [uU] [0-9a-fA-F]{4};
      SETCHAR    = [^\t\r\n\\\]\000] | '\\' [trn\\\]] | '\\-' | '\\' [uU] [0-9a-fA-F]{4};
      COMMENT    = "//" [^\r\n\000]*;
     */

    INIT:
    unsigned int startPos = this->getPos();
    TokenKind kind = INVALID;
    /*!re2c
      IDENTIFIER        { RET(IDENTIFIER); }
      [=:]              { RET(DEF); }
      ";"               { RET(SEMI_COLON); }
      "."               { RET(DOT); }
      ['] SCHAR+ [']    { RET(STRING); }
      ["] DCHAR+ ["]    { RET(STRING); }
      '[' SETCHAR+ ']'  { RET(CHARSET); }
      "&"               { RET(AND); }
      "!"               { RET(NOT); }
      "*"               { RET(ZERO); }
      "+"               { RET(ONE); }
      "?"               { RET(OPT); }
      "("               { RET(POPEN); }
      ")"               { RET(PCLOSE); }
      "/"               { RET(ALT); }

      COMMENT           { SKIP(); }
      [ \t\r\n]+        { SKIP(); }
      "\000"            { REACH_EOS(); }

      *                 { RET(INVALID); }
     */

    END:
    token.pos = startPos;
    token.size = this->getPos() - startPos;
    goto RET;

    EOS:
    kind = EOS;
    token.pos = this->limit - this->buf;
    token.size = 0;

    RET:
    return kind;
}

} // namespace fuzzyrat