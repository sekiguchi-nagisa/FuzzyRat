//
// Created by skgchxngsxyz-carbon on 16/09/18.
//

#ifndef FUZZYRAT_VERIFY_H
#define FUZZYRAT_VERIFY_H

#include "node.h"
#include "error.h"

namespace fuzzyrat {

void verify(ProductionMap &map) throw(SemanticError);

} // namespace fuzzyrat

#endif //FUZZYRAT_VERIFY_H
