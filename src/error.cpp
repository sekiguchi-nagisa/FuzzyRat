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