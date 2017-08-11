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

#ifndef FUZZYRAT_VERIFY_H
#define FUZZYRAT_VERIFY_H

#include "error.h"
#include "state.h"

namespace fuzzyrat {

using MaybeError = std::unique_ptr<SemanticError>;

MaybeError verify(GrammarState &state);

void desugar(ProductionMap &map);

void insertSpace(GrammarState &state, NodePtr &&pattern);

} // namespace fuzzyrat

#endif //FUZZYRAT_VERIFY_H
