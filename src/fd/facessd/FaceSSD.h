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

#pragma once
#include <cros-camera/camera_face_detection.h>
#include <ia_types.h>

#include <memory>
#include <queue>
#include <unordered_map>

#include "Camera3BufferPool.h"
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
    virtual bool threadLoop();

 protected:
    virtual int getFaceNum();
    virtual void getResultFor3A(cca::cca_face_state* faceState);
    virtual void getResultForApp(CVFaceDetectionAbstractResult* result);

 private:
    int initFaceDetection(int width, int height, int gfxFmt, int usage);
    std::shared_ptr<camera3::Camera3Buffer> acquireRunCCBuf();
    void returnRunBuf(std::shared_ptr<camera3::Camera3Buffer> gbmRunBuf);
    static void convertCoordinate(int width, int height, const human_sensing::CrosFace& src,
                                  human_sensing::CrosFace* dst);

    // Guard for running buffer queue of thread
    std::queue<std::shared_ptr<camera3::Camera3Buffer>> mRunGoogleBufQueue;

    std::unique_ptr<camera3::Camera3BufferPool> mBufferPool;
    std::unique_ptr<cros::FaceDetector> mFaceDetector;

    FaceSSDResult mResult;

    DISALLOW_COPY_AND_ASSIGN(FaceSSD);
};

}  // namespace icamera
