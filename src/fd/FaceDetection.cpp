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

#define LOG_TAG FaceDetection
#include "src/fd/FaceDetection.h"

#include <algorithm>
#include <fstream>

#include "AiqUtils.h"
#include "PlatformData.h"
#include "iutils/CameraLog.h"
#include "iutils/Utils.h"

namespace icamera {
#define FPS_FD_COUNT 60  // the face detection interval to print fps

std::unordered_map<int, FaceDetection*> FaceDetection::sInstances;
Mutex FaceDetection::sLock;

FaceDetection* FaceDetection::getInstance(int cameraId) {
    if (sInstances.find(cameraId) == sInstances.end()) {
        return nullptr;
    }

    return sInstances[cameraId];
}

FaceDetection* FaceDetection::createInstance(int cameraId, unsigned int maxFaceNum,
                                             int32_t halStreamId, int width, int height) {
    LOG1("<id%d> @%s, maxFaceNum:%u, halStreamId:%u, width:%d, height:%d", cameraId, __func__,
         maxFaceNum, halStreamId, width, height);
    CheckAndLogError(maxFaceNum > MAX_FACES_DETECTABLE, nullptr, "maxFaceNum %d is error",
                     maxFaceNum);

    AutoMutex lock(sLock);
    if (sInstances.find(cameraId) == sInstances.end()) {
        FaceDetection* fd = new FaceDetection(cameraId, maxFaceNum, halStreamId, width, height);
        if (!fd->isInitialized()) {
            delete fd;
            return nullptr;
        }
        sInstances[cameraId] = fd;
    }

    return sInstances[cameraId];
}

void FaceDetection::destoryInstance(int cameraId) {
    LOG1("<id%d> @%s", cameraId, __func__);
    AutoMutex lock(sLock);
    if (sInstances.find(cameraId) != sInstances.end()) {
        delete sInstances[cameraId];
        sInstances.erase(cameraId);
    }
}

FaceDetection::FaceDetection(int cameraId, unsigned int maxFaceNum, int32_t halStreamId, int width,
                             int height)
        : mCameraId(cameraId),
          mInitialized(false),
          mHalStreamId(halStreamId),
          mWidth(width),
          mHeight(height),
          mFDRunDefaultInterval(icamera::PlatformData::faceEngineRunningInterval(cameraId)),
          mFDRunIntervalNoFace(icamera::PlatformData::faceEngineRunningIntervalNoFace(cameraId)),
          mFDRunInterval(icamera::PlatformData::faceEngineRunningInterval(cameraId)),
          mFrameCnt(0),
          mRunCount(0) {
    LOG1("<id%d> default interval:%d, interval no face:%d, run interval:%d", cameraId,
         mFDRunDefaultInterval, mFDRunIntervalNoFace, mFDRunInterval);

    /* start face engine pthread */
    int ret = run("FaceDetection" + std::to_string(cameraId), PRIORITY_NORMAL);
    CheckAndLogError(ret != OK, VOID_VALUE, "Camera thread failed to start, ret %d", ret);

    /* init IntelFaceDetection */
    FaceDetectionInitParams params;
    params.max_face_num = maxFaceNum;
    params.cameraId = mCameraId;
    mFace = std::unique_ptr<IntelFaceDetection>(new IntelFaceDetection());
    ret = mFace->init(&params, sizeof(FaceDetectionInitParams));
    CheckAndLogError(ret != OK, VOID_VALUE, "mFace init failed, ret %d", ret);

    for (int i = 0; i < MAX_STORE_FACE_DATA_BUF_NUM; i++) {
        FaceDetectionRunParams* memRunBuf = mFace->prepareRunBuffer(i);
        CheckAndLogError(!memRunBuf, VOID_VALUE, "prepareRunBuffer failed");
        mMemRunPool.push(memRunBuf);
    }

    gettimeofday(&mRequestRunTime, nullptr);
    CLEAR(mResult);

    /* TODO: we should add CameraOrientationDetector to change the camera orientation */
    camera_info_t info;
    PlatformData::getCameraInfo(mCameraId, info);
    mSensorOrientation = info.orientation;

    mInitialized = true;
}

FaceDetection::~FaceDetection() {
    LOG1("<id%d> @%s", mCameraId, __func__);

    FaceDetectionDeinitParams params;
    params.cameraId = mCameraId;
    mFace->deinit(&params, sizeof(FaceDetectionDeinitParams));

    requestExit();

    AutoMutex l(mRunBufQueueLock);
    mRunCondition.notify_one();
}

FaceDetectionRunParams* FaceDetection::acquireRunBuf() {
    AutoMutex l(mMemRunPoolLock);
    FaceDetectionRunParams* runBuffer = nullptr;
    if (!mMemRunPool.empty()) {
        runBuffer = mMemRunPool.front();
        mMemRunPool.pop();
        CLEAR(*runBuffer);
    }
    return runBuffer;
}

void FaceDetection::returnRunBuf(FaceDetectionRunParams* memRunBuf) {
    AutoMutex l(mMemRunPoolLock);
    mMemRunPool.push(memRunBuf);
}

bool FaceDetection::faceRunningByCondition() {
    /*
     * FD runs 1 frame every mFDRunInterval frames.
     * And the default value of mFDRunInterval is mFDRunDefaultInterval
     */
    if (mFrameCnt % mFDRunInterval == 0) {
        ++mFrameCnt;
        return true;
    }

    /*
     * When face doesn't be detected during mFDRunIntervalNoFace's frame,
     * we may change FD running's interval frames.
     */
    if (mFDRunIntervalNoFace > mFDRunDefaultInterval) {
        static unsigned int noFaceCnt = 0;
        int faceNum = getFaceNum();

        /*
         * The purpose of changing the value of the variable is to run FD
         * immediately when face is detected.
         */
        if (faceNum == 0) {
            if (mFDRunInterval != mFDRunIntervalNoFace) {
                noFaceCnt = ++noFaceCnt % mFDRunIntervalNoFace;
                if (noFaceCnt == 0) {
                    mFDRunInterval = mFDRunIntervalNoFace;
                }
            }
        } else {
            if (mFDRunInterval != mFDRunDefaultInterval) {
                mFDRunInterval = mFDRunDefaultInterval;
                mFrameCnt = mFDRunInterval - 1;
                noFaceCnt = 0;
            }
        }

        LOG2("Running face detection for every %d frames, faceNum %d", mFDRunInterval, faceNum);
    }

    mFrameCnt = ++mFrameCnt % mFDRunInterval;
    return false;
}

void FaceDetection::printfFDRunRate() {
    if (!Log::isDebugLevelEnable(CAMERA_DEBUG_LOG_FACE_DETECTION_FPS)) return;

    mRunCount++;
    if (mRunCount % FPS_FD_COUNT != 0) return;

    struct timeval curTime;
    gettimeofday(&curTime, nullptr);
    int duration = static_cast<int>(curTime.tv_usec - mRequestRunTime.tv_usec +
                                    ((curTime.tv_sec - mRequestRunTime.tv_sec) * 1000000));
    float curFps = static_cast<float>(1000000) / static_cast<float>(duration / FPS_FD_COUNT);
    LOGFDFPS("@%s, face detection fps: %02f", __func__, curFps);
    mRequestRunTime = curTime;
}

void FaceDetection::runFaceDetection(const std::shared_ptr<camera3::Camera3Buffer> ccBuf) {
    if (PlatformData::isFaceEngineSyncRunning(mCameraId)) {
        runFaceDetectionBySync(ccBuf);
    } else {
        runFaceDetectionByAsync(ccBuf);
    }
}

void FaceDetection::runFaceDetectionBySync(const std::shared_ptr<camera3::Camera3Buffer> ccBuf) {
    LOG2("@%s", __func__);

    if (!faceRunningByCondition()) return;

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
    /* TODO: image.rotation is (mSensorOrientation + mCamOriDetector->getOrientation()) % 360 */
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
        }
    }

    returnRunBuf(params);
}

void FaceDetection::runFaceDetectionByAsync(const std::shared_ptr<camera3::Camera3Buffer> ccBuf) {
    LOG2("@%s", __func__);

    if (!faceRunningByCondition()) return;

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
    mRunBufQueue.push(params);
    mRunCondition.notify_one();
}

bool FaceDetection::threadLoop() {
    FaceDetectionRunParams* faceParams = nullptr;

    {
        ConditionLock lock(mRunBufQueueLock);
        if (mRunBufQueue.empty()) {
            std::cv_status ret = mRunCondition.wait_for(
                lock, std::chrono::nanoseconds(kMaxDuration * SLOWLY_MULTIPLIER));
            if (ret == std::cv_status::timeout) {
                LOGW("wait camera buffer time out");
            }

            return true;
        }
        faceParams = mRunBufQueue.front();
        mRunBufQueue.pop();
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
        }
    }

    returnRunBuf(faceParams);
    return true;
}

int FaceDetection::getFaceNum() {
    AutoMutex l(mFaceResultLock);
    return mResult.faceNum;
}

/* The result is pvl's original reuslt */
int FaceDetection::getFaceDetectionResult(FaceDetectionResult* result, bool resetRes) {
    AutoMutex l(mFaceResultLock);
    MEMCPY_S(result, sizeof(FaceDetectionResult), &mResult, sizeof(FaceDetectionResult));

    if (resetRes) {
        mResult.faceUpdated = false;
    }

    return OK;
}

/* Get current frame width and hight */
void FaceDetection::getCurrentFrameWidthAndHight(int* frameWidth, int* frameHigth) {
    *frameWidth = mWidth;
    *frameHigth = mHeight;
}

/* Get current hal stream id */
void FaceDetection::getHalStreamId(int32_t* halStreamId) {
    *halStreamId = mHalStreamId;
}

/* The result for 3A AE */
int FaceDetection::getResult(int cameraId, cca::cca_face_state* faceState) {
    LOG2("@%s", __func__);
    CheckAndLogError(!faceState, UNKNOWN_ERROR, "faceState is nullptr");

    int width = 0;
    int height = 0;
    int32_t halStreamId = 0;

    FaceDetectionResult faceDetectionResult;
    {
        AutoMutex lock(sLock);
        FaceDetection* fdInstance = FaceDetection::getInstance(cameraId);
        if (!fdInstance) {
            LOG2("%s, It doesn't have face detection instance", __func__);
            return UNKNOWN_ERROR;
        }
        int ret = fdInstance->getFaceDetectionResult(&faceDetectionResult, true);
        CheckAndLogError(ret != OK, UNKNOWN_ERROR, "Failed to get result, ret %d", ret);
        fdInstance->getCurrentFrameWidthAndHight(&width, &height);
        fdInstance->getHalStreamId(&halStreamId);
    }

    /*
     * face rectangle from face lib: (Ln, Tn, Rn, Bn)
     * 3A statistics Surface: ((IA_COORDINATE_RIGHT - IA_COORDINATE_LEFT) *
     *                         (IA_COORDINATE_BOTTOM - IA_COORDINATE_TOP))
     * target coordinate of face rectangle to the 3A lib: (LL, TT, RR, BB)
     * FOV ratio (which is <= 1): (fovRatioW * fovRatioH)
     *
     * formular:
     * LL = Ln * fovRatioW + (1 - fovRatioW) / 2 * (IA_COORDINATE_RIGHT - IA_COORDINATE_LEFT)
     * TT = Tn * fovRatioH + (1 - fovRatioH) / 2 * (IA_COORDINATE_BOTTOM - IA_COORDINATE_TOP)
     * RR and BB are the similar.
     */

    float fovRatioW = 1;
    float fovRatioH = 1;
    int ret = PlatformData::getScalerInfo(cameraId, halStreamId, &fovRatioW, &fovRatioH);
    LOG2("getScalerInfo ret:%d, fovRatioW:%f, fovRatioH:%f", ret, fovRatioW, fovRatioH);

    camera_coordinate_system_t activePixelArray = PlatformData::getActivePixelArray(cameraId);
    float fovRatioWTmp = fovRatioW * width / (activePixelArray.right - activePixelArray.left);
    float fovRatioHTmp = fovRatioH * height / (activePixelArray.bottom - activePixelArray.top);
    float offsetW = (1.0 - fovRatioWTmp) / 2.0 * (IA_COORDINATE_RIGHT - IA_COORDINATE_LEFT);
    float offsetH = (1.0 - fovRatioHTmp) / 2.0 * (IA_COORDINATE_BOTTOM - IA_COORDINATE_TOP);

    LOG2("faceNum:%d, mHeight:%d, mWidth:%d, faceUpdated: %d", faceDetectionResult.faceNum,
         height, width, faceDetectionResult.faceUpdated);

    faceState->is_video_conf = true;
    faceState->updated = faceDetectionResult.faceUpdated;
    faceState->num_faces = faceDetectionResult.faceNum;

    for (int i = 0; i < faceDetectionResult.faceNum; i++) {
        CLEAR(faceState->faces[i]);
        faceState->faces[i].face_area.left =
            static_cast<int>(faceDetectionResult.faceResults[i].rect.left * fovRatioWTmp + offsetW);
        faceState->faces[i].face_area.top =
            static_cast<int>(faceDetectionResult.faceResults[i].rect.top * fovRatioHTmp + offsetH);
        faceState->faces[i].face_area.bottom = static_cast<int>(
            faceDetectionResult.faceResults[i].rect.bottom * fovRatioHTmp + offsetH);
        faceState->faces[i].face_area.right = static_cast<int>(
            faceDetectionResult.faceResults[i].rect.right * fovRatioWTmp + offsetW);
        faceState->faces[i].rip_angle = faceDetectionResult.faceResults[i].rip_angle;
        faceState->faces[i].rop_angle = faceDetectionResult.faceResults[i].rop_angle;
        faceState->faces[i].tracking_id = faceDetectionResult.faceResults[i].tracking_id;
        faceState->faces[i].confidence = faceDetectionResult.faceResults[i].confidence;
        faceState->faces[i].person_id = -1;
        faceState->faces[i].similarity = 0;
        faceState->faces[i].best_ratio = 0;
        faceState->faces[i].face_condition = 0;

        faceState->faces[i].smile_state = 0;
        faceState->faces[i].smile_score = 0;
        faceState->faces[i].mouth.x =
            static_cast<int>(faceDetectionResult.mouthResults[i].mouth.x * fovRatioWTmp + offsetW);
        faceState->faces[i].mouth.y =
            static_cast<int>(faceDetectionResult.mouthResults[i].mouth.y * fovRatioHTmp + offsetH);

        faceState->faces[i].eye_validity = 0;
    }

    return OK;
}

/* The result for android statistics metadata */
int FaceDetection::getResult(int cameraId, CVFaceDetectionAbstractResult* result) {
    LOG2("@%s", __func__);
    CheckAndLogError(!result, UNKNOWN_ERROR, "result is nullptr");

    int width = 0;
    int height = 0;
    FaceDetectionResult faceDetectionResult;
    {
        AutoMutex lock(sLock);
        FaceDetection* fdInstance = FaceDetection::getInstance(cameraId);
        if (!fdInstance) {
            LOG2("%s, It doesn't have face detection instance", __func__);
            return UNKNOWN_ERROR;
        }

        int ret = fdInstance->getFaceDetectionResult(&faceDetectionResult);
        CheckAndLogError(ret != OK, UNKNOWN_ERROR, "Failed to get result");
        fdInstance->getCurrentFrameWidthAndHight(&width, &height);
    }

    const camera_coordinate_system_t iaCoord = {IA_COORDINATE_LEFT, IA_COORDINATE_TOP,
                                                IA_COORDINATE_RIGHT, IA_COORDINATE_BOTTOM};

    // construct android coordinate based on active pixel array
    camera_coordinate_system_t activePixelArray = PlatformData::getActivePixelArray(cameraId);

    int activeHeight = activePixelArray.bottom - activePixelArray.top;
    int activeWidth = activePixelArray.right - activePixelArray.left;
    const camera_coordinate_system_t sysCoord = {0, 0, activeWidth, activeHeight};
    camera_coordinate_t srcCoord = {0, 0};
    camera_coordinate_t destCoord = {0, 0};

    int verticalCrop = 0, horizontalCrop = 0;
    bool imageRotationUnchanged = true;

    // do extra conversion if the image ratio is not the same ratio with the android coordinate.
    if (height * activeWidth != width * activeHeight) {
        imageRotationUnchanged = false;
        int gap = (width * activeHeight / activeWidth) - height;

        if (gap > 0) {
            // vertical crop pixel
            verticalCrop = gap;
        } else if (gap < 0) {
            // horizontal crop pixel
            horizontalCrop = height * activeWidth / activeHeight - width;
        }
    }

    const camera_coordinate_system_t fillFrameCoord = {0, 0, width + horizontalCrop,
                                                       height + verticalCrop};
    const camera_coordinate_system_t frameCoord = {0, 0, width, height};

    CLEAR(*result);
    for (int i = 0; i < faceDetectionResult.faceNum; i++) {
        if (i == MAX_FACES_DETECTABLE) break;

        camera_coordinate_t pointCoord = {0, 0};
        result->faceScores[i] = faceDetectionResult.faceResults[i].confidence;
        result->faceIds[i] = faceDetectionResult.faceResults[i].tracking_id;

        if (imageRotationUnchanged) {
            srcCoord = {faceDetectionResult.faceResults[i].rect.left,
                        faceDetectionResult.faceResults[i].rect.top};
            destCoord = AiqUtils::convertCoordinateSystem(iaCoord, sysCoord, srcCoord);
            result->faceRect[i * 4] = destCoord.x;      // rect.left
            result->faceRect[i * 4 + 1] = destCoord.y;  // rect.top

            srcCoord = {faceDetectionResult.faceResults[i].rect.right,
                        faceDetectionResult.faceResults[i].rect.bottom};
            destCoord = AiqUtils::convertCoordinateSystem(iaCoord, sysCoord, srcCoord);
            result->faceRect[i * 4 + 2] = destCoord.x;  // rect.right
            result->faceRect[i * 4 + 3] = destCoord.y;  // rect.bottom

            srcCoord = {faceDetectionResult.eyeResults[i].left_eye.x,
                        faceDetectionResult.eyeResults[i].left_eye.y};
            destCoord = AiqUtils::convertCoordinateSystem(iaCoord, sysCoord, srcCoord);
            result->faceLandmarks[i * 6] = destCoord.x;      // left_eye.x;
            result->faceLandmarks[i * 6 + 1] = destCoord.y;  // left_eye.y;

            srcCoord = {faceDetectionResult.eyeResults[i].right_eye.x,
                        faceDetectionResult.eyeResults[i].right_eye.y};
            destCoord = AiqUtils::convertCoordinateSystem(iaCoord, sysCoord, srcCoord);
            result->faceLandmarks[i * 6 + 2] = destCoord.x;  // right_eye.x;
            result->faceLandmarks[i * 6 + 3] = destCoord.y;  // right_eye.y;

            srcCoord = {faceDetectionResult.mouthResults[i].mouth.x,
                        faceDetectionResult.mouthResults[i].mouth.y};
            destCoord = AiqUtils::convertCoordinateSystem(iaCoord, sysCoord, srcCoord);
            result->faceLandmarks[i * 6 + 4] = destCoord.x;  // mouth.x;
            result->faceLandmarks[i * 6 + 5] = destCoord.y;  // mouth.y;
        } else {
            srcCoord = {faceDetectionResult.faceResults[i].rect.left,
                        faceDetectionResult.faceResults[i].rect.top};
            pointCoord = AiqUtils::convertCoordinateSystem(iaCoord, frameCoord, srcCoord);
            pointCoord.x += horizontalCrop / 2;
            pointCoord.y += verticalCrop / 2;
            destCoord = AiqUtils::convertCoordinateSystem(fillFrameCoord, sysCoord, pointCoord);
            result->faceRect[i * 4] = destCoord.x;      // rect.left
            result->faceRect[i * 4 + 1] = destCoord.y;  // rect.top

            srcCoord = {faceDetectionResult.faceResults[i].rect.right,
                        faceDetectionResult.faceResults[i].rect.bottom};
            pointCoord = AiqUtils::convertCoordinateSystem(iaCoord, frameCoord, srcCoord);
            pointCoord.x += horizontalCrop / 2;
            pointCoord.y += verticalCrop / 2;
            destCoord = AiqUtils::convertCoordinateSystem(fillFrameCoord, sysCoord, pointCoord);
            result->faceRect[i * 4 + 2] = destCoord.x;  // rect.right
            result->faceRect[i * 4 + 3] = destCoord.y;  // rect.bottom

            srcCoord = {faceDetectionResult.eyeResults[i].left_eye.x,
                        faceDetectionResult.eyeResults[i].left_eye.y};
            pointCoord = AiqUtils::convertCoordinateSystem(iaCoord, frameCoord, srcCoord);
            pointCoord.x += horizontalCrop / 2;
            pointCoord.y += verticalCrop / 2;
            destCoord = AiqUtils::convertCoordinateSystem(fillFrameCoord, sysCoord, pointCoord);
            result->faceLandmarks[i * 6] = destCoord.x;      // left_eye.x;
            result->faceLandmarks[i * 6 + 1] = destCoord.y;  // left_eye.y;

            srcCoord = {faceDetectionResult.eyeResults[i].right_eye.x,
                        faceDetectionResult.eyeResults[i].right_eye.y};
            pointCoord = AiqUtils::convertCoordinateSystem(iaCoord, frameCoord, srcCoord);
            pointCoord.x += horizontalCrop / 2;
            pointCoord.y += verticalCrop / 2;
            destCoord = AiqUtils::convertCoordinateSystem(fillFrameCoord, sysCoord, pointCoord);
            result->faceLandmarks[i * 6 + 2] = destCoord.x;  // right_eye.x;
            result->faceLandmarks[i * 6 + 3] = destCoord.y;  // right_eye.y;

            srcCoord = {faceDetectionResult.mouthResults[i].mouth.x,
                        faceDetectionResult.mouthResults[i].mouth.y};
            pointCoord = AiqUtils::convertCoordinateSystem(iaCoord, frameCoord, srcCoord);
            pointCoord.x += horizontalCrop / 2;
            pointCoord.y += verticalCrop / 2;
            destCoord = AiqUtils::convertCoordinateSystem(fillFrameCoord, sysCoord, pointCoord);
            result->faceLandmarks[i * 6 + 4] = destCoord.x;  // mouth.x;
            result->faceLandmarks[i * 6 + 5] = destCoord.y;  // mouth.y;
        }
    }
    result->faceNum =
        (faceDetectionResult.faceNum < MAX_FACES_DETECTABLE ? faceDetectionResult.faceNum
                                                            : MAX_FACES_DETECTABLE);
    return OK;
}

}  // namespace icamera
