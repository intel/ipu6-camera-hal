/*
 * Copyright (C) 2021 Intel Corporation
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

#define LOG_TAG FaceSSD
#include "src/fd/facessd/FaceSSD.h"

#include <algorithm>
#include <fstream>
#include <vector>

#include "AiqUtils.h"
#include "PlatformData.h"
#include "iutils/CameraLog.h"
#include "iutils/Errors.h"
#include "iutils/Utils.h"

namespace icamera {

FaceSSD::FaceSSD(int cameraId, unsigned int maxFaceNum, int32_t halStreamId, int width, int height,
                 int gfxFmt, int usage)
        : FaceDetection(cameraId, maxFaceNum, halStreamId, width, height) {
    CLEAR(mResult);
    int ret = initFaceDetection(width, height, gfxFmt, usage);
    CheckAndLogError(ret != OK, VOID_VALUE, "failed to init face detection, ret %d", ret);
}

FaceSSD::~FaceSSD() {
    LOG1("<id%d> @%s", mCameraId, __func__);

    if (!PlatformData::isFaceEngineSyncRunning(mCameraId)) {
        requestExit();
        AutoMutex l(mRunBufQueueLock);
        mRunCondition.notify_one();
    }

    if (mBufferPool) {
        mBufferPool->destroyBufferPool();
    }
}

int FaceSSD::initFaceDetection(int width, int height, int gfxFmt, int usage) {
    mBufferPool = std::unique_ptr<camera3::Camera3BufferPool>(new camera3::Camera3BufferPool());
    // Create the buffer pool with DMA handle buffer
    int ret = mBufferPool->createBufferPool(mCameraId, MAX_STORE_FACE_DATA_BUF_NUM, width, height,
                                            gfxFmt, usage);
    CheckAndLogError(ret != icamera::OK, NO_MEMORY, "[%p]@%s Failed to createBufferPool.", this,
                     __func__);
    mFaceDetector = cros::FaceDetector::Create();

    if (!PlatformData::isFaceEngineSyncRunning(mCameraId)) {
        /* start face engine pthread */
        ret = run("fdSSD" + std::to_string(mCameraId), PRIORITY_NORMAL);
        CheckAndLogError(ret != OK, NO_INIT, "Camera thread failed to start, ret %d", ret);
    }

    mInitialized = true;
    return OK;
}

std::shared_ptr<camera3::Camera3Buffer> FaceSSD::acquireRunCCBuf() {
    std::shared_ptr<camera3::Camera3Buffer> buf = mBufferPool->acquireBuffer();
    CheckAndLogError(buf == nullptr, nullptr, "@%s no available internal buffer", __func__);

    return buf;
}

void FaceSSD::returnRunBuf(std::shared_ptr<camera3::Camera3Buffer> gbmRunBuf) {
    mBufferPool->returnBuffer(gbmRunBuf);
}

void FaceSSD::runFaceDetectionBySync(const std::shared_ptr<camera3::Camera3Buffer>& ccBuf) {
    LOG2("@%s", __func__);
    CheckAndLogError(mInitialized == false, VOID_VALUE, "@%s, mInitialized is false", __func__);
    CheckAndLogError(!ccBuf, VOID_VALUE, "@%s, ccBuf buffer is nullptr", __func__);

    nsecs_t startTime = CameraUtils::systemTime();
    std::vector<human_sensing::CrosFace> faces;
    cros::FaceDetectResult ret = mFaceDetector->Detect(*(ccBuf->getBufferHandle()), &faces);
    printfFDRunRate();
    LOG2("@%s: ret:%d, it takes need %ums", __func__, ret,
         (unsigned)((CameraUtils::systemTime() - startTime) / 1000000));

    {
        AutoMutex l(mFaceResultLock);
        CLEAR(mResult);
        if (ret == cros::FaceDetectResult::kDetectOk) {
            int faceCount = 0;
            for (auto& face : faces) {
                if (faceCount >= mMaxFaceNum) break;
                mResult.faceSsdResults[faceCount] = face;
                faceCount++;
            }
            mResult.faceNum = faceCount;
            mResult.faceUpdated = true;
            LOG2("@%s, faceNum:%d", __func__, mResult.faceNum);
        } else {
            LOGE("@%s, Faile to detect face", __func__);
        }
    }
}

void FaceSSD::runFaceDetectionByAsync(const std::shared_ptr<camera3::Camera3Buffer>& ccBuf) {
    LOG2("@%s", __func__);
    CheckAndLogError(mInitialized == false, VOID_VALUE, "@%s, mInitialized is false", __func__);

    std::shared_ptr<camera3::Camera3Buffer> bufferTmp = acquireRunCCBuf();
    CheckAndLogError(!bufferTmp || !bufferTmp->data(), VOID_VALUE, "No avalible buffer");
    MEMCPY_S(bufferTmp->data(), bufferTmp->size(), ccBuf->data(), ccBuf->size());

    AutoMutex l(mRunBufQueueLock);
    mRunGoogleBufQueue.push(bufferTmp);
    mRunCondition.notify_one();
}

bool FaceSSD::threadLoop() {
    std::shared_ptr<camera3::Camera3Buffer> faceParams = nullptr;

    {
        ConditionLock lock(mRunBufQueueLock);
        if (mRunGoogleBufQueue.empty()) {
            std::cv_status ret = mRunCondition.wait_for(
                lock, std::chrono::nanoseconds(kMaxDuration * SLOWLY_MULTIPLIER));
            if (ret == std::cv_status::timeout) {
                LOGW("wait camera buffer time out");
            }

            return true;
        }
        faceParams = mRunGoogleBufQueue.front();
        mRunGoogleBufQueue.pop();
    }
    CheckAndLogError(!faceParams, false, "@%s, faceParams buffer is nullptr", __func__);

    runFaceDetectionBySync(faceParams);
    returnRunBuf(faceParams);
    return true;
}

void FaceSSD::convertCoordinate(int width, int height, const human_sensing::CrosFace& src,
                                human_sensing::CrosFace* dst) {
    CheckAndLogError(!dst, VOID_VALUE, "dst is nullptr");

    const camera_coordinate_system_t iaCoordinate = {IA_COORDINATE_LEFT, IA_COORDINATE_TOP,
                                                     IA_COORDINATE_RIGHT, IA_COORDINATE_BOTTOM};
    const camera_coordinate_system_t faceCoordinate = {0, 0, width, height};

    camera_coordinate_t topLeft = AiqUtils::convertCoordinateSystem(
        faceCoordinate, iaCoordinate,
        {static_cast<int>(src.bounding_box.x1), static_cast<int>(src.bounding_box.y1)});
    camera_coordinate_t bottomRight = AiqUtils::convertCoordinateSystem(
        faceCoordinate, iaCoordinate,
        {static_cast<int>(src.bounding_box.x2), static_cast<int>(src.bounding_box.y2)});

    dst->bounding_box.x1 = static_cast<float>(topLeft.x);     /* left */
    dst->bounding_box.y1 = static_cast<float>(topLeft.y);     /* top */
    dst->bounding_box.x2 = static_cast<float>(bottomRight.x); /* right */
    dst->bounding_box.y2 = static_cast<float>(bottomRight.y); /* bottom */
}

int FaceSSD::getFaceNum() {
    AutoMutex l(mFaceResultLock);
    return mResult.faceNum;
}

void FaceSSD::getResultFor3A(cca::cca_face_state* faceState) {
    LOG2("@%s", __func__);

    float fovRatioW = mFovInfo.fovRatioW;
    float fovRatioH = mFovInfo.fovRatioH;
    float offsetW = mFovInfo.offsetW;
    float offsetH = mFovInfo.offsetH;

    AutoMutex l(mFaceResultLock);
    FaceSSDResult* faceSsdResult = &mResult;
    faceState->is_video_conf = true;
    faceState->updated = mResult.faceUpdated;
    faceState->num_faces = faceSsdResult->faceNum;
    mResult.faceUpdated = false;

    for (int i = 0; i < faceSsdResult->faceNum; i++) {
        CLEAR(faceState->faces[i]);
        human_sensing::CrosFace googleFaceResult = {};
        convertCoordinate(mWidth, mHeight, faceSsdResult->faceSsdResults[i], &googleFaceResult);
        faceState->faces[i].face_area.left = static_cast<int>(
            static_cast<int>(googleFaceResult.bounding_box.x1) * fovRatioW + offsetW);
        faceState->faces[i].face_area.top = static_cast<int>(
            static_cast<int>(googleFaceResult.bounding_box.y1) * fovRatioH + offsetH);
        faceState->faces[i].face_area.bottom = static_cast<int>(
            static_cast<int>(googleFaceResult.bounding_box.y2) * fovRatioH + offsetH);
        faceState->faces[i].face_area.right = static_cast<int>(
            static_cast<int>(googleFaceResult.bounding_box.x2) * fovRatioW + offsetW);
        faceState->faces[i].rip_angle = 0;
        faceState->faces[i].rop_angle = 0;
        faceState->faces[i].tracking_id = i;
        faceState->faces[i].confidence = faceSsdResult->faceSsdResults[i].confidence;
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

void FaceSSD::getResultForApp(CVFaceDetectionAbstractResult* result) {
    LOG2("@%s", __func__);

    camera_coordinate_system_t sysCoord = mRatioInfo.sysCoord;
    int verticalCrop = mRatioInfo.verticalCrop;
    int horizontalCrop = mRatioInfo.horizontalCrop;
    bool imageRotationChanged = mRatioInfo.imageRotationChanged;
    camera_coordinate_t srcCoord = {0, 0};
    camera_coordinate_t destCoord = {0, 0};
    const camera_coordinate_system_t fillFrameCoord = {0, 0, mWidth + horizontalCrop,
                                                       mHeight + verticalCrop};

    CLEAR(*result);
    AutoMutex l(mFaceResultLock);
    FaceSSDResult* faceSsdResult = &mResult;

    for (int i = 0; i < faceSsdResult->faceNum; i++) {
        result->faceScores[i] = static_cast<int>(faceSsdResult->faceSsdResults[i].confidence * 100);
        result->faceIds[i] = i;
        result->faceRect[i * 4] =
            static_cast<int>(faceSsdResult->faceSsdResults[i].bounding_box.x1);  // rect.left
        result->faceRect[i * 4 + 1] =
            static_cast<int>(faceSsdResult->faceSsdResults[i].bounding_box.y1);  // rect.top
        result->faceRect[i * 4 + 2] =
            static_cast<int>(faceSsdResult->faceSsdResults[i].bounding_box.x2);  // rect.right
        result->faceRect[i * 4 + 3] =
            static_cast<int>(faceSsdResult->faceSsdResults[i].bounding_box.y2);  // rect.bottom
        if (imageRotationChanged) {
            camera_coordinate_t pointCoord = {0, 0};
            pointCoord.x = result->faceRect[i * 4] + (horizontalCrop / 2);
            pointCoord.y = result->faceRect[i * 4 + 1] + (verticalCrop / 2);
            destCoord = AiqUtils::convertCoordinateSystem(fillFrameCoord, sysCoord, pointCoord);
            result->faceRect[i * 4] = destCoord.x;      // rect.left
            result->faceRect[i * 4 + 1] = destCoord.y;  // rect.top
            pointCoord.x = result->faceRect[i * 4 + 2] + (horizontalCrop / 2);
            pointCoord.y = result->faceRect[i * 4 + 3] + (verticalCrop / 2);
            destCoord = AiqUtils::convertCoordinateSystem(fillFrameCoord, sysCoord, pointCoord);
            result->faceRect[i * 4 + 2] = destCoord.x;  // rect.right
            result->faceRect[i * 4 + 3] = destCoord.y;  // rect.bottom
        } else {
            srcCoord = {result->faceRect[i * 4], result->faceRect[i * 4 + 1]};
            destCoord = AiqUtils::convertCoordinateSystem(fillFrameCoord, sysCoord, srcCoord);
            result->faceRect[i * 4] = destCoord.x;      // rect.left
            result->faceRect[i * 4 + 1] = destCoord.y;  // rect.top

            srcCoord = {result->faceRect[i * 4 + 2], result->faceRect[i * 4 + 3]};
            destCoord = AiqUtils::convertCoordinateSystem(fillFrameCoord, sysCoord, srcCoord);
            result->faceRect[i * 4 + 2] = destCoord.x;  // rect.right
            result->faceRect[i * 4 + 3] = destCoord.y;  // rect.bottom
        }
        LOG2("@%s, face info, id:%d, left:%d, top:%d, right:%d, bottom:%d", __func__, i,
             result->faceRect[i * 4], result->faceRect[i * 4 + 1], result->faceRect[i * 4 + 2],
             result->faceRect[i * 4 + 3]);
    }

    result->faceNum = faceSsdResult->faceNum;
}

}  // namespace icamera
