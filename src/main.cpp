#include <iostream>
#include <cstdio>

#include "misc/fatal.h"

#include "parser.h"
#include "error.h"

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
    } catch(const ParseError &e) {
        std::cerr << formatSourceName(lexer, e.getErrorToken()) << " [syntax error] " << e.getMessage() << std::endl;
        Token lineToken = lexer.getLineToken(e.getErrorToken());
        std::cerr << lexer.toTokenText(lineToken) << std::endl;
        std::cerr << lexer.formatLineMarker(lineToken, e.getErrorToken()) << std::endl;

        exit(1);
    } catch(const SemanticError &e) {
        std::cerr << formatSourceName(lexer, e.token()) << " [semantic error] " << toString(e.kind()) << std::endl;
        Token lineToken = lexer.getLineToken(e.token());
        std::cerr << lexer.toTokenText(lineToken) << std::endl;
        std::cerr << lexer.formatLineMarker(lineToken, e.token()) << std::endl;

        exit(1);
    }
    exit(0);
}