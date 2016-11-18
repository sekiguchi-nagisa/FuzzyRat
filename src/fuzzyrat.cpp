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

#include <string>

#include <fuzzyrat.h>
#include "logger.h"
#include "parser.h"
#include "error.h"
#include "verify.h"
#include "compile.h"

using namespace fuzzyrat;

struct FuzzyRatInputContext {
    std::string sourceName;
    FILE *fp;
    std::string startProduction;

    FuzzyRatInputContext(const char *sourceName, FILE *fp) :
            sourceName(sourceName), fp(fp), startProduction() {}
};

FuzzyRatInputContext *FuzzyRat_newContext(const char *sourceName, FILE *fp) {
    return new FuzzyRatInputContext(sourceName, fp);
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

struct FuzzyRatCode {
    CompiledUnit unit;

    FuzzyRatCode(CompiledUnit &&unit) : unit(std::move(unit)) {}
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

FuzzyRatCode *FuzzyRat_compile(FuzzyRatInputContext **ptr) {
    assert(ptr != nullptr);
    auto input = *ptr;

    GrammarState state;
    state.setStartSymbol(input->startProduction);
    defineSpace(state);

    {
        Lexer lexer(input->sourceName.c_str(), input->fp);
        parseAndVerify(state, lexer);
    }

    insertSpace(state); //FIXME: disable space insertion by specifying an option.
    log<LogLevel::debug>([&](std::ostream &stream) {
        stream << "before desugar" << std::endl;
        NodePrinter printer(stream); printer(state.map());
    });

    desugar(state);
    log<LogLevel::debug>([&](std::ostream &stream) {
        stream << "after desugar" << std::endl;
        NodePrinter printer(stream); printer(state.map());
    });

    auto code = new FuzzyRatCode(Compiler()(state));
    FuzzyRat_deleteContext(ptr);
    return code;
}

void FuzzyRat_deleteCode(FuzzyRatCode **code) {
    if(code != nullptr) {
        delete *code;
        *code = nullptr;
    }
}

int FuzzyRat_exec(const FuzzyRatCode *code, FuzzyRatResult *result) {
    if(code != nullptr && result != nullptr) {
        auto buf = eval(code->unit);
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