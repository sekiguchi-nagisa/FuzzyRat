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

#ifndef FUZZYRAT_LOGGER_HPP
#define FUZZYRAT_LOGGER_HPP

#include <iostream>
#include <cstdlib>
#include <type_traits>

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

std::ostream &operator<<(std::ostream &, LogLevel);

template <bool cond, typename T>
using enable_if_t = typename std::enable_if<cond, T>::type;

template <bool cond>
using enable_when = enable_if_t<cond, std::nullptr_t>;


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

    LogLevel level() const {
        return this->level_;
    }

    bool checkLevel(LogLevel level) const {
        return static_cast<unsigned int>(level) <= static_cast<unsigned int>(this->level());
    }

    static Logger &instance();

    template <LogLevel level, typename Func, enable_when<level != LogLevel::error> = nullptr>
    void apply(Func func) {
        if(this->checkLevel(level)) {
            auto &s = *this->stream_;
            s << level << " ";
            func(s);
            s << std::endl;
        }
    }

    template <LogLevel level, typename Func, enable_when<level == LogLevel::error> = nullptr>
    [[noreturn]]
    void apply(Func func) {
        auto &s = *this->stream_;
        s << level << " ";
        func(s);
        s << std::endl;
        exit(1);
    }
};

template <LogLevel level, typename Func, enable_when<level != LogLevel::error> = nullptr>
inline void log(Func func) {
    Logger::instance().apply<level>(std::forward<Func>(func));
}

template <LogLevel level, typename Func, enable_when<level == LogLevel::error> = nullptr>
[[noreturn]]
inline void log(Func func) {
    Logger::instance().apply<level>(std::forward<Func>(func));
}

template <LogLevel level>
class FuncTracer {
private:
    const char *funcName;

public:
    NON_COPYABLE(FuncTracer);

    FuncTracer(const char *funcName) : funcName(funcName) {
        log<level>([&](std::ostream &stream) { stream << "enter:" << this->funcName; });
    }

    ~FuncTracer() {
        log<level>([&](std::ostream &stream) { stream << "exit:" << this->funcName; });
    }
};

} // namespace fuzzyrat

#define LOG(L, V) log<L>([&](std::ostream &stream) { stream << V; })

#define LOG_ERROR(V) LOG(fuzzyrat::LogLevel::error, V)
#define LOG_WARN(V) LOG(fuzzyrat::LogLevel::warn, V)
#define LOG_INFO(V) LOG(fuzzyrat::LogLevel::info, V)
#define LOG_DEBUG(V) LOG(fuzzyrat::LogLevel::debug, V)



#endif //FUZZYRAT_LOGGER_HPP
