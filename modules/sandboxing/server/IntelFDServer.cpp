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

#define LOG_TAG IntelFDServer

#include "modules/sandboxing/server/IntelFDServer.h"

#include <pvl_types.h>

#include "CameraLog.h"
#include "iutils/Utils.h"

namespace icamera {
IntelFDServer::IntelFDServer() {
    LOG1("@%s Construct", __func__);
}

IntelFDServer::~IntelFDServer() {
    LOG1("@%s Destroy", __func__);
}

status_t IntelFDServer::init(void* pData, int dataSize) {
    CheckAndLogError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);
    CheckAndLogError(dataSize < static_cast<int>(sizeof(FaceDetectionInitParams)), UNKNOWN_ERROR,
                     "@%s, buffer size: %d is small", __func__, dataSize);

    FaceDetectionInitParams* inParams = static_cast<FaceDetectionInitParams*>(pData);
    if (mFaceDetection.find(inParams->cameraId) == mFaceDetection.end()) {
        mFaceDetection[inParams->cameraId] =
            std::unique_ptr<IntelFaceDetection>(new IntelFaceDetection());
    }

    return mFaceDetection[inParams->cameraId]->init(inParams, dataSize);
}

status_t IntelFDServer::run(void* pData, int dataSize, void* imageData) {
    PERF_CAMERA_ATRACE();
    TRACE_LOG_PROCESS("IntelFDServer", "runFaceDetection");

    CheckAndLogError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);
    CheckAndLogError(dataSize < static_cast<int>(sizeof(FaceDetectionRunParams)), UNKNOWN_ERROR,
                     "@%s, buffer size: %d is small", __func__, dataSize);
    pvl_image image;
    int cameraId;
    FaceDetectionRunParams* pFdRunParams = static_cast<FaceDetectionRunParams*>(pData);
    mIpcFD.serverUnflattenRun(*pFdRunParams, imageData, &image, &cameraId);
    CheckAndLogError((mFaceDetection.find(cameraId) == mFaceDetection.end()), UNKNOWN_ERROR,
                     "<id%d> @%s, mFaceDetection is nullptr", cameraId, __func__);

    return mFaceDetection[cameraId]->run(&image, &pFdRunParams->results);
}

status_t IntelFDServer::deinit(void* pData, int dataSize) {
    CheckAndLogError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);
    CheckAndLogError(dataSize < static_cast<int>(sizeof(FaceDetectionDeinitParams)), UNKNOWN_ERROR,
                     "@%s, buffer size: %d is small", __func__, dataSize);

    FaceDetectionDeinitParams* deinitParams = static_cast<FaceDetectionDeinitParams*>(pData);
    CheckAndLogError((mFaceDetection.find(deinitParams->cameraId) == mFaceDetection.end()),
                     UNKNOWN_ERROR, "<id%d> @%s, mFaceDetection is nullptr", deinitParams->cameraId,
                     __func__);

    return mFaceDetection[deinitParams->cameraId]->deinit(deinitParams, dataSize);
}
} /* namespace icamera */
