/*
 * Copyright (C) 2015-2024 Intel Corporation.
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

#define LOG_TAG IspParamAdaptor

#include "IspParamAdaptor.h"

#include <math.h>
#include <stdio.h>
#include <utility>
#include <memory>

#include "3a/AiqResultStorage.h"
#include "iutils/Utils.h"
#include "iutils/CameraLog.h"
#include "iutils/CameraDump.h"
#include "iutils/Errors.h"
#include "PlatformData.h"
#include "IGraphConfig.h"

#include "ia_pal_types_isp_ids_autogen.h"
#include "ia_pal_types_isp_parameters_autogen.h"
#include "ia_pal_types_isp.h"

namespace icamera {

IspParamAdaptor::IspParamAdaptor(int cameraId)
        : mIspAdaptorState(ISP_ADAPTOR_NOT_INIT),
          mCameraId(cameraId),
          mTuningMode(TUNING_MODE_VIDEO),
          mIpuOutputFormat(V4L2_PIX_FMT_NV12),
          mLastLscSequece(-1),
          mLastGdcSequence(-1),
          mGraphConfig(nullptr),
          mIntelCca(nullptr),
          mGammaTmOffset(-1) {
    LOG1("<id%d>@%s", mCameraId, __func__);
    CLEAR(mLastPalDataForVideoPipe);

    PalRecord palRecordArray[] = {{ia_pal_uuid_isp_call_info, -1},
                                  {ia_pal_uuid_isp_bnlm_3_2, -1},
                                  {ia_pal_uuid_isp_lsc_1_1, -1},
                                  {ia_pal_uuid_isp_gdc5, -1}};
    for (uint32_t i = 0; i < sizeof(palRecordArray) / sizeof(PalRecord); i++) {
        mPalRecords.push_back(palRecordArray[i]);
    }
}

IspParamAdaptor::~IspParamAdaptor() {}

int IspParamAdaptor::init() {
    PERF_CAMERA_ATRACE();
    HAL_TRACE_CALL(CAMERA_DEBUG_LOG_LEVEL1);
    AutoMutex l(mIspAdaptorLock);

    mIspAdaptorState = ISP_ADAPTOR_INIT;
    return OK;
}

int IspParamAdaptor::deinit() {
    LOG1("<id%d>@%s", mCameraId, __func__);
    AutoMutex l(mIspAdaptorLock);
    {
        AutoMutex l(mIpuParamLock);
        mStreamIdToPGOutSizeMap.clear();
        releaseIspParamBuffers();
    }

    CLEAR(mLastPalDataForVideoPipe);
    for (uint32_t i = 0; i < mPalRecords.size(); i++) {
        mPalRecords[i].offset = -1;
    }
    mGammaTmOffset = -1;

    mIspAdaptorState = ISP_ADAPTOR_NOT_INIT;
    return OK;
}

int IspParamAdaptor::deepCopyProgramGroup(const ia_isp_bxt_program_group* pgPtr,
                                          cca::cca_program_group* programGroup) {
    CheckAndLogError(!programGroup, UNKNOWN_ERROR, "%s, the programGroup is nullptr", __func__);
    CheckAndLogError(pgPtr->kernel_count > cca::MAX_KERNEL_NUMBERS_IN_PIPE, NO_MEMORY,
                     "%s, memory for program group is too small, kernel count: %d", __func__,
                     pgPtr->kernel_count);

    programGroup->base = *pgPtr;
    uint32_t& kernelCnt = programGroup->base.kernel_count;
    kernelCnt = 0;

    for (unsigned int i = 0; i < pgPtr->kernel_count; ++i) {
        // Skip those kernels with 0 uuid which isn't PAL uuid
        if (pgPtr->run_kernels[i].kernel_uuid == 0) {
            LOG2("There is 0 uuid found, skip it");
            continue;
        }

        programGroup->run_kernels[kernelCnt] = pgPtr->run_kernels[i];
        if (programGroup->run_kernels[kernelCnt].resolution_info) {
            programGroup->resolution_info[kernelCnt] = *pgPtr->run_kernels[i].resolution_info;
            programGroup->run_kernels[kernelCnt].resolution_info =
                &(programGroup->resolution_info[kernelCnt]);
        }

        if (programGroup->run_kernels[kernelCnt].resolution_history) {
            programGroup->resolution_history[kernelCnt] = *pgPtr->run_kernels[i].resolution_history;
            programGroup->run_kernels[kernelCnt].resolution_history =
                &(programGroup->resolution_history[kernelCnt]);
        }

        if (programGroup->base.pipe) {
            programGroup->pipe[kernelCnt] = pgPtr->pipe[i];
        }

        kernelCnt++;
    }

    if (programGroup->base.run_kernels) {
        programGroup->base.run_kernels = programGroup->run_kernels;
    }

    if (programGroup->base.pipe) {
        programGroup->base.pipe = programGroup->pipe;
    }

    LOG2("%s, the kernel count: %d", __func__, kernelCnt);
    return OK;
}

int IspParamAdaptor::getDataFromProgramGroup() {
    HAL_TRACE_CALL(CAMERA_DEBUG_LOG_LEVEL1);
    CheckAndLogError(!mGraphConfig || !mIntelCca, UNKNOWN_ERROR,
                     "%s, mGraphConfig or mIntelCca is nullptr", __func__);

    mStreamIdToPGOutSizeMap.clear();
    mStreamIdToMbrDataMap.clear();

    status_t ret = OK;
    std::vector<int32_t> streamIds;
    ret = mGraphConfig->graphGetStreamIds(streamIds);
    CheckAndLogError(ret != OK, UNKNOWN_ERROR, "Failed to get the PG streamIds");

    for (auto id : streamIds) {
        ia_isp_bxt_program_group* pgPtr = mGraphConfig->getProgramGroup(id);
        CheckAndLogError(!pgPtr, UNKNOWN_ERROR,
                         "%s, Failed to get the programGroup for streamId: %d", __func__, id);

        cca::cca_program_group programGroup = {};
        ret = deepCopyProgramGroup(pgPtr, &programGroup);
        CheckAndLogError(ret != OK, UNKNOWN_ERROR,
                         "%s, Failed to convert cca programGroup. streamId %d", __func__, id);
        mStreamIdToPGOutSizeMap[id] = mIntelCca->getPalDataSize(programGroup);

        ia_isp_bxt_gdc_limits mbrData;
        ret = mGraphConfig->getMBRData(id, &mbrData);
        if (ret == OK) {
            mStreamIdToMbrDataMap[id] = mbrData;
            LOG2("get mbr data for stream:%d:%f,%f,%f,%f", id, mbrData.rectilinear.zoom,
                 mbrData.rectilinear.pitch, mbrData.rectilinear.yaw, mbrData.rectilinear.roll);
        }
    }

    return OK;
}

void IspParamAdaptor::initInputParams(cca::cca_pal_input_params* params) {
    CheckAndLogError(params == nullptr, VOID_VALUE, "NULL input parameter");

    params->ee_setting.feature_level = ia_isp_feature_level_low;
    params->ee_setting.strength = 0;
    params->nr_setting.feature_level = ia_isp_feature_level_high;
    params->nr_setting.strength = 0;
}

/**
 * configure
 * This is the method used when the spatial parameters change, usually during
 * stream configuration.
 *
 * \param stream[IN]: frame info.
 * \param configMode[IN]: The real configure mode.
 * \param tuningMode[IN]:  The tuning mode.
 * \param ipuOutputFormat[IN]: format of output frame
 * \return OK: everything went ok.
 * \return UNKNOWN_ERROR: First run of ISP adaptation failed.
 */
int IspParamAdaptor::configure(const stream_t& stream, ConfigMode configMode, TuningMode tuningMode,
                               int ipuOutputFormat) {
    HAL_TRACE_CALL(CAMERA_DEBUG_LOG_LEVEL1);

    int ret = OK;
    if (!PlatformData::isEnableAIQ(mCameraId)) {
        LOG2("%s, AIQ is disabled", __func__);
        return ret;
    }

    AutoMutex l(mIspAdaptorLock);
    if (ipuOutputFormat != -1) mIpuOutputFormat = ipuOutputFormat;
    LOG2("%s, configMode: %x, PSys output format 0x%x", __func__, configMode, mIpuOutputFormat);
    mTuningMode = tuningMode;
    CLEAR(mLastPalDataForVideoPipe);
    mLastLscSequece = -1;
    mLastGdcSequence = -1;
    for (uint32_t i = 0; i < mPalRecords.size(); i++) {
        mPalRecords[i].offset = -1;
    }
    mGammaTmOffset = -1;

    mIntelCca = IntelCca::getInstance(mCameraId, tuningMode);
    CheckAndLogError(!mIntelCca, UNKNOWN_ERROR, "%s, mIntelCca is nullptr, tuningMode:%d", __func__,
                     mTuningMode);

    IGraphConfigManager* gcm = IGraphConfigManager::getInstance(mCameraId);
    CheckAndLogError(!gcm, UNKNOWN_ERROR, "%s, Failed to get graph config manager for cameraId: %d",
                     __func__, mCameraId);
    CheckAndLogError(!gcm->isGcConfigured(), UNKNOWN_ERROR, "%s, graph isn't configured", __func__);

    mGraphConfig = gcm->getGraphConfig(configMode);
    CheckAndLogError(!mGraphConfig, UNKNOWN_ERROR, "%s, There isn't GraphConfig for configMode: %d",
                     __func__, configMode);

    {
        /*
         * The number of streamId is identified in configure stream,
         * get data from program group and allocate the IspParameter memory
         */
        AutoMutex l(mIpuParamLock);
        ret = getDataFromProgramGroup();
        CheckAndLogError(ret != OK, ret, "%s, Failed to init programGroup for all streams",
                         __func__);
        ret = allocateIspParamBuffers();
        CheckAndLogError(ret != OK, ret, "%s, Failed to allocate isp parameter buffers", __func__);
    }

    if (PlatformData::supportUpdateTuning(mCameraId)) {
        for (auto& ispParamIt : mStreamIdToIspParameterMap) {
            int ispTuningIndex = mGraphConfig->getTuningModeByStreamId(ispParamIt.first);
            // Use the tuning mode in graph to update the isp tuning data
            if (ispTuningIndex != -1) {
                uint8_t lardTag = cca::CCA_LARD_ISP;
                ia_lard_input_params lardParam = {
                    IA_MKN_CHTOUL('D', 'F', 'L', 'T'),
                    IA_MKN_CHTOUL('D', 'F', 'L', 'T'),
                    static_cast<uint32_t>(ispTuningIndex),
                    IA_MKN_CHTOUL('D', 'F', 'L', 'T')
                };
                cca::cca_nvm tmpNvm = {};

                ia_err iaErr =
                    mIntelCca->updateTuning(lardTag, lardParam, tmpNvm, ispParamIt.first);
                CheckAndLogError(iaErr != ia_err_none, UNKNOWN_ERROR,
                                 "%s, Failed to update isp tuning data. tuning_mode %d", __func__,
                                 ispTuningIndex);
                LOG2("%s, Update isp tuning data. tuning_mode:%d, streamId: %d,", __func__,
                     ispTuningIndex, ispParamIt.first);
            }
        }
    }
    /*
     *  IA_ISP_BXT can run without 3A results to produce the defaults for a
     *  given sensor configuration.
     */
    ia_binary_data binaryData = {};
    for (auto& ispParamIt : mStreamIdToIspParameterMap) {
        cca::cca_pal_input_params* inputParams = mStreamIdToPalInputParamsMap[ispParamIt.first];
        inputParams->seq_id = -1;
        initInputParams(inputParams);
        inputParams->stream_id = ispParamIt.first;
        inputParams->dvs_id = inputParams->stream_id;

        ia_isp_bxt_program_group* pgPtr = mGraphConfig->getProgramGroup(ispParamIt.first);
        CheckAndLogError(!pgPtr, UNKNOWN_ERROR,
                         "%s, Failed to get the programGroup for streamId: %d", __func__,
                         ispParamIt.first);

        ret = deepCopyProgramGroup(pgPtr, &(inputParams->program_group));
        CheckAndLogError(ret != OK, UNKNOWN_ERROR,
                         "%s, Failed to convert cca programGroup. streamId %d", __func__,
                         ispParamIt.first);
        dumpProgramGroup(&inputParams->program_group.base);

        {
            AutoMutex l(mIpuParamLock);
            binaryData = ispParamIt.second.mSequenceToDataMap.begin()->second;
        }
        binaryData.size = mStreamIdToPGOutSizeMap[ispParamIt.first];

        PERF_CAMERA_ATRACE_PARAM1_IMAGING("ia_isp_bxt_run", 1);
        ia_err iaErr = mIntelCca->runAIC(-1, inputParams, &binaryData);
        CheckAndLogError(iaErr != ia_err_none && iaErr != ia_err_not_run, UNKNOWN_ERROR,
                         "ISP parameter adaptation has failed %d", iaErr);

        AutoMutex l(mIpuParamLock);
        updateIspParameterMap(&(ispParamIt.second), -1, -1, binaryData);
        ispParamIt.second.mSequenceToDataMap.erase(ispParamIt.second.mSequenceToDataMap.begin());

        dumpIspParameter(ispParamIt.first, 0, binaryData);
    }

    mIspAdaptorState = ISP_ADAPTOR_CONFIGURED;
    return OK;
}

int IspParamAdaptor::decodeStatsData(TuningMode tuningMode,
                                     std::shared_ptr<CameraBuffer> statsBuffer,
                                     int32_t streamId) {
    CheckAndLogError(mIspAdaptorState != ISP_ADAPTOR_CONFIGURED, INVALID_OPERATION,
                     "%s, wrong state %d", __func__, mIspAdaptorState);
    CheckAndLogError(!mIntelCca, UNKNOWN_ERROR, "%s, mIntelCca is nullptr", __func__);

    int64_t sequence = statsBuffer->getSequence();
    LOG2("<seq:%ld>@%s", sequence, __func__);
    cca::cca_out_stats outStatsTemp;
    cca::cca_out_stats* outStats = &outStatsTemp;
    outStats->get_rgbs_stats = false;
    AiqResult* aiqResult =
        const_cast<AiqResult*>(AiqResultStorage::getInstance(mCameraId)->getAiqResult(sequence));
    if (aiqResult && aiqResult->mAiqParam.callbackRgbs) {
        outStats = &aiqResult->mOutStats;
        outStats->get_rgbs_stats = true;
    }

    AiqResultStorage* aiqResultStorage = AiqResultStorage::getInstance(mCameraId);
    AiqStatistics* aiqStatistics = aiqResultStorage->acquireAiqStatistics();
    aiqStatistics->mSequence = sequence;
    aiqStatistics->mTimestamp = TIMEVAL2USECS(statsBuffer->getTimestamp());
    aiqStatistics->mTuningMode = tuningMode;
    aiqStatistics->mPendingDecode = false;
    aiqStatistics->mStreamId = streamId;
    if (PlatformData::isStatsRunningRateSupport(mCameraId) && !outStats->get_rgbs_stats) {
        aiqStatistics->mPendingDecode = true;
    }
    aiqResultStorage->updateAiqStatistics(sequence);

    // Pend stats decoding to running 3A
    if (aiqStatistics->mPendingDecode) return OK;

    ia_binary_data* hwStatsData = (ia_binary_data*)(statsBuffer->getBufferAddr());
    if (CameraDump::isDumpTypeEnable(DUMP_PSYS_DECODED_STAT) && hwStatsData != nullptr) {
        BinParam_t bParam;
        bParam.bType = BIN_TYPE_GENERAL;
        bParam.mType = M_PSYS;
        bParam.sequence = statsBuffer->getSequence();
        bParam.gParam.appendix = "p2p_decoded_stats";
        bParam.sUsage = (streamId == VIDEO_STREAM_ID) ? 0 : 2;
        CameraDump::dumpBinary(mCameraId, hwStatsData->data, hwStatsData->size, &bParam);
    }

    CheckAndLogError(!hwStatsData, UNKNOWN_ERROR, "%s, hwStatsData is nullptr", __func__);

    ia_isp_bxt_statistics_query_results_t queryResults = {};
    uint32_t bitmap = getRequestedStats();
    ia_err iaErr = mIntelCca->decodeStats(reinterpret_cast<uint64_t>(hwStatsData->data),
                                          hwStatsData->size, bitmap, &queryResults, outStats);
    CheckAndLogError(iaErr != ia_err_none, UNKNOWN_ERROR, "%s, Faield convert statistics",
                     __func__);

    return OK;
}

void IspParamAdaptor::updateKernelToggles(cca::cca_program_group* programGroup) {
    if (!Log::isDebugLevelEnable(CAMERA_DEBUG_LOG_KERNEL_TOGGLE)) return;

    const char* ENABLED_KERNELS = "/tmp/enabledKernels";
    const char* DISABLED_KERNELS = "/tmp/disabledKernels";
    const int FLIE_CONT_MAX_LENGTH = 1024;
    char enabledKernels[FLIE_CONT_MAX_LENGTH] = {0};
    char disabledKernels[FLIE_CONT_MAX_LENGTH] = {0};

    int enLen =
        CameraUtils::getFileContent(ENABLED_KERNELS, enabledKernels, FLIE_CONT_MAX_LENGTH - 1);
    int disLen =
        CameraUtils::getFileContent(DISABLED_KERNELS, disabledKernels, FLIE_CONT_MAX_LENGTH - 1);

    if (enLen == 0 && disLen == 0) {
        LOG2("%s: no explicit kernel toggle.", __func__);
        return;
    }

    LOG2("%s: enabled kernels: %s, disabled kernels %s", __func__, enabledKernels, disabledKernels);

    for (unsigned int i = 0; i < programGroup->base.kernel_count; i++) {
        ia_isp_bxt_run_kernels_t* curKernel = &(programGroup->base.run_kernels[i]);
        std::string curKernelUUID = std::to_string(curKernel->kernel_uuid);

        if (strstr(enabledKernels, curKernelUUID.c_str()) != nullptr) {
            curKernel->enable = 1;
        }
        if (strstr(disabledKernels, curKernelUUID.c_str()) != nullptr) {
            curKernel->enable = 0;
        }
    }
}

/*
 * PAL output buffer is a reference data for next output buffer,
 * but currently a ring buffer is used in HAL, which caused logic mismatching issue.
 * So temporarily copy latest PAL data into PAL output buffer.
 */
void IspParamAdaptor::updatePalDataForVideoPipe(ia_binary_data dest, int64_t bufSeq,
                                                int64_t settingSeq) {
    if (mLastPalDataForVideoPipe.data == nullptr || mLastPalDataForVideoPipe.size == 0) {
        mLastGdcSequence = settingSeq;
        return;
    }

    if (mPalRecords.empty()) return;

    ia_pal_record_header* header = nullptr;
    char* src = static_cast<char*>(mLastPalDataForVideoPipe.data);
    // find uuid offset in saved PAL buffer
    if (mPalRecords[0].offset < 0) {
        uint32_t offset = 0;
        while (offset < mLastPalDataForVideoPipe.size) {
            ia_pal_record_header* header = reinterpret_cast<ia_pal_record_header*>(src + offset);
            // check if header is valid or not
            CheckWarning(header->uuid == 0 || header->size == 0, VOID_VALUE,
                         "%s, source header info isn't correct", __func__);
            for (uint32_t i = 0; i < mPalRecords.size(); i++) {
                if (mPalRecords[i].offset < 0 && mPalRecords[i].uuid == header->uuid) {
                    mPalRecords[i].offset = offset;
                    LOG2("find uuid %d, offset %d, size %d", header->uuid, offset, header->size);
                    break;
                }
            }
            offset += header->size;
        }
    }

    char* destData = static_cast<char*>(dest.data);
    ia_pal_record_header* headerSrc = nullptr;
    for (uint32_t i = 0; i < mPalRecords.size(); i++) {
        if (mPalRecords[i].offset >= 0) {
            // find source record header
            header = reinterpret_cast<ia_pal_record_header*>(src + mPalRecords[i].offset);
            if (header->uuid == mPalRecords[i].uuid) {
                headerSrc = header;
            }

            if (ia_pal_uuid_isp_lsc_1_1 == header->uuid) {
                if (isLscCopy(bufSeq, settingSeq)) {
                    LOG2("settingSeq %ld, copy LSC for buf %ld", settingSeq, bufSeq);
                    updateLscSeqMap(bufSeq);
                } else {
                    LOG2("settingSeq %ld, not copy LSC for buf %ld", settingSeq, bufSeq);
                    continue;
                }
            }

            if (ia_pal_uuid_isp_gdc5 == header->uuid) {
                if (isGdcCopy(bufSeq, settingSeq)) {
                    LOG2("settingSeq %ld, copy GDC for buf %ld", settingSeq, bufSeq);
                    updateGdcSeqMap(bufSeq);
                } else {
                    LOG2("settingSeq %ld, not copy GDC for buf %ld", settingSeq, bufSeq);
                    continue;
                }
            }

            if (!headerSrc) {
                LOGW("Failed to find PAL recorder header %d", mPalRecords[i].uuid);
                continue;
            }
            header = reinterpret_cast<ia_pal_record_header*>(destData + mPalRecords[i].offset);
            if (header->uuid == mPalRecords[i].uuid) {
                MEMCPY_S(header, header->size, headerSrc, headerSrc->size);
                LOG2("%s, PAL data of kernel uuid %d has been updated", __func__, header->uuid);
            }
        }
    }
}

void IspParamAdaptor::updateIspParameterMap(IspParameter* ispParam, int64_t dataSeq,
                                            int64_t settingSeq, ia_binary_data binaryData) {
    LOG2("%s, data seq %ld, setting sequence %ld", __func__, dataSeq, settingSeq);

    // if dataSeq doesn't equal to settingSeq, only update sequence map
    if (dataSeq == settingSeq) {
        std::pair<int64_t, ia_binary_data> p(settingSeq, binaryData);
        ispParam->mSequenceToDataMap.insert(p);
    }
    if (ispParam->mSequenceToDataId.size() >= ISP_PARAM_QUEUE_SIZE) {
        ispParam->mSequenceToDataId.erase(ispParam->mSequenceToDataId.begin());
    }
    ispParam->mSequenceToDataId[settingSeq] = dataSeq;
}

bool IspParamAdaptor::isLscCopy(int64_t bufSeq, int64_t settingSeq) {
    AiqResult* aiqResults = const_cast<AiqResult*>(
        AiqResultStorage::getInstance(mCameraId)->getAiqResult(settingSeq));
    if (aiqResults == nullptr) return true;

    if (aiqResults->mLscUpdate) {
        mLastLscSequece = settingSeq;
        LOG2("%s, LSC update %ld", __func__, settingSeq);
        return false;
    } else {
        if (mSeqIdToLscSeqIdMap.find(bufSeq) != mSeqIdToLscSeqIdMap.end()) {
            if (mLastLscSequece >= 0 && mSeqIdToLscSeqIdMap[bufSeq] == mLastLscSequece) {
                return false;
            }
        }
    }

    return true;
}

void IspParamAdaptor::updateLscSeqMap(int64_t settingSeq) {
    mSeqIdToLscSeqIdMap[settingSeq] = mLastLscSequece;

    if (mSeqIdToLscSeqIdMap.size() > ISP_PARAM_QUEUE_SIZE) {
        mSeqIdToLscSeqIdMap.erase(mSeqIdToLscSeqIdMap.begin());
    }
}

bool IspParamAdaptor::isGdcCopy(int64_t bufSeq, int64_t settingSeq) {
    if (!PlatformData::isDvsSupported(mCameraId)) return false;

    if (AiqResultStorage::getInstance(mCameraId)->isDvsRun(settingSeq)) {
        mLastGdcSequence = settingSeq;
        LOG2("%s, GDC update %ld", __func__, settingSeq);
        return false;
    } else {
        if (mSeqIdToGdcSeqIdMap.find(bufSeq) != mSeqIdToGdcSeqIdMap.end()) {
            if (mLastGdcSequence >= 0 && mSeqIdToGdcSeqIdMap[bufSeq] == mLastGdcSequence) {
                return false;
            }
        }
    }

    return true;
}

void IspParamAdaptor::updateGdcSeqMap(int64_t settingSeq) {
    mSeqIdToGdcSeqIdMap[settingSeq] = mLastGdcSequence;

    if (mSeqIdToGdcSeqIdMap.size() > ISP_PARAM_QUEUE_SIZE) {
        mSeqIdToGdcSeqIdMap.erase(mSeqIdToGdcSeqIdMap.begin());
    }
}

/**
 * runIspAdapt
 * Convert the results of the 3A algorithms and parse with P2P.
 */
int IspParamAdaptor::runIspAdapt(const IspSettings* ispSettings, int64_t settingSequence,
                                 int32_t streamId) {
    PERF_CAMERA_ATRACE();
    HAL_TRACE_CALL(CAMERA_DEBUG_LOG_LEVEL2);
    AutoMutex l(mIspAdaptorLock);
    CheckAndLogError(mIspAdaptorState != ISP_ADAPTOR_CONFIGURED, INVALID_OPERATION,
                     "%s, wrong state %d", __func__, mIspAdaptorState);
    CheckAndLogError(!mGraphConfig, UNKNOWN_ERROR, "%s, mGraphConfig is nullptr", __func__);

    for (auto& it : mStreamIdToIspParameterMap) {
        if (streamId != -1 && it.first != streamId) continue;

        ia_binary_data binaryData = {};
        IspParameter* ispParam = &(it.second);
        auto dataIt = ispParam->mSequenceToDataMap.end();

        {
            AutoMutex l(mIpuParamLock);
            // Only one sequence key will be saved if settingSequence is larger than 0
            if (settingSequence >= 0) {
                dataIt = ispParam->mSequenceToDataMap.find(settingSequence);
            }

            if (dataIt == ispParam->mSequenceToDataMap.end()) {
                dataIt = ispParam->mSequenceToDataMap.begin();
            }
            CheckAndLogError(dataIt == ispParam->mSequenceToDataMap.end(), UNKNOWN_ERROR,
                             "No PAL buf!");
            binaryData = dataIt->second;

            LOG2("<seq%ld:streamId%d>@%s, Pal data buffer seq: %ld", settingSequence, it.first,
                 __func__, dataIt->first);
        }

        ia_isp_bxt_gdc_limits* mbrData = nullptr;
        binaryData.size = mStreamIdToPGOutSizeMap[it.first];
        if (mStreamIdToMbrDataMap.find(it.first) != mStreamIdToMbrDataMap.end())
            mbrData = &(mStreamIdToMbrDataMap[it.first]);

        // Update some PAL data to latest PAL result
        if (it.first == VIDEO_STREAM_ID) {
            updatePalDataForVideoPipe(binaryData, dataIt->first, settingSequence);
        }

        ia_isp_bxt_program_group* pgPtr = mGraphConfig->getProgramGroup(it.first);
        CheckAndLogError(!pgPtr, UNKNOWN_ERROR,
                         "%s, Failed to get the programGroup for streamId: %d", __func__, it.first);

        int ret = runIspAdaptL(pgPtr, mbrData, ispSettings, settingSequence, &binaryData, it.first);
        CheckAndLogError(ret != OK, ret, "run isp adaptor error for streamId %d, sequence: %ld",
                         it.first, settingSequence);
        {
            AutoMutex l(mIpuParamLock);
            int64_t dataSequence = settingSequence;
            if (binaryData.size == 0) {
                dataSequence = ispParam->mSequenceToDataMap.rbegin()->first;
                if (streamId == VIDEO_STREAM_ID) {
                    updateResultFromAlgo(&(ispParam->mSequenceToDataMap.rbegin()->second),
                                         settingSequence);
                }
            }
            updateIspParameterMap(ispParam, dataSequence, settingSequence, binaryData);
            if (binaryData.size > 0) {
                ispParam->mSequenceToDataMap.erase(dataIt);

                if (it.first == VIDEO_STREAM_ID) {
                    mLastPalDataForVideoPipe = binaryData;
                    updateResultFromAlgo(&binaryData, settingSequence);
                    updateLscSeqMap(settingSequence);
                    updateGdcSeqMap(settingSequence);
                }
            }
        }
    }

    return OK;
}

ia_binary_data* IspParamAdaptor::getIpuParameter(int64_t sequence, int streamId) {
    AutoMutex l(mIpuParamLock);

    // This is only for getting the default ipu parameter
    if (sequence == -1 && streamId == -1) {
        return &(mStreamIdToIspParameterMap.begin()->second.mSequenceToDataMap.begin()->second);
    }
    CheckAndLogError(streamId == -1, nullptr, "stream id is -1, but seq isn't -1");

    IspParameter& ispParam = mStreamIdToIspParameterMap[streamId];
    ia_binary_data* binaryData = nullptr;
    if (sequence == -1) {
        // get the latest ipu param when sequence is -1
        auto rit = ispParam.mSequenceToDataMap.rbegin();
        for (; rit != ispParam.mSequenceToDataMap.rend(); ++rit) {
            // sequence -1 is valid for default PAL data
            if (rit->first >= -1) {
                binaryData = &(rit->second);
                break;
            }
        }
    } else {
        auto seqIt = ispParam.mSequenceToDataId.find(sequence);
        if (seqIt != ispParam.mSequenceToDataId.end()) {
            auto dataIt = ispParam.mSequenceToDataMap.find(seqIt->second);
            if (dataIt != ispParam.mSequenceToDataMap.end()) binaryData = &(dataIt->second);
        }
    }

    if (!binaryData) {
        LOG1("Failed to find ISP parameter for stream id %d, sequence: %ld", streamId, sequence);
    }

    return binaryData;
}

int IspParamAdaptor::getPalOutputDataSize(const ia_isp_bxt_program_group* programGroup) {
    CheckAndLogError(programGroup == nullptr, 0, "Request programGroup is nullptr");
    CheckAndLogError(!mIntelCca, UNKNOWN_ERROR, "%s, mIntelCca is nullptr", __func__);

    cca::cca_program_group pg = {};
    deepCopyProgramGroup(programGroup, &pg);
    return mIntelCca->getPalDataSize(pg);
}

/*
 * Allocate memory for mIspParameters
 */
int IspParamAdaptor::allocateIspParamBuffers() {
    CheckAndLogError(!mIntelCca, UNKNOWN_ERROR, "%s, mIntelCca is nullptr", __func__);

    releaseIspParamBuffers();
    for (int i = 0; i < ISP_PARAM_QUEUE_SIZE; i++) {
        for (auto& pgMap : mStreamIdToPGOutSizeMap) {
            ia_binary_data binaryData = {};
            int size = pgMap.second;
            binaryData.size = size;
            binaryData.data = mIntelCca->allocMem(pgMap.first, "palData", i, size);
            CheckAndLogError(binaryData.data == nullptr, NO_MEMORY, "Faile to calloc PAL data");
            int64_t index = i * (-1) - 2;  // default index list: -2, -3, -4, ...
            std::pair<int64_t, ia_binary_data> p(index, binaryData);
            mStreamIdToIspParameterMap[pgMap.first].mSequenceToDataMap.insert(p);
        }
    }

    for (auto& pgMap : mStreamIdToPGOutSizeMap) {
        cca::cca_pal_input_params* p = static_cast<cca::cca_pal_input_params*>(mIntelCca->allocMem(
            pgMap.first, "palData", ISP_PARAM_QUEUE_SIZE, sizeof(cca::cca_pal_input_params)));
        CheckAndLogError(p == nullptr, NO_MEMORY, "Cannot alloc memory for cca_pal_input_params!");
        CLEAR(*p);
        mStreamIdToPalInputParamsMap[pgMap.first] = p;
    }

    return OK;
}

void IspParamAdaptor::releaseIspParamBuffers() {
    for (int i = 0; i < ISP_PARAM_QUEUE_SIZE; i++) {
        for (auto& it : mStreamIdToIspParameterMap) {
            for (auto& binaryMap : it.second.mSequenceToDataMap) {
                mIntelCca->freeMem(binaryMap.second.data);
            }

            it.second.mSequenceToDataId.clear();
            it.second.mSequenceToDataMap.clear();
        }
    }

    for (auto& it : mStreamIdToPalInputParamsMap) {
        mIntelCca->freeMem(it.second);
    }
    mStreamIdToPalInputParamsMap.clear();
}

void IspParamAdaptor::applyMediaFormat(const AiqResult* aiqResult, ia_media_format* mediaFormat,
                                       bool* useLinearGamma) {
    CheckAndLogError(!mediaFormat || !aiqResult, VOID_VALUE, "mediaFormat or aiqResult is nullptr");

    if (aiqResult->mAiqParam.tonemapMode == TONEMAP_MODE_GAMMA_VALUE) {
        if (aiqResult->mAiqParam.tonemapGamma == 1.0) {
            *useLinearGamma = true;
            *mediaFormat = media_format_custom;
            LOG2("%s: a linear 1.0 gamma value.", __func__);
        } else {
            LOGW("%s, dynamic gamma value(%f) is not supported", __func__,
                 aiqResult->mAiqParam.tonemapGamma);
        }
    } else if (aiqResult->mAiqParam.tonemapMode == TONEMAP_MODE_CONTRAST_CURVE) {
        const camera_tonemap_curves_t* curves = &aiqResult->mAiqParam.tonemapCurves;
        CheckAndLogError((curves->rSize != curves->gSize || curves->bSize != curves->gSize),
                         VOID_VALUE, "%s, the size of rgb channels must be same", __func__);

        // User's curve is 2-d array: (in, out)
        // Use the gamma curve to select the media format
        float curveX = curves->gCurve[curves->gSize / 2];
        float curveY = curves->gCurve[curves->gSize / 2 + 1];

        if (curveX == curveY) {
            // It's a linear gamma curves(same with gamma = 1.0)
            *useLinearGamma = true;
            *mediaFormat = media_format_custom;
            LOG2("%s: a linear gamma curve. curveX: %f, curveY: %f", __func__, curveX, curveY);
        } else if (abs(curveY - pow(curveX, (1 / 2.2))) < EPSILON) {
            // Its a standard 2_2 gamma curves
            *mediaFormat = media_format_bt709_8b;
            LOG2("%s: a 2.2 gamma curve. curveX: %f, curveY: %f", __func__, curveX, curveY);
        } else {
            LOGW("%s, dynamic gamma curve(%f, %f) is not supported", __func__, curveX, curveY);
        }
    }
}

void IspParamAdaptor::applyCscMatrix(ia_isp_bxt_csc* cscMatrix) {
    size_t matrixCount = sizeof(cscMatrix->rgb2yuv_coef) / sizeof(int32_t);
    // This is one standard RGB2YUV matrix reverse from YUV2RGB matrix in ITS
    const float oriMatrix[] = {0.299, 0.587, 0.114, -0.169, -0.331, 0.5, 0.5, -0.419, -0.081};
    CheckAndLogError(matrixCount != (sizeof(oriMatrix) / sizeof(float)), VOID_VALUE,
                     "Matrix count mismatching with algo: %zu", matrixCount);

    const int cscNorm = 1000;  // this value need to align with algo
    for (size_t i = 0; i < matrixCount; i++) {
        cscMatrix->rgb2yuv_coef[i] = static_cast<int32_t>(oriMatrix[i] * cscNorm);
    }
}

int IspParamAdaptor::runIspAdaptL(ia_isp_bxt_program_group* pgPtr, ia_isp_bxt_gdc_limits* mbrData,
                                  const IspSettings* ispSettings, int64_t settingSequence,
                                  ia_binary_data* binaryData, int streamId) {
    PERF_CAMERA_ATRACE();
    CheckAndLogError(!mIntelCca, UNKNOWN_ERROR, "%s, mIntelCca is nullptr", __func__);

    AiqResult* aiqResults = const_cast<AiqResult*>(
        AiqResultStorage::getInstance(mCameraId)->getAiqResult(settingSequence));
    if (aiqResults == nullptr) {
        LOGW("<seq%ld>@%s: no result! use the latest instead", settingSequence, __func__);
        aiqResults =
            const_cast<AiqResult*>(AiqResultStorage::getInstance(mCameraId)->getAiqResult());
        CheckAndLogError((aiqResults == nullptr), INVALID_OPERATION,
                         "Cannot find available aiq result.");
    }
    LOG2("<id%d:streamId:%d>@%s: aiq result id %ld", mCameraId, streamId, __func__,
         aiqResults->mFrameId);

    cca::cca_pal_input_params* inputParams = mStreamIdToPalInputParamsMap[streamId];
    inputParams->seq_id = settingSequence;

    bool useLinearGamma = false;
    inputParams->media_format = PlatformData::getMediaFormat(mCameraId);
    applyMediaFormat(aiqResults, &inputParams->media_format, &useLinearGamma);
    LOG2("%s, media format: 0x%x, gamma lut size: %d", __func__, inputParams->media_format,
         aiqResults->mGbceResults.gamma_lut_size);

    if (inputParams->media_format == media_format_custom) {
        applyCscMatrix(&inputParams->csc_matrix);
        dumpCscMatrix(&inputParams->csc_matrix);
    }

    if ((VIDEO_STREAM_ID == streamId) && !PlatformData::getRunIspAlways(mCameraId)) {
        inputParams->call_rate_control.mode = ia_isp_call_rate_never_on_converged;
    } else {
        inputParams->call_rate_control.mode = ia_isp_call_rate_always;
        inputParams->force_lsc_update = true;
    }

    int ret = deepCopyProgramGroup(pgPtr, &(inputParams->program_group));
    CheckAndLogError(ret != OK, UNKNOWN_ERROR, "%s, Failed to convert cca programGroup", __func__);
    dumpProgramGroup(&(inputParams->program_group.base));

    // update metadata of runnning kernels
    for (unsigned int i = 0; i < inputParams->program_group.base.kernel_count; i++) {
        switch (inputParams->program_group.base.run_kernels[i].kernel_uuid) {
            case ia_pal_uuid_isp_tnr5_21:
            case ia_pal_uuid_isp_tnr5_22:
            case ia_pal_uuid_isp_tnr5_25:
                inputParams->program_group.base.run_kernels[i].metadata[0] = aiqResults->mSequence;
                LOG2("%s, ia_pal_uuid_isp_tnr5_2x frame count = %d", __func__,
                     inputParams->program_group.base.run_kernels[i].metadata[0]);
                break;
            case ia_pal_uuid_isp_ofa_2_mp:
            case ia_pal_uuid_isp_ofa_2_dp:
            case ia_pal_uuid_isp_ofa_2_ppp:
                // These metadata options map to ofa_format_t
                if (mIpuOutputFormat == V4L2_PIX_FMT_YUYV) {
                    inputParams->program_group.base.run_kernels[i].metadata[1] = 5;
                } else if (mIpuOutputFormat == V4L2_PIX_FMT_P010) {
                    inputParams->program_group.base.run_kernels[i].metadata[1] = 15;
                } else {
                    inputParams->program_group.base.run_kernels[i].metadata[1] = 2;
                }
                break;
            case ia_pal_uuid_isp_bxt_ofa_dp:
            case ia_pal_uuid_isp_bxt_ofa_mp:
            case ia_pal_uuid_isp_bxt_ofa_ppp:
                inputParams->program_group.base.run_kernels[i].metadata[2] =
                    aiqResults->mAiqParam.flipMode;
                LOG2("%s: flip mode set to %d", __func__,
                     inputParams->program_group.base.run_kernels[i].metadata[2]);

                inputParams->program_group.base.run_kernels[i].metadata[3] =
                    aiqResults->mAiqParam.yuvColorRangeMode;
                LOG2("%s: ofa yuv color range mode %d", __func__,
                     inputParams->program_group.base.run_kernels[i].metadata[3]);
                break;
            case ia_pal_uuid_isp_bxt_blc:
            case ia_pal_uuid_isp_b2i_sie_1_1:
            case ia_pal_uuid_isp_gammatm_v3:
                if (aiqResults->mAiqParam.testPatternMode != TEST_PATTERN_OFF) {
                    LOG2("%s: disable kernel(%d) in test pattern mode", __func__,
                         inputParams->program_group.base.run_kernels[i].kernel_uuid);
                    inputParams->program_group.base.run_kernels[i].enable = false;
                }
                break;
            case ia_pal_uuid_isp_bxt_wb:
                if (PlatformData::getSensorAwbEnable(mCameraId)) {
                    LOG2("%s: disable kernel(%d) in sensor awb mode", __func__,
                         inputParams->program_group.base.run_kernels[i].kernel_uuid);
                    inputParams->program_group.base.run_kernels[i].enable = false;
                }
                break;
        }
    }

    // Enable or disable kernels according to environment variables for debug purpose.
    updateKernelToggles(&(inputParams->program_group));
    inputParams->stream_id = inputParams->program_group.base.run_kernels[0].stream_id;

    if (ispSettings) {
        if (VIDEO_STREAM_ID == inputParams->stream_id) {
            inputParams->nr_setting = ispSettings->nrSetting;
            inputParams->ee_setting = ispSettings->eeSetting;
        } else {
            inputParams->nr_setting = ispSettings->nrStillSetting;
            inputParams->ee_setting = ispSettings->eeStillSetting;
        }
        LOG2("%s: ISP NR setting, level: %d, strength: %d", __func__,
             static_cast<int>(inputParams->nr_setting.feature_level),
             static_cast<int>(inputParams->nr_setting.strength));
        LOG2("%s: ISP EE setting, level: %d, strength: %d", __func__,
             inputParams->ee_setting.feature_level, inputParams->ee_setting.strength);

        inputParams->effects = ispSettings->effects;
        inputParams->manual_brightness = ispSettings->manualSettings.manualBrightness;
        inputParams->manual_contrast = ispSettings->manualSettings.manualContrast;
        inputParams->manual_hue = ispSettings->manualSettings.manualHue;
        inputParams->manual_saturation = ispSettings->manualSettings.manualSaturation;

        if (ispSettings->palOverride) {
            CheckAndLogError(ispSettings->palOverride->size > cca::MAX_PAL_TUNING_SIZE, NO_MEMORY,
                             "buffer for pal override is too small, MAX_PAL_TUNING_SIZE:%d,size:%d",
                             cca::MAX_PAL_TUNING_SIZE, ispSettings->palOverride->size);

            LOG2("%s, palOverride size:%d", __func__, ispSettings->palOverride->size);
            MEMCPY_S(inputParams->pal_override.data, cca::MAX_PAL_TUNING_SIZE,
                     ispSettings->palOverride->data, ispSettings->palOverride->size);
            inputParams->pal_override.size = ispSettings->palOverride->size;
        }
    }

    inputParams->custom_controls.count = aiqResults->mCustomControls.count;
    uint32_t cnt = static_cast<uint32_t>(inputParams->custom_controls.count);
    if (cnt > 0) {
        CheckAndLogError(cnt > cca::MAX_CUSTOM_CONTROLS_PARAM_SIZE, UNKNOWN_ERROR,
                         "%s, buffer for custom control[%d] is too small", __func__, cnt);

        MEMCPY_S(inputParams->custom_controls.parameters, cnt,
                 aiqResults->mCustomControls.parameters, cca::MAX_CUSTOM_CONTROLS_PARAM_SIZE);
    }

    inputParams->manual_pa_setting = aiqResults->mPaResults;
    if (aiqResults->mGbceResults.have_manual_settings == true) {
        inputParams->manual_gbce_setting = aiqResults->mGbceResults;
        if (useLinearGamma) {
            inputParams->manual_gbce_setting.gamma_lut_size = 0;
        }
    }

    if (CameraUtils::isUllPsysPipe(mTuningMode)) {
        // The situation that all DG passed to ISP, not sensor.
        if (!PlatformData::isUsingSensorDigitalGain(mCameraId)) {
            inputParams->manual_digital_gain =
                aiqResults->mAeResults.exposures[0].exposure[0].digital_gain;
        }

        // Fine-tune DG passed to ISP if partial ISP DG is needed.
        if (PlatformData::isUsingIspDigitalGain(mCameraId)) {
            inputParams->manual_digital_gain = PlatformData::getIspDigitalGain(
                mCameraId, aiqResults->mAeResults.exposures[0].exposure[0].digital_gain);
        }

        LOG2("%s: set digital gain for ULL pipe: %f", __func__, inputParams->manual_digital_gain);
    } else if (PlatformData::isMultiExposureCase(mCameraId, mTuningMode) &&
               PlatformData::getSensorGainType(mCameraId) == ISP_DG_AND_SENSOR_DIRECT_AG) {
        inputParams->manual_digital_gain =
            aiqResults->mAeResults.exposures[0].exposure[0].digital_gain;

        LOG2("%s: all digital gain is passed to ISP, DG(%ld): %f", __func__, aiqResults->mSequence,
             aiqResults->mAeResults.exposures[0].exposure[0].digital_gain);
    }

    inputParams->dvs_id = streamId;
    ia_err iaErr = ia_err_none;
    {
        PERF_CAMERA_ATRACE_PARAM1_IMAGING("ia_isp_bxt_run", 1);

        // HDR_FEATURE_S
        if (PlatformData::getSensorAeEnable(mCameraId)) {
            inputParams->gain_id_gaic = 1;
        }
        // HDR_FEATURE_E

        iaErr = mIntelCca->runAIC(aiqResults->mFrameId, inputParams, binaryData);
    }
    CheckAndLogError(iaErr != ia_err_none && iaErr != ia_err_not_run, UNKNOWN_ERROR,
                     "ISP parameter adaptation has failed %d", iaErr);

    dumpIspParameter(streamId, settingSequence, *binaryData);

    return OK;
}

void IspParamAdaptor::updateResultFromAlgo(ia_binary_data* binaryData, int64_t sequence) {
    AiqResult* aiqResults =
        const_cast<AiqResult*>(AiqResultStorage::getInstance(mCameraId)->getAiqResult(sequence));
    if (aiqResults == nullptr) {
        LOGW("<seq%ld>@%s: no result! use the latest instead", sequence, __func__);
        aiqResults =
            const_cast<AiqResult*>(AiqResultStorage::getInstance(mCameraId)->getAiqResult());
        CheckAndLogError((aiqResults == nullptr), VOID_VALUE, "Cannot find available aiq result.");
    }

    // update tone map result from pal algo
    if (aiqResults->mAiqParam.callbackTmCurve &&
        aiqResults->mGbceResults.have_manual_settings == false) {
        char* src = static_cast<char*>(binaryData->data);
        if (mGammaTmOffset < 0) {
            uint32_t offset = 0;
            bool foundRes = false;

            while (offset < binaryData->size) {
                ia_pal_record_header* header =
                    reinterpret_cast<ia_pal_record_header*>(src + offset);
                if (header->uuid == ia_pal_uuid_isp_gammatm_v3) {
                    LOG2("src uuid %d, offset %d, size %d", header->uuid, offset, header->size);
                    foundRes = true;
                    break;
                }
                offset += header->size;
            }
            if (!foundRes) return;
            mGammaTmOffset = offset;
        }

        ia_pal_isp_gammatm_v3_t* TM = reinterpret_cast<ia_pal_isp_gammatm_v3_t*>(
            src + mGammaTmOffset + ALIGN_8(sizeof(ia_pal_record_header)));
        uint32_t tmSize = (reinterpret_cast<char*>(&(TM->prog_shift)) -
                           reinterpret_cast<char*>(TM->tm_lut_gen_lut)) /
                          sizeof(int32_t);

        if (aiqResults->mGbceResults.tone_map_lut_size == 0) {
            LOG2("%s, gbce running in bypass mode, reset to max value", __func__);
            aiqResults->mGbceResults.tone_map_lut_size = cca::MAX_TONE_MAP_LUT_SIZE;
        }

        CheckAndLogError(tmSize < aiqResults->mGbceResults.tone_map_lut_size, VOID_VALUE,
                         "memory is mismatch to store tone map from algo");

        LOG2("%s, Tonemap Curve. enable: %d, prog_shift: %d, table size: %u", __func__, TM->enable,
             TM->prog_shift, tmSize);

        const int shiftBase = 1 << TM->prog_shift;
        for (uint32_t i = 0; i < aiqResults->mGbceResults.tone_map_lut_size; i++) {
            aiqResults->mGbceResults.tone_map_lut[i] =
                static_cast<float>(TM->tm_lut_gen_lut[i]) / shiftBase;
        }
    }
}

void IspParamAdaptor::dumpIspParameter(int streamId, int64_t sequence, ia_binary_data binaryData) {
    if (!CameraDump::isDumpTypeEnable(DUMP_PSYS_PAL)) return;

    BinParam_t bParam;
    bParam.bType = BIN_TYPE_GENERAL;
    bParam.mType = M_PSYS;
    bParam.sequence = sequence;
    bParam.gParam.appendix = ("pal_" + std::to_string(streamId)).c_str();
    bParam.sUsage = (streamId == VIDEO_STREAM_ID) ? 0 : 2;
    CameraDump::dumpBinary(mCameraId, binaryData.data, binaryData.size, &bParam);
}

void IspParamAdaptor::dumpProgramGroup(ia_isp_bxt_program_group* pgPtr) {
    if (!Log::isLogTagEnabled(GET_FILE_SHIFT(IspParamAdaptor), CAMERA_DEBUG_LOG_LEVEL3)) return;

    LOG3("the kernel count: %d, run_kernels: %p", pgPtr->kernel_count, pgPtr->run_kernels);
    for (unsigned int i = 0; i < pgPtr->kernel_count; i++) {
        LOG3("kernel uuid: %d, stream_id: %d, enable: %d", pgPtr->run_kernels[i].kernel_uuid,
             pgPtr->run_kernels[i].stream_id, pgPtr->run_kernels[i].enable);

        if (pgPtr->run_kernels[i].resolution_info) {
            LOG3("resolution info: input: %dx%d, output: %dx%d",
                 (pgPtr->run_kernels[i].resolution_info)->input_width,
                 (pgPtr->run_kernels[i].resolution_info)->input_height,
                 (pgPtr->run_kernels[i].resolution_info)->output_width,
                 (pgPtr->run_kernels[i].resolution_info)->output_height);
        }

        if (pgPtr->run_kernels[i].resolution_history) {
            LOG3("resolution history: input: %dx%d, output: %dx%d",
                 (pgPtr->run_kernels[i].resolution_history)->input_width,
                 (pgPtr->run_kernels[i].resolution_history)->input_height,
                 (pgPtr->run_kernels[i].resolution_history)->output_width,
                 (pgPtr->run_kernels[i].resolution_history)->output_height);
        }

        if (pgPtr->pipe) {
            LOG3("pipe info, uuid: %d, flags: %x", pgPtr->pipe[i].kernel_uuid,
                 pgPtr->pipe[i].flags);
        }
    }
}

void IspParamAdaptor::dumpCscMatrix(const ia_isp_bxt_csc* cscMatrix) {
    LOG3("%s, manual rgb2yuv matrix: %d, %d, %d, %d, %d, %d, %d, %d, %d", __func__,
         cscMatrix->rgb2yuv_coef[0], cscMatrix->rgb2yuv_coef[1], cscMatrix->rgb2yuv_coef[2],
         cscMatrix->rgb2yuv_coef[3], cscMatrix->rgb2yuv_coef[4], cscMatrix->rgb2yuv_coef[5],
         cscMatrix->rgb2yuv_coef[6], cscMatrix->rgb2yuv_coef[7], cscMatrix->rgb2yuv_coef[8]);
}

uint32_t IspParamAdaptor::getRequestedStats() {
    uint32_t bitmap = cca::CCA_STATS_RGBS | cca::CCA_STATS_HIST | cca::CCA_STATS_AF |
                      cca::CCA_STATS_YV | cca::CCA_STATS_LTM | cca::CCA_STATS_DVS;

    if (PlatformData::isPdafEnabled(mCameraId)) bitmap |= cca::CCA_STATS_PDAF;

    return bitmap;
}

}  // namespace icamera
