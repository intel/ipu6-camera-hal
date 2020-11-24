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

#define LOG_TAG "LiveTuning"

#include "iutils/CameraLog.h"
#include "iutils/CameraDump.h"
#include "modules/livetune/LiveTuning.h"
#include "modules/sandboxing/client/IntelAlgoClient.h"

#include "PlatformData.h"

using std::vector;

/**
 * The file implements the APIs for Live Tuning
 */

namespace icamera {

int setupIPCEnvForLiveTune(cros::CameraMojoChannelManager* mojoManager) {
    PERF_CAMERA_ATRACE();
    HAL_TRACE_CALL(1);

    CheckError(mojoManager == nullptr, BAD_VALUE, "@%s, Invalid mojoManager!", __func__);

    // Set debug level and dump level
    icamera::Log::setDebugLevel();
    icamera::CameraDump::setDumpLevel();

    // Create IntelAlgoClient and set the mojo manager
    icamera::IntelAlgoClient::getInstance()->setMojoManager(mojoManager);

    // Run initialization of IntelAlgoClient
    CheckError(icamera::IntelAlgoClient::getInstance()->initialize() != icamera::OK, -EINVAL,
               "%s, Connect to algo service fails", __func__);

    return OK;
}

int tearDownIPCEnvForLiveTune() {
    PERF_CAMERA_ATRACE();
    HAL_TRACE_CALL(1);

    icamera::IntelAlgoClient::releaseInstance();
    return OK;
}

int getSupportedRawInfo(int cameraId, int* width, int* height, int* format, int* mcId) {
    PERF_CAMERA_ATRACE();
    HAL_TRACE_CALL(1);

    bool isInvalidParam = (!width || !height || !format || !mcId);
    CheckError(isInvalidParam == true, BAD_VALUE, "@%s, Invalid Params!", __func__);

    MediaCtlConf* selectedMc = PlatformData::getMediaCtlConf(cameraId);
    CheckError(selectedMc == nullptr, BAD_VALUE, "@%s, getMediaCtlConf failed!", __func__);

    *width = selectedMc->outputWidth;
    *height = selectedMc->outputHeight;
    *format = selectedMc->format;
    *mcId = selectedMc->mcId;

    return OK;
}

int acquireMakernoteData(int cameraId, int64_t sequence, uint64_t timestamp,
                         Parameters* param) {
    PERF_CAMERA_ATRACE();
    HAL_TRACE_CALL(1);

    CheckError(param == nullptr, BAD_VALUE, "@%s, Invalid Params!", __func__);

    PlatformData::updateMakernoteTimeStamp(cameraId, sequence, timestamp);
    PlatformData::acquireMakernoteData(cameraId, timestamp, param);

    return OK;
}

}  // namespace icamera
