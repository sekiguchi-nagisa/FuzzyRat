#include <iostream>

#include "misc/argv.hpp"
#include "misc/fatal.h"

#include "logger.h"
#include "parser.h"
#include "error.h"
#include "verify.h"

using namespace fuzzyrat;
using namespace ydsh;

#define EACH_OPT(OP) \
    OP(HELP,  "--help", 0,             "show this help message") \
    OP(HELP2, "-h",     0,             "equivalent to '-h'") \
    OP(START, "-s",     argv::HAS_ARG, "specify start production")

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
    std::cout << "Hello, World!" << std::endl;

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

    for(auto &cmdline : cmdLines) {
        switch(cmdline.first) {
        case HELP:
        case HELP2:
            std::cout << options << std::endl;
            exit(0);
        case START:
            startSymbol = cmdline.second;
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

    GrammarState state(fileName, fp);
    if(startSymbol != nullptr) {
        state.setStartSymbol(startSymbol);
    }

    try {
        Parser()(state);

        // check start production
        if(state.map().find(state.startSymbol()) != state.map().end()) {
            LOG_INFO("start production: " << state.startSymbol());
        } else {
            LOG_ERROR("undefined start production: " << state.startSymbol());
        }

        verify(state);
    } catch(const ParseError &e) {
        Token lineToken = state.lexer().getLineToken(e.getErrorToken());
        LOG_ERROR(formatSourceName(state.lexer(), e.getErrorToken())
                          << " " << e.getMessage() << std::endl
                          << state.lexer().toTokenText(lineToken) << std::endl
                          << state.lexer().formatLineMarker(lineToken, e.getErrorToken()));
    } catch(const SemanticError &e) {
        Token lineToken = state.lexer().getLineToken(e.token());
        LOG_ERROR(formatSourceName(state.lexer(), e.token())
                          << " " << toString(e.kind()) << std::endl
                          << state.lexer().toTokenText(lineToken) << std::endl
                          << state.lexer().formatLineMarker(lineToken, e.token()));
    }
    exit(0);
}