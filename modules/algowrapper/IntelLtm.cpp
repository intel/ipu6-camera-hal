/*
 * Copyright (C) 2019-2020 Intel Corporation
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

#define LOG_TAG "IntelLtm"

#include "modules/algowrapper/IntelLtm.h"

#include "iutils/CameraLog.h"
#include "iutils/Errors.h"
#include "iutils/Utils.h"

namespace icamera {
IntelLtm::IntelLtm() {
    LOG1("@%s", __func__);
}

IntelLtm::~IntelLtm() {
    LOG1("@%s", __func__);
}

ia_ltm* IntelLtm::init(const ia_binary_data* lard_data_ptr, ia_mkn* mkn) {
    LOG1("%s", __func__);
    CheckError(!lard_data_ptr, nullptr, "@%s, lard_data_ptr is null", __func__);

    ia_ltm* ltm = ia_ltm_init(lard_data_ptr, mkn);
    CheckError(!ltm, nullptr, "@%s, ia_ltm_init fails", __func__);

    return ltm;
}

void IntelLtm::deinit(ia_ltm* ltm) {
    LOG1("%s", __func__);
    CheckError(!ltm, VOID_VALUE, "@%s, ltm is null", __func__);

    ia_ltm_deinit(ltm);
}

ia_err IntelLtm::run(ia_ltm* ltm, const ia_ltm_input_params* inputParams,
                     ia_ltm_results** ltmResults, ia_ltm_drc_params** drcResults) {
    LOG1("%s", __func__);
    CheckError(!ltm, ia_err_general, "@%s, ltm is null", __func__);
    CheckError(!inputParams, ia_err_general, "@%s, inputParams is null", __func__);
    CheckError(!ltmResults, ia_err_general, "@%s, ltmResults is null", __func__);
    CheckError(!drcResults, ia_err_general, "@%s, drcResults is null", __func__);

    ia_err ret = ia_ltm_run(ltm, inputParams, ltmResults, drcResults);

    return ret;
}

} /* namespace icamera */
