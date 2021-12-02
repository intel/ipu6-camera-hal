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
#ifdef ENABLE_SANDBOXING
#include "modules/sandboxing/client/IntelFaceDetectionClient.h"
#else
#include "modules/algowrapper/IntelFaceDetection.h"
#endif
#include <memory>
#include <queue>

#include "Camera3Buffer.h"
#include "FaceDetection.h"
#include "IntelCCATypes.h"
#include "modules/sandboxing/IPCIntelFD.h"

namespace icamera {

class FaceDetectionPVL : public FaceDetection {
 public:
    FaceDetectionPVL(int cameraId, unsigned int maxFaceNum, int32_t halStreamId, int width,
                     int height);
    virtual ~FaceDetectionPVL();

    virtual void runFaceDetectionBySync(const std::shared_ptr<camera3::Camera3Buffer>& ccBuf);
    virtual void runFaceDetectionByAsync(const std::shared_ptr<camera3::Camera3Buffer>& ccBuf);
    virtual bool threadLoop();

 protected:
    virtual int getFaceNum();
    virtual void getResultFor3A(cca::cca_face_state* faceState);
    virtual void getResultForApp(CVFaceDetectionAbstractResult* result);

 private:
    int initFaceDetection();
    FaceDetectionRunParams* acquireRunBuf();
    void returnRunBuf(FaceDetectionRunParams* memRunBuf);

    std::unique_ptr<IntelFaceDetection> mFace;

    // Guard for running buffer queue of thread
    std::queue<FaceDetectionRunParams*> mRunPvlBufQueue;

    // Guard for running buffer pool of face engine
    std::mutex mMemRunPoolLock;
    std::queue<FaceDetectionRunParams*> mMemRunPool;

    FaceDetectionPVLResult mResult;

    int mSensorOrientation;

    DISALLOW_COPY_AND_ASSIGN(FaceDetectionPVL);
};

}  // namespace icamera
