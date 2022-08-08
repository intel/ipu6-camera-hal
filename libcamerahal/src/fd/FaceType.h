/*
 * Copyright (C) 2019-2021 Intel Corporation.
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
    FD_MODE_SIMPLE, /**< Provide face area */
    FD_MODE_FULL,   /**< Provide face area, eye and mouth coordinates */
} face_detection_mode;

/* Face Detection results */
typedef struct CVFaceDetectionAbstractResult {
    int faceNum;
    int faceIds[MAX_FACES_DETECTABLE];
    int faceLandmarks[LM_SIZE * MAX_FACES_DETECTABLE];
    int faceRect[RECT_SIZE * MAX_FACES_DETECTABLE];
    uint8_t faceScores[MAX_FACES_DETECTABLE];
} CVFaceDetectionAbstractResult;

}  // namespace icamera
