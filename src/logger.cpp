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

#include <cstring>
#include <fstream>

#include "misc/size.hpp"
#include "logger.h"

namespace fuzzyrat {

static const char *levelStrTable[] = {
#define GEN_STR(E) #E,
        EACH_LOG_LEVEL(GEN_STR)
#undef GEN_STR
};

static LogLevel parseLevel(const char *str, LogLevel defaultLevel) {
    if(str != nullptr) {
        unsigned int index = 0;
        for(; index < ydsh::arraySize(levelStrTable); index++) {
            if(strcasecmp(str, levelStrTable[index]) == 0) {
                return static_cast<LogLevel>(index);
            }
        }
    }
    return defaultLevel;
}

std::ostream &operator<<(std::ostream &stream, LogLevel level) {
    return stream << "[" << levelStrTable[static_cast<unsigned int>(level)] << "]";
}

// ####################
// ##     Logger     ##
// ####################

Logger::Logger() : stream_(nullptr), level_(LogLevel::info) {
    // init appender
    const char *appender = getenv("FRAT_APPENDER");
    if(appender != nullptr) {
        std::ostream *os = new std::ofstream(appender);
        if(!(*os)) {
            delete os;
            os = nullptr;
        }
        this->stream_ = os;
    }
    if(this->stream_ == nullptr) {
        this->stream_ = &std::cerr;
    }

    // log level
    this->level_ = parseLevel(getenv("FRAT_LEVEL"), LogLevel::info);
}

Logger::~Logger() {
    if(this->stream_ != &std::cerr) {
        delete this->stream_;
    }
}

Logger &Logger::instance() {
    static Logger logger;
    return logger;
}

} // namespace fuzzyrat