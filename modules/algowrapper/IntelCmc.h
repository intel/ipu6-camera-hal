/*
 * Copyright (C) 2019-2020 Intel Corporation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <ia_cmc_parser.h>
#include <ia_cmc_types.h>

namespace icamera {
class IntelCmc {
 public:
    IntelCmc();
    ~IntelCmc();

    bool init(const ia_binary_data* aiqbData, const ia_binary_data* nvmData);

    ia_cmc_t* getCmc() const;
    uintptr_t getCmcHandle() const;

    void deinit();

 private:
    ia_cmc_t* mHandle;
};
} /* namespace icamera */
