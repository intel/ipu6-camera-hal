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

#define LOG_TAG IFaceDetection
#include "src/fd/IFaceDetection.h"

#include "FaceDetectionPVL.h"
#include "FaceSSD.h"
#include "PlatformData.h"
#include "iutils/CameraLog.h"

namespace icamera {

FaceDetection* IFaceDetection::createFaceDetection(int cameraId, unsigned int maxFaceNum,
                                                   int32_t halStreamId, int width, int height,
                                                   int gfxFmt, int usage) {
    CheckAndLogError(maxFaceNum > MAX_FACES_DETECTABLE, nullptr, "maxFaceNum %d is error",
                     maxFaceNum);

    FaceDetection* fd = nullptr;
    int fdVendor = icamera::PlatformData::faceEngineVendor(cameraId);
    if (fdVendor == FACE_ENGINE_INTEL_PVL) {
        fd = new FaceDetectionPVL(cameraId, maxFaceNum, halStreamId, width, height);
    } else if (fdVendor == FACE_ENGINE_GOOGLE_FACESSD) {
        fd = new FaceSSD(cameraId, maxFaceNum, halStreamId, width, height, gfxFmt, usage);
    }

    return fd;
}

}  // namespace icamera
