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

#include <lexer.h>
#include <logger.h>

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

      HEX        = '\\' [x] [0-9a-fA-F]{1,2};
      UNICODE    = '\\' [uU] [0-9a-fA-F]{4};
      TERM       = [_A-Z] [_A-Z0-9]*;
      NTERM      = [a-z] [_a-zA-Z0-9]*;
      SCHAR      = [\x20-\x26\x28-\x5B\x5D-\x7E] | '\\' [trn'\\] | HEX;
      DCHAR      = [\x20-\x21\x23-\x5B\x5D-\x7E] | '\\' [trn"\\] | HEX;
      SETCHAR    = [\x20-\x5B\x5E-\x7E] | '\\' [trn\\\]^] | HEX;
      COMMENT    = "//" [\x20-\x7E]*;
     */

    INIT:
    unsigned int startPos = this->getPos();
    TokenKind kind = INVALID;
    /*!re2c
      TERM              { RET(TERM); }
      NTERM             { RET(NTERM); }
      [=:]              { RET(DEF); }
      ";"               { RET(SEMI_COLON); }
      "."               { RET(DOT); }
      ['] SCHAR+ [']    { RET(STRING); }
      ["] DCHAR+ ["]    { RET(STRING); }
      '[' SETCHAR+ ']'  { RET(CHARSET); }
      "*"               { RET(ZERO); }
      "+"               { RET(ONE); }
      "?"               { RET(OPT); }
      "("               { RET(POPEN); }
      ")"               { RET(PCLOSE); }
      "|"               { RET(ALT); }

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
    LOG_DEBUG("nextToken(): " << toString(kind) << ", " << token << ", text = " << this->toTokenText(token));
    return kind;
}

} // namespace fuzzyrat