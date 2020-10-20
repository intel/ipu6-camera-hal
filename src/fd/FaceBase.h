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

#ifdef FACE_DETECTION
#include <pvl_config.h>
#include <pvl_eye_detection.h>
#include <pvl_face_detection.h>
#include <pvl_mouth_detection.h>
#include <pvl_types.h>
#endif

namespace icamera {

#define RECT_SIZE 4
#define LM_SIZE 6
#define MAX_STORE_FACE_DATA_BUF_NUM 3

#define MAX_FACES_DETECTABLE 10
#define MAX_FACE_FRAME_WIDTH 1920
#define MAX_FACE_FRAME_HEIGHT 1280

#define MAX_FACE_FRAME_SIZE_ASYNC (MAX_FACE_FRAME_WIDTH * MAX_FACE_FRAME_HEIGHT)  // only using Y
#define MAX_FACE_FRAME_SIZE_SYNC (MAX_FACE_FRAME_WIDTH * MAX_FACE_FRAME_HEIGHT * 3 / 2)

typedef enum {
    FD_MODE_OFF,
    FD_MODE_SIMPLE,  /**< Provide face area */
    FD_MODE_FULL,    /**< Provide face area, eye and mouth coordinates */
} face_detection_mode;

/* Face Detection results */
typedef struct CVFaceDetectionAbstractResult {
    int faceNum;
    int faceIds[MAX_FACES_DETECTABLE];
    int faceLandmarks[LM_SIZE * MAX_FACES_DETECTABLE];
    int faceRect[RECT_SIZE * MAX_FACES_DETECTABLE];
    uint8_t faceScores[MAX_FACES_DETECTABLE];
} CVFaceDetectionAbstractResult;

struct FaceDetectionInitParams {
    unsigned int max_face_num;
};

#ifdef FACE_DETECTION
struct FaceDetectionResult {
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

    FaceDetectionResult results;
};
#endif

}  // namespace icamera
