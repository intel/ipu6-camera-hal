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

#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "CameraTypes.h"
#include "IntelAlgoCommonClient.h"
#include "iutils/Thread.h"
#include "modules/sandboxing/IPCIntelCca.h"

namespace icamera {
class IntelCca {
 public:
    static IntelCca* getInstance(int cameraId, TuningMode mode);
    static void releaseInstance(int cameraId, TuningMode mode);
    static void releaseAllInstances();

    ia_err init(const cca::cca_init_params& initParams);

    ia_err setStatsParams(const cca::cca_stats_params& params);

    ia_err runAEC(uint64_t frameId, const cca::cca_ae_input_params& params,
                  cca::cca_ae_results* results);
    ia_err runAIQ(uint64_t frameId, const cca::cca_aiq_params& params,
                  cca::cca_aiq_results* results, camera_makernote_mode_t mode = MAKERNOTE_MODE_OFF);

    ia_err runLTM(uint64_t frameId, const cca::cca_ltm_input_params& params);

    ia_err updateZoom(uint32_t streamId, const cca::cca_dvs_zoom& params);

    ia_err runDVS(uint32_t streamId, uint64_t frameId);

    ia_err runAIC(uint64_t frameId, cca::cca_pal_input_params* params, ia_binary_data* pal);

    ia_err getCMC(cca::cca_cmc* cmc, const cca::cca_cpf* cpf = nullptr);
    ia_err getMKN(ia_mkn_trg type, cca::cca_mkn* mkn);
    ia_err getAiqd(cca::cca_aiqd* aiqd);
    ia_err updateTuning(uint8_t lardTags, const ia_lard_input_params& lardParams,
                        const cca::cca_nvm& nvm, int32_t streamId);

    bool allocStatsDataMem(unsigned int size);
    void* getStatsDataBuffer();
    void decodeHwStatsDone(int64_t sequence, unsigned int byteUsed);
    void* fetchHwStatsData(int64_t sequence, unsigned int* byteUsed);

    void deinit();

    // No decoding here because decoding should be done
    // in IPC_CCA_RUN_AEC or IPC_PG_PARAM_DECODE if it is required.
    ia_err decodeStats(uint64_t statsPointer, uint32_t statsSize, uint32_t bitmap,
                       ia_isp_bxt_statistics_query_results_t* results = nullptr,
                       cca::cca_out_stats* outStats = nullptr);

    uint32_t getPalDataSize(const cca::cca_program_group& programGroup);
    void* allocMem(int streamId, const std::string& name, int index, int size);
    void freeMem(void* addr);

 private:
    IntelCca(int cameraId, TuningMode mode);
    virtual ~IntelCca();
    void freeStatsDataMem();

 private:
    int mCameraId;
    TuningMode mTuningMode;
    bool mHasMknData;

    IntelAlgoCommon mCommon;

    ShmMemInfo mMemStruct;
    ShmMemInfo mMemInit;
    ShmMemInfo mMemAEC;
    ShmMemInfo mMemAIQ;
    ShmMemInfo mMemLTM;
    ShmMemInfo mMemZoom;
    ShmMemInfo mMemDVS;
    ShmMemInfo mMemAIC;
    ShmMemInfo mMemCMC;
    ShmMemInfo mMemAIQD;
    ShmMemInfo mMemTuning;
    ShmMemInfo mMemDeinit;
    ShmMemInfo mMemDecodeStats;
    ShmMemInfo mMemPalSize;

    std::vector<ShmMem> mMems;

    // Only 3 buffers will be held in AiqResultStorage (kAiqResultStorageSize is 3)
    // So it is safe to use other 3 buffers.
    static const int kMaxQueueSize = 6;
    Mutex mMemStatsMLock;  // protect mMemStatsInfoMap
    struct StatsBufInfo {
        unsigned int bufSize;
        ShmMemInfo shmMem;
        unsigned int usedSize;
    };
    // first: sequence id, second: stats buffer info
    std::map<int64_t, StatsBufInfo> mMemStatsInfoMap;

    std::unordered_map<void*, ShmMemInfo> mMemsOuter;

 private:
    struct CCAHandle {
        int cameraId;
        std::unordered_map<TuningMode, IntelCca*> ccaHandle;  // TuningMode to IntelCca map
    };
    static std::vector<CCAHandle> sCcaInstance;
    static Mutex sLock;
};
} /* namespace icamera */
