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
#include <vector>

#include "AiqUtils.h"
#include "IFaceDetection.h"
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
                                             int32_t halStreamId, int width, int height, int gfxFmt,
                                             int usage) {
    LOG1("<id%d> @%s, maxFaceNum:%u, halStreamId:%u, width:%d, height:%d, gfxFmt:%d, usage:%d",
         cameraId, __func__, maxFaceNum, halStreamId, width, height, gfxFmt, usage);
    CheckAndLogError(maxFaceNum > MAX_FACES_DETECTABLE, nullptr, "maxFaceNum %d is error",
                     maxFaceNum);

    AutoMutex lock(sLock);
    if (sInstances.find(cameraId) == sInstances.end()) {
        FaceDetection* fd = IFaceDetection::createFaceDetection(cameraId, maxFaceNum, halStreamId,
                                                                width, height, gfxFmt, usage);
        if (!fd || !fd->isInitialized()) {
            if (fd) delete fd;
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
          mMaxFaceNum(maxFaceNum),
          mWidth(width),
          mHeight(height),
          mHalStreamId(halStreamId),
          mFDRunDefaultInterval(icamera::PlatformData::faceEngineRunningInterval(cameraId)),
          mFDRunIntervalNoFace(icamera::PlatformData::faceEngineRunningIntervalNoFace(cameraId)),
          mFDRunInterval(icamera::PlatformData::faceEngineRunningInterval(cameraId)),
          mFrameCnt(0),
          mRunCount(0) {
    LOG1("<id%d> default interval:%d, interval no face:%d, run interval:%d", cameraId,
         mFDRunDefaultInterval, mFDRunIntervalNoFace, mFDRunInterval);
    initFovInfoFor3A(&mFovInfo);
    initRatioInfoForApp(&mRatioInfo);
}

FaceDetection::~FaceDetection() {
    LOG1("<id%d> @%s", mCameraId, __func__);
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
    if (!Log::isLogTagEnabled(ST_FPS)) return;

    mRunCount++;
    if (mRunCount % FPS_FD_COUNT != 0) return;

    struct timeval curTime;
    gettimeofday(&curTime, nullptr);
    int duration = static_cast<int>(curTime.tv_usec - mRequestRunTime.tv_usec +
                                    ((curTime.tv_sec - mRequestRunTime.tv_sec) * 1000000));
    float curFps = static_cast<float>(1000000) / static_cast<float>(duration / FPS_FD_COUNT);
    LOG2(ST_FPS, "@%s, face detection fps: %02f", __func__, curFps);
    mRequestRunTime = curTime;
}

void FaceDetection::runFaceDetection(const std::shared_ptr<camera3::Camera3Buffer>& ccBuf) {
    CheckAndLogError(mInitialized == false, VOID_VALUE, "mInitialized is false");

    if (!faceRunningByCondition()) return;

    if (PlatformData::isFaceEngineSyncRunning(mCameraId)) {
        runFaceDetectionBySync(ccBuf);
    } else {
        runFaceDetectionByAsync(ccBuf);
    }
}

void FaceDetection::initFovInfoFor3A(struct FovInfoFor3A* fovInfo) {
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
    CLEAR(*fovInfo);
    float scalerWidth = 1;
    float scalerHeight = 1;
    int ret = PlatformData::getScalerInfo(mCameraId, mHalStreamId, &scalerWidth, &scalerHeight);
    LOG2("getScalerInfo ret:%d, scalerWidth:%f, scalerHeight:%f", ret, scalerWidth, scalerHeight);

    camera_coordinate_system_t activePixelArray = PlatformData::getActivePixelArray(mCameraId);
    float fovRatioW = scalerWidth * mWidth / (activePixelArray.right - activePixelArray.left);
    float fovRatioH = scalerHeight * mHeight / (activePixelArray.bottom - activePixelArray.top);
    float offsetW = (1.0 - fovRatioW) / 2.0 * (IA_COORDINATE_RIGHT - IA_COORDINATE_LEFT);
    float offsetH = (1.0 - fovRatioH) / 2.0 * (IA_COORDINATE_BOTTOM - IA_COORDINATE_TOP);

    LOG2("mHeight:%d, mWidth:%d, fovRatioW:%f, fovRatioH:%f, offsetW:%f, offsetH:%f", mHeight,
         mWidth, fovRatioW, fovRatioH, offsetW, offsetH);

    *fovInfo = {fovRatioW, fovRatioH, offsetW, offsetH};
}

void FaceDetection::initRatioInfoForApp(struct RatioInfoForApp* ratioInfo) {
    CLEAR(*ratioInfo);
    // construct android coordinate based on active pixel array
    camera_coordinate_system_t activePixelArray = PlatformData::getActivePixelArray(mCameraId);

    int verticalCrop = 0;
    int horizontalCrop = 0;
    bool imageRotationChanged = false;

    int activeHeight = activePixelArray.bottom - activePixelArray.top;
    int activeWidth = activePixelArray.right - activePixelArray.left;

    // do extra conversion if the image ratio is not the same ratio with the android coordinate.
    if (mHeight * activeWidth != mWidth * activeHeight) {
        imageRotationChanged = true;
        int gap = (mWidth * activeHeight / activeWidth) - mHeight;

        if (gap > 0) {
            // vertical crop pixel
            verticalCrop = gap;
        } else if (gap < 0) {
            // horizontal crop pixel
            horizontalCrop = mHeight * activeWidth / activeHeight - mWidth;
        }
    }
    LOG2(
        "%s, imageRotationChanged:%d, height:%d, width:%d, activeWidth:%d, activeHeight:%d, "
        "verticalCrop:%d, horizontalCrop:%d",
        __func__, imageRotationChanged, mHeight, mWidth, activeWidth, activeHeight, verticalCrop,
        horizontalCrop);

    *ratioInfo = {{0, 0, activeWidth, activeHeight}, verticalCrop, horizontalCrop,
                  imageRotationChanged};
}

int FaceDetection::getResult(int cameraId, cca::cca_face_state* faceState) {
    CheckAndLogError(!faceState, UNKNOWN_ERROR, "faceState is nullptr");

    AutoMutex lock(sLock);
    FaceDetection* fdInstance = FaceDetection::getInstance(cameraId);
    if (!fdInstance) {
        LOG2("%s, It doesn't have face detection instance", __func__);
        return UNKNOWN_ERROR;
    }
    fdInstance->getResultFor3A(faceState);

    return OK;
}

int FaceDetection::getResult(int cameraId, CVFaceDetectionAbstractResult* result) {
    CheckAndLogError(!result, UNKNOWN_ERROR, "result is nullptr");

    AutoMutex lock(sLock);
    FaceDetection* fdInstance = FaceDetection::getInstance(cameraId);
    if (!fdInstance) {
        LOG2("%s, It doesn't have face detection instance", __func__);
        return UNKNOWN_ERROR;
    }
    fdInstance->getResultForApp(result);

    return OK;
}

}  // namespace icamera
