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

namespace icamera {

Ltm::Ltm(int cameraId)
        : mCameraId(cameraId),
          mTuningMode(TUNING_MODE_MAX),
          mLtmState(LTM_NOT_INIT),
          mThreadRunning(false),
          mInputParamIndex(-1) {
    CLEAR(mLtmParams);

    if (PlatformData::isEnableLtmThread(mCameraId)) {
        mLtmThread = new LtmThread(this);
    } else {
        mLtmThread = nullptr;
    }
    LOG3A("%s", __func__);
}

Ltm::~Ltm() {
    LOG3A("%s", __func__);

    if (PlatformData::isEnableLtmThread(mCameraId)) {
        mLtmThread->join();
        delete mLtmThread;
    }
}

int Ltm::init() {
    LOG3A("%s", __func__);

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
    LOG3A("%s", __func__);

    AutoMutex l(mLtmLock);

    for (int i = 0; i < kMaxLtmParamsNum; i++) {
        delete mLtmParams[i];
        mLtmParams[i] = nullptr;
    }
    mLtmState = LTM_NOT_INIT;

    return OK;
}

int Ltm::configure(const std::vector<ConfigMode>& configModes) {
    LOG3A("%s", __func__);

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

    if (tMode == TUNING_MODE_MAX) {
        return OK;
    }

    if (mLtmState == LTM_CONFIGURED && mTuningMode == tMode) {
        return OK;
    }

    mTuningMode = tMode;
    mLtmState = LTM_CONFIGURED;

    LOG3A("%s Ltm algo is Configured", __func__);
    return OK;
}

int Ltm::start() {
    LOG1("@%s", __func__);
    AutoMutex l(mLtmLock);

    if (!PlatformData::isEnableLtmThread(mCameraId)) return OK;

    mLtmThread->run("ltm_thread", PRIORITY_NORMAL);
    mThreadRunning = true;

    return OK;
}

void Ltm::stop() {
    LOG1("@%s", __func__);

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
    if ((eventData.type != EVENT_PSYS_STATS_SIS_BUF_READY) &&
        (eventData.pipeType != STILL_STREAM_ID))
        return;

    LOG3A("%s: handle EVENT_PSYS_STATS_SIS_BUF_READY", __func__);
    handleSisLtm(eventData.buffer);
}

AiqResult* Ltm::getAiqResult(long sequence) {
    long ltmSequence = sequence;
    AiqResultStorage* resultStorage = AiqResultStorage::getInstance(mCameraId);
    if (ltmSequence > 0) {
        ltmSequence += PlatformData::getLtmGainLag(mCameraId);
    }

    LOG3A("%s, ltmSequence %ld, sequence %ld", __func__, ltmSequence, sequence);
    AiqResult* feedback = const_cast<AiqResult*>(resultStorage->getAiqResult(ltmSequence));
    if (feedback == nullptr) {
        LOGW("%s: no feed back result for sequence %ld! use the latest instead", __func__,
             ltmSequence);
        feedback = const_cast<AiqResult*>(resultStorage->getAiqResult());
    }

    return feedback;
}

int Ltm::handleSisLtm(const std::shared_ptr<CameraBuffer>& cameraBuffer) {
    LOG3A("@%s", __func__);
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
    mLtmParams[mInputParamIndex]->ltmParams.frame_width = feedback->mAiqParam.resolution.width;
    mLtmParams[mInputParamIndex]->ltmParams.frame_height = feedback->mAiqParam.resolution.height;

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
    LOG3A("%s", __func__);
    PERF_CAMERA_ATRACE();

    LOG3A("%s: begin running LTM", __func__);
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
