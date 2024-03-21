/*
 * Copyright (C) 2021-2023 Intel Corporation
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

#include <cros-camera/cros_camera_hal.h>
#include <algorithm>
#include <fstream>
#include <vector>

#include "AiqUtils.h"
#include "PlatformData.h"
#include "aal/FaceDetectionResultCallbackManager.h"
#include "iutils/CameraLog.h"
#include "iutils/Errors.h"
#include "iutils/Utils.h"

namespace icamera {

FaceSSD::FaceSSD(int cameraId, unsigned int maxFaceNum, int32_t halStreamId, int width, int height,
                 int gfxFmt, int usage)
        : FaceDetection(cameraId, maxFaceNum, halStreamId, width, height) {
    CLEAR(mResult);

    mFaceDetector = cros::FaceDetector::Create();
    CheckAndLogError(!mFaceDetector, VOID_VALUE, "mFaceDetector is nullptr");

    mInitialized = true;
}

FaceSSD::~FaceSSD() {
    mFaceDetector = nullptr;
}

void FaceSSD::faceDetectResult(cros::FaceDetectResult ret,
                               std::vector<human_sensing::CrosFace> faces) {
    AutoMutex l(mFaceResultLock);
    CLEAR(mResult);

    if (ret == cros::FaceDetectResult::kDetectOk) {
        std::vector<human_sensing::CrosFace> sortFaces = faces;
        std::sort(sortFaces.begin(), sortFaces.end(),
                  [](const human_sensing::CrosFace& a, const human_sensing::CrosFace& b) {
                      auto area1 = (a.bounding_box.x2 - a.bounding_box.x1) *
                                   (a.bounding_box.y2 - a.bounding_box.y1);
                      auto area2 = (b.bounding_box.x2 - b.bounding_box.x1) *
                                   (b.bounding_box.y2 - b.bounding_box.y1);
                      return area1 > area2;
                  });

        int faceCount = 0;
        for (auto& face : sortFaces) {
            if (faceCount >= mMaxFaceNum) break;
            mResult.faceSsdResults[faceCount] = face;
            faceCount++;
            LOG2("face result: box: %f,%f,%f,%f", face.bounding_box.x1, face.bounding_box.y1,
                 face.bounding_box.x2, face.bounding_box.y2);
        }
        mResult.faceNum = faceCount;
        mResult.faceUpdated = true;
        LOG2("@%s, faceNum:%d", __func__, mResult.faceNum);
    } else {
        LOGE("@%s, Faile to detect face", __func__);
    }
}

void FaceSSD::runFaceDetectionBySync(const std::shared_ptr<camera3::Camera3Buffer>& ccBuf) {
    LOG2("@%s", __func__);
    CheckAndLogError(mInitialized == false, VOID_VALUE, "@%s, mInitialized is false", __func__);
    CheckAndLogError(!ccBuf, VOID_VALUE, "@%s, ccBuf buffer is nullptr", __func__);

    printfFDRunRate();

    std::optional<cros::FaceDetectionResult> face_detection_result =
        camera3::FaceDetectionResultCallbackManager::getInstance().getFaceDetectionResult(
            mCameraId);

    if (face_detection_result) {
        LOG2("FrameNum:%zu, run with the cros::FaceDetector from stream manipulator.",
             face_detection_result->frame_number);

        auto array = PlatformData::getActivePixelArray(mCameraId);
        camera_coordinate_system_t dst = {0, 0, mWidth, mHeight};
        int vCrop = 0;
        int hCrop = 0;
        int gap = (mWidth * (array.bottom - array.top) / (array.right - array.left)) - mHeight;
        if (gap > 0) {
            vCrop = gap;
        } else if (gap < 0) {
            hCrop = (mHeight * (array.right - array.left) / (array.bottom - array.top)) - mWidth;
        }

        for (auto& face : face_detection_result->faces) {
            camera_coordinate_t leftTop = {static_cast<int>(face.bounding_box.x1) + hCrop / 2,
                                           static_cast<int>(face.bounding_box.y1) + vCrop / 2};
            leftTop = AiqUtils::convertCoordinateSystem(array, dst, leftTop);
            face.bounding_box.x1 = leftTop.x;
            face.bounding_box.y1 = leftTop.y;
            camera_coordinate_t rightBottom = {static_cast<int>(face.bounding_box.x2) + hCrop / 2,
                                               static_cast<int>(face.bounding_box.y2) + vCrop / 2};
            rightBottom = AiqUtils::convertCoordinateSystem(array, dst, rightBottom);
            face.bounding_box.x2 = rightBottom.x;
            face.bounding_box.y2 = rightBottom.y;
        }

        faceDetectResult(cros::FaceDetectResult::kDetectOk, face_detection_result->faces);
        return;
    }

    int input_stride = ccBuf->stride();
    cros::Size input_size = cros::Size(ccBuf->width(), ccBuf->height());
    const uint8_t* buffer_addr = static_cast<uint8_t*>(ccBuf->data());

    // base::Unretained is safe since 'this' joins 'face thread' in the destructor.
    mFaceDetector->DetectAsync(buffer_addr, input_stride, input_size, std::nullopt,
                               base::BindOnce(&FaceSSD::faceDetectResult, base::Unretained(this)));
}

void FaceSSD::runFaceDetectionByAsync(const std::shared_ptr<camera3::Camera3Buffer>& ccBuf) {
    LOG2("@%s", __func__);
    CheckAndLogError(mInitialized == false, VOID_VALUE, "@%s, mInitialized is false", __func__);

    runFaceDetectionBySync(ccBuf);
}

int FaceSSD::getFaceNum() {
    AutoMutex l(mFaceResultLock);
    return mResult.faceNum;
}

void FaceSSD::getResultFor3A(cca::cca_face_state* faceState) {
    LOG2("@%s", __func__);

    camera_coordinate_system_t sysCoord = {IA_COORDINATE_LEFT, IA_COORDINATE_TOP,
                                           IA_COORDINATE_RIGHT, IA_COORDINATE_BOTTOM};
    AutoMutex l(mFaceResultLock);
    FaceSSDResult* faceSsdResult = &mResult;
    faceState->is_video_conf = true;
    faceState->updated = mResult.faceUpdated;
    faceState->num_faces = faceSsdResult->faceNum;
    faceState->fd_algo = OS_FD;
    mResult.faceUpdated = false;

    for (int i = 0; i < faceSsdResult->faceNum; i++) {
        CLEAR(faceState->faces[i]);
        faceState->faces[i].face_area.left =
            static_cast<int>(faceSsdResult->faceSsdResults[i].bounding_box.x1);  // rect.left
        faceState->faces[i].face_area.top =
            static_cast<int>(faceSsdResult->faceSsdResults[i].bounding_box.y1);  // rect.top
        faceState->faces[i].face_area.right =
            static_cast<int>(faceSsdResult->faceSsdResults[i].bounding_box.x2);  // rect.right
        faceState->faces[i].face_area.bottom =
            static_cast<int>(faceSsdResult->faceSsdResults[i].bounding_box.y2);  // rect.bottom
        convertFaceCoordinate(
            sysCoord, &faceState->faces[i].face_area.left, &faceState->faces[i].face_area.top,
            &faceState->faces[i].face_area.right, &faceState->faces[i].face_area.bottom);
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
        convertFaceCoordinate(mRatioInfo.sysCoord, &result->faceRect[i * 4],
                              &result->faceRect[i * 4 + 1], &result->faceRect[i * 4 + 2],
                              &result->faceRect[i * 4 + 3]);
        LOG2("@%s, face info, id:%d, left:%d, top:%d, right:%d, bottom:%d", __func__, i,
             result->faceRect[i * 4], result->faceRect[i * 4 + 1], result->faceRect[i * 4 + 2],
             result->faceRect[i * 4 + 3]);
    }

    result->faceNum = faceSsdResult->faceNum;
}

}  // namespace icamera
