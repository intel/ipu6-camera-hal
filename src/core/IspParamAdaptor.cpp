/*
 * Copyright (C) 2015-2020 Intel Corporation.
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

#define LOG_TAG "IspParamAdaptor"

#include "IspParamAdaptor.h"

#include <stdio.h>
#include <utility>

#include "3a/AiqResult.h"
#include "3a/AiqResultStorage.h"
#include "iutils/Utils.h"
#include "iutils/CameraLog.h"
#include "iutils/CameraDump.h"
#include "iutils/Errors.h"
#include "PlatformData.h"
#include "IGraphConfig.h"

#include "ia_pal_types_isp_ids_autogen.h"
#include "ia_pal_types_isp.h"

namespace icamera {

IspParamAdaptor::IspParamAdaptor(int cameraId, PgParamType type) :
        mIspAdaptorState(ISP_ADAPTOR_NOT_INIT),
        mCameraId(cameraId),
        mPgParamType(type),
        mTuningMode(TUNING_MODE_VIDEO),
        mIspAdaptHandle(nullptr),
        mBCompResults(nullptr),
        mGCM(nullptr),
        mAdaptor(nullptr)
{
    LOG1("IspParamAdaptor was created for id:%d type:%d", mCameraId, mPgParamType);
    CLEAR(mFrameParam);
    CLEAR(mLastPalDataForVideoPipe);

    if (PlatformData::getGraphConfigNodes(cameraId)) {
        mGCM = IGraphConfigManager::getInstance(cameraId);
    }

    mAdaptor = std::unique_ptr<IntelIspParamAdaptor>(new IntelIspParamAdaptor());

    PalRecord palRecordArray[] = {
        { ia_pal_uuid_isp_call_info, -1 },
        { ia_pal_uuid_isp_bnlm_3_2, -1  },
        { ia_pal_uuid_isp_lsc_1_1, -1   }
    };
    for (uint32_t i = 0; i < sizeof(palRecordArray) / sizeof(PalRecord); i++) {
        mPalRecords.push_back(palRecordArray[i]);
    }
}

IspParamAdaptor::~IspParamAdaptor()
{
    LOG1("IspParamAdaptor was created for id:%d type:%d", mCameraId, mPgParamType);
}

int IspParamAdaptor::init()
{
    PERF_CAMERA_ATRACE();
    HAL_TRACE_CALL(CAMERA_DEBUG_LOG_LEVEL1);
    AutoMutex l(mIspAdaptorLock);

    mIspAdaptorState = ISP_ADAPTOR_INIT;
    return OK;
}

int IspParamAdaptor::deinit()
{
    LOG1("ISP HW param adaptor de-initialized for camera id:%d type:%d", mCameraId, mPgParamType);
    AutoMutex l(mIspAdaptorLock);

    deinitIspAdaptHandle();

    {
        AutoMutex l(mIpuParamLock);

        // Release the memory and clear the mapping
        for (auto& pgMap: mStreamIdToProgramGroupMap) {
            delete[] pgMap.second.run_kernels;
        }
        mStreamIdToProgramGroupMap.clear();
        mStreamIdToPGOutSizeMap.clear();
        releaseIspParamBuffers();
    }

    CLEAR(mFrameParam);
    CLEAR(mLastPalDataForVideoPipe);
    for (uint32_t i = 0; i < mPalRecords.size(); i++) {
        mPalRecords[i].offset = -1;
    }

    mIspAdaptorState = ISP_ADAPTOR_NOT_INIT;
    return OK;
}

int IspParamAdaptor::initIspAdaptHandle(ConfigMode configMode, TuningMode tuningMode)
{
    int ret = OK;

    if (!PlatformData::isEnableAIQ(mCameraId)) {
        return ret;
    }

    ia_binary_data ispData;
    ia_cmc_t *cmcData = nullptr;
    uintptr_t cmcHandle = reinterpret_cast<uintptr_t>(nullptr);

    ret = PlatformData::getCpfAndCmc(mCameraId, &ispData, nullptr, nullptr,
                                     &cmcHandle, tuningMode, &cmcData);
    CheckError(ret != OK, NO_INIT, "get cpf and cmc data failed");

    int statsNum = PlatformData::getExposureNum(mCameraId,
                                                CameraUtils::isMultiExposureCase(tuningMode));
    mIspAdaptHandle = mAdaptor->init(&ispData, reinterpret_cast<ia_cmc_t*>(cmcHandle),
                                     MAX_STATISTICS_WIDTH, MAX_STATISTICS_HEIGHT,
                                     statsNum, nullptr);
    CheckError(!mIspAdaptHandle, NO_INIT, "ISP adaptor failed to initialize");

    /*
     * The number of streamId is identified in configure stream,
     * fill the mStreamIdToProgramGroupMap and allocate the IspParameter memory
     */
    if (mGCM != nullptr && mGCM->isGcConfigured()) {
        AutoMutex l(mIpuParamLock);

        ret = initProgramGroupForAllStreams(configMode);
        CheckError(ret != OK, ret, "%s, Failed to init programGroup for all streams", __func__);
        ret = allocateIspParamBuffers();
        CheckError(ret != OK, ret, "%s, Failed to allocate isp parameter buffers", __func__);
    }

    LOG1("ISP HW param adaptor initialized successfully camera id:%d", mCameraId);

    return ret;
}

void IspParamAdaptor::deinitIspAdaptHandle()
{
    if (mIspAdaptHandle) {
        mAdaptor->deInit(mIspAdaptHandle);
        mIspAdaptHandle = nullptr;
    }

}

int IspParamAdaptor::initProgramGroupForAllStreams(ConfigMode configMode)
{
    HAL_TRACE_CALL(CAMERA_DEBUG_LOG_LEVEL1);

    std::vector<int32_t> streamIds;

    //Release the memory and clear the mapping
    for (auto& pgMap: mStreamIdToProgramGroupMap) {
        delete[] pgMap.second.run_kernels;
    }
    mStreamIdToProgramGroupMap.clear();
    mStreamIdToPGOutSizeMap.clear();
    mStreamIdToMbrDataMap.clear();

    std::shared_ptr<IGraphConfig> graphConfig = mGCM->getGraphConfig(configMode);
    if(graphConfig == nullptr) {
        LOGW("There isn't GraphConfig for camera configMode: %d", configMode);
        return UNKNOWN_ERROR;
    }

    if (mPgParamType == PG_PARAM_ISYS) {
        int streamId = 0; // 0 is for PG_PARAM_ISYS
        streamIds.push_back(streamId);
    } else {
        status_t ret = graphConfig->graphGetStreamIds(streamIds);
        CheckError(ret != OK, UNKNOWN_ERROR, "Failed to get the PG streamIds");
    }

    for (auto id : streamIds) {
        ia_isp_bxt_program_group *pgPtr = graphConfig->getProgramGroup(id);
        if (pgPtr != nullptr) {
            ia_isp_bxt_program_group programGroup;
            CLEAR(programGroup);
            programGroup.run_kernels = new ia_isp_bxt_run_kernels_t[pgPtr->kernel_count];

            // Skip those kernels with 0 uuid which isn't PAL uuid
            for (unsigned int i = 0; i < pgPtr->kernel_count; i++) {
                if (pgPtr->run_kernels[i].kernel_uuid != 0) {
                    MEMCPY_S(&programGroup.run_kernels[programGroup.kernel_count],
                             sizeof(ia_isp_bxt_run_kernels_t),
                             &pgPtr->run_kernels[i],
                             sizeof(ia_isp_bxt_run_kernels_t));
                    programGroup.kernel_count++;
                } else {
                    LOG1("There is 0 uuid found, stream id %d", id);
                }
            }

            // Override the stream id in kernel list with the one in sensor's config file.
            // Remove this after the sensor's tuning file uses correct stream id.
            int streamId = PlatformData::getStreamIdByConfigMode(mCameraId, configMode);
            if (streamId != -1) {
                programGroup.run_kernels->stream_id = streamId;
            }

            mStreamIdToProgramGroupMap[id] = programGroup;
            mStreamIdToPGOutSizeMap[id] = mAdaptor->getPalDataSize(&programGroup);
            ia_isp_bxt_gdc_limits mbrData;
            status_t ret  = graphConfig->getMBRData(id, &mbrData);
            if (ret == OK) {
                mStreamIdToMbrDataMap[id] = mbrData;
                LOG2("get mbr data for stream:%d:%f,%f,%f,%f",
                     id, mbrData.rectilinear.zoom, mbrData.rectilinear.pitch,
                     mbrData.rectilinear.yaw, mbrData.rectilinear.roll);
            }
        }
    }

    return OK;
}

void IspParamAdaptor::initInputParams(ia_isp_bxt_input_params_v2 *params, PgParamType type)
{
    CheckError(params == nullptr, VOID_VALUE, "NULL input parameter");

    if (type == PG_PARAM_PSYS_ISA) {
        params->ee_setting.feature_level = ia_isp_feature_level_low;
        params->ee_setting.strength = 0;
        LOG2("%s: set initial default edge enhancement setting: level: %d, strengh: %d",
            __func__, params->ee_setting.feature_level, params->ee_setting.strength);

        params->nr_setting.feature_level = ia_isp_feature_level_high;
        params->nr_setting.strength = 0;
        LOG2("%s: set initial default noise setting: level: %d, strengh: %d",
            __func__, params->nr_setting.feature_level, params->nr_setting.strength);
    }
}

int IspParamAdaptor::postConfigure(int width, int height, ia_binary_data *binaryData)
{
    // The PG wrapper init is done by the imaging controller.
    if(mPgParamType == PG_PARAM_PSYS_ISA) {
        mIspAdaptorState = ISP_ADAPTOR_CONFIGURED;
        return OK; //No need to do anything for P2P. It id done by libiacss
    }

    return OK;
}

/**
 * configure
 *
 * (graph config version)
 * This is the method used when the spatial parameters change, usually during
 * stream configuration.
 *
 * We initialize the ISP adaptor to produce worst case scenario for memory
 * allocation.
 *
 * At this state we initialize the wrapper code that helps encoding the PG
 * descriptor and terminal payloads (i.e. the parameters for the PG).
 *
 * \param configMode[IN]: The real configure mode.
 * \param tuningMode[IN]:  The tuning mode.
 * \param stream[IN]: frame info.
 * \return OK: everything went ok.
 * \return UNKNOWN_ERROR: First run of ISP adaptation failed.
 * \return NO_INIT: Initialization of P2P or PG_DIE wrapper failed.
 */
int IspParamAdaptor::configure(const stream_t &stream,
        ConfigMode configMode, TuningMode tuningMode)
{
    HAL_TRACE_CALL(CAMERA_DEBUG_LOG_LEVEL1);
    AutoMutex l(mIspAdaptorLock);

    mTuningMode = tuningMode;
    CLEAR(mLastPalDataForVideoPipe);
    for (uint32_t i = 0; i < mPalRecords.size(); i++) {
        mPalRecords[i].offset = -1;
    }

    ia_isp_bxt_input_params_v2 inputParams;
    CLEAR(inputParams);
    ia_aiq_sa_results_v1 fakeSaResults;
    CLEAR(fakeSaResults);

    deinitIspAdaptHandle();
    int ret = initIspAdaptHandle(configMode, tuningMode);
    CheckError(ret != OK, ret, "%s, init Isp Adapt Handle failed %d", __func__, ret);

    SensorFrameParams param;
    int status = PlatformData::calculateFrameParams(mCameraId, param);
    CheckError(status != OK, status, "%s: Failed to calculate frame params", __func__);
    AiqUtils::convertToAiqFrameParam(param, mFrameParam);

    LOG1("horizontal_crop_offset:%d", mFrameParam.horizontal_crop_offset);
    LOG1("vertical_crop_offset:%d", mFrameParam.vertical_crop_offset);
    LOG1("cropped_image_width:%d", mFrameParam.cropped_image_width);
    LOG1("cropped_image_height:%d", mFrameParam.cropped_image_height);
    LOG1("horizontal_scaling_numerator:%d", mFrameParam.horizontal_scaling_numerator);
    LOG1("horizontal_scaling_denominator:%d", mFrameParam.horizontal_scaling_denominator);
    LOG1("vertical_scaling_numerator:%d", mFrameParam.vertical_scaling_numerator);
    LOG1("vertical_scaling_denominator:%d", mFrameParam.vertical_scaling_denominator);

    /*
     * Construct the dummy Shading Adaptor  results to force the creation of
     * the LSC table.
     * Assign them to the AIC input parameter structure.
     */
    unsigned short fakeLscTable[4] = {1,1,1,1};
    for (int i = 0; i < MAX_BAYER_ORDER_NUM; i++) {
        for (int j = 0; j < MAX_BAYER_ORDER_NUM; j++) {
            fakeSaResults.lsc_grid[i][j] = fakeLscTable;
        }
    }
    fakeSaResults.fraction_bits = 0;
    fakeSaResults.color_order = cmc_bayer_order_grbg;
    fakeSaResults.lsc_update = true;
    fakeSaResults.width = 2;
    fakeSaResults.height = 2;
    inputParams.sa_results = &fakeSaResults;

    initInputParams(&inputParams, mPgParamType);

    /*
     *  IA_ISP_BXT can run without 3A results to produce the defaults for a
     *  given sensor configuration.
     */
    ia_binary_data binaryData = {};
    for (auto& ispParamIt : mStreamIdToIspParameterMap) {
        inputParams.program_group = &(mStreamIdToProgramGroupMap[ispParamIt.first]);
        inputParams.sensor_frame_params = &mFrameParam;
        {
            AutoMutex l(mIpuParamLock);
            binaryData = ispParamIt.second.mSequenceToDataMap.begin()->second;
        }
        binaryData.size = mStreamIdToPGOutSizeMap[ispParamIt.first];

        PERF_CAMERA_ATRACE_PARAM1_IMAGING("ia_isp_bxt_run", 1);

        int ret = mAdaptor->runPal(mIspAdaptHandle, &inputParams, &binaryData);
        CheckError(ret != OK, UNKNOWN_ERROR, "ISP parameter adaptation has failed %d", ret);

        AutoMutex l(mIpuParamLock);
        updateIspParameterMap(&(ispParamIt.second), -1, -1, binaryData);
        ispParamIt.second.mSequenceToDataMap.erase(ispParamIt.second.mSequenceToDataMap.begin());
    }

    dumpIspParameter(0, binaryData);

    return postConfigure(stream.width, stream.height, &binaryData);
}

int IspParamAdaptor::getParameters(Parameters& param)
{
    AutoMutex l(mIspAdaptorLock);

    return OK;
}

int IspParamAdaptor::decodeStatsData(TuningMode tuningMode,
                                     std::shared_ptr<CameraBuffer> statsBuffer,
                                     std::shared_ptr<IGraphConfig> graphConfig)
{
    CheckError(mIspAdaptorState != ISP_ADAPTOR_CONFIGURED,
               INVALID_OPERATION, "%s, wrong state %d", __func__, mIspAdaptorState);

    long sequence = statsBuffer->getSequence();
    AiqResultStorage *aiqResultStorage = AiqResultStorage::getInstance(mCameraId);

    const AiqResult *feedback = aiqResultStorage->getAiqResult(sequence);
    if (feedback == nullptr) {
        LOGW("No aiq result of sequence %ld! Use the latest instead", sequence);
        feedback = aiqResultStorage->getAiqResult();
    }

    camera_resolution_t dvsInReso = {};
    if (graphConfig) {
        ia_isp_bxt_resolution_info_t resolution = {};
        uint32_t gdcKernelId;
        graphConfig->getGdcKernelSetting(&gdcKernelId, &resolution);
        dvsInReso.width = resolution.input_width;
        dvsInReso.height = resolution.input_height;
    }

    ia_binary_data *hwStatsData = (ia_binary_data *)(statsBuffer->getBufferAddr());
    ConvertInputParam inputParams = {CameraUtils::isMultiExposureCase(tuningMode),
                                     hwStatsData, &dvsInReso, &feedback->mAeResults, mBCompResults};

    ConvertResult result;
    ia_isp_bxt_statistics_query_results_t queryResults;
    CLEAR(result);
    CLEAR(queryResults);
    result.queryResults = &queryResults;

    int ret = mAdaptor->queryAndConvertStats(mIspAdaptHandle, &inputParams, &result);
    CheckError(ret != OK, ret, "%s, Faield to query and convert statistics", __func__);

    // Decode DVS statistics
    if (queryResults.dvs_stats) {
        if (CameraDump::isDumpTypeEnable(DUMP_PSYS_DECODED_STAT) && hwStatsData != nullptr) {
            BinParam_t bParam;
            bParam.bType = BIN_TYPE_GENERAL;
            bParam.mType = M_PSYS;
            bParam.sequence = statsBuffer->getSequence();
            bParam.gParam.appendix = "dvs_p2p_decoded_stats";
            CameraDump::dumpBinary(mCameraId, hwStatsData->data, hwStatsData->size, &bParam);
        }

        if (result.dvsStats) {
            DvsStatistics dvsStatsStorage(result.dvsStats, statsBuffer->getSequence());
            aiqResultStorage->updateDvsStatistics(dvsStatsStorage);
        } else {
            LOGW("Failed to get GDC kernel setting, DVS stats not decoded");
        }
    }

    if (queryResults.rgbs_grid && queryResults.af_grid) {
        int exposureNum = PlatformData::getExposureNum(mCameraId, false);
        AiqStatistics *aiqStatistics = aiqResultStorage->acquireAiqStatistics();

        if (*(result.rgbsGrid)) {
            dumpRgbsStats(*(result.rgbsGrid),
                          statsBuffer->getSequence(), feedback->mAeResults.num_exposures);
        }
        if (result.afGrid) {
            dumpAfStats(result.afGrid, statsBuffer->getSequence());
        }

        aiqStatistics->saveRgbsGridData(result.rgbsGrid, exposureNum);
        aiqStatistics->saveAfGridData(result.afGrid);
        aiqStatistics->mSequence = sequence;
        aiqStatistics->mTimestamp = TIMEVAL2USECS(statsBuffer->getTimestamp());
        aiqStatistics->mTuningMode = tuningMode;
        aiqResultStorage->updateAiqStatistics(sequence);
    }

    return OK;
}

void IspParamAdaptor::updateKernelToggles(ia_isp_bxt_program_group programGroup) {

    if (!Log::isDebugLevelEnable(CAMERA_DEBUG_LOG_KERNEL_TOGGLE)) return;

    const char* ENABLED_KERNELS = "/tmp/enabledKernels";
    const char* DISABLED_KERNELS = "/tmp/disabledKernels";
    const int FLIE_CONT_MAX_LENGTH = 1024;
    ia_isp_bxt_run_kernels_t* curKernel = programGroup.run_kernels;
    char enabledKernels[FLIE_CONT_MAX_LENGTH] = { 0 };
    char disabledKernels[FLIE_CONT_MAX_LENGTH] = { 0 };

    int enLen = CameraUtils::getFileContent(ENABLED_KERNELS, enabledKernels, FLIE_CONT_MAX_LENGTH - 1);
    int disLen = CameraUtils::getFileContent(DISABLED_KERNELS, disabledKernels, FLIE_CONT_MAX_LENGTH - 1);

    if (enLen == 0 && disLen == 0) {
        LOG2("%s: no explicit kernel toggle.", __func__);
        return;
    }

    LOG2("%s: enabled kernels: %s, disabled kernels %s", __func__,
        enabledKernels, disabledKernels);

    for (unsigned int i = 0; i < programGroup.kernel_count; i++) {

        std::string curKernelUUID = std::to_string(curKernel->kernel_uuid);

        LOG2("%s: checking kernel %s", __func__, curKernelUUID.c_str());

        if (strstr(enabledKernels, curKernelUUID.c_str()) != nullptr) {
            curKernel->enable = 1;
            LOG2("%s: kernel %d is explicitly enabled", __func__,
                curKernel->kernel_uuid);
        }

        if (strstr(disabledKernels, curKernelUUID.c_str()) != nullptr) {
            curKernel->enable = 0;
            LOG2("%s: kernel %d is explicitly disabled", __func__,
                curKernel->kernel_uuid);
        }

        curKernel ++;
    }
}

/*
 * PAL output buffer is a reference data for next output buffer,
 * but currently a ring buffer is used in HAL, which caused logic mismatching issue.
 * So temporarily copy lastest PAL datas into PAL output buffer.
 */
void IspParamAdaptor::updatePalDataForVideoPipe(ia_binary_data dest)
{
    if (mLastPalDataForVideoPipe.data == nullptr || mLastPalDataForVideoPipe.size == 0)
        return;

    if (mPalRecords.empty()) return;

    ia_pal_record_header *header = nullptr;
    char* src = static_cast<char*>(mLastPalDataForVideoPipe.data);
    // find uuid offset in saved PAL buffer
    if (mPalRecords[0].offset < 0) {
        uint32_t offset = 0;
        while (offset < mLastPalDataForVideoPipe.size) {
            ia_pal_record_header *header = reinterpret_cast<ia_pal_record_header*>(src + offset);
            // check if header is valid or not
            if (header->uuid == 0 || header->size == 0) {
                LOGW("%s, source header info isn't correct", __func__);
                return;
            }

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
    ia_pal_record_header *headerSrc = nullptr;
    for (uint32_t i = 0; i < mPalRecords.size(); i++) {
        if (mPalRecords[i].offset >= 0) {
            // find source record header
            header = reinterpret_cast<ia_pal_record_header*>(src + mPalRecords[i].offset);
            if (header->uuid == mPalRecords[i].uuid) {
                headerSrc = header;
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
                                            int64_t settingSeq, ia_binary_data binaryData)
{
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

/**
 * runIspAdapt
 * Convert the results of the 3A algorithms and parse with P2P.
 */
int IspParamAdaptor::runIspAdapt(const IspSettings* ispSettings, long settingSequence, int32_t streamId)
{
    PERF_CAMERA_ATRACE();
    HAL_TRACE_CALL(CAMERA_DEBUG_LOG_LEVEL2);
    AutoMutex l(mIspAdaptorLock);
    CheckError(mIspAdaptorState != ISP_ADAPTOR_CONFIGURED, INVALID_OPERATION, "%s, wrong state %d",
          __func__, mIspAdaptorState);

    for (auto& it : mStreamIdToIspParameterMap) {
        if (streamId != -1 && it.first != streamId) continue;

        ia_binary_data binaryData = {};
        IspParameter *ispParam = &(it.second);
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
            CheckError(dataIt == ispParam->mSequenceToDataMap.end(), UNKNOWN_ERROR, "No PAL buf!");
            binaryData = dataIt->second;

            LOG2("%s, PAL data buffer seq:%ld, for sequence: %ld, stream %d",
                 __func__, dataIt->first, settingSequence, it.first);
        }

        ia_isp_bxt_gdc_limits* mbrData = nullptr;
        binaryData.size = mStreamIdToPGOutSizeMap[it.first];
        if (mStreamIdToMbrDataMap.find(it.first) != mStreamIdToMbrDataMap.end())
            mbrData = &(mStreamIdToMbrDataMap[it.first]);

        // Update some PAL data to lastest PAL result
        if (it.first == VIDEO_STREAM_ID) {
            updatePalDataForVideoPipe(binaryData);
        }
        int ret = runIspAdaptL(mStreamIdToProgramGroupMap[it.first], mbrData,
                               ispSettings, settingSequence, &binaryData, it.first);
        CheckError(ret != OK, ret, "run isp adaptor error for streamId %d, sequence: %ld",
                   it.first, settingSequence);

        {
            AutoMutex l(mIpuParamLock);
            int64_t dataSequence = settingSequence;
            if (binaryData.size == 0) {
                dataSequence = ispParam->mSequenceToDataMap.rbegin()->first;
            }

            updateIspParameterMap(ispParam, dataSequence, settingSequence, binaryData);
            if (binaryData.size > 0) {
                ispParam->mSequenceToDataMap.erase(dataIt);

                if (it.first == VIDEO_STREAM_ID) {
                    mLastPalDataForVideoPipe = binaryData;
                }
            }
        }
    }

    // Only Store the new sequence
    LOG2("%s, the sequence list size: %zu", __func__, mSequenceList.size());
    if (mSequenceList.size() >= PlatformData::getMaxRawDataNum(mCameraId)) {
        mSequenceList.pop_front();
    }
    mSequenceList.push_back(settingSequence);

    return OK;
}

ia_binary_data* IspParamAdaptor::getIpuParameter(long sequence, int streamId)
{
    AutoMutex l(mIpuParamLock);

    // This is only for getting the default ipu parameter
    if (sequence == -1 && streamId == -1) {
        return &(mStreamIdToIspParameterMap.begin()->second.mSequenceToDataMap.begin()->second);
    }
    CheckError(streamId == -1, nullptr, "stream id is -1, but seq isn't -1");

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
        auto seqIt =ispParam.mSequenceToDataId.find(sequence);
        if (seqIt != ispParam.mSequenceToDataId.end()) {
            auto dataIt = ispParam.mSequenceToDataMap.find(seqIt->second);
            if (dataIt != ispParam.mSequenceToDataMap.end())
                binaryData = &(dataIt->second);
        }
    }

    if (!binaryData) {
        LOG1("Failed to find ISP parameter for stream id %d, sequence: %ld", streamId, sequence);
    }

    return binaryData;
}

int IspParamAdaptor::getPalOutputDataSize(const ia_isp_bxt_program_group* programGroup)
{
    CheckError(programGroup == nullptr, 0, "Request programGroup is nullptr");
    return mAdaptor->getPalDataSize(const_cast<ia_isp_bxt_program_group*>(programGroup));
}

/*
 * Allocate memory for mSequenceToDataMap
 * TODO: Let PAL to expose the max ia_binary_data buffer size which
 * come from mIspAdaptHandle->ia_pal.m_output_isp_parameters_size
 */
int IspParamAdaptor::allocateIspParamBuffers()
{
    releaseIspParamBuffers();

    for (int i = 0; i < ISP_PARAM_QUEUE_SIZE; i++) {
        for (auto& pgMap : mStreamIdToProgramGroupMap) {
            ia_binary_data binaryData = {};
            int size = mStreamIdToPGOutSizeMap[pgMap.first];
            binaryData.size = size;
            binaryData.data = mAdaptor->allocatePalBuffer(pgMap.first, i, size);
            CheckError(binaryData.data == nullptr, NO_MEMORY, "Faile to calloc PAL data");
            int64_t index = i * (-1) - 2; // default index list: -2, -3, -4, ...
            std::pair<int64_t, ia_binary_data> p(index, binaryData);
            mStreamIdToIspParameterMap[pgMap.first].mSequenceToDataMap.insert(p);
        }
    }

    return OK;
}

void IspParamAdaptor::releaseIspParamBuffers()
{
    for (int i = 0; i < ISP_PARAM_QUEUE_SIZE; i++) {
        for (auto& it : mStreamIdToIspParameterMap) {
            for (auto& binaryMap : it.second.mSequenceToDataMap) {
                mAdaptor->freePalBuffer(binaryMap.second.data);
            }

            it.second.mSequenceToDataId.clear();
            it.second.mSequenceToDataMap.clear();
        }
    }
}

int IspParamAdaptor::runIspAdaptL(ia_isp_bxt_program_group programGroup,
                                  ia_isp_bxt_gdc_limits *mbrData,
                                  const IspSettings* ispSettings, long settingSequence,
                                  ia_binary_data *binaryData, int32_t streamId)
{
    PERF_CAMERA_ATRACE_IMAGING();
    AiqResult* aiqResults = const_cast<AiqResult*>(AiqResultStorage::getInstance(mCameraId)->getAiqResult(settingSequence));
    if (aiqResults == nullptr) {
        LOGW("%s: no result for sequence %ld! use the latest instead", __func__, settingSequence);
        aiqResults = const_cast<AiqResult*>(AiqResultStorage::getInstance(mCameraId)->getAiqResult());
        CheckError((aiqResults == nullptr), INVALID_OPERATION, "Cannot find available aiq result.");
    }
    CheckError((aiqResults->mSaResults.width * aiqResults->mSaResults.height == 0),
            INVALID_OPERATION, "No invalid aiq result needed to run Generic AIC");

    LOG2("%s: device type: %d", __func__, mPgParamType);

    ia_isp_bxt_input_params_v2 inputParams;
    ia_view_config_t viewConfig;
    CLEAR(inputParams);
    CLEAR(viewConfig);

    // LOCAL_TONEMAP_S
    bool hasLtm = PlatformData::isLtmEnabled(mCameraId);

    if (hasLtm) {
        size_t ltmLag = PlatformData::getLtmGainLag(mCameraId);
        long ltmSequence = settingSequence;

        // Consider there may be skipped frames, so according to the gain lag and current
        // sequence to find the actual ltm sequence in history list.
        if (mSequenceList.size() > ltmLag) {
            size_t index = 0;
            for(auto iter = mSequenceList.begin(); iter != mSequenceList.end(); iter++) {
                if (*iter == settingSequence && index >= ltmLag) {
                    ltmSequence = *(std::prev(iter, ltmLag));
                    break;
                }
                index++;
            }
        }
        ltm_result_t* ltmResult = const_cast<ltm_result_t*>(AiqResultStorage::getInstance(mCameraId)->getLtmResult(ltmSequence));
        if (ltmResult != nullptr) {
            LOG2("%s: frame sequence %ld, ltm sequence %ld, actual sequence: %ld",
                    __func__, settingSequence, ltmSequence, ltmResult->sequence);
            inputParams.ltm_results = &ltmResult->ltmResults;
            inputParams.ltm_drc_params = &ltmResult->ltmDrcParams;
        }
    }
    // LOCAL_TONEMAP_E

    // update metadata of runnning kernels
    if (mPgParamType == PG_PARAM_PSYS_ISA) {
        for (unsigned int i=0; i<programGroup.kernel_count; i++) {
            switch (programGroup.run_kernels[i].kernel_uuid) {
            case ia_pal_uuid_isp_tnr5_21:
            case ia_pal_uuid_isp_tnr5_22:
            case ia_pal_uuid_isp_tnr5_25:
                programGroup.run_kernels[i].metadata[0] = aiqResults->mSequence;
                LOG2("ia_pal_uuid_isp_tnr5_2x frame count = %d", programGroup.run_kernels[i].metadata[0]);
                break;
            case ia_pal_uuid_isp_bxt_ofa_dp:
            case ia_pal_uuid_isp_bxt_ofa_mp:
            case ia_pal_uuid_isp_bxt_ofa_ppp:
                programGroup.run_kernels[i].metadata[2] = aiqResults->mAiqParam.flipMode;
                LOG2("%s: flip mode set to %d", __func__, programGroup.run_kernels[i].metadata[2]);

                programGroup.run_kernels[i].metadata[3] = aiqResults->mAiqParam.yuvColorRangeMode;
                LOG2("ofa yuv color range mode %d", programGroup.run_kernels[i].metadata[3]);
                break;
            case ia_pal_uuid_isp_gammatm_v3:
                // Bypass the GammaTM for manual color transform awb mode
                programGroup.run_kernels[i].enable =
                    (aiqResults->mAiqParam.awbMode == AWB_MODE_MANUAL_COLOR_TRANSFORM) ? 0 : 1;
                LOG2("GammaTM enabled: %d", programGroup.run_kernels[i].enable);
                break;
            }
        }
    }

    // Enable or disable kernels according to environment variables for debug purpose.
    updateKernelToggles(programGroup);

    inputParams.timestamp = aiqResults->mTimestamp;
    inputParams.program_group = &programGroup;
    inputParams.sensor_frame_params = &mFrameParam;

    inputParams.ae_results = &aiqResults->mAeResults;
    inputParams.gbce_results = &aiqResults->mGbceResults;
    inputParams.awb_results = &aiqResults->mAwbResults;
    inputParams.pa_results = &aiqResults->mPaResults;
    inputParams.sa_results = &aiqResults->mSaResults;
    inputParams.weight_grid = aiqResults->mAeResults.weight_grid;

    inputParams.media_format = media_format_legacy;
    if (aiqResults->mAiqParam.tonemapMode != TONEMAP_MODE_FAST &&
        aiqResults->mAiqParam.tonemapMode != TONEMAP_MODE_HIGH_QUALITY) {
        inputParams.media_format = media_format_custom;
    }
    LOG2("%s, media format: 0x%x", __func__, inputParams.media_format);

    if (VIDEO_STREAM_ID == streamId)
        inputParams.call_rate_control.mode = ia_isp_call_rate_never_on_converged;
    else
        inputParams.call_rate_control.mode = ia_isp_call_rate_always;

    if (aiqResults->mCustomControls.count > 0) {
        inputParams.custom_controls = &aiqResults->mCustomControls;
    }

    if (ispSettings) {
        inputParams.nr_setting = ispSettings->nrSetting;
        inputParams.ee_setting = ispSettings->eeSetting;
        LOG2("%s: ISP NR setting, level: %d, strength: %d",
                __func__, (int)ispSettings->nrSetting.feature_level,
                (int)ispSettings->nrSetting.strength);
        inputParams.effects = ispSettings->effects;
        inputParams.manual_brightness = ispSettings->manualSettings.manualBrightness;
        inputParams.manual_contrast = ispSettings->manualSettings.manualContrast;
        inputParams.manual_hue = ispSettings->manualSettings.manualHue;
        inputParams.manual_saturation = ispSettings->manualSettings.manualSaturation;
        LOG2("%s: ISP EE setting, level: %d, strength: %d",
                __func__, ispSettings->eeSetting.feature_level,
                ispSettings->eeSetting.strength);
        // INTEL_DVS_S
        if (ispSettings->videoStabilization) {
            int dvsType = PlatformData::getDVSType(mCameraId);
            LOG2("%s: ISP Video Stabilization Mode Enable, dvs type %d", __func__, dvsType);
            DvsResult* dvsResult = const_cast<DvsResult*>(AiqResultStorage::getInstance(mCameraId)->getDvsResult());
            if (dvsType == MORPH_TABLE) {
                inputParams.dvs_morph_table = (dvsResult == nullptr) ? nullptr : &dvsResult->mMorphTable;
            } else if (dvsType == IMG_TRANS) {
                inputParams.gdc_transformation = (dvsResult == nullptr) ? nullptr : &dvsResult->mTransformation;
            }
        }
        // INTEL_DVS_E

        inputParams.pal_override = ispSettings->palOverride;
    }

    if (CameraUtils::isUllPsysPipe(mTuningMode)) {
        CheckError((aiqResults->mAeResults.exposures[0].exposure == nullptr), BAD_VALUE, "Aiq exposure is NULL.");
        // The situation that all DG passed to ISP, not sensor.
        if (!PlatformData::isUsingSensorDigitalGain(mCameraId)) {
            inputParams.manual_digital_gain = aiqResults->mAeResults.exposures[0].exposure->digital_gain;
        }
        // Fine-tune DG passed to ISP if partial ISP DG is needed.
        if (PlatformData::isUsingIspDigitalGain(mCameraId)) {
            inputParams.manual_digital_gain = PlatformData::getIspDigitalGain(mCameraId,
                                aiqResults->mAeResults.exposures[0].exposure->digital_gain);
        }

        LOG3A("%s: set digital gain for ULL pipe: %f", __func__, inputParams.manual_digital_gain);
    } else if (CameraUtils::isMultiExposureCase(mTuningMode) &&
               PlatformData::getSensorGainType(mCameraId) == ISP_DG_AND_SENSOR_DIRECT_AG) {
        CheckError((aiqResults->mAeResults.exposures[0].exposure == nullptr), BAD_VALUE, "Aiq exposure is NULL.");

        LOG3A("%s: all digital gain is passed to ISP, DG(%ld): %f",
              __func__, aiqResults->mSequence, aiqResults->mAeResults.exposures[0].exposure->digital_gain);
        inputParams.manual_digital_gain = aiqResults->mAeResults.exposures[0].exposure->digital_gain;
    }

    int ret = OK;
    {
        PERF_CAMERA_ATRACE_PARAM1_IMAGING("ia_isp_bxt_run", 1);
        ret = mAdaptor->runPal(mIspAdaptHandle, &inputParams, binaryData);
    }
    CheckError(ret != OK, UNKNOWN_ERROR, "ISP parameter adaptation has failed %d", ret);

    dumpIspParameter(aiqResults->mSequence, *binaryData);

    return OK;
}

void IspParamAdaptor::dumpAfStats(const ia_aiq_af_grid *afGrid, long sequence)
{
    if (!afGrid) return;

    if (mPgParamType == PG_PARAM_PSYS_ISA && !CameraDump::isDumpTypeEnable(DUMP_PSYS_AIQ_STAT))
        return;
    if (mPgParamType == PG_PARAM_ISYS && !CameraDump::isDumpTypeEnable(DUMP_ISYS_AIQ_STAT))
        return;

    BinParam_t bParam;
    bParam.bType = BIN_TYPE_STATISTIC;
    bParam.mType = mPgParamType == PG_PARAM_PSYS_ISA ? M_PSYS : M_ISYS;
    bParam.sequence = sequence;
    bParam.sParam.gridWidth = afGrid->grid_width;
    bParam.sParam.gridHeight = afGrid->grid_height;
    bParam.sParam.appendix = "af_stats_filter_response_1";
    CameraDump::dumpBinary(mCameraId, afGrid->filter_response_1,
                           afGrid->grid_width * afGrid->grid_height * sizeof(int), &bParam);
    bParam.sParam.appendix = "af_stats_filter_response_2";
    CameraDump::dumpBinary(mCameraId, afGrid->filter_response_2,
                           afGrid->grid_width * afGrid->grid_height * sizeof(int), &bParam);
}

void IspParamAdaptor::dumpRgbsStats(ia_aiq_rgbs_grid *rgbsGrid, long sequence, unsigned int num)
{
    if (rgbsGrid == nullptr) return;

    if (Log::isDebugLevelEnable(CAMERA_DEBUG_LOG_AIQ)) {
        for (unsigned int i = 0; i < num; i++ ) {
            rgbs_grid_block *rgbsPtr = rgbsGrid[i].blocks_ptr;
            int size = rgbsGrid[i].grid_width * rgbsGrid[i].grid_height;
            // Print out some value to check if it's reasonable
            for (int j = 100; j < 105 && j < size; j++) {
                LOG3A("RGBS: [%d]:%d, %d, %d, %d, %d", j, rgbsPtr[j].avg_b, rgbsPtr[j].avg_gb,
                            rgbsPtr[j].avg_gr, rgbsPtr[j].avg_r, rgbsPtr[j].sat);
            }

            // Only print last Rgbs Stats's y_mean for validation purpose
            if (i < num - 1) continue;

            int sumLuma = 0;
            for (int j = 0; j < size; j++) {
                sumLuma += (rgbsPtr[j].avg_b + rgbsPtr[j].avg_r + (rgbsPtr[j].avg_gb + rgbsPtr[j].avg_gr) / 2) / 3;
            }
            LOG3A("RGB stat grid[%d] %dx%d, y_mean %d", i, rgbsGrid[i].grid_width, rgbsGrid[i].grid_height, sumLuma/size);
        }
    }

    if ((mPgParamType == PG_PARAM_PSYS_ISA && CameraDump::isDumpTypeEnable(DUMP_PSYS_AIQ_STAT)) ||
        (mPgParamType == PG_PARAM_ISYS && CameraDump::isDumpTypeEnable(DUMP_ISYS_AIQ_STAT))) {
        char name[30];
        BinParam_t bParam;
        bParam.bType    = BIN_TYPE_STATISTIC;
        bParam.mType    = mPgParamType == PG_PARAM_PSYS_ISA ? M_PSYS : M_ISYS;
        bParam.sequence = sequence;
        for (unsigned int i = 0; i < num; i++ ) {
            CLEAR(name);
            snprintf(name, sizeof(name), "%s_stats_%u_%u",
                    mPgParamType == PG_PARAM_PSYS_ISA ? "hdr_rgbs" : "rgbs", num, i);
            bParam.sParam.gridWidth  = rgbsGrid[i].grid_width;
            bParam.sParam.gridHeight = rgbsGrid[i].grid_height;
            bParam.sParam.appendix   = name;
            if (rgbsGrid[i].grid_width != 0 && rgbsGrid[i].grid_height != 0) {
                CameraDump::dumpBinary(mCameraId, rgbsGrid[i].blocks_ptr,
                                       rgbsGrid[i].grid_width * rgbsGrid[i].grid_height * sizeof(rgbs_grid_block),
                                       &bParam);
            }
        }
    }
}

void IspParamAdaptor::dumpIspParameter(long sequence, ia_binary_data binaryData) {
    if (mPgParamType == PG_PARAM_PSYS_ISA && !CameraDump::isDumpTypeEnable(DUMP_PSYS_PAL)) return;
    if (mPgParamType == PG_PARAM_ISYS && !CameraDump::isDumpTypeEnable(DUMP_ISYS_PAL)) return;

    BinParam_t bParam;
    bParam.bType    = BIN_TYPE_GENERAL;
    bParam.mType    = mPgParamType == PG_PARAM_PSYS_ISA ? M_PSYS : M_ISYS;
    bParam.sequence = sequence;
    bParam.gParam.appendix = "pal";
    CameraDump::dumpBinary(mCameraId, binaryData.data, binaryData.size, &bParam);
}

} // namespace icamera
