//
// Created by skgchxngsxyz-carbon on 16/09/18.
//

#include <cstring>
#include <fstream>

#include "misc/size.hpp"
#include "logger.h"

namespace fuzzyrat {

// ####################
// ##     Logger     ##
// ####################

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

std::ostream& Logger::stream(LogLevel level) {
    return *this->stream_ << "[" << levelStrTable[static_cast<unsigned int>(level)] << "] ";
}

Logger &Logger::instance() {
    static Logger logger;
    return logger;
}

} // namespace fuzzyrat