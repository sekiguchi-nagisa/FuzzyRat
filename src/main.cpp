#include <iostream>
#include <cstdio>

#include "misc/fatal.h"

#include "logger.h"
#include "parser.h"
#include "error.h"
#include "verify.h"

using namespace fuzzyrat;

int main(int argc, char **argv) {
    std::cout << "Hello, World!" << std::endl;

    if(argc == 1) {
        fatal("require file name\n");
    }

    const char *fileName = argv[1];
    FILE *fp = fopen(fileName, "rb");
    if(fp == nullptr) {
        fatal("cannot read file: %s\n", fileName);
    }

    Lexer lexer(fileName, fp);

    ProductionMap map;
    try {
        map = Parser()(lexer);
        verify(map);
    } catch(const ParseError &e) {
        Token lineToken = lexer.getLineToken(e.getErrorToken());
        LOG_ERROR(formatSourceName(lexer, e.getErrorToken())
                          << " " << e.getMessage() << std::endl
                          << lexer.toTokenText(lineToken) << std::endl
                          << lexer.formatLineMarker(lineToken, e.getErrorToken()));

        exit(1);
    } catch(const SemanticError &e) {
        Token lineToken = lexer.getLineToken(e.token());
        LOG_ERROR(formatSourceName(lexer, e.token())
                          << " " << toString(e.kind()) << std::endl
                          << lexer.toTokenText(lineToken) << std::endl
                          << lexer.formatLineMarker(lineToken, e.token()));

        exit(1);
    }
    exit(0);
}