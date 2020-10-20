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

#define LOG_TAG "IntelAiq"

#include "modules/algowrapper/IntelAiq.h"

#include "iutils/CameraLog.h"
#include "iutils/Utils.h"

namespace icamera {
IntelAiq::IntelAiq() : mAiq(nullptr) {
    LOG2("@%s", __func__);
}

IntelAiq::~IntelAiq() {
    LOG2("@%s", __func__);
}

ia_aiq* IntelAiq::init(const ia_binary_data* aiqbData, const ia_binary_data* nvmData,
                       const ia_binary_data* aiqdData, unsigned int statsMaxWidth,
                       unsigned int statsMaxHeight, unsigned int maxNumStatsIn, ia_cmc_t* cmc,
                       ia_mkn* mkn) {
    LOG2("@%s, aiqbData:%p, nvmData:%p, aiqdData:%p", __func__, aiqbData, nvmData, aiqdData);

    mAiq = ia_aiq_init(aiqbData, nvmData, aiqdData, statsMaxWidth, statsMaxHeight, maxNumStatsIn,
                       cmc, mkn);

    return mAiq;
}

ia_err IntelAiq::aeRun(const ia_aiq_ae_input_params* inputParams, ia_aiq_ae_results** results) {
    LOG2("@%s, inputParams:%p, results:%p", __func__, inputParams, results);
    CheckError(!mAiq, ia_err_argument, "@%s, mAiq is nullptr", __func__);
    CheckError(!inputParams, ia_err_argument, "@%s, inputParams is nullptr", __func__);
    CheckError(!results, ia_err_argument, "@%s, results is nullptr", __func__);

    return ia_aiq_ae_run(mAiq, inputParams, results);
}

ia_err IntelAiq::afRun(const ia_aiq_af_input_params* inputParams, ia_aiq_af_results** results) {
    LOG2("@%s, inputParams:%p, results:%p", __func__, inputParams, results);
    CheckError(!mAiq, ia_err_argument, "@%s, mAiq is nullptr", __func__);
    CheckError(!inputParams, ia_err_argument, "@%s, inputParams is nullptr", __func__);
    CheckError(!results, ia_err_argument, "@%s, results is nullptr", __func__);

    return ia_aiq_af_run(mAiq, inputParams, results);
}

ia_err IntelAiq::awbRun(const ia_aiq_awb_input_params* inputParams, ia_aiq_awb_results** results) {
    LOG2("@%s, inputParams:%p, results:%p", __func__, inputParams, results);
    CheckError(!mAiq, ia_err_argument, "@%s, mAiq is nullptr", __func__);
    CheckError(!inputParams, ia_err_argument, "@%s, inputParams is nullptr", __func__);
    CheckError(!results, ia_err_argument, "@%s, results is nullptr", __func__);

    return ia_aiq_awb_run(mAiq, inputParams, results);
}

ia_err IntelAiq::gbceRun(const ia_aiq_gbce_input_params* inputParams,
                         ia_aiq_gbce_results** results) {
    LOG2("@%s, inputParams:%p, results:%p", __func__, inputParams, results);
    CheckError(!mAiq, ia_err_argument, "@%s, mAiq is nullptr", __func__);
    CheckError(!inputParams, ia_err_argument, "@%s, inputParams is nullptr", __func__);
    CheckError(!results, ia_err_argument, "@%s, results is nullptr", __func__);

    return ia_aiq_gbce_run(mAiq, inputParams, results);
}

ia_err IntelAiq::paRunV1(const ia_aiq_pa_input_params* inputParams,
                         ia_aiq_pa_results_v1** results) {
    LOG2("@%s, inputParams:%p, results:%p", __func__, inputParams, results);
    CheckError(!mAiq, ia_err_argument, "@%s, mAiq is nullptr", __func__);
    CheckError(!inputParams, ia_err_argument, "@%s, inputParams is nullptr", __func__);
    CheckError(!results, ia_err_argument, "@%s, results is nullptr", __func__);

    return ia_aiq_pa_run_v1(mAiq, inputParams, results);
}

ia_err IntelAiq::saRunV2(const ia_aiq_sa_input_params_v1* inputParams,
                         ia_aiq_sa_results_v1** results) {
    LOG2("@%s, inputParams:%p, results:%p", __func__, inputParams, results);
    CheckError(!mAiq, ia_err_argument, "@%s, mAiq is nullptr", __func__);
    CheckError(!inputParams, ia_err_argument, "@%s, inputParams is nullptr", __func__);
    CheckError(!results, ia_err_argument, "@%s, results is nullptr", __func__);

    return ia_aiq_sa_run_v2(mAiq, inputParams, results);
}

ia_err IntelAiq::statisticsSetV4(const ia_aiq_statistics_input_params_v4* inputParams) {
    LOG2("@%s, inputParams:%p", __func__, inputParams);
    CheckError(!mAiq, ia_err_argument, "@%s, mAiq is nullptr", __func__);
    CheckError(!inputParams, ia_err_argument, "@%s, inputParams is nullptr", __func__);

    return ia_aiq_statistics_set_v4(mAiq, inputParams);
}

ia_err IntelAiq::getAiqdData(ia_binary_data* outData) {
    LOG2("@%s, outData:%p", __func__, outData);
    CheckError(!mAiq, ia_err_argument, "@%s, mAiq is nullptr", __func__);
    CheckError(!outData, ia_err_argument, "@%s, outData is nullptr", __func__);

    return ia_aiq_get_aiqd_data(mAiq, outData);
}

void IntelAiq::deinit() {
    LOG2("@%s", __func__);
    CheckError(!mAiq, VOID_VALUE, "@%s, mAiq is nullptr", __func__);

    ia_aiq_deinit(mAiq);
}

void IntelAiq::getVersion(std::string* version) {
    LOG2("@%s", __func__);

    *version = std::string(ia_aiq_get_version());
}

} /* namespace icamera */
