//
// Created by skgchxngsxyz-carbon on 16/10/10.
//

#ifndef FUZZYRAT_FUZZYRAT_H
#define FUZZYRAT_FUZZYRAT_H

#include <stdio.h>


#ifdef __cplusplus
extern "C" {
#endif

struct FuzzyRatInputContext;
typedef struct FuzzyRatInputContext FuzzyRatInputContext;

FuzzyRatInputContext *FuzzyRat_newContext(const char *sourceName, FILE *fp);

void FuzzyRat_deleteContext(FuzzyRatInputContext **input);

void FuzzyRat_setStartProduction(FuzzyRatInputContext *input, const char *productionName);


struct FuzzyRatCode;
typedef struct FuzzyRatCode FuzzyRatCode;

FuzzyRatCode *FuzzyRat_compile(FuzzyRatInputContext *input);

void FuzzyRat_deleteCode(FuzzyRatCode **code);


typedef struct {
    char *data;
    size_t size;
} FuzzyRatResult;

int FuzzyRat_exec(const FuzzyRatCode *code, FuzzyRatResult *result);

void FuzzyRat_releaseResult(FuzzyRatResult *result);


#ifdef __cplusplus
}
#endif

#endif /* FUZZYRAT_FUZZYRAT_H */
