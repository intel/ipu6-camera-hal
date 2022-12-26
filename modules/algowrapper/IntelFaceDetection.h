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

#pragma once
#include <memory>

#include "FaceBase.h"
#include "iutils/Errors.h"
#include "iutils/Utils.h"

namespace icamera {
class IntelFaceDetection {
 public:
    IntelFaceDetection();
    ~IntelFaceDetection();

    status_t init(FaceDetectionInitParams* pData, int dataSize);
    status_t deinit(FaceDetectionDeinitParams* pData, int dataSize);
    status_t run(pvl_image* pImage, FaceDetectionResult* fdResults);
    status_t run(FaceDetectionRunParams* fdRunParams, int dataSize, void* addr = nullptr);
    FaceDetectionRunParams* prepareRunBuffer(unsigned int index);

 private:
    pvl_face_detection* mFDHandle;
    unsigned int mMaxFacesNum;
    FaceDetectionRunParams mMemRunBufs[MAX_STORE_FACE_DATA_BUF_NUM];
    void convertCoordinate(int faceId, int width, int height, const pvl_rect& src, pvl_rect* dst);

    DISALLOW_COPY_AND_ASSIGN(IntelFaceDetection);
};
}  // namespace icamera
