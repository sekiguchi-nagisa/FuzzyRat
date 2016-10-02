//
// Created by skgchxngsxyz-carbon on 16/09/18.
//

#ifndef FUZZYRAT_VERIFY_H
#define FUZZYRAT_VERIFY_H

#include "error.h"

namespace fuzzyrat {

class GrammarState;

void verify(GrammarState &state) throw(SemanticError);

void desugar(GrammarState &state);

} // namespace fuzzyrat

#endif //FUZZYRAT_VERIFY_H
