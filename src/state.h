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
class OpCode;

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

    const ProductionMap &map() const {
        return this->map_;
    }
};

class ExecState {
private:
    /**
     * indicate start production id
     */
    unsigned int statrtId_;

    std::vector<std::shared_ptr<OpCode>> codes_;

public:
    NON_COPYABLE(ExecState);

    ExecState() : statrtId_(0), codes_() {}
    ~ExecState() = default;

    unsigned int startId() const {
        return this->statrtId_;
    }

    void setStartId(unsigned int id) {
        this->statrtId_ = id;
    }

    std::vector<std::shared_ptr<OpCode>> &codes() {
        return this->codes_;
    }
};

} // namespace fuzzyrat

#endif //FUZZYRAT_STATE_H
