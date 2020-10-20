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

#include <ia_ltm.h>

namespace icamera {
class IntelLtm {
 public:
    IntelLtm();
    virtual ~IntelLtm();

    ia_ltm* init(const ia_binary_data* lard_data_ptr, ia_mkn* mkn);
    ia_err run(ia_ltm* ltm, const ia_ltm_input_params* inputParams, ia_ltm_results** ltmResults,
               ia_ltm_drc_params** drcResults);
    void deinit(ia_ltm* ltm);
};
} /* namespace icamera */
