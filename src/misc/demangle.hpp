/*
 * Copyright (C) 2015-2016 Nagisa Sekiguchi
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

#ifndef YDSH_MISC_DEMANGLE_HPP
#define YDSH_MISC_DEMANGLE_HPP

#include <string>
#include <cxxabi.h>

#include "fatal.h"

namespace ydsh {

inline std::string demangle(const std::type_info &info) {
    int status;

    char *className = abi::__cxa_demangle(info.name(), 0, 0, &status);
    if(className == nullptr || status != 0) {
        fatal("demangle typeinfo failed: %s\n", info.name());
    }

    std::string str(className);

    // free demangled name
    free(className);

    return str;
}

} // namespace ydsh

#endif //YDSH_MISC_DEMANGLE_HPP
