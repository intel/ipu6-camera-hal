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

#define LOG_TAG "AiqResult"

#include "iutils/Errors.h"
#include "iutils/CameraLog.h"

#include "AiqResult.h"

namespace icamera {

AiqResult::AiqResult(int cameraId) :
    mCameraId(cameraId),
    mTimestamp(0),
    mSequence(-1),
    mTuningMode(TUNING_MODE_VIDEO),
    mAfDistanceDiopters(0.0f),
    mSkip(false),
    mFrameDuration(0),
    mRollingShutter(0)
{
    LOG3A("@%s", __func__);

    CLEAR(mGrid);
    CLEAR(mFlashes);
    CLEAR(mRGammaLut);
    CLEAR(mGGammaLut);
    CLEAR(mBGammaLut);
    CLEAR(mToneMapLut);
    CLEAR(mHueSectors);
    CLEAR(mAdvancedCCM);
    CLEAR(mIrWeightGridR);
    CLEAR(mIrWeightGridG);
    CLEAR(mIrWeightGridB);
    CLEAR(mCustomControls);
    CLEAR(mCustomControlsParams);
    CLEAR(mSaResults);
    CLEAR(mAwbResults);
    CLEAR(mIrWeight);
    CLEAR(mApertureControl);
    CLEAR(mGbceResults);
    CLEAR(mWeightGrid);
    CLEAR(mPreferredAcm);
    CLEAR(mPaResults);
    CLEAR(mAeResults);
    CLEAR(mAfResults);
    CLEAR(mFocusRange);
    CLEAR(mLensShadingMap);
}

AiqResult::~AiqResult()
{
    LOG3A("@%s", __func__);
    deinit();
}

int AiqResult::init()
{
    LOG3A("@%s", __func__);

    CLEAR(mAeResults);
    CLEAR(mAfResults);
    CLEAR(mAwbResults);
    CLEAR(mGbceResults);
    CLEAR(mSaResults);
    CLEAR(mPaResults);

    CLEAR(mExposureResults);
    CLEAR(mWeightGrid);
    CLEAR(mGrid);
    CLEAR(mFlashes);
    CLEAR(mGenericExposure);
    CLEAR(mSensorExposure);
    CLEAR(mApertureControl);
    CLEAR(mPreferredAcm);
    CLEAR(mIrWeight);

    mAiqParam.reset();

    /*AE results init */
    mAeResults.num_exposures = 1;
    mAeResults.exposures = mExposureResults;
    mAeResults.aperture_control = &mApertureControl;
    mAeResults.weight_grid = &mWeightGrid;
    mAeResults.weight_grid->weights = mGrid;
    mAeResults.flashes = mFlashes;
    for (unsigned int i = 0; i< MAX_EXPOSURES_NUM; i++) {
        mAeResults.exposures[i].exposure = &mGenericExposure[i];
        mAeResults.exposures[i].sensor_exposure = &mSensorExposure[i];
    }
    /* GBCE results init */
    mGbceResults.r_gamma_lut = mRGammaLut;
    mGbceResults.g_gamma_lut = mGGammaLut;
    mGbceResults.b_gamma_lut = mBGammaLut;
    mGbceResults.tone_map_lut = mToneMapLut;

    /* SA results init */
    mSaResults.width = MAX_LSC_WIDTH;
    mSaResults.height = MAX_LSC_HEIGHT;

    for (int i = 0; i < MAX_BAYER_ORDER_NUM; i++) {
        for (int j = 0; j < MAX_BAYER_ORDER_NUM; j++) {
            mSaResults.lsc_grid[i][j] = new unsigned short[MAX_LSC_WIDTH * MAX_LSC_HEIGHT];
            memset(mSaResults.lsc_grid[i][j], 0,
                   sizeof(unsigned short) * MAX_LSC_WIDTH * MAX_LSC_HEIGHT);
        }
    }

    /* PA results init */
    mPaResults.ir_weight = &mIrWeight;

    mPaResults.ir_weight->ir_weight_grid_R = mIrWeightGridR;
    mPaResults.ir_weight->ir_weight_grid_G = mIrWeightGridG;
    mPaResults.ir_weight->ir_weight_grid_B = mIrWeightGridB;

    mPreferredAcm.hue_of_sectors = mHueSectors;
    mPreferredAcm.advanced_color_conversion_matrices = mAdvancedCCM;

    /* Custom Controls init */
    mCustomControls.parameters = mCustomControlsParams;

    return OK;
}

int AiqResult::deinit()
{
    LOG3A("@%s", __func__);

    for (int i = 0; i < MAX_BAYER_ORDER_NUM; ++i) {
        for (int j = 0; j < MAX_BAYER_ORDER_NUM; ++j) {
            delete []  mSaResults.lsc_grid[i][j];
            mSaResults.lsc_grid[i][j] = nullptr;
        }
    }

    return OK;
}

AiqResult &AiqResult::operator=(const AiqResult &other)
{
    deepCopyAiqResult(other, this);
    mSequence = other.mSequence;
    mTimestamp = other.mTimestamp;
    mTuningMode = other.mTuningMode;
    mSkip = other.mSkip;
    mCustomControls.count = other.mCustomControls.count;
    for (int i = 0; i < mCustomControls.count; i++) {
        mCustomControlsParams[i] = other.mCustomControlsParams[i];
    }
    mAiqParam = other.mAiqParam;
    mFrameDuration = other.mFrameDuration;
    mRollingShutter = other.mRollingShutter;

    return *this;
}

int AiqResult::deepCopyAiqResult(const AiqResult &src, AiqResult *dst)
{
    int ret = AiqUtils::deepCopyAeResults(src.mAeResults, &dst->mAeResults);
    ret |= AiqUtils::deepCopyAfResults(src.mAfResults, &dst->mAfResults);
    ret |= AiqUtils::deepCopyAwbResults(src.mAwbResults, &dst->mAwbResults);
    ret |= AiqUtils::deepCopyGbceResults(src.mGbceResults, &dst->mGbceResults);
    ret |= AiqUtils::deepCopyPaResults(src.mPaResults, &dst->mPaResults, &dst->mPreferredAcm);
    ret |= AiqUtils::deepCopySaResults(src.mSaResults, &dst->mSaResults);

    return ret;
}

} /* namespace icamera */
