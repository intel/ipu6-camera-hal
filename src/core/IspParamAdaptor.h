/*
 * Copyright (C) 2015-2021 Intel Corporation.
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

#include <v4l2_device.h>

#include <memory>
#include <vector>
#include <list>
#include <unordered_map>

#include "iutils/Errors.h"
#include "CameraBuffer.h"
#include "CameraTypes.h"
#include "PlatformData.h"

#include "NodeInfo.h"

extern "C" {
#ifndef CAL_BUILD
#include "ia_camera/ipu_process_group_wrapper.h"
#endif
}

#ifdef ENABLE_SANDBOXING
#include "modules/sandboxing/client/IntelCca.h"
#else
#include "modules/algowrapper/IntelCca.h"
#endif

#include "3a/AiqResult.h"
#include "ia_aiq_types.h"
#include "ia_isp_bxt_types.h"
#include "ia_isp_bxt_statistics_types.h"
#include "ia_isp_bxt.h"
#include "ia_bcomp_types.h"
#include "gc/IGraphConfigManager.h"
#include "IspSettings.h"

namespace icamera {

enum PgParamType {
    PG_PARAM_VIDEO = 0,
    PG_PARAM_PSYS_ISA,
    PG_PARAM_ISYS,
    PG_PARAM_STILL_4k,
    PG_PARAM_STILL_8m
};

/**
 * This class is for isp parameter converting including:
 * 1. Convert hw statistics to aiq statistics
 * 2. Convert aiq result to isa config
 * 3. Run isp config
 * 4. Provide p2p handle
 */
class IspParamAdaptor {
public:
    IspParamAdaptor(int cameraId, PgParamType type);
    virtual ~IspParamAdaptor();

    int init();
    int deinit();
    int configure(const stream_t &stream, ConfigMode configMode, TuningMode tuningMode);

    int getParameters(Parameters& param);
    int decodeStatsData(TuningMode tuningMode,
                        std::shared_ptr<CameraBuffer> statsBuffer,
                        std::shared_ptr<IGraphConfig> graphConfig = nullptr);

    int runIspAdapt(const IspSettings* ispSettings, long requestId, long settingSequence = -1, int32_t streamId = -1);
    //Get ISP param from mult-stream ISP param adaptation
    ia_binary_data* getIpuParameter(long sequence = -1, int streamId = -1);
    int getPalOutputDataSize(const ia_isp_bxt_program_group* programGroup);

private:
    DISALLOW_COPY_AND_ASSIGN(IspParamAdaptor);

    int deepCopyProgramGroup(const ia_isp_bxt_program_group *pgPtr,
                             cca::cca_program_group *programGroup);
    int getDataFromProgramGroup();
    int initProgramGroupForAllStreams(ConfigMode configMode);
    int postConfigure(int width, int height, ia_binary_data *binaryData);
    void initInputParams(cca::cca_pal_input_params *params);

    void updatePalDataForVideoPipe(ia_binary_data dest);

    struct IspParameter {
        /*
         * map from setting sequnce to PAL data sequence to look up PAL data
         * for PAL may not run if no scene changes, so setting sequence will
         * map PAL data sequence with latest PAL data.
         */
        std::map<int64_t, int64_t> mSequenceToDataId;
        // map from sequence to ia_binary_data
        std::multimap<int64_t, ia_binary_data> mSequenceToDataMap;
    };
    void updateIspParameterMap(IspParameter* ispParam, int64_t dataSeq,
                               int64_t settingSeq, ia_binary_data curIpuParam);
    int runIspAdaptL(ia_isp_bxt_program_group *pgPtr, ia_isp_bxt_gdc_limits* mbrData,
                     const IspSettings* ispSettings, long requestId, long settingSequence,
                     ia_binary_data *binaryData, int32_t streamId = -1);

    //Allocate memory for mIspParameters
    int allocateIspParamBuffers();
    //Release memory for mIspParameters
    void releaseIspParamBuffers();

    // Dumping methods for debugging purposes.
    void dumpIspParameter(long sequence, ia_binary_data binaryData);

    // Enable or disable kernels according to environment variables for debug purpose.
    void updateKernelToggles(cca::cca_program_group *programGroup);
    void dumpProgramGroup(ia_isp_bxt_program_group *pgPtr);
    void applyMediaFormat(const AiqResult* aiqResult,
                          ia_media_format* mediaFormat, bool* useLinearGamma);
    void dumpCscMatrix(const ia_isp_bxt_csc* cscMatrix);
    void applyCscMatrix(ia_isp_bxt_csc* cscMatrix);

 private:
    enum IspAdaptorState {
        ISP_ADAPTOR_NOT_INIT,
        ISP_ADAPTOR_INIT,
        ISP_ADAPTOR_CONFIGURED
    } mIspAdaptorState;

    int mCameraId;
    PgParamType mPgParamType;
    TuningMode mTuningMode;

    //Guard for IspParamAdaptor public API
    Mutex mIspAdaptorLock;
    std::map<int, int> mStreamIdToPGOutSizeMap;
    std::map<int, ia_isp_bxt_gdc_limits> mStreamIdToMbrDataMap;
    static const int ISP_PARAM_QUEUE_SIZE = MAX_SETTING_COUNT;
    std::map<int, IspParameter> mStreamIdToIspParameterMap; // map from stream id to IspParameter
    ia_binary_data mLastPalDataForVideoPipe;
    //Guard lock for ipu parameter
    Mutex mIpuParamLock;
    std::unordered_map<int, cca::cca_pal_input_params*> mStreamIdToPalInputParamsMap;

    std::shared_ptr<IGraphConfig> mGraphConfig;
    IntelCca *mIntelCca;

    struct PalRecord {
        int uuid;
        int offset;
    };
    std::vector<PalRecord> mPalRecords;  // Save PAL offset info for overwriting PAL
};

} // namespace icamera
