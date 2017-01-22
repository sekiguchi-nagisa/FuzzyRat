/*
 * Copyright (C) 2016-2017 Nagisa Sekiguchi
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

#include <string>
#include <fstream>
#include <random>

#include <fuzzyrat.h>
#include "logger.h"
#include "parser.h"
#include "error.h"
#include "verify.h"
#include "compile.h"
#include "opcode.h"

using namespace fuzzyrat;

struct FuzzyRatInputContext {
    bool spaceInsertion;
    std::string sourceName;
    ydsh::ByteBuffer buffer;
    std::string startProduction;

    FuzzyRatInputContext(const char *sourceName, ydsh::ByteBuffer &&buffer) :
            spaceInsertion(true), sourceName(sourceName), buffer(std::move(buffer)), startProduction() {}
};

FuzzyRatInputContext *FuzzyRat_newContextFromFile(const char *sourceName) {
    std::ifstream stream(sourceName);
    if(!stream) {
        return nullptr;
    }

    ydsh::ByteBuffer buffer;
    for(std::string line; std::getline(stream, line);) {
        buffer.append(line.c_str(), line.size());
        buffer += '\n';
    }
    return new FuzzyRatInputContext(sourceName, std::move(buffer));
}

FuzzyRatInputContext *FuzzyRat_newContext(const char *sourceName, const char *data, unsigned int size) {
    if(data == nullptr || size == 0) {
        return nullptr;
    }

    if(sourceName == nullptr) {
        sourceName = "(null)";
    }

    ydsh::ByteBuffer buffer;
    buffer.append(data, size);
    return new FuzzyRatInputContext(sourceName, std::move(buffer));
}

void FuzzyRat_setSpaceInsertion(FuzzyRatInputContext *input, int flag) {
    input->spaceInsertion = flag != 0;
}

void FuzzyRat_deleteContext(FuzzyRatInputContext **input) {
    if(input != nullptr) {
        delete *input;
        *input = nullptr;
    }
}

void FuzzyRat_setStartProduction(FuzzyRatInputContext *input, const char *productionName) {
    if(input != nullptr && productionName != nullptr) {
        input->startProduction = productionName;
    }
}

class DefaultRandomFactory : public RandFactory {
private:
    std::default_random_engine engine;

    static std::default_random_engine init() {
        std::vector<int> v(32);
        std::random_device rdev;
        std::generate(v.begin(), v.end(), std::ref(rdev));
        std::seed_seq seed(v.begin(), v.end());
        return std::default_random_engine(seed);
    }

public:
    DefaultRandomFactory() : engine(init()) {}
    ~DefaultRandomFactory() = default;

    unsigned int generate(unsigned int start, unsigned int stop) override {
        return std::uniform_int_distribution<unsigned int>(start, stop)(this->engine);
    }
};

struct FuzzyRatCode {
    CompiledUnit unit;
    DefaultRandomFactory randomFactory;

    FuzzyRatCode(CompiledUnit &&unit) : unit(std::move(unit)), randomFactory() {}
};

static void defineSpace(GrammarState &state) {  //FIXME: support non-unit newline(carriage return)
    const char *name = "_";

    Token token = {0, 0};
    auto node = shared<ZeroOrMoreNode>(shared<CharSetNode>(token, "[ \\t\\n]"), token);

    state.map().insert(std::make_pair(name, std::move(node)));
}

static void parseAndVerify(GrammarState &state, Lexer &lexer) {
    try {
        Parser()(state, lexer);

        // check start production
        if(state.startSymbol().empty()) {
            LOG_ERROR("start production not found");
        }

        if(state.map().find(state.startSymbol()) != state.map().end()) {
            LOG_INFO("start production: " << state.startSymbol());
        } else {
            LOG_ERROR("undefined start production: " << state.startSymbol());
        }

        verify(state);
    } catch(const ParseError &e) {
        Token errorToken = lexer.shiftEOS(e.getErrorToken());
        Token lineToken = lexer.getLineToken(errorToken);
        LOG_ERROR(formatSourceName(lexer, errorToken)
                          << " " << e.getMessage() << std::endl
                          << lexer.toTokenText(lineToken) << std::endl
                          << lexer.formatLineMarker(lineToken, errorToken));
    } catch(const SemanticError &e) {
        Token lineToken = lexer.getLineToken(e.token());
        LOG_ERROR(formatSourceName(lexer, e.token())
                          << " " << toString(e.kind()) << std::endl
                          << lexer.toTokenText(lineToken) << std::endl
                          << lexer.formatLineMarker(lineToken, e.token()));
    }
}

FuzzyRatCode *FuzzyRat_compile(const FuzzyRatInputContext *input) {
    GrammarState state;
    state.setStartSymbol(input->startProduction);

    if(input->spaceInsertion) {
        defineSpace(state);
    }

    {
        Lexer lexer(input->sourceName.c_str(), input->buffer.get(), input->buffer.size());
        parseAndVerify(state, lexer);
    }

    if(input->spaceInsertion) {
        insertSpace(state);
    }
    log<LogLevel::debug>([&](std::ostream &stream) {
        stream << "before desugar" << std::endl;
        NodePrinter printer(stream); printer(state.map());
    });

    desugar(state);
    log<LogLevel::debug>([&](std::ostream &stream) {
        stream << "after desugar" << std::endl;
        NodePrinter printer(stream); printer(state.map());
    });

    return new FuzzyRatCode(Compiler()(state));
}

void FuzzyRat_deleteCode(FuzzyRatCode **code) {
    if(code != nullptr) {
        delete *code;
        *code = nullptr;
    }
}

int FuzzyRat_exec(const FuzzyRatCode *code, FuzzyRatResult *result) {
    if(code != nullptr && result != nullptr) {
        auto buf = eval(code->unit, const_cast<DefaultRandomFactory *>(&code->randomFactory));
        result->size = buf.size();
        result->data = extract(std::move(buf));
        return 0;
    }
    return -1;
}

void FuzzyRat_releaseResult(FuzzyRatResult *result) {
    if(result != nullptr) {
        free(result->data);
        result->data = nullptr;
        result->size = 0;
    }
}