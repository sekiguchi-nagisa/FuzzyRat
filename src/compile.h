//
// Created by skgchxngsxyz-carbon on 16/09/22.
//

#ifndef FUZZYRAT_COMPILE_H
#define FUZZYRAT_COMPILE_H

#include "misc/fatal.h"
#include "node.h"
#include "opcode.h"
#include "state.h"

namespace fuzzyrat {

class Compiler : public NodeVisitor {
private:
    OpCodePtr head;
    OpCodePtr tail;

    unsigned int idCount;
    std::unordered_map<std::string, unsigned int> name2IdMap;
    std::vector<std::pair<OpCodePtr, unsigned int>> codePairs;

public:
    Compiler() : head(nullptr), tail(nullptr), idCount(0), name2IdMap(), codePairs() {}
    ~Compiler() = default;

#define GEN_VISIT(E) void visit(E ## Node &node) override;
    EACH_NODE_KIND(GEN_VISIT)
#undef GEN_VISIT

    CompiledUnit operator()(const GrammarState &state);

private:
    void append(OpCodePtr &&code);

    OpCodePtr extract() {
        this->tail = nullptr;
        return std::move(this->head);
    }

    template <typename T, typename ... Arg>
    void generate(Arg && ... arg) {
        this->append(std::make_shared<T>(std::forward<Arg>(arg)...));
    }

    void generateAlternative(Node *leftNode, Node *rightNode);

    void generateRepeat(Node &exprNode);

    unsigned int generateProduction(const std::string &name, Node &node);

    void registerProductionName(const std::string &name);

    unsigned int getProductionId(const std::string &name);
};

} // namespace fuzzyrat

#endif //FUZZYRAT_COMPILE_H
