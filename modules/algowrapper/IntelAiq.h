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

#include <ia_aiq.h>
#include <ia_types.h>

#include <string>

namespace icamera {
class IntelAiq {
 public:
    IntelAiq();
    virtual ~IntelAiq();

    // the return pointer (ia_aiq*) is just valid in the sandboxing process.
    ia_aiq* init(const ia_binary_data* aiqbData, const ia_binary_data* nvmData,
                 const ia_binary_data* aiqdData, unsigned int statsMaxWidth,
                 unsigned int statsMaxHeight, unsigned int maxNumStatsIn, ia_cmc_t* cmc,
                 ia_mkn* mkn);
    ia_err aeRun(const ia_aiq_ae_input_params* inputParams, ia_aiq_ae_results** results);
    ia_err afRun(const ia_aiq_af_input_params* inputParams, ia_aiq_af_results** results);
    ia_err awbRun(const ia_aiq_awb_input_params* inputParams, ia_aiq_awb_results** results);
    ia_err gbceRun(const ia_aiq_gbce_input_params* inputParams, ia_aiq_gbce_results** results);
    ia_err paRunV1(const ia_aiq_pa_input_params* inputParams, ia_aiq_pa_results_v1** results);
    ia_err saRunV2(const ia_aiq_sa_input_params_v1* inputParams, ia_aiq_sa_results_v1** results);
    ia_err statisticsSetV4(const ia_aiq_statistics_input_params_v4* inputParams);
    ia_err getAiqdData(ia_binary_data* outData);
    void deinit();
    void getVersion(std::string* version);

 private:
    ia_aiq* mAiq;
};
} /* namespace icamera */
