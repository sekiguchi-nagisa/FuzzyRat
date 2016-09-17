//
// Created by skgchxngsxyz-carbon on 16/09/17.
//

#ifndef FUZZYRAT_ERROR_H
#define FUZZYRAT_ERROR_H

#include "lexer.h"

namespace fuzzyrat {

#define EACH_SEMANTIC_ERROR(E) \
    E(DefinedProduction, "already defined production") \
    E(UndefinedNonTerminal, "undefined non-terminal")

class SemanticError {
public:
    enum ErrorKind : unsigned int {
#define GEN_ENUM(E, M) E,
        EACH_SEMANTIC_ERROR(GEN_ENUM)
#undef GEN_ENUM
    };

private:
    ErrorKind kind_;
    Token token_;

public:
    SemanticError(ErrorKind kind, Token token) : kind_(kind), token_(token) {}
    ~SemanticError() = default;

    ErrorKind kind() const {
        return this->kind_;
    }

    Token token() const {
        return this->token_;
    }
};

const char *toString(SemanticError::ErrorKind kind);

std::string formatSourceName(Lexer &lexer, Token token);


} // namespace fuzzyrat

#endif //FUZZYRAT_ERROR_H
