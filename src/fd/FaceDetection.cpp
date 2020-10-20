/*
 * Copyright (C) 2019-2020 Intel Corporation
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

#define LOG_TAG "FaceDetection"
#include "src/fd/FaceDetection.h"

#include <algorithm>
#include <fstream>

#include "AiqUtils.h"
#include "iutils/CameraLog.h"
#include "iutils/Utils.h"
#include "PlatformData.h"

namespace icamera {
std::unordered_map<int, FaceDetection*> FaceDetection::sInstances;
Mutex FaceDetection::sLock;
FaceDetection *FaceDetection::getInstance(int cameraId) {
    LOG1("@%s, cameraId:%d", __func__, cameraId);
    CheckError(cameraId < 0 || cameraId >= PlatformData::numberOfCameras(),
               nullptr, "cameraId %d is error", cameraId);

    if (sInstances.find(cameraId) == sInstances.end()) {
        return nullptr;
    }

    return sInstances[cameraId];
}

FaceDetection *FaceDetection::createInstance(int cameraId,
                                             unsigned int maxFaceNum, int32_t halStreamId,
                                             int width, int height) {
    LOG1("@%s, cameraId:%d, maxFaceNum:%u, halStreamId:%u, width:%d, height:%d",
         __func__, cameraId, maxFaceNum, halStreamId, width, height);
    CheckError(maxFaceNum > MAX_FACES_DETECTABLE, nullptr,
               "maxFaceNum %d is error", maxFaceNum);
    CheckError(cameraId < 0 || cameraId >= PlatformData::numberOfCameras(),
               nullptr, "cameraId %d is error", cameraId);

    AutoMutex lock(sLock);
    if (sInstances.find(cameraId) == sInstances.end()) {
        sInstances[cameraId] = new FaceDetection(cameraId, maxFaceNum,
                                                 halStreamId, width, height);
    }

    return sInstances[cameraId];
}

void FaceDetection::destoryInstance(int cameraId) {
    LOG1("@%s, cameraId:%d", __func__, cameraId);
    CheckError(cameraId < 0 || cameraId >= PlatformData::numberOfCameras(),
               VOID_VALUE, "cameraId is error");

    AutoMutex lock(sLock);
    if (sInstances.find(cameraId) != sInstances.end()) {
        delete sInstances[cameraId];
        sInstances.erase(cameraId);
    }
}

FaceDetection::FaceDetection(int cameraId, unsigned int maxFaceNum, int32_t halStreamId,
                             int width, int height) :
    mCameraId(cameraId),
    mInitialized(false),
    mHalStreamId(halStreamId),
    mWidth(width),
    mHeight(height) {
    LOG1("@%s, cameraId:%d, maxFaceNum:%d", __func__, cameraId, maxFaceNum);
    CLEAR(mResult);

    /* TODO: we should add CameraOrientationDetector to change the camera orientation */
    camera_info_t info;
    PlatformData::getCameraInfo(mCameraId, info);
    mSensorOrientation = info.orientation;

    /* start face engine pthread */
    int ret = run("FaceDetection" + std::to_string(cameraId), PRIORITY_NORMAL);
    CheckError(ret != OK, VOID_VALUE, "Camera thread failed to start, ret %d", ret);

    /* init IntelFaceDetection */
    FaceDetectionInitParams params;
    params.max_face_num = maxFaceNum;
    mFace = std::unique_ptr<IntelFaceDetection>(new IntelFaceDetection());
    ret = mFace->init(&params, sizeof(FaceDetectionInitParams));
    CheckError(ret != OK, VOID_VALUE, "mFace.init fails, ret %d", ret);

    for (int i = 0; i < MAX_STORE_FACE_DATA_BUF_NUM; i++) {
        FaceDetectionRunParams *memRunBuf = mFace->prepareRunBuffer(i);
        CheckError(!memRunBuf, VOID_VALUE, "prepareRunBuffer fails");
        mMemRunPool.push(memRunBuf);
    }

    mInitialized = true;
}

FaceDetection::~FaceDetection() {
    LOG1("@%s", __func__);
    mFace->deinit();
    requestExit();

    AutoMutex l(mRunBufQueueLock);
    mRunCondition.notify_one();
}

FaceDetectionRunParams *FaceDetection::acquireRunBuf() {
    AutoMutex l(mMemRunPoolLock);
    LOG2("@%s, mRunPool.size is %zu", __func__, mMemRunPool.size());

    FaceDetectionRunParams *runBuffer = nullptr;
    if (!mMemRunPool.empty()) {
        runBuffer = mMemRunPool.front();
        mMemRunPool.pop();
        CLEAR(*runBuffer);
    }
    return runBuffer;
}

void FaceDetection::returnRunBuf(FaceDetectionRunParams *memRunBuf) {
    LOG2("@%s, Push back run face engine buffer", __func__);

    AutoMutex l(mMemRunPoolLock);
    mMemRunPool.push(memRunBuf);
}

void FaceDetection::runFaceDetection(const camera_buffer_t &buffer) {
    LOG1("@%s", __func__);
    CheckError(mInitialized == false, VOID_VALUE, "mInitialized is false");

    if (PlatformData::isFaceEngineSyncRunning(mCameraId)) {
        runFaceDetectionBySync(buffer);
    } else {
        runFaceDetectionByAsync(buffer);
    }
}

void FaceDetection::runFaceDetectionBySync(const camera_buffer_t &buffer) {
    LOG1("@%s", __func__);
    CheckError(mInitialized == false, VOID_VALUE, "mInitialized is false");

    int size = buffer.s.size;
    CheckError(size > MAX_FACE_FRAME_SIZE_SYNC, VOID_VALUE,
               "face frame buffer is too small!, w:%d,h:%d,size:%d",
               buffer.s.width, buffer.s.height, size);

    FaceDetectionRunParams *params = acquireRunBuf();
    CheckError(!params, VOID_VALUE, "Fail to acquire face engine buffer");

    params->size = size;
    params->width = buffer.s.width;
    params->height = buffer.s.height;
    /* TODO: image.rotation is (mSensorOrientation + mCamOriDetector->getOrientation()) % 360 */
    params->rotation = mSensorOrientation % 360;
    params->format = pvl_image_format_nv12;
    params->stride = buffer.s.stride;
    params->bufferHandle = -1;

    nsecs_t startTime = CameraUtils::systemTime();

#ifdef ENABLE_SANDBOXING
    int ret = mFace->run(params, sizeof(FaceDetectionRunParams), buffer.dmafd);
#else
    int ret = mFace->run(params, sizeof(FaceDetectionRunParams), buffer.addr);
#endif

    LOG2("@%s: ret:%d, it takes need %ums", __func__, ret,
         (unsigned)((CameraUtils::systemTime() - startTime) / 1000000));

    {
        AutoMutex l(mFaceResultLock);
        if (ret == OK) {
            mResult = params->results;
        } else {
            CLEAR(mResult);
        }
    }

    returnRunBuf(params);
}

void FaceDetection::runFaceDetectionByAsync(const camera_buffer_t &buffer) {
    LOG1("@%s", __func__);
    CheckError(mInitialized == false, VOID_VALUE, "mInitialized is false");

    int size = buffer.s.stride * buffer.s.height;
    CheckError(size > MAX_FACE_FRAME_SIZE_ASYNC, VOID_VALUE,
               "face frame buffer is too small!, w:%d,h:%d,size:%d",
               buffer.s.width, buffer.s.height, size);

    FaceDetectionRunParams *params = acquireRunBuf();
    CheckError(!params, VOID_VALUE, "Fail to acquire face engine buffer");

    params->size = size;
    MEMCPY_S(params->data, MAX_FACE_FRAME_SIZE_ASYNC, buffer.addr, size);
    params->width = buffer.s.width;
    params->height = buffer.s.height;
    /* TODO: image.rotation is (mSensorOrientation + mCamOriDetector->getOrientation()) % 360 */
    params->rotation = mSensorOrientation % 360;
    params->format = pvl_image_format_gray;
    params->stride = buffer.s.stride;
    params->bufferHandle = -1;

    AutoMutex l(mRunBufQueueLock);
    mRunBufQueue.push(params);
    mRunCondition.notify_one();
}

bool FaceDetection::threadLoop() {
    LOG1("@%s", __func__);

    FaceDetectionRunParams *faceParams = nullptr;

    {
        ConditionLock lock(mRunBufQueueLock);
        if (mRunBufQueue.empty()) {
            std::cv_status ret = mRunCondition.wait_for(
                                     lock,
                                     std::chrono::nanoseconds(kMaxDuration * SLOWLY_MULTIPLIER));
            if (ret == std::cv_status::timeout) {
                LOGW("@%s, wait request time out", __func__);
            }

            return true;
        }
        faceParams = mRunBufQueue.front();
        mRunBufQueue.pop();
    }

    nsecs_t startTime = CameraUtils::systemTime();

    int ret = mFace->run(faceParams, sizeof(FaceDetectionRunParams));
    LOG2("@%s: ret:%d, it takes need %ums", __func__, ret,
         (unsigned)((CameraUtils::systemTime() - startTime) / 1000000));

    {
        AutoMutex l(mFaceResultLock);
        if (ret == OK) {
            mResult = faceParams->results;
        } else {
            CLEAR(mResult);
        }
    }

    returnRunBuf(faceParams);
    return true;
}

int FaceDetection::getFaceNum() {
    LOG2("@%s", __func__);
    CheckError(mInitialized == false, 0, "mInitialized is false");

    AutoMutex l(mFaceResultLock);
    return mResult.faceNum;
}

/* The result is pvl's original reuslt */
int FaceDetection::getFaceDetectionResult(FaceDetectionResult *result) {
    LOG1("@%s", __func__);
    CheckError(mInitialized == false, UNKNOWN_ERROR, "mInitialized is false");
    CheckError(!result, UNKNOWN_ERROR, "mResult is nullptr");

    AutoMutex l(mFaceResultLock);
    MEMCPY_S(result, sizeof(FaceDetectionResult), &mResult, sizeof(FaceDetectionResult));
    return OK;
}

/* Get current frame width and hight */
void FaceDetection::getCurrentFrameWidthAndHight(int *frameWidth, int *frameHigth) {
    LOG2("@%s", __func__);
    CheckError(mInitialized == false, VOID_VALUE, "mInitialized is false");
    CheckError(!frameWidth || !frameHigth, VOID_VALUE, "input paramter is error");

    *frameWidth = mWidth;
    *frameHigth = mHeight;
}

/* Get current hal stream id */
void FaceDetection::getHalStreamId(int32_t *halStreamId) {
    LOG2("@%s", __func__);
    CheckError(mInitialized == false, VOID_VALUE, "mInitialized is false");
    CheckError(!halStreamId, VOID_VALUE, "halStreamId is nullptr");

    *halStreamId = mHalStreamId;
}

/* The result for 3A AE */
int FaceDetection::getResult(int cameraId, ia_atbx_face_state *faceState) {
    LOG1("@%s", __func__);
    CheckError(!faceState, UNKNOWN_ERROR, "faceState is nullptr");
    CheckError(cameraId < 0 || cameraId >= PlatformData::numberOfCameras(),
               UNKNOWN_ERROR, "cameraId %d is error", cameraId);

    int width = 0;
    int height = 0;
    int32_t halStreamId = 0;

    FaceDetectionResult faceDetectionResult;
    {
        AutoMutex lock(sLock);
        FaceDetection *fdInstance = FaceDetection::getInstance(cameraId);
        CheckError(!fdInstance, UNKNOWN_ERROR, "Failed to get instance");

        int ret = fdInstance->getFaceDetectionResult(&faceDetectionResult);
        CheckError(ret != OK, UNKNOWN_ERROR, "Failed to get result, ret %d", ret);
        fdInstance->getCurrentFrameWidthAndHight(&width, &height);
        fdInstance->getHalStreamId(&halStreamId);
    }

    /*
    face rectangle from face lib: (Ln, Tn, Rn, Bn)
    3A statistics Surface: ((IA_COORDINATE_RIGHT - IA_COORDINATE_LEFT) *
                            (IA_COORDINATE_BOTTOM - IA_COORDINATE_TOP))
    target coordinate of face rectangle to the 3A lib: (LL, TT, RR, BB)
    FOV ratio (which is <= 1): (fovRatioW * fovRatioH)

    formular:
    LL = Ln * fovRatioW + (1 - fovRatioW) / 2 * (IA_COORDINATE_RIGHT - IA_COORDINATE_LEFT)
    TT = Tn * fovRatioH + (1 - fovRatioH) / 2 * (IA_COORDINATE_BOTTOM - IA_COORDINATE_TOP)
    RR and BB are the similar.
    */

    float fovRatioW = 1;
    float fovRatioH = 1;
    int ret = PlatformData::getScalerInfo(cameraId, halStreamId, &fovRatioW, &fovRatioH);
    LOG2("@%s, getScalerInfo ret:%d, fovRatioW:%f, fovRatioH:%f",
         __func__, ret, fovRatioW, fovRatioH);

    camera_coordinate_system_t activePixelArray = PlatformData::getActivePixelArray(cameraId);
    float fovRatioWTmp = fovRatioW * width / (activePixelArray.right - activePixelArray.left);
    float fovRatioHTmp = fovRatioH * height / (activePixelArray.bottom - activePixelArray.top);
    float offsetW = (1.0 - fovRatioWTmp) / 2.0 * (IA_COORDINATE_RIGHT - IA_COORDINATE_LEFT);
    float offsetH = (1.0 - fovRatioHTmp) / 2.0 * (IA_COORDINATE_BOTTOM - IA_COORDINATE_TOP);

    LOG1("@%s, faceNum:%d, mHeight:%d, mWidth:%d", __func__,
         faceDetectionResult.faceNum, height, width);

    faceState->num_faces = faceDetectionResult.faceNum;

    for (int i = 0; i < faceDetectionResult.faceNum; i++) {
        CLEAR(faceState->faces[i]);
        faceState->faces[i].face_area.left =
         static_cast<int>(faceDetectionResult.faceResults[i].rect.left * fovRatioWTmp + offsetW);
        faceState->faces[i].face_area.top =
         static_cast<int>(faceDetectionResult.faceResults[i].rect.top * fovRatioHTmp + offsetH);
        faceState->faces[i].face_area.bottom =
         static_cast<int>(
                         faceDetectionResult.faceResults[i].rect.bottom * fovRatioHTmp + offsetH);
        faceState->faces[i].face_area.right =
         static_cast<int>(
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
int FaceDetection::getResult(int cameraId, CVFaceDetectionAbstractResult *result) {
    LOG1("@%s", __func__);
    CheckError(!result, UNKNOWN_ERROR, "result is nullptr");
    CheckError(cameraId < 0 || cameraId >= PlatformData::numberOfCameras(),
               UNKNOWN_ERROR, "cameraId %d is error", cameraId);

    int width = 0;
    int height = 0;
    FaceDetectionResult faceDetectionResult;
    {
        AutoMutex lock(sLock);
        FaceDetection *fdInstance = FaceDetection::getInstance(cameraId);
        CheckError(!fdInstance, UNKNOWN_ERROR, "Failed to get instance");

        int ret = fdInstance->getFaceDetectionResult(&faceDetectionResult);
        CheckError(ret != OK, UNKNOWN_ERROR, "Failed to get result");
        fdInstance->getCurrentFrameWidthAndHight(&width, &height);
    }

    const camera_coordinate_system_t iaCoord = {IA_COORDINATE_LEFT, IA_COORDINATE_TOP,
                                                IA_COORDINATE_RIGHT, IA_COORDINATE_BOTTOM};

    // construct android coordinate based on active pixel array
    camera_coordinate_system_t activePixelArray = PlatformData::getActivePixelArray(cameraId);

    int activeHeight = activePixelArray.bottom - activePixelArray.top;
    int activeWidth = activePixelArray.right - activePixelArray.left;
    const camera_coordinate_system_t sysCoord = {0, 0,  activeWidth, activeHeight};
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

    const camera_coordinate_system_t fillFrameCoord = {0, 0,
                                                       width + horizontalCrop,
                                                       height + verticalCrop};
    const camera_coordinate_system_t frameCoord = {0, 0, width, height};

    CLEAR(*result);
    for (int i = 0; i < faceDetectionResult.faceNum; i++) {
        if (i == MAX_FACES_DETECTABLE)
            break;

        camera_coordinate_t pointCoord = {0, 0};
        result->faceScores[i] = faceDetectionResult.faceResults[i].confidence;
        result->faceIds[i] = faceDetectionResult.faceResults[i].tracking_id;

        if (imageRotationUnchanged) {
            srcCoord = {faceDetectionResult.faceResults[i].rect.left,
                        faceDetectionResult.faceResults[i].rect.top};
            destCoord = AiqUtils::convertCoordinateSystem(iaCoord, sysCoord, srcCoord);
            result->faceRect[i * 4] = destCoord.x;  // rect.left
            result->faceRect[i * 4 + 1] = destCoord.y;  // rect.top

            srcCoord = {faceDetectionResult.faceResults[i].rect.right,
                        faceDetectionResult.faceResults[i].rect.bottom};
            destCoord = AiqUtils::convertCoordinateSystem(iaCoord, sysCoord, srcCoord);
            result->faceRect[i * 4 + 2] = destCoord.x;  // rect.right
            result->faceRect[i * 4 + 3] = destCoord.y;  // rect.bottom

            srcCoord = {faceDetectionResult.eyeResults[i].left_eye.x,
                        faceDetectionResult.eyeResults[i].left_eye.y};
            destCoord = AiqUtils::convertCoordinateSystem(iaCoord, sysCoord, srcCoord);
            result->faceLandmarks[i * 6] = destCoord.x;  // left_eye.x;
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
            result->faceRect[i * 4] = destCoord.x;  // rect.left
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
            result->faceLandmarks[i * 6] = destCoord.x;  // left_eye.x;
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
    result->faceNum = (faceDetectionResult.faceNum < MAX_FACES_DETECTABLE ?
                       faceDetectionResult.faceNum : MAX_FACES_DETECTABLE);
    return OK;
}
}  // namespace icamera

