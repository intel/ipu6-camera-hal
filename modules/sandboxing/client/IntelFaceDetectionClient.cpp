/*
 * Copyright (C) 2019-2021 Intel Corporation
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

#define LOG_TAG IntelFaceDetectionClient

#include "modules/sandboxing/client/IntelFaceDetectionClient.h"

#include <algorithm>

#include "FaceBase.h"

namespace icamera {
IntelFaceDetection::IntelFaceDetection() : mInitialized(false) {
    LOG1("@%s", __func__);

    uintptr_t personal = reinterpret_cast<uintptr_t>(this);
    mMems = {{("/faceDetectionInit" + std::to_string(personal) + "Shm"),
              sizeof(FaceDetectionInitParams), &mMemInit, false},
              {("/faceDetectionDeinit" + std::to_string(personal) + "Shm"),
              sizeof(FaceDetectionDeinitParams), &mMemDeinit, false}};

    for (int i = 0; i < MAX_STORE_FACE_DATA_BUF_NUM; i++) {
        mMems.push_back(
            {("/faceDetectionRun" + std::to_string(i) + std::to_string(personal) + "Shm"),
             sizeof(FaceDetectionRunParams), &mMemRunBufs[i], false});
    }

    bool success = mCommon.allocateAllShmMems(&mMems);
    if (!success) {
        LOGE("@%s,Failed to call allocateAllShmMems", __func__);
        mCommon.releaseAllShmMems(mMems);
        return;
    }

    LOG1("@%s, done", __func__);
    mInitialized = true;
}

IntelFaceDetection::~IntelFaceDetection() {
    LOG1("@%s", __func__);
    mCommon.releaseAllShmMems(mMems);
}

status_t IntelFaceDetection::init(FaceDetectionInitParams* initParams, int dataSize) {
    CheckAndLogError(!mInitialized, UNKNOWN_ERROR, "@%s, mInitialized is false", __func__);

    LOG1("@%s, initParams:%p, dataSize:%d", __func__, initParams, dataSize);
    CheckAndLogError(initParams == nullptr, UNKNOWN_ERROR, "@%s, initParams is nullptr", __func__);
    CheckAndLogError(dataSize < static_cast<int>(sizeof(FaceDetectionInitParams)), UNKNOWN_ERROR,
                     "@%s, buffer is small", __func__);

    unsigned int maxFacesNum =
        std::min(initParams->max_face_num, static_cast<unsigned int>(MAX_FACES_DETECTABLE));
    LOG2("@%s, maxFacesNum:%d, cameraId:%d", __func__, maxFacesNum, initParams->cameraId);

    FaceDetectionInitParams* params = static_cast<FaceDetectionInitParams*>(mMemInit.mAddr);

    bool ret = mIpc.clientFlattenInit(maxFacesNum, initParams->cameraId, params);
    CheckAndLogError(ret == false, UNKNOWN_ERROR, "@%s, clientFlattenInit fails", __func__);

    ret = mCommon.requestSync(IPC_FD_INIT, mMemInit.mHandle);
    CheckAndLogError(ret == false, UNKNOWN_ERROR, "@%s, requestSync fails", __func__);

    return OK;
}

status_t IntelFaceDetection::deinit(FaceDetectionDeinitParams* deinitParams, int dataSize) {
    LOG1("@%s", __func__);
    CheckAndLogError(!mInitialized, UNKNOWN_ERROR, "@%s, mInitialized is false", __func__);
    CheckAndLogError(deinitParams == nullptr, UNKNOWN_ERROR, "@%s, deinitParams is nullptr",
                     __func__);
    CheckAndLogError(dataSize < static_cast<int>(sizeof(FaceDetectionDeinitParams)), UNKNOWN_ERROR,
                     "@%s, buffer is small", __func__);

    FaceDetectionDeinitParams* params = static_cast<FaceDetectionDeinitParams*>(mMemDeinit.mAddr);
    params->cameraId = deinitParams->cameraId;
    bool ret = mCommon.requestSync(IPC_FD_DEINIT, mMemDeinit.mHandle);
    CheckAndLogError(ret == false, UNKNOWN_ERROR, "@%s, requestSync fails", __func__);

    return OK;
}

FaceDetectionRunParams* IntelFaceDetection::prepareRunBuffer(unsigned int index) {
    LOG1("@%s", __func__);
    CheckAndLogError(index >= MAX_STORE_FACE_DATA_BUF_NUM, nullptr, "@%s, index is error %d",
                     __func__, index);
    CheckAndLogError(!mInitialized, nullptr, "@%s, mInitialized is false", __func__);

    return static_cast<FaceDetectionRunParams*>(mMemRunBufs[index].mAddr);
}

status_t IntelFaceDetection::run(FaceDetectionRunParams* runParams, int dataSize, int dmafd) {
    LOG2("@%s", __func__);
    CheckAndLogError(!mInitialized, UNKNOWN_ERROR, "@%s, mInitialized is false", __func__);
    CheckAndLogError(!runParams, UNKNOWN_ERROR, "@%s,runParams is nullptr", __func__);
    CheckAndLogError(dataSize < static_cast<int>(sizeof(FaceDetectionRunParams)), UNKNOWN_ERROR,
                     "@%s, buffer is small", __func__);

    if (dmafd >= 0) {
        runParams->bufferHandle = mCommon.registerGbmBuffer(dmafd);
        CheckAndLogError((runParams->bufferHandle < 0), false,
                         "@%s, call mCommon.registerGbmBuffer", __func__);
    }

    int32_t runBufHandle = mCommon.getShmMemHandle(static_cast<void*>(runParams));
    CheckAndLogError(runBufHandle < 0, UNKNOWN_ERROR, "@%s, getShmMemHandle fails", __func__);

    bool ret = mCommon.requestSync(IPC_FD_RUN, runBufHandle);
    CheckAndLogError(ret == false, UNKNOWN_ERROR, "@%s, requestSync fails", __func__);

    if (dmafd >= 0) {
        mCommon.deregisterGbmBuffer(runParams->bufferHandle);
    }

    return OK;
}
} /* namespace icamera */
