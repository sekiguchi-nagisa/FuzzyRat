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

#ifndef FUZZYRAT_FUZZYRAT_H
#define FUZZYRAT_FUZZYRAT_H

#include <stdio.h>


#ifdef __cplusplus
extern "C" {
#endif

struct FuzzyRatInputContext;
typedef struct FuzzyRatInputContext FuzzyRatInputContext;

/**
 * 
 * @param sourceName
 * @return
 * return null, if cannot open file.
 */
FuzzyRatInputContext *FuzzyRat_newContextFromFile(const char *sourceName);

/**
 *
 * @param sourceName
 * @param data
 * @param size
 * @return
 * return null, if data is null or size is 0.
 */
FuzzyRatInputContext *FuzzyRat_newContext(const char *sourceName, const char *data, unsigned int size);

/**
 *
 * @param input
 * @param pattern
 * if null, set default pattern. if empty string, disable space insertion.
 */
void FuzzyRat_setSpacePattern(FuzzyRatInputContext *input, const char *pattern);

void FuzzyRat_deleteContext(FuzzyRatInputContext **input);

void FuzzyRat_setStartProduction(FuzzyRatInputContext *input, const char *productionName);


struct FuzzyRatCode;
typedef struct FuzzyRatCode FuzzyRatCode;

FuzzyRatCode *FuzzyRat_compile(const FuzzyRatInputContext *input);

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
