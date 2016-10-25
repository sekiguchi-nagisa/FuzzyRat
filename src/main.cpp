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

#include <iostream>

#include "misc/argv.hpp"
#include <fuzzyrat.h>

using namespace ydsh;

#define EACH_OPT(OP) \
    OP(HELP,  "--help", 0,             "show this help message") \
    OP(HELP2, "-h",     0,             "equivalent to '-h'") \
    OP(START, "-s",     argv::HAS_ARG, "specify start production") \
    OP(COUNT, "-c",     argv::HAS_ARG, "specify generation times")

enum OptionKind {
#define GEN_ENUM(E, S, A, M) E,
    EACH_OPT(GEN_ENUM)
#undef GEN_ENUM
};

static const argv::Option<OptionKind> options[] = {
#define GEN_OPT(E, S, A, M) {E, S, A, M},
        EACH_OPT(GEN_OPT)
#undef GEN_OPT
};

int main(int argc, char **argv) {
    argv::CmdLines<OptionKind> cmdLines;
    int index;
    try {
        index = argv::parseArgv(argc, argv, options, cmdLines);
    } catch(const argv::ParseError &e) {
        std::cerr << e.getMessage() << std::endl
                  << options << std::endl;
        exit(1);
    }

    const char *startSymbol = nullptr;
    const char *fileName = nullptr;
    unsigned int count = 1;

    for(auto &cmdline : cmdLines) {
        switch(cmdline.first) {
        case HELP:
        case HELP2:
            std::cout << options << std::endl;
            exit(0);
        case START:
            startSymbol = cmdline.second;
            break;
        case COUNT:
            count = std::stoi(std::string(cmdline.second));
            break;
        }
    }

    if(index < argc ) {
        fileName = argv[index];
    }

    if(fileName == nullptr) {
        std::cerr << "require file name" << std::endl;
        exit(1);
    }

    FILE *fp = fopen(fileName, "rb");
    if(fp == nullptr) {
        std::cerr << "cannot read file: " << fileName << std::endl;
        exit(1);
    }


    auto *input = FuzzyRat_newContext(fileName, fp);
    FuzzyRat_setStartProduction(input, startSymbol);
    auto code = FuzzyRat_compile(&input);

    for(unsigned int i = 0; i < count; i++) {
        FuzzyRatResult result;
        FuzzyRat_exec(code, &result);
        fwrite(result.data, sizeof(char), result.size, stdout);
        fputc('\n', stdout);
        fflush(stdout);
        FuzzyRat_releaseResult(&result);
    }

    FuzzyRat_deleteCode(&code);
    FuzzyRat_deleteContext(&input);
    exit(0);
}