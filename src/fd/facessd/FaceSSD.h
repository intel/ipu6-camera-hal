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

#pragma once
#include <cros-camera/camera_face_detection.h>
#include <ia_types.h>

#include <memory>

#include "FaceDetection.h"
#include "IntelCCATypes.h"

namespace icamera {
struct FaceSSDResult {
    bool faceUpdated;
    int faceNum;
    human_sensing::CrosFace faceSsdResults[MAX_FACES_DETECTABLE];
};

class FaceSSD : public FaceDetection {
 public:
    FaceSSD(int cameraId, unsigned int maxFaceNum, int32_t halStreamId, int width, int height,
            int gfxFmt, int usage);
    virtual ~FaceSSD();

    virtual void runFaceDetectionBySync(const std::shared_ptr<camera3::Camera3Buffer>& ccBuf);
    virtual void runFaceDetectionByAsync(const std::shared_ptr<camera3::Camera3Buffer>& ccBuf);

 protected:
    virtual int getFaceNum();
    virtual void getResultFor3A(cca::cca_face_state* faceState);
    virtual void getResultForApp(CVFaceDetectionAbstractResult* result);

 private:
    void faceDetectResult(cros::FaceDetectResult ret, std::vector<human_sensing::CrosFace> faces);

    std::unique_ptr<cros::FaceDetector> mFaceDetector;
    FaceSSDResult mResult;
    DISALLOW_COPY_AND_ASSIGN(FaceSSD);
};

}  // namespace icamera
