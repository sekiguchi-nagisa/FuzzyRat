//
// Created by skgchxngsxyz-carbon on 16/09/19.
//

#ifndef FUZZYRAT_STATE_H
#define FUZZYRAT_STATE_H

#include <unordered_map>
#include <string>
#include <memory>

#include "lexer.h"

namespace fuzzyrat {

class Node;

using ProductionMap = std::unordered_map<std::string, std::unique_ptr<Node>>;

class GrammarState {
private:
    Lexer lexer_;

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

    GrammarState(const char *fileName, FILE *fp) : lexer_(fileName, fp), startSymbol_(), map_() {}
    ~GrammarState() = default;

    Lexer &lexer() {
        return this->lexer_;
    }

    const std::string &startSymbol() const {
        return this->startSymbol_;
    }

    void setStartSymbol(const std::string &str) {
        this->startSymbol_ = str;
    }

    ProductionMap &map() {
        return this->map_;
    }
};

} // namespace fuzzyrat

#endif //FUZZYRAT_STATE_H
