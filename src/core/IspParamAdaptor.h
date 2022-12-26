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

#ifdef CAL_BUILD
#include <cros-camera/v4l2_device.h>
#else
#include <v4l2_device.h>
#endif

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
// USE_ISA_S
#include "ia_css_isys_parameter_defs.h"
// USE_ISA_E
#endif
}

#ifdef ENABLE_SANDBOXING
#include "modules/sandboxing/client/IntelCcaClient.h"
#else
#include "modules/algowrapper/IntelCca.h"
#endif

#include "3a/AiqResult.h"
#include "ia_aiq_types.h"
#include "ia_isp_bxt_types.h"
#include "ia_isp_bxt_statistics_types.h"
#include "ia_isp_bxt.h"
// DOL_FEATURE_S
#include "ia_bcomp.h"
// DOL_FEATURE_E
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
    int configure(const stream_t &stream, ConfigMode configMode, TuningMode tuningMode,
                  int ipuOutputFormat = -1);

    int getParameters(Parameters& param);
    int decodeStatsData(TuningMode tuningMode,
                        std::shared_ptr<CameraBuffer> statsBuffer,
                        std::shared_ptr<IGraphConfig> graphConfig = nullptr);
    // USE_ISA_S
    int decodeIsaParams(const std::shared_ptr<CameraBuffer> &hwStats,
                        ia_css_isys_kernel_id kernelId, ia_binary_data *statsData);
    int encodeIsaParams(const std::shared_ptr<CameraBuffer> &buf,
                        EncodeBufferType type, long settingSequence = -1);
    int getProcessGroupSize();
    int getInputPayloadSize();
    int getOutputPayloadSize();
    // USE_ISA_E

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

    // HDR_FEATURE_S
    void getHdrExposureInfo(const ia_aiq_ae_results &aeResults,
                            ia_isp_hdr_exposure_info_t* hdrExposureInfo);
    int convertHdrRgbsStatistics(const std::shared_ptr<CameraBuffer> &hdrStats,
                                 const ia_aiq_ae_results &aeResults,
                                 const ia_aiq_color_channels &colorChannels,
                                 ia_aiq_rgbs_grid** rgbsGrid,
                                 ia_aiq_hdr_rgbs_grid** hdrRgbsGrid);
    int convertHdrAfStatistics(const std::shared_ptr<CameraBuffer> &hdrStats,
                               ia_aiq_af_grid** afGrid);
    // HDR_FEATURE_E

    // Dumping methods for debugging purposes.
    void dumpIspParameter(int streamId, long sequence, ia_binary_data binaryData);

    // USE_ISA_S
    int  queryMemoryReqs();
    int convertIsaStatsData(TuningMode tuningMode, std::shared_ptr<CameraBuffer> statsBuffer);
    void dumpP2PContent(const std::shared_ptr<CameraBuffer> &buf,
                        ia_binary_data* pg, EncodeBufferType type);
    // USE_ISA_E
    // Enable or disable kernels according to environment variables for debug purpose.
    void updateKernelToggles(cca::cca_program_group *programGroup);
    void dumpProgramGroup(ia_isp_bxt_program_group *pgPtr);
    void applyMediaFormat(const AiqResult* aiqResult,
                          ia_media_format* mediaFormat, bool* useLinearGamma);
    void dumpCscMatrix(const ia_isp_bxt_csc* cscMatrix);
#ifdef PAL_DEBUG
    void loadPalBinFile(ia_binary_data *binaryData);
#endif
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
    int mIpuOutputFormat;

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

    // USE_ISA_S
    // Process group memory requirements
    ipu_pg_die_t  mP2PWrapper;
    int    mProcessGroupSize;    // Size in bytes required for the Process group descriptor
    int    mInputTerminalsSize;  // Bytes required to store all the input terminal payloads
    int    mOutputTerminalsSize; // Bytes required to store all the output terminal payloads

    struct TerminalPayloadDescriptor {
        int size;         // Size of the terminal payload
        int paddedSize;   // Size of the terminal payload plus padding to meet memory alignment requirements
        uint64_t offset;     // Offset from the base of the payload buffer to the start of the terminal payload
    };
    std::vector<TerminalPayloadDescriptor> mTerminalBuffers;
    // USE_ISA_E
    std::shared_ptr<IGraphConfig> mGraphConfig;
    IntelCca *mIntelCca;

    struct PalRecord {
        int uuid;
        int offset;
    };
    std::vector<PalRecord> mPalRecords;  // Save PAL offset info for overwriting PAL
};

} // namespace icamera
