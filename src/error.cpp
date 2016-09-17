//
// Created by skgchxngsxyz-carbon on 16/09/17.
//

#include "error.h"

namespace fuzzyrat {

const char *toString(SemanticError::ErrorKind kind) {
    const char *msg[] = {
#define GEN_MSG(E, M) M,
            EACH_SEMANTIC_ERROR(GEN_MSG)
#undef GEN_MSG
    };
    return msg[static_cast<unsigned int>(kind)];
}

std::string formatSourceName(Lexer &lexer, Token token) {
    std::string str;
    str += '(';
    str += lexer.filename();
    str += "):";
    str += std::to_string(lexer.getLineNum(token));
    str += ':';
    return str;
}

} // namespace fuzzyrat