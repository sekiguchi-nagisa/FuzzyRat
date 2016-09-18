//
// Created by skgchxngsxyz-carbon on 16/09/17.
//

#ifndef FUZZYRAT_LOGGER_HPP
#define FUZZYRAT_LOGGER_HPP

#include <iostream>
#include <cstdlib>

#include "misc/noncopyable.h"

#define EACH_LOG_LEVEL(E) \
    E(error) \
    E(warn) \
    E(info) \
    E(debug)

namespace fuzzyrat {

enum class LogLevel : unsigned int {
#define GEN_ENUM(E) E,
    EACH_LOG_LEVEL(GEN_ENUM)
#undef GEN_ENUM
};


/**
 * specify log appender by FRAT_APPENDER=<appender>
 * specify log level by FRAT_LEVEL=<error, warn, info, debug>
 */
class Logger {
private:
    std::ostream *stream_;

    LogLevel level_;

    Logger();

public:
    ~Logger();

    /**
     *
     * @param level
     * if level is error, exit(1)
     * @return
     */
    std::ostream &stream(LogLevel level);

    LogLevel level() const {
        return this->level_;
    }

    bool checkLevel(LogLevel level) const {
        return static_cast<unsigned int>(level) <= static_cast<unsigned int>(this->level());
    }

    static Logger &instance();
};

class FuncTracer {
private:
    const char *funcName;
    LogLevel level;

public:
    NON_COPYABLE(FuncTracer);

    FuncTracer(const char *funcName, LogLevel level);
    ~FuncTracer();
};

} // namespace fuzzyrat

#define LOG(L, V) \
do { \
    using namespace fuzzyrat; \
    if(Logger::instance().checkLevel(L)) { \
        Logger::instance().stream(L) << V << std::endl; \
    } \
    if(L == LogLevel::error) { exit(1); } \
} while(false)

#define LOG_ERROR(V) LOG(fuzzyrat::LogLevel::error, V)
#define LOG_WARN(V) LOG(fuzzyrat::LogLevel::warn, V)
#define LOG_INFO(V) LOG(fuzzyrat::LogLevel::info, V)
#define LOG_DEBUG(V) LOG(fuzzyrat::LogLevel::debug, V)



#endif //FUZZYRAT_LOGGER_HPP
