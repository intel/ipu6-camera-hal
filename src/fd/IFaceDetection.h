/*
 * Copyright (C) 2021 Intel Corporation.
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
#include "FaceDetection.h"

namespace icamera {

/*
 * \factory class IFaceDetection
 * This class is used to create FaceDetection
 */
class IFaceDetection {
 public:
    static FaceDetection* createFaceDetection(int cameraId, unsigned int maxFaceNum,
                                              int32_t halStreamId, int width, int height,
                                              int gfxFmt, int usage);
};

}  // namespace icamera
