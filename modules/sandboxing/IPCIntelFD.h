/*
 * Copyright (C) 2019-2020 Intel Corporation.
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

#include <pvl_config.h>
#include <pvl_eye_detection.h>
#include <pvl_face_detection.h>
#include <pvl_mouth_detection.h>
#include <pvl_types.h>

#include "FaceType.h"
#include "modules/sandboxing/IPCCommon.h"

namespace icamera {
struct FaceDetectionInitParams {
    unsigned int max_face_num;
    int cameraId;
};

struct FaceDetectionDeinitParams {
    int cameraId;
};

struct FaceDetectionPVLResult {
    bool faceUpdated;
    int faceNum;
    pvl_face_detection_result faceResults[MAX_FACES_DETECTABLE];
    pvl_eye_detection_result eyeResults[MAX_FACES_DETECTABLE];
    pvl_mouth_detection_result mouthResults[MAX_FACES_DETECTABLE];
};

struct FaceDetectionRunParams {
    uint8_t data[MAX_FACE_FRAME_SIZE_ASYNC];
    int32_t bufferHandle;
    uint32_t size;
    int32_t width;
    int32_t height;
    pvl_image_format format;
    int32_t stride;
    int32_t rotation;
    int cameraId;

    FaceDetectionPVLResult results;
};

class IPCIntelFD {
 public:
    IPCIntelFD() {}
    virtual ~IPCIntelFD() {}

    bool clientFlattenInit(unsigned int max_face_num, int cameraId,
                           FaceDetectionInitParams* params);
    bool serverUnflattenRun(const FaceDetectionRunParams& inParams, void* imageData,
                            pvl_image* image, int* cameraId);
};
} /* namespace icamera */
