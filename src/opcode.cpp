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

#include "eval.hpp"

namespace fuzzyrat {

struct DefaultRandomEngineFactory {
    std::default_random_engine operator()() const {
        std::vector<int> v(32);
        std::random_device rdev;
        std::generate(v.begin(), v.end(), std::ref(rdev));
        std::seed_seq seed(v.begin(), v.end());
        return std::default_random_engine(seed);
    }
};

ydsh::ByteBuffer eval(const CompiledUnit &unit) {
    EvalState<DefaultRandomEngineFactory> state(unit);

    auto entryPoint = std::make_shared<CallOp>(unit.startId());
    for(const OpCode *code = entryPoint.get(); (code = eval(code, state)) != nullptr;);

    return std::move(state.buffer);
}

} // namespace fuzzyrat
