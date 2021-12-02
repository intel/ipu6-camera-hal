/*
 * Copyright (C) 2015-2021 Intel Corporation.
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

#include "ia_aiq.h"
#include "ia_ltm.h"
#include "ia_isp_types.h"

#include "AiqUtils.h"
#include "AiqSetting.h"
#include "iutils/Utils.h"

namespace icamera {

/**
 * \class AiqResult
 * The private structs are part of AE, AF, AWB, PA and SA results.
 * They need to be separately introduced to store the contents of the results
 * that the AIQ algorithms return as pointers.
 * Then we can do deep copy of the results
 */
class AiqResult {

public:
    AiqResult(int cameraId);
    ~AiqResult();

    int init();
    int deinit();

    AiqResult& operator=(const AiqResult& other);

public:
    int mCameraId;
    unsigned long long mTimestamp;
    int64_t mSequence;
    TuningMode mTuningMode;
    float mAfDistanceDiopters;
    bool mSkip;
    camera_range_t mFocusRange;
    uint32_t mLensPosition;
    camera_scene_mode_t mSceneMode;

    cca::cca_ae_results mAeResults;
    cca::cca_awb_results mAwbResults;
    cca::cca_af_results mAfResults;
    cca::cca_gbce_params mGbceResults;
    cca::cca_pa_params mPaResults;
    cca::cca_out_stats mOutStats;

    ia_isp_custom_controls mCustomControls;

    aiq_parameter_t mAiqParam;

    float mLensShadingMap[DEFAULT_LSC_GRID_SIZE * 4];

    int64_t mFrameDuration;   // us
    int64_t mRollingShutter;  // us

private:
    /*!< ia_isp_custom_controls pointer content */
    float mCustomControlsParams[MAX_CUSTOM_CONTROLS_PARAM_SIZE];

};

} /* namespace icamera */
