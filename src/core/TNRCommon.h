/*
 * Copyright (C) 2020-2021 Intel Corporation.
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
#ifdef TNR7_CM
#include "tnr7us_parameters_definition.h"
#endif

namespace icamera {

#ifdef TNR7_CM
typedef tnr_scale_1_0_t tnrScaleParam;
typedef tnr7_ims_1_0_t tnrImsParam;
typedef tnr7_bc_1_0_t tnrBCParam;
typedef tnr7_blend_1_0_t tnrBlendParam;
#else
typedef struct {
    double is_first_frame;
    double do_update;
    double coeffs[3];
    double tune_sensitivity;
    double global_protection;
    double global_protection_sensitivity_lut_values[3];
    double global_protection_sensitivity_lut_slopes[2];
    double global_protection_inv_num_pixels;
} subway_tnr7_bc_1_1_t;

typedef struct {
    double max_recursive_similarity;
} subway_tnr7_blend_1_0_t;

typedef struct {
    double update_limit;
    double update_coeff;
    double d_ml[16];
    double d_slopes[16];
    double d_top[16];
    double outofbounds[16];
    double radial_start;
    double radial_coeff;
    double frame_center_y;
    double frame_center_x;
    double r_coeff;
} subway_tnr7_ims_1_1_t;

typedef int32_t tnrScaleParam;
typedef subway_tnr7_ims_1_1_t tnrImsParam;
typedef subway_tnr7_bc_1_1_t tnrBCParam;
typedef subway_tnr7_blend_1_0_t tnrBlendParam;
#endif

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
    int width;
    int height;
    uint32_t surfaceSize;
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

typedef struct Tnr7Param {
    tnrScaleParam scale;
    tnrImsParam ims;
    tnrBCParam bc;
    tnrBlendParam blend;
} Tnr7Param;
}  // namespace icamera
