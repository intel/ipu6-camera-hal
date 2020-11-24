/*
 * Copyright (C) 2020 Intel Corporation.
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

#include <IntelCCA.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "iutils/Thread.h"
#include "CameraTypes.h"

namespace icamera {
class IntelCca {
 public:
    IntelCca();
    virtual ~IntelCca();

    static IntelCca* getInstance(int cameraId, TuningMode mode);
    static void releaseInstance(int cameraId, TuningMode mode);

    ia_err init(const cca::cca_init_params& initParams);

    ia_err setStatsParams(const cca::cca_stats_params& params);

    ia_err runAEC(uint64_t frameId, const cca::cca_ae_input_params& params,
                  cca::cca_ae_results* results);
    ia_err runAIQ(uint64_t frameId, const cca::cca_aiq_params& params,
                  cca::cca_aiq_results* results);

    ia_err runLTM(uint64_t frameId, const cca::cca_ltm_input_params& params);

    ia_err updateZoom(const cca::cca_dvs_zoom& params);

    ia_err runDVS(uint64_t frameId);

    ia_err runAIC(uint64_t frameId, const cca::cca_pal_input_params* params, ia_binary_data* pal);

    ia_err getCMC(cca::cca_cmc* cmc);
    ia_err getMKN(ia_mkn_trg type, cca::cca_mkn* mkn);
    ia_err getAiqd(cca::cca_aiqd* aiqd);
    ia_err updateTuning(uint8_t lardTags, const ia_lard_input_params& lardParams);

    void deinit();
    void getVersion(std::string* version);

    ia_err decodeStats(uint64_t statsPointer, uint32_t statsSize,
                       ia_isp_bxt_statistics_query_results_t* results);

    uint32_t getPalDataSize(const cca::cca_program_group& programGroup);
    void* allocatePalBuffer(int streamId, int index, int palDataSize);
    void freePalBuffer(void* addr);

 private:
    cca::IntelCCA* getIntelCCA();
    void releaseIntelCCA();

 private:
    struct CCAHandle {
        int cameraId;
        std::unordered_map<TuningMode, IntelCca*> ccaHandle;  // TuningMode to IntelCca map
    };
    static std::vector<CCAHandle> sCcaInstance;
    static Mutex sLock;

    cca::IntelCCA* mIntelCCA;
};
} /* namespace icamera */
