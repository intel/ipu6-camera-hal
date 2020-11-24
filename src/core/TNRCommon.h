/*
 * Copyright (C) 2020 Intel Corporation.
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

#include "ia_pal_types_isp_parameters_autogen.h"
#include "tnr7us_parameters_definition.h"

#define TNR7US_RESTART_THRESHOLD 3

namespace icamera {

typedef struct Tnr7Param {
    tnr_scale_1_0_t scale;
    tnr7_ims_1_0_t ims;
    tnr7_bc_1_0_t bc;
    tnr7_blend_1_0_t blend;
} Tnr7Param;

typedef enum TnrType {
    TNR_INSTANCE0 = 0,
    TNR_INSTANCE1 = 1,
    TNR_INSTANCE_MAX,
} TnrType;

typedef struct TnrInitInfo {
    int width;
    int height;
    int cameraId;
    TnrType type;
} TnrInitInfo;

typedef struct TnrRequestInfo {
    int32_t inHandle;
    int32_t outHandle;
    int32_t paramHandle;
    int32_t surfaceHandle;
    int cameraId;
    TnrType type;
    int gain;
    int outBufFd;
    bool isForceUpdate;
} TnrRequestInfo;
}  // namespace icamera
