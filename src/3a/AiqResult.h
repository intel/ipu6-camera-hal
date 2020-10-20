/*
 * Copyright (C) 2015-2020 Intel Corporation.
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

private:
    int deepCopyAiqResult(const AiqResult &src, AiqResult *dst);

public:
    int mCameraId;
    unsigned long long mTimestamp;
    long mSequence;
    TuningMode mTuningMode;
    float mAfDistanceDiopters;
    bool mSkip;
    camera_range_t mFocusRange;

    ia_aiq_ae_results mAeResults;
    ia_aiq_awb_results mAwbResults;
    ia_aiq_af_results mAfResults;
    ia_aiq_gbce_results mGbceResults;
    ia_aiq_pa_results_v1 mPaResults;
    ia_aiq_sa_results_v1 mSaResults;

    ia_aiq_advanced_ccm_t mPreferredAcm;

    ia_isp_custom_controls mCustomControls;

    aiq_parameter_t mAiqParam;

    float mLensShadingMap[DEFAULT_LSC_GRID_SIZE * 4];

    int64_t mFrameDuration;   // us
    int64_t mRollingShutter;  // us

private:
    /*!< ia_aiq_ae_results pointer contents */
    ia_aiq_ae_exposure_result mExposureResults[MAX_EXPOSURES_NUM];
    ia_aiq_aperture_control   mApertureControl;
    ia_aiq_hist_weight_grid   mWeightGrid;
    unsigned char mGrid[MAX_AE_GRID_SIZE];
    ia_aiq_flash_parameters   mFlashes[NUM_FLASH_LEDS];

    /*!< ia_aiq_ae_exposure_result pointer contents */
    ia_aiq_exposure_parameters        mGenericExposure[MAX_EXPOSURES_NUM];
    ia_aiq_exposure_sensor_parameters mSensorExposure[MAX_EXPOSURES_NUM];

    /*!< ia_aiq_gbce results */
    /* The actual size of this table can be calculated by running cold
     * GBCE, it will provide those tables.
     */
    float mRGammaLut[MAX_GAMMA_LUT_SIZE];
    float mGGammaLut[MAX_GAMMA_LUT_SIZE];
    float mBGammaLut[MAX_GAMMA_LUT_SIZE];
    float mToneMapLut[MAX_TONEMAP_LUT_SIZE];

    /*!< ia_aiq_pa_results pointer content */
    unsigned int mHueSectors[MAX_NUM_SECTORS];
    float mAdvancedCCM[MAX_NUM_SECTORS][3][3];
    ia_aiq_ir_weight_t mIrWeight;
    unsigned short mIrWeightGridR[MAX_IR_WEIGHT_GRID_SIZE];
    unsigned short mIrWeightGridG[MAX_IR_WEIGHT_GRID_SIZE];
    unsigned short mIrWeightGridB[MAX_IR_WEIGHT_GRID_SIZE];

    /*!< ia_isp_custom_controls pointer content */
    float mCustomControlsParams[MAX_CUSTOM_CONTROLS_PARAM_SIZE];

};

} /* namespace icamera */
