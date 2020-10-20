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

#include <vector>

#include "IntelAlgoCommon.h"
#include "iutils/Errors.h"
#include "modules/sandboxing/IPCIspParamAdaptor.h"

namespace icamera {

class IntelIspParamAdaptor {
 public:
    IntelIspParamAdaptor();
    virtual ~IntelIspParamAdaptor();

    ia_isp_bxt* init(const ia_binary_data* ispData, const ia_cmc_t* iaCmc,
                     unsigned int maxStatsWidth, unsigned int maxStatsHeight,
                     unsigned int maxNumStatsIn, ia_mkn* iaMkn);
    void deInit(const ia_isp_bxt* ispBxtHandle);
    int getPalDataSize(const ia_isp_bxt_program_group* programGroup);
    status_t runPal(ia_isp_bxt* ispBxtHandle, const ia_isp_bxt_input_params_v2* inputParams,
                    ia_binary_data* outputData);
    status_t queryAndConvertStats(const ia_isp_bxt* ispBxtHandle,
                                  const ConvertInputParam* inputParams, ConvertResult* result);
    void* allocatePalBuffer(int streamId, int index, int palDataSize);
    void freePalBuffer(void* addr);

 private:
    IPCIspParamAdaptor mIpc;
    IntelAlgoCommon mCommon;

    bool mInitialized;
    ShmMemInfo mMemInit;
    ShmMemInfo mMemDeinit;
    ShmMemInfo mMemGetSize;
    ShmMemInfo mMemStats;
    ShmMemInfo mMemRunPal;
    std::vector<ShmMemInfo> mPalDataMems;

    std::vector<ShmMem> mMems;
};

}  // namespace icamera
