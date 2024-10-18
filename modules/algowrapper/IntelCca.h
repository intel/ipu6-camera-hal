/*
 * Copyright (C) 2020-2023 Intel Corporation.
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
#include <map>

#include "iutils/Thread.h"
#include "CameraTypes.h"

namespace icamera {
class IntelCca {
 public:
    IntelCca(int cameraId, TuningMode mode);
    virtual ~IntelCca();

    static IntelCca* getInstance(int cameraId, TuningMode mode);
    static void releaseInstance(int cameraId, TuningMode mode);
    static void releaseAllInstances();

    ia_err init(const cca::cca_init_params& initParams);

    ia_err setStatsParams(const cca::cca_stats_params& params);

    ia_err runAEC(uint64_t frameId, const cca::cca_ae_input_params& params,
                  cca::cca_ae_results* results, bool lowPower);
    ia_err runAIQ(uint64_t frameId, const cca::cca_aiq_params& params,
                  cca::cca_aiq_results* results, camera_makernote_mode_t mode = MAKERNOTE_MODE_OFF);

    ia_err runLTM(uint64_t frameId, const cca::cca_ltm_input_params& params);

    ia_err reconfigDvs(const cca::cca_dvs_init_param& dvsInitParam,
                       const cca::cca_gdc_configurations& gdcConfigs);

    ia_err updateZoom(uint32_t streamId, const cca::cca_dvs_zoom& params);

    ia_err runDVS(uint32_t streamId, uint64_t frameId);

    ia_err runAIC(uint64_t frameId, const cca::cca_pal_input_params* params, ia_binary_data* pal);

    ia_err getCMC(cca::cca_cmc* cmc, const cca::cca_cpf* cpf = nullptr);
    ia_err getMKN(ia_mkn_trg type, cca::cca_mkn* mkn);
    ia_err getAiqd(cca::cca_aiqd* aiqd);
    ia_err updateTuning(uint8_t lardTags, const ia_lard_input_params& lardParams,
                        const cca::cca_nvm& nvm, int32_t streamId);
    // PRIVACY_MODE_S
    ia_err getBrightestIndex(uint32_t *outMaxBin);
    // PRIVACY_MODE_E

    bool allocStatsDataMem(unsigned int size);
    void freeStatsDataMem();
    void* getStatsDataBuffer();
    void decodeHwStatsDone(int64_t sequence, unsigned int byteUsed);
    void* fetchHwStatsData(int64_t sequence, unsigned int* byteUsed);

    void deinit();

    // Do decoding if results is valid
    ia_err decodeStats(uint64_t statsPointer, uint32_t statsSize, uint32_t bitmap,
                       ia_isp_bxt_statistics_query_results_t* results = nullptr,
                       cca::cca_out_stats* outStats = nullptr);

    uint32_t getPalDataSize(const cca::cca_program_group& programGroup);
    void* allocMem(int streamId, const std::string& name, int index, int size);
    void freeMem(void* addr);

 private:
    cca::IntelCCA* getIntelCCA();
    void releaseIntelCCA();

 private:
    int mCameraId;
    TuningMode mTuningMode;

    // Only 3 buffers will be held in AiqResultStorage (kAiqResultStorageSize is 3),
    // So it is safe to use other 3 buffers.
    static const int kMaxQueueSize = 6;
    struct StatsBufInfo {
        unsigned int bufSize;
        void* ptr;
        unsigned int usedSize;
    };
    Mutex mMemStatsMLock;  // protect mMemStatsInfoMap
    // first: sequence id, second: stats buffer info
    std::map<int64_t, StatsBufInfo> mMemStatsInfoMap;

    struct CCAHandle {
        int cameraId;
        std::unordered_map<TuningMode, IntelCca*> ccaHandle;  // TuningMode to IntelCca map
    };
    static std::vector<CCAHandle> sCcaInstance;
    static Mutex sLock;

    cca::IntelCCA* mIntelCCA;
};
} /* namespace icamera */
