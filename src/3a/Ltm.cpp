/*
 * Copyright (C) 2017-2021 Intel Corporation.
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

#define LOG_TAG Ltm

#include "Ltm.h"

#include <cmath>
#include <memory>

#include "AiqResultStorage.h"
#include "AiqUtils.h"
#include "PlatformData.h"
#include "ia_log.h"
#include "iutils/CameraLog.h"
#include "iutils/Errors.h"
#include "iutils/Utils.h"

#include "ia_pal_types_isp_ids_autogen.h"
#include "ia_pal_types_isp.h"

namespace icamera {

Ltm::Ltm(int cameraId)
        : mCameraId(cameraId),
          mTuningMode(TUNING_MODE_MAX),
          mLtmState(LTM_NOT_INIT),
          mThreadRunning(false),
          mInputParamIndex(-1) {
    CLEAR(mLtmParams);
    CLEAR(mFrameResolution);

    if (PlatformData::isEnableLtmThread(mCameraId)) {
        mLtmThread = new LtmThread(this);
    } else {
        mLtmThread = nullptr;
    }
}

Ltm::~Ltm() {
    if (PlatformData::isEnableLtmThread(mCameraId)) {
        mLtmThread->join();
        delete mLtmThread;
    }
    LOG2("%s", __func__);
}

int Ltm::init() {
    AutoMutex l(mLtmLock);

    for (int i = 0; i < kMaxLtmParamsNum; i++) {
        mLtmParams[i] = new LtmInputParams;
        CheckAndLogError(!mLtmParams[i], NO_MEMORY, "%s, No memory for Ltm input params", __func__);
        mLtmParams[i]->ltmParams.ltm_level = ia_ltm_level_use_tuning;
        mLtmParams[i]->ltmParams.ev_shift = 0;
        mLtmParams[i]->ltmParams.ltm_strength_manual = 100;
    }
    mLtmState = LTM_INIT;

    return OK;
}

int Ltm::deinit() {
    AutoMutex l(mLtmLock);

    for (int i = 0; i < kMaxLtmParamsNum; i++) {
        delete mLtmParams[i];
        mLtmParams[i] = nullptr;
    }
    mLtmState = LTM_NOT_INIT;

    return OK;
}

int Ltm::getPixelCropperResolution(std::shared_ptr<IGraphConfig> graphConfig, int32_t streamId,
                                   camera_resolution_t* resolution) {
    ia_isp_bxt_program_group* pgPtr = graphConfig->getProgramGroup(streamId);
    for (unsigned int i = 0; i < pgPtr->kernel_count; i++) {
        // The kernel value is for cca_ltm_input_params::frame_width and frame_height.
        if (pgPtr->run_kernels[i].kernel_uuid == ia_pal_uuid_isp_pxl_crop_yuv_a) {
            if (pgPtr->run_kernels[i].resolution_info) {
                resolution->width = pgPtr->run_kernels[i].resolution_info->output_width;
                resolution->height = pgPtr->run_kernels[i].resolution_info->output_height;
                return OK;
            } else {
                resolution->width = pgPtr->run_kernels[i].resolution_history->output_width;
                resolution->height = pgPtr->run_kernels[i].resolution_history->output_height;
            }
        }
    }

    return UNKNOWN_ERROR;
}

int Ltm::configure(const std::vector<ConfigMode>& configModes,
                   std::shared_ptr<IGraphConfig> graphConfig, int32_t streamId) {
    TuningMode tMode = TUNING_MODE_MAX;
    for (auto cfg : configModes) {
        // Only support the 1st tuning mode if multiple config mode is configured.
        if (cfg == CAMERA_STREAM_CONFIGURATION_MODE_NORMAL) {
            tMode = TUNING_MODE_VIDEO;
            break;
            // HDR_FEATURE_S
        } else if (cfg == CAMERA_STREAM_CONFIGURATION_MODE_HLC) {
            tMode = TUNING_MODE_VIDEO_HLC;
            break;
        } else if (cfg == CAMERA_STREAM_CONFIGURATION_MODE_HDR) {
            tMode = TUNING_MODE_VIDEO_HDR;
            break;
        } else if (cfg == CAMERA_STREAM_CONFIGURATION_MODE_HDR2) {
            tMode = TUNING_MODE_VIDEO_HDR2;
            break;
            // HDR_FEATURE_E
        }
    }

    if (graphConfig) {
        int ret = getPixelCropperResolution(graphConfig, streamId, &mFrameResolution);
        CheckAndLogError(ret != OK, ret, "failed to get sis output resolution");
    }

    if (tMode == TUNING_MODE_MAX) {
        return OK;
    }

    if (mLtmState == LTM_CONFIGURED && mTuningMode == tMode) {
        return OK;
    }

    mTuningMode = tMode;
    mLtmState = LTM_CONFIGURED;

    return OK;
}

int Ltm::start() {
    AutoMutex l(mLtmLock);

    if (!PlatformData::isEnableLtmThread(mCameraId)) return OK;

    mLtmThread->run("ltm_thread", PRIORITY_NORMAL);
    mThreadRunning = true;

    return OK;
}

void Ltm::stop() {
    if (!PlatformData::isEnableLtmThread(mCameraId)) return;

    mLtmThread->requestExit();
    {
        AutoMutex l(mLtmLock);
        mThreadRunning = false;
        mParamAvailableSignal.signal();
    }
    mLtmThread->requestExitAndWait();

    while (!mLtmParamsQ.empty()) {
        mLtmParamsQ.pop();
    }
}

void Ltm::handleEvent(EventData eventData) {
    if ((eventData.type != EVENT_PSYS_STATS_SIS_BUF_READY) ||
        (eventData.pipeType != VIDEO_STREAM_ID))
        return;

    LOG2("%s: handle EVENT_PSYS_STATS_SIS_BUF_READY", __func__);
    handleSisLtm(eventData.buffer);
}

AiqResult* Ltm::getAiqResult(int64_t sequence) {
    int64_t ltmSequence = sequence;
    AiqResultStorage* resultStorage = AiqResultStorage::getInstance(mCameraId);
    if (ltmSequence > 0) {
        ltmSequence += PlatformData::getLtmGainLag(mCameraId);
    }

    LOG2("<seq%ld>%s, ltmSequence %ld", sequence, __func__, ltmSequence);
    AiqResult* feedback = const_cast<AiqResult*>(resultStorage->getAiqResult(ltmSequence));
    if (feedback == nullptr) {
        LOGW("%s: no feed back result for sequence %ld! use the latest instead", __func__,
             ltmSequence);
        feedback = const_cast<AiqResult*>(resultStorage->getAiqResult());
    }

    return feedback;
}

int Ltm::handleSisLtm(const std::shared_ptr<CameraBuffer>& cameraBuffer) {
    AutoMutex l(mLtmLock);

    ia_binary_data* sisFrame = (ia_binary_data*)cameraBuffer->getBufferAddr();
    CheckAndLogError(sisFrame == nullptr, BAD_VALUE, "sis frame buffer is nullptr!");
    unsigned int size = sisFrame->size;
    CheckAndLogError((size == 0), BAD_VALUE, "sis data size err!");
    void* data = sisFrame->data;
    CheckAndLogError((data == nullptr), BAD_VALUE, "sis data ptr err!");

    mInputParamIndex++;
    mInputParamIndex %= kMaxLtmParamsNum;

    int sequence = cameraBuffer->getSequence();
    mLtmParams[mInputParamIndex]->sequence = sequence;
    AiqResult* feedback = getAiqResult(sequence);
    mLtmParams[mInputParamIndex]->ltmParams.ev_shift = feedback->mAiqParam.evShift;
    mLtmParams[mInputParamIndex]->ltmParams.ltm_strength_manual = feedback->mAiqParam.ltmStrength;
    mLtmParams[mInputParamIndex]->ltmParams.frame_width = mFrameResolution.width;
    mLtmParams[mInputParamIndex]->ltmParams.frame_height = mFrameResolution.height;

    ia_image_full_info* imageInfo = &mLtmParams[mInputParamIndex]->ltmParams.sis.image_info;
    CLEAR(*imageInfo);
    imageInfo->raw_image.data_format = ia_image_data_format_rawplain16_interleaved;
    imageInfo->raw_image.bayer_order = cmc_bayer_order_grbg;
    imageInfo->raw_image.data_format_bpp = 16;
    imageInfo->raw_image.data_bpp = 15;

    // width_cols and height_lines are quad count, need to divide 2 for them.
    int sisWidth = cameraBuffer->getWidth();
    int sisHeight = cameraBuffer->getHeight();
    imageInfo->raw_image.width_cols = sisWidth / 2;
    imageInfo->raw_image.height_lines = sisHeight / 2;

    cca::cca_ltm_statistics* sis = &mLtmParams[mInputParamIndex]->ltmParams.sis;
    MEMCPY_S(sis->data, sizeof(sis->data), data, size);
    sis->size = sizeof(sis->data) > size ? size : sizeof(sis->data);
    LOG3(
        "LTM data_format %d, bayer_order %d, data_format_bpp %d, data_bpp %d, frame_width and "
        "height(%d, %d), SIS_image_width & height and right padder(%d, %d, %d), image data size %d",
        imageInfo->raw_image.data_format, imageInfo->raw_image.bayer_order,
        imageInfo->raw_image.data_format_bpp, imageInfo->raw_image.data_bpp,
        mLtmParams[mInputParamIndex]->ltmParams.frame_width,
        mLtmParams[mInputParamIndex]->ltmParams.frame_height, imageInfo->raw_image.width_cols,
        imageInfo->raw_image.height_lines, imageInfo->extra_cols_right, sis->size);

    if ((!PlatformData::isEnableLtmThread(mCameraId)) || sequence == 0) {
        runLtm(*mLtmParams[mInputParamIndex]);
    } else {
        bool needSignal = mLtmParamsQ.empty();
        mLtmParamsQ.push(mLtmParams[mInputParamIndex]);
        if (needSignal) {
            mParamAvailableSignal.signal();
        }
    }

    return OK;
}

int Ltm::runLtmAsync() {
    LtmInputParams* inputParams = NULL;

    ConditionLock lock(mLtmLock);

    while (mLtmParamsQ.empty()) {
        // To prevent possible dead lock during stop of ltm thread.
        if (!mThreadRunning) {
            LOG2("%s, ltm thread is not active, no need to wait ltm stat", __func__);
            return OK;
        }

        mParamAvailableSignal.wait(lock);

        if (!mThreadRunning) {
            LOG2("%s, ltm thread is not active while waiting ltm stat", __func__);
            return OK;
        }
    }

    CheckAndLogError(mLtmParamsQ.empty(), UNKNOWN_ERROR, "Failed to get ltm input params buffers");
    inputParams = mLtmParamsQ.front();
    mLtmParamsQ.pop();
    CheckAndLogError(!inputParams, OK, "%s, the inputParams is NULL", __func__);

    runLtm(*inputParams);

    return OK;
}

int Ltm::runLtm(const LtmInputParams& ltmInputParams) {
    LOG2("%s", __func__);
    PERF_CAMERA_ATRACE();

    ia_err iaErr;
    {
        PERF_CAMERA_ATRACE_PARAM1_IMAGING("ia_ltm_run", 0);
        IntelCca* intelCcaHandle = IntelCca::getInstance(mCameraId, mTuningMode);
        CheckAndLogError(!intelCcaHandle, BAD_VALUE, "@%s, Failed to get IntelCca instance",
                         __func__);
        iaErr = intelCcaHandle->runLTM(ltmInputParams.sequence, ltmInputParams.ltmParams);
    }

    int ret = AiqUtils::convertError(iaErr);
    CheckAndLogError(ret != OK, ret, "Error running LTM: %d", ret);

    return OK;
}
} /* namespace icamera */
