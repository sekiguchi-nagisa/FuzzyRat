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

#ifndef FUZZYRAT_ERROR_H
#define FUZZYRAT_ERROR_H

#include "lexer.h"

namespace fuzzyrat {

#define EACH_SEMANTIC_ERROR(E) \
    E(DefinedProduction,    "already defined production") \
    E(UndefinedNonTerminal, "undefined non-terminal") \
    E(UndefinedStart,       "undefined start production")

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
