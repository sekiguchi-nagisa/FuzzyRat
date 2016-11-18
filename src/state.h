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

#ifndef FUZZYRAT_STATE_H
#define FUZZYRAT_STATE_H

#include <unordered_map>
#include <string>
#include <memory>

#include "lexer.h"
#include "misc/resource.hpp"

namespace fuzzyrat {

class Node;

using ProductionMap = std::unordered_map<std::string, ydsh::IntrusivePtr<Node>>;

class GrammarState {
private:
    /**
     * indicate start production
     */
    std::string startSymbol_;

    /**
     * maintain production name to expression mapping.
     */
    ProductionMap map_;

public:
    NON_COPYABLE(GrammarState);

    GrammarState() = default;
    ~GrammarState() = default;

    const std::string &startSymbol() const {
        return this->startSymbol_;
    }

    void setStartSymbol(const std::string &str) {
        this->startSymbol_ = str;
    }

    ProductionMap &map() {
        return this->map_;
    }

    const ProductionMap &map() const {
        return this->map_;
    }
};

} // namespace fuzzyrat

#endif //FUZZYRAT_STATE_H
