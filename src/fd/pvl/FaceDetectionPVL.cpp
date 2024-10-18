/*
 * Copyright (C) 2021-2022 Intel Corporation
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

#define LOG_TAG FaceDetectionPVL
#include "src/fd/pvl/FaceDetectionPVL.h"

#include <algorithm>
#include <fstream>
#include <vector>

#include "AiqUtils.h"
#include "PlatformData.h"
#include "iutils/CameraLog.h"
#include "iutils/Errors.h"
#include "iutils/Utils.h"

namespace icamera {
FaceDetectionPVL::FaceDetectionPVL(int cameraId, unsigned int maxFaceNum, int32_t halStreamId,
                                   int width, int height)
        : FaceDetection(cameraId, maxFaceNum, halStreamId, width, height) {
    CLEAR(mResult);
    int ret = initFaceDetection();
    CheckAndLogError(ret != OK, VOID_VALUE, "failed to init face detection, ret %d", ret);
}

FaceDetectionPVL::~FaceDetectionPVL() {
    LOG1("<id%d> @%s", mCameraId, __func__);

    if (mFace) {
        FaceDetectionDeinitParams params;
        params.cameraId = mCameraId;
        mFace->deinit(&params, sizeof(FaceDetectionDeinitParams));
    }

    if (!PlatformData::isFaceEngineSyncRunning(mCameraId)) {
        requestExit();

        AutoMutex l(mRunBufQueueLock);
        mRunCondition.notify_one();
    }
}

int FaceDetectionPVL::initFaceDetection() {
    /* init IntelFaceDetection */
    FaceDetectionInitParams params;
    params.max_face_num = mMaxFaceNum;
    params.cameraId = mCameraId;
    mFace = std::unique_ptr<IntelFaceDetection>(new IntelFaceDetection());
    int ret = mFace->init(&params, sizeof(FaceDetectionInitParams));
    CheckAndLogError(ret != OK, ret, "mFace init failed, ret %d", ret);

    for (int i = 0; i < MAX_STORE_FACE_DATA_BUF_NUM; i++) {
        FaceDetectionRunParams* memRunBuf = mFace->prepareRunBuffer(i);
        CheckAndLogError(!memRunBuf, NO_MEMORY, "prepareRunBuffer failed");
        mMemRunPool.push(memRunBuf);
    }

    camera_info_t info;
    PlatformData::getCameraInfo(mCameraId, info);
    mSensorOrientation = info.orientation;

    if (!PlatformData::isFaceEngineSyncRunning(mCameraId)) {
        /* start face engine pthread */
        ret = run("fdPVL" + std::to_string(mCameraId), PRIORITY_NORMAL);
        CheckAndLogError(ret != OK, NO_INIT, "Camera thread failed to start, ret %d", ret);
    }

    mInitialized = true;
    return OK;
}

FaceDetectionRunParams* FaceDetectionPVL::acquireRunBuf() {
    AutoMutex l(mMemRunPoolLock);
    FaceDetectionRunParams* runBuffer = nullptr;
    if (!mMemRunPool.empty()) {
        runBuffer = mMemRunPool.front();
        mMemRunPool.pop();
        CLEAR(*runBuffer);
    }
    return runBuffer;
}

void FaceDetectionPVL::returnRunBuf(FaceDetectionRunParams* memRunBuf) {
    AutoMutex l(mMemRunPoolLock);
    mMemRunPool.push(memRunBuf);
}

void FaceDetectionPVL::runFaceDetectionBySync(
    const std::shared_ptr<camera3::Camera3Buffer>& ccBuf) {
    LOG2("@%s", __func__);
    CheckAndLogError(mInitialized == false, VOID_VALUE, "@%s, mInitialized is false", __func__);

    const icamera::camera_buffer_t buffer = ccBuf->getHalBuffer();
    int size = buffer.s.size;
    CheckAndLogError(size > MAX_FACE_FRAME_SIZE_SYNC, VOID_VALUE,
                     "Face frame buffer is too small, w:%d,h:%d,size:%d", buffer.s.width,
                     buffer.s.height, size);

    FaceDetectionRunParams* params = acquireRunBuf();
    CheckAndLogError(!params, VOID_VALUE, "Failed to acquire face engine buffer");

    params->size = size;
    params->width = buffer.s.width;
    params->height = buffer.s.height;
    params->rotation = mSensorOrientation % 360;
    params->format = pvl_image_format_nv12;
    params->stride = buffer.s.stride;
    params->bufferHandle = -1;
    params->cameraId = mCameraId;

    nsecs_t startTime = CameraUtils::systemTime();
#ifdef ENABLE_SANDBOXING
    LOG2("@%s, w:%d, h:%d, dmafd:%d", __func__, params->width, params->height, buffer.dmafd);
    int ret = mFace->run(params, sizeof(FaceDetectionRunParams), buffer.dmafd);
#else
    int ret = mFace->run(params, sizeof(FaceDetectionRunParams), buffer.addr);
#endif

    printfFDRunRate();
    LOG2("@%s: ret:%d, mFace runs %ums", __func__, ret,
         (unsigned)((CameraUtils::systemTime() - startTime) / 1000000));

    {
        AutoMutex l(mFaceResultLock);
        if (ret == OK) {
            mResult = params->results;
            mResult.faceUpdated = true;
        } else {
            CLEAR(mResult);
            LOGE("@%s, Faile to detect face", __func__);
        }
    }

    returnRunBuf(params);
}

void FaceDetectionPVL::runFaceDetectionByAsync(
    const std::shared_ptr<camera3::Camera3Buffer>& ccBuf) {
    LOG2("@%s", __func__);
    CheckAndLogError(mInitialized == false, VOID_VALUE, "@%s, mInitialized is false", __func__);

    const icamera::camera_buffer_t buffer = ccBuf->getHalBuffer();
    int size = buffer.s.stride * buffer.s.height;
    CheckAndLogError(size > MAX_FACE_FRAME_SIZE_ASYNC, VOID_VALUE,
                     "face frame buffer is too small!, w:%d,h:%d,size:%d", buffer.s.width,
                     buffer.s.height, size);

    FaceDetectionRunParams* params = acquireRunBuf();
    CheckAndLogError(!params, VOID_VALUE, "Fail to acquire face engine buffer");

    params->size = size;
    MEMCPY_S(params->data, MAX_FACE_FRAME_SIZE_ASYNC, buffer.addr, size);
    params->width = buffer.s.width;
    params->height = buffer.s.height;
    /* TODO: image.rotation is (mSensorOrientation + mCamOriDetector->getOrientation()) % 360 */
    params->rotation = mSensorOrientation % 360;
    params->format = pvl_image_format_gray;
    params->stride = buffer.s.stride;
    params->bufferHandle = -1;
    params->cameraId = mCameraId;

    AutoMutex l(mRunBufQueueLock);
    mRunPvlBufQueue.push(params);
    mRunCondition.notify_one();
}

bool FaceDetectionPVL::threadLoop() {
    FaceDetectionRunParams* faceParams = nullptr;
    {
        ConditionLock lock(mRunBufQueueLock);
        if (mRunPvlBufQueue.empty()) {
            mRunCondition.wait_for(lock,
                                   std::chrono::nanoseconds(kMaxDuration * SLOWLY_MULTIPLIER));
            return true;
        }
        faceParams = mRunPvlBufQueue.front();
        mRunPvlBufQueue.pop();
    }

    nsecs_t startTime = CameraUtils::systemTime();
    int ret = mFace->run(faceParams, sizeof(FaceDetectionRunParams));
    printfFDRunRate();
    LOG2("@%s: ret:%d, it takes need %ums", __func__, ret,
         (unsigned)((CameraUtils::systemTime() - startTime) / 1000000));

    {
        AutoMutex l(mFaceResultLock);
        if (ret == OK) {
            mResult = faceParams->results;
            mResult.faceUpdated = true;
        } else {
            CLEAR(mResult);
            LOGE("@%s, Faile to detect face", __func__);
        }
    }

    returnRunBuf(faceParams);
    return true;
}

int FaceDetectionPVL::getFaceNum() {
    AutoMutex l(mFaceResultLock);
    return mResult.faceNum;
}

/* The result for 3A AE */
void FaceDetectionPVL::getResultFor3A(cca::cca_face_state* faceState) {
    LOG2("@%s", __func__);

    camera_coordinate_system_t sysCoord = {IA_COORDINATE_LEFT, IA_COORDINATE_TOP,
                                           IA_COORDINATE_RIGHT, IA_COORDINATE_BOTTOM};
    AutoMutex l(mFaceResultLock);
    FaceDetectionPVLResult* pvlResult = &mResult;
    faceState->is_video_conf = true;
    faceState->updated = mResult.faceUpdated;
    faceState->num_faces = pvlResult->faceNum;
    faceState->fd_algo = PVL;
    mResult.faceUpdated = false;

    for (int i = 0; i < pvlResult->faceNum; i++) {
        CLEAR(faceState->faces[i]);
        faceState->faces[i].face_area.left = pvlResult->faceResults[i].rect.left;
        faceState->faces[i].face_area.top = pvlResult->faceResults[i].rect.top;
        faceState->faces[i].face_area.bottom = pvlResult->faceResults[i].rect.bottom;
        faceState->faces[i].face_area.right = pvlResult->faceResults[i].rect.right;
        convertFaceCoordinate(
            sysCoord, &faceState->faces[i].face_area.left, &faceState->faces[i].face_area.top,
            &faceState->faces[i].face_area.right, &faceState->faces[i].face_area.bottom);
        faceState->faces[i].rip_angle = pvlResult->faceResults[i].rip_angle;
        faceState->faces[i].rop_angle = pvlResult->faceResults[i].rop_angle;
        faceState->faces[i].tracking_id = pvlResult->faceResults[i].tracking_id;
        faceState->faces[i].confidence = pvlResult->faceResults[i].confidence;
        faceState->faces[i].person_id = -1;
        faceState->faces[i].similarity = 0;
        faceState->faces[i].best_ratio = 0;
        faceState->faces[i].face_condition = 0;

        faceState->faces[i].smile_state = 0;
        faceState->faces[i].smile_score = 0;
        faceState->faces[i].mouth.x = 0;
        faceState->faces[i].mouth.y = 0;

        faceState->faces[i].eye_validity = 0;
        LOG2("@%s, face info, id:%d, left:%d, top:%d, right:%d, bottom:%d", __func__, i,
             faceState->faces[i].face_area.left, faceState->faces[i].face_area.top,
             faceState->faces[i].face_area.right, faceState->faces[i].face_area.bottom);
    }
}

/* The result for android statistics metadata */
void FaceDetectionPVL::getResultForApp(CVFaceDetectionAbstractResult* result) {
    LOG2("@%s", __func__);

    CLEAR(*result);
    AutoMutex l(mFaceResultLock);
    FaceDetectionPVLResult* pvlResult = &mResult;

    for (int i = 0; i < pvlResult->faceNum; i++) {
        if (i == MAX_FACES_DETECTABLE) break;

        result->faceScores[i] = pvlResult->faceResults[i].confidence;
        result->faceIds[i] = pvlResult->faceResults[i].tracking_id;
        result->faceRect[i * 4] = pvlResult->faceResults[i].rect.left;
        result->faceRect[i * 4 + 1] = pvlResult->faceResults[i].rect.top;
        result->faceRect[i * 4 + 2] = pvlResult->faceResults[i].rect.right;
        result->faceRect[i * 4 + 3] = pvlResult->faceResults[i].rect.bottom;  // rect.bottom
        convertFaceCoordinate(mRatioInfo.sysCoord, &result->faceRect[i * 4],
                              &result->faceRect[i * 4 + 1], &result->faceRect[i * 4 + 2],
                              &result->faceRect[i * 4 + 3]);
        LOG2("@%s, face info, id:%d, left:%d, top:%d, right:%d, bottom:%d", __func__, i,
             result->faceRect[i * 4], result->faceRect[i * 4 + 1], result->faceRect[i * 4 + 2],
             result->faceRect[i * 4 + 3]);
    }
    result->faceNum = pvlResult->faceNum;
}

}  // namespace icamera
