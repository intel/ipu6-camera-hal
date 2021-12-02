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

#define LOG_TAG AiqCore

#include "AiqCore.h"

#include <math.h>

#include <memory>
#include <string>

#include "AiqUtils.h"
#include "Parameters.h"
#include "PlatformData.h"
#include "iutils/CameraLog.h"
#include "iutils/Errors.h"
#include "iutils/Utils.h"

namespace icamera {
AiqCore::AiqCore(int cameraId)
        : mCameraId(cameraId),
          mTimestamp(0),
          mSensorPixelClock(0.0),
          mAeForceLock(false),
          mAwbForceLock(false),
          mAfForceLock(false),
          mAeRunTime(0),
          mAwbRunTime(0),
          mAiqRunTime(0),
          mAiqState(AIQ_NOT_INIT),
          mHyperFocalDistance(0.0f),
          mTuningMode(TUNING_MODE_MAX),
          mShadingMode(SHADING_MODE_FAST),
          mLensShadingMapMode(LENS_SHADING_MAP_MODE_OFF),
          mLscGridRGGBLen(0),
          mLastEvShift(0.0f),
          mAeAndAwbConverged(false),
          mRgbStatsBypassed(false),
          mAeBypassed(false),
          mAfBypassed(false),
          mAwbBypassed(false),
          mLockedExposureTimeUs(0),
          mLockedIso(0) {
    mIntel3AParameter = std::unique_ptr<Intel3AParameter>(new Intel3AParameter(cameraId));

    CLEAR(mFrameParams);
    CLEAR(mLastAeResult);

    CLEAR(mGbceParams);
    CLEAR(mPaParams);
    CLEAR(mSaParams);
    CLEAR(mPaColorGains);

    CLEAR(mResizeLscGridR);
    CLEAR(mResizeLscGridGr);
    CLEAR(mResizeLscGridGb);
    CLEAR(mResizeLscGridB);

    CLEAR(mLensShadingMapSize);
    CLEAR(mLscGridRGGB);

    // init LscOffGrid to 1.0f
    std::fill(std::begin(mLscOffGrid), std::end(mLscOffGrid), 1.0f);

    mAiqParams = std::unique_ptr<cca::cca_aiq_params>(new cca::cca_aiq_params);
    mAiqResults = std::unique_ptr<cca::cca_aiq_results>(new cca::cca_aiq_results);
}

AiqCore::~AiqCore() {}

int AiqCore::initAiqPlusParams() {
    CLEAR(mGbceParams);
    CLEAR(mPaParams);
    CLEAR(mPaColorGains);
    CLEAR(mSaParams);

    /* use convergence time from tunings */
    mSaParams.manual_convergence_time = -1.0;

    camera_info_t info = {};
    PlatformData::getCameraInfo(mCameraId, info);
    int32_t tonemapMaxCurvePoints = 0;
    info.capability->getTonemapMaxCurvePoints(tonemapMaxCurvePoints);
    if (tonemapMaxCurvePoints > 0 && tonemapMaxCurvePoints < MIN_TONEMAP_POINTS) {
        LOGW("%s: wrong tonemap points", __func__);
        tonemapMaxCurvePoints = 0;
    }
    mGbceParams.gbce_on = (tonemapMaxCurvePoints > 0) ? true : false;
    mGbceParams.athena_mode = PlatformData::getPLCEnable(mCameraId);
    LOG1("%s, gbce_on: %d, plc enable: %d", __func__, mGbceParams.gbce_on,
         mGbceParams.athena_mode);

    return OK;
}

int AiqCore::init() {
    initAiqPlusParams();

#ifndef ENABLE_SANDBOXING
    ia_env env = {&Log::ccaPrintInfo, &Log::ccaPrintError, &Log::ccaPrintInfo};
    ia_log_init(&env);
#endif

    mAiqState = AIQ_INIT;

    int ret = mIntel3AParameter->init();
    CheckAndLogError(ret != OK, ret, "@%s, Init 3a parameter failed ret: %d", __func__, ret);

    CLEAR(mLastAeResult), mAeRunTime = 0;
    mAwbRunTime = 0;
    mAiqRunTime = 0;

    return OK;
}

int AiqCore::deinit() {
#ifndef ENABLE_SANDBOXING
    ia_log_deinit();
#endif

    mAiqState = AIQ_NOT_INIT;

    return OK;
}

int AiqCore::configure() {
    if (mAiqState == AIQ_CONFIGURED) {
        return OK;
    }

    mAiqState = AIQ_CONFIGURED;

    return OK;
}

int AiqCore::setSensorInfo(const ia_aiq_frame_params& frameParams,
                           const ia_aiq_exposure_sensor_descriptor& descriptor) {
    LOG2("@%s", __func__);

    mFrameParams = frameParams;
    mSensorPixelClock = descriptor.pixel_clock_freq_mhz;
    mIntel3AParameter->setSensorInfo(descriptor);

    return OK;
}

/**
 *
 * Calculate the Depth of field (DOF) for a given AF Result.
 *
 * The Formulas to calculate the near and afar DOF are:
 *          H * s
 * Dn = ---------------
 *         H + (s-f)
 *
 *          H * s
 * Df =  ------------
 *         H - (s-f)
 *
 * Where:
 * H is the hyperfocal distance (that we get from CPF) (it cannot be 0)
 * s is the distance to focused object (current focus distance)
 * f is the focal length
 *
 * \param[in] afResults with current focus distance in mm
 * \param[out] dof info: DOF for near and far limit in diopters
 */
int AiqCore::calculateDepthOfField(const cca::cca_af_results& afResults,
                                   camera_range_t* focusRange) {
    CheckAndLogError(!focusRange, BAD_VALUE, "@%s, Bad input values", __func__);

    const float DEFAULT_DOF = 5000.0f;
    focusRange->min = 1000.0f / DEFAULT_DOF;
    focusRange->max = 1000.0f / DEFAULT_DOF;

    float focusDistance = 1.0f * afResults.current_focus_distance;
    if (focusDistance == 0.0f) {
        // Not reporting error since this may be normal in fixed focus sensors
        return OK;
    }

    // focal length is stored in CMC in hundreds of millimeters
    float focalLengthMillis =
        static_cast<float>(mIntel3AParameter->mCMC.optics.effect_focal_length) / 100;

    float num = mHyperFocalDistance * focusDistance;
    float denom = (mHyperFocalDistance + focusDistance - focalLengthMillis);
    if (denom != 0.0f) {
        focusRange->min = num / denom;
    }

    denom = (mHyperFocalDistance - focusDistance + focalLengthMillis);
    if (denom != 0.0f) {
        focusRange->max = num / denom;
    }

    focusRange->min = 1000.0f / focusRange->min;
    focusRange->max = 1000.0f / focusRange->max;

    return OK;
}

int AiqCore::updateParameter(const aiq_parameter_t& param) {
    if (mTuningMode != param.tuningMode) {
        int ret = mIntel3AParameter->getCMCInfo(param.tuningMode);
        CheckAndLogError(ret != OK, ret, "failed to get CMC info");

        mHyperFocalDistance = AiqUtils::calculateHyperfocalDistance(mIntel3AParameter->mCMC);
        mTuningMode = param.tuningMode;
    }
    mShadingMode = param.shadingMode;
    mLensShadingMapMode = param.lensShadingMapMode;
    mLensShadingMapSize = param.lensShadingMapSize;

    mGbceParams.ev_shift = param.evShift;

    // In still frame use force update by setting convergence time to 0.
    // in other cases use tunings.
    mSaParams.manual_convergence_time = (param.frameUsage == FRAME_USAGE_STILL) ? 0.0 : -1.0;

    mIntel3AParameter->updateParameter(param);
    mAeForceLock = param.aeForceLock;
    mAwbForceLock = param.awbForceLock;
    mAfForceLock = mIntel3AParameter->mAfForceLock;

    // Enable AE/AWB/AF running rate if working in AUTO modes
    mAeBypassed = bypassAe(param);
    mAfBypassed = bypassAf(param);
    mAwbBypassed = bypassAwb(param);
    LOG2("Ae Bypass: %d, Af Bypass: %d, Awb Bypass: %d", mAeBypassed, mAfBypassed, mAwbBypassed);

    mRgbStatsBypassed = mAeBypassed && mAwbBypassed;

    return OK;
}

int AiqCore::setStatsParams(const cca::cca_stats_params& statsParams, cca::cca_out_stats* outStats,
                            AiqStatistics* aiqStats) {
    LOG2("<aiq%lu>@%s, frame_timestamp:%lu, mTuningMode:%d", statsParams.frame_id, __func__,
         statsParams.frame_timestamp, mTuningMode);
    CheckAndLogError(!outStats, BAD_VALUE, "@%s, outStats is nullptr", __func__);

    int ret = OK;
    IntelCca* intelCca = getIntelCca(mTuningMode);
    CheckAndLogError(!intelCca, UNKNOWN_ERROR, "%s, intelCca is nullptr, mode:%d", __func__,
                     mTuningMode);

    if (aiqStats && aiqStats->mPendingDecode) {
        uint32_t bitmap = 0;
        bool decodeRgbsStats = !mRgbStatsBypassed || outStats->get_rgbs_stats;
        if (decodeRgbsStats) bitmap |= cca::CCA_STATS_RGBS | cca::CCA_STATS_HIST;
        if (!mAfBypassed) {
            bitmap |= cca::CCA_STATS_AF;

            if (PlatformData::isPdafEnabled(mCameraId)) bitmap |= cca::CCA_STATS_PDAF;
        }
        LOG3("<seq%ld> bypass bitmap %x", aiqStats->mSequence, bitmap);

        if (decodeRgbsStats && !mAfBypassed) {
            aiqStats->mPendingDecode = false;
        }
        unsigned int byteUsed = 0;
        void* pStatsData = intelCca->fetchHwStatsData(aiqStats->mSequence, &byteUsed);
        CheckAndLogError(!pStatsData, UNKNOWN_ERROR, "%s, pStatsData is nullptr", __func__);
        ia_isp_bxt_statistics_query_results_t queryResults = {};
        ia_err iaErr = intelCca->decodeStats(reinterpret_cast<uint64_t>(pStatsData), byteUsed,
                                             bitmap, &queryResults);
        CheckAndLogError(iaErr != ia_err_none, UNKNOWN_ERROR, "%s, Faield convert statistics",
                         __func__);
        LOG2("%s, query results: rgbs_grid(%d), af_grid(%d), dvs_stats(%d)", __func__,
             queryResults.rgbs_grid, queryResults.af_grid, queryResults.dvs_stats);
    }

    {
        PERF_CAMERA_ATRACE_PARAM1_IMAGING("intelCca->setStatsParams", 1);
        ia_err iaErr = intelCca->setStatsParams(statsParams, outStats);
        ret = AiqUtils::convertError(iaErr);
        CheckAndLogError(ret != OK, ret, "setStatsParams fails, ret: %d", ret);
    }

    mTimestamp = statsParams.frame_timestamp;

    return ret;
}

int AiqCore::runAe(long requestId, AiqResult* aiqResult) {
    CheckAndLogError(!aiqResult, BAD_VALUE, "@%s, aiqResult is nullptr", __func__);
    LOG2("<req%ld>@%s, aiqResult %p", requestId, __func__, aiqResult);

    // run AE
    return runAEC(requestId, &aiqResult->mAeResults);
}

int AiqCore::runAiq(long requestId, AiqResult* aiqResult) {
    CheckAndLogError(!aiqResult, BAD_VALUE, "@%s, aiqResult is nullptr", __func__);

    int aaaRunType = IMAGING_ALGO_AWB | IMAGING_ALGO_GBCE | IMAGING_ALGO_PA;
    if (PlatformData::getLensHwType(mCameraId) == LENS_VCM_HW) {
        aaaRunType |= IMAGING_ALGO_AF;
    }
    if (mShadingMode != SHADING_MODE_OFF) {
        aaaRunType |= IMAGING_ALGO_SA;
    }
    LOG2("<req%ld>@%s, aiqResult %p, aaaRunType %x", requestId, __func__, aiqResult, aaaRunType);

    // get the IntelCca instance
    IntelCca* intelCca = getIntelCca(mTuningMode);
    CheckAndLogError(!intelCca, UNKNOWN_ERROR, "%s, intelCca is null, mode:%d", __func__,
                     mTuningMode);

    mAiqParams->bitmap = 0;

    // fill the parameter
    if (aaaRunType & IMAGING_ALGO_AWB) {
        mIntel3AParameter->mAwbParams.is_bypass = mAwbBypassed;
        mAiqParams->awb_input = mIntel3AParameter->mAwbParams;
        LOG2("AWB bypass %d", mAiqParams->awb_input.is_bypass);
        mAiqParams->bitmap |= cca::CCA_MODULE_AWB;
    }

    if (aaaRunType & IMAGING_ALGO_AF && !mAfBypassed) {
        mAiqParams->bitmap |= cca::CCA_MODULE_AF;
        mAiqParams->af_input = mIntel3AParameter->mAfParams;
    }

    if (aaaRunType & IMAGING_ALGO_GBCE) {
        // run gbce with bypass level if AE lock
        if (mAeForceLock || mIntel3AParameter->mTestPatternMode != TEST_PATTERN_OFF ||
            mRgbStatsBypassed) {
            mGbceParams.is_bypass = true;
        } else {
            mGbceParams.is_bypass = false;
        }
        mAiqParams->bitmap |= cca::CCA_MODULE_GBCE;
        mAiqParams->gbce_input = mGbceParams;
    }

    if (aaaRunType & IMAGING_ALGO_PA) {
        mPaParams.color_gains = {};
        mAiqParams->bitmap |= cca::CCA_MODULE_PA;
        mAiqParams->pa_input = mPaParams;
    }

    if (aaaRunType & IMAGING_ALGO_SA) {
        if (!mRgbStatsBypassed) {
            mAiqParams->bitmap |= cca::CCA_MODULE_SA;
            mSaParams.lsc_on = mLensShadingMapMode == LENS_SHADING_MAP_MODE_ON ? true : false;
            mAiqParams->sa_input = mSaParams;
        }
    }
    LOG2("bitmap:%d, mAiqRunTime:%lu, mRgbStatsBypassed %d", mAiqParams->bitmap, mAiqRunTime,
         mRgbStatsBypassed);

    // runAIQ for awb/af/gbce/pa/sa
    int ret = OK;
    {
        PERF_CAMERA_ATRACE_PARAM1_IMAGING("intelAiq->runAIQ", 1);

        ia_err iaErr = intelCca->runAIQ(requestId, *mAiqParams.get(), mAiqResults.get());
        mAiqRunTime++;
        ret = AiqUtils::convertError(iaErr);
        CheckAndLogError(ret != OK, ret, "@%s, runAIQ, ret: %d", __func__, ret);
    }

    // handle awb result
    if (aaaRunType & IMAGING_ALGO_AWB) {
        cca::cca_awb_results* newAwbResults = &mAiqResults->awb_output;

        if (!PlatformData::isIsysEnabled(mCameraId)) {
            // Fix AWB gain to 1 for none-ISYS cases
            newAwbResults->accurate_r_per_g = 1.0;
            newAwbResults->accurate_b_per_g = 1.0;
        }

        mIntel3AParameter->updateAwbResult(newAwbResults);
        aiqResult->mAwbResults = *newAwbResults;
        AiqUtils::dumpAwbResults(aiqResult->mAwbResults);
        ++mAwbRunTime;
    }

    // handle af result
    if (aaaRunType & IMAGING_ALGO_AF) {
        focusDistanceResult(&mAiqResults->af_output, &aiqResult->mAfDistanceDiopters,
                            &aiqResult->mFocusRange);
        aiqResult->mAfResults = mAiqResults->af_output;
        AiqUtils::dumpAfResults(aiqResult->mAfResults);

        aiqResult->mLensPosition = mIntel3AParameter->mAfParams.lens_position;
        mIntel3AParameter->fillAfTriggerResult(&aiqResult->mAfResults);
    }

    // handle gbce result
    if (aaaRunType & IMAGING_ALGO_GBCE) {
        aiqResult->mGbceResults = mAiqResults->gbce_output;
        AiqUtils::dumpGbceResults(aiqResult->mGbceResults);
    }

    // handle pa result
    if (aaaRunType & IMAGING_ALGO_PA) {
        mIntel3AParameter->updatePaResult(&mAiqResults->pa_output);
        aiqResult->mPaResults = mAiqResults->pa_output;
        AiqUtils::dumpPaResults(aiqResult->mPaResults);
    }

    // handle sa result
    if (aaaRunType & IMAGING_ALGO_SA) {
        AiqUtils::dumpSaResults(mAiqResults->sa_output);
        ret |= processSAResults(&mAiqResults->sa_output, aiqResult->mLensShadingMap);
    }
    CheckAndLogError(ret != OK, ret, "run3A failed, ret: %d", ret);

    uint16_t pixelInLine = aiqResult->mAeResults.exposures[0].sensor_exposure->line_length_pixels;
    uint16_t lineInFrame = aiqResult->mAeResults.exposures[0].sensor_exposure->frame_length_lines;
    aiqResult->mFrameDuration = pixelInLine * lineInFrame / mSensorPixelClock;
    aiqResult->mRollingShutter =
        pixelInLine * (mFrameParams.cropped_image_height - 1) / mSensorPixelClock;

    mLastEvShift = mIntel3AParameter->mAeParams.ev_shift;
    aiqResult->mTimestamp = mTimestamp;

    if (PlatformData::isStatsRunningRateSupport(mCameraId)) {
        bool bothConverged = (mLastAeResult.exposures[0].converged &&
                              mAiqResults->awb_output.distance_from_convergence < EPSILON);
        if (!mAeAndAwbConverged && bothConverged) {
            mAeRunRateInfo.reset();
            mAwbRunRateInfo.reset();
        }
        mAeAndAwbConverged = bothConverged;
    }

    return OK;
}

int AiqCore::runAEC(long requestId, cca::cca_ae_results* aeResults) {
    PERF_CAMERA_ATRACE();

    int ret = OK;
    cca::cca_ae_results* newAeResults = &mLastAeResult;

    // Run AEC with setting bypass mode to false
    mIntel3AParameter->mAeParams.is_bypass = mAeBypassed;

    if (mAeForceLock && mIntel3AParameter->mAeMode != AE_MODE_MANUAL && mAeRunTime != 0 &&
        !mIntel3AParameter->mAeParams.is_bypass) {
        // Use manual setttings if AE had been locked
        mIntel3AParameter->mAeParams.manual_exposure_time_us[0] = mLockedExposureTimeUs;
        mIntel3AParameter->mAeParams.manual_iso[0] = mLockedIso;
    }

    IntelCca* intelCca = getIntelCca(mTuningMode);
    CheckAndLogError(!intelCca, UNKNOWN_ERROR, "%s, intelCca is null, m:%d", __func__, mTuningMode);
    {
        PERF_CAMERA_ATRACE_PARAM1_IMAGING("intelCca->runAEC", 1);
        ia_err iaErr = intelCca->runAEC(requestId, mIntel3AParameter->mAeParams, newAeResults);
        ret = AiqUtils::convertError(iaErr);
        CheckAndLogError(ret != OK, ret, "Error running AE, ret: %d", ret);
    }

    if (!mAeForceLock) {
        // Save exposure results if unlocked
        mLockedExposureTimeUs = newAeResults->exposures[0].exposure[0].exposure_time_us;
        mLockedIso = newAeResults->exposures[0].exposure[0].iso;
    }

    mIntel3AParameter->updateAeResult(newAeResults);
    *aeResults = *newAeResults;
    AiqUtils::dumpAeResults(*aeResults);
    ++mAeRunTime;

    return ret;
}

void AiqCore::focusDistanceResult(const cca::cca_af_results* afResults, float* afDistanceDiopters,
                                  camera_range_t* focusRange) {
    LOG2("@%s, afResults:%p, afDistanceDiopters:%p, focusRange:%p", __func__, afResults,
         afDistanceDiopters, focusRange);

    *afDistanceDiopters = 1.2f;
    if (mIntel3AParameter->mAfParams.focus_mode == ia_aiq_af_operation_mode_infinity) {
        // infinity mode is special: we need to report 0.0f (1/inf = 0)
        *afDistanceDiopters = 0.0f;
    } else if (mIntel3AParameter->mAfParams.focus_mode == ia_aiq_af_operation_mode_manual &&
               (mIntel3AParameter->mAfParams.manual_focus_parameters.manual_focus_action ==
                ia_aiq_manual_focus_action_set_distance) &&
               (mIntel3AParameter->mAfParams.manual_focus_parameters.manual_focus_distance ==
                afResults->current_focus_distance)) {
        // Don't need to calculate diopter in manual mode
        *afDistanceDiopters = mIntel3AParameter->mManualFocusDistance;
    } else if (afResults->current_focus_distance != 0) {
        // In AIQ, 'current_focus_distance' is in millimeters
        // For rounding multiply by extra 100.
        // This allows the diopters to have 2 decimal values
        *afDistanceDiopters = 100 * 1000 * (1.0 / afResults->current_focus_distance);
        *afDistanceDiopters = ceil(*afDistanceDiopters);
        // Divide by 100 for final result.
        *afDistanceDiopters = *afDistanceDiopters / 100;
    }

    calculateDepthOfField(*afResults, focusRange);
}

int AiqCore::checkColorOrder(cmc_bayer_order bayerOrder, ColorOrder* colorOrder) {
    LOG2("@%s, bayerOrder = %d, colorOrder:%p", __func__, bayerOrder, colorOrder);

    int ret = OK;
    switch (bayerOrder) {
        case cmc_bayer_order_grbg:
            /* use gr r b gb constitute 2X2 array
             * gr    r
             * b     gb
             * The four channel use x y coordinate to indicate
             * gr(0, 0) r(1, 0) b(0, 1) gb(1, 1)
             */
            colorOrder->r[0] = 1;
            colorOrder->r[1] = 0;
            colorOrder->b[0] = 0;
            colorOrder->b[1] = 1;
            colorOrder->gr[0] = 0;
            colorOrder->gr[1] = 0;
            colorOrder->gb[0] = 1;
            colorOrder->gb[1] = 1;
            break;
        case cmc_bayer_order_rggb:
            colorOrder->r[0] = 0;
            colorOrder->r[1] = 0;
            colorOrder->b[0] = 1;
            colorOrder->b[1] = 1;
            colorOrder->gr[0] = 1;
            colorOrder->gr[1] = 0;
            colorOrder->gb[0] = 0;
            colorOrder->gb[1] = 1;
            break;
        case cmc_bayer_order_bggr:
            colorOrder->r[0] = 1;
            colorOrder->r[1] = 1;
            colorOrder->b[0] = 0;
            colorOrder->b[1] = 0;
            colorOrder->gr[0] = 0;
            colorOrder->gr[1] = 1;
            colorOrder->gb[0] = 1;
            colorOrder->gb[1] = 0;
            break;
        case cmc_bayer_order_gbrg:
            colorOrder->r[0] = 0;
            colorOrder->r[1] = 1;
            colorOrder->b[0] = 1;
            colorOrder->b[1] = 0;
            colorOrder->gr[0] = 1;
            colorOrder->gr[1] = 1;
            colorOrder->gb[0] = 0;
            colorOrder->gb[1] = 0;
            break;
        default:
            ret = BAD_VALUE;
            break;
    }
    return ret;
}

int AiqCore::reFormatLensShadingMap(const LSCGrid& inputLscGrid, float* dstLscGridRGGB) {
    CheckAndLogError(inputLscGrid.isBad() || !dstLscGridRGGB, BAD_VALUE,
                     "@%s, Bad input values for lens shading map reformatting", __func__);
    LOG2("@%s, width %d, height %d", __func__, inputLscGrid.width, inputLscGrid.height);

    // Metadata spec request order [R, Geven, Godd, B]
    // the lensShading from ISP is 4 width * height block,
    // for ia_aiq_bayer_order_grbg, the four block is G, R, B, G
    size_t size = inputLscGrid.height * inputLscGrid.width;
    for (size_t i = 0; i < size; i++) {
        *dstLscGridRGGB++ = inputLscGrid.gridR[i];
        *dstLscGridRGGB++ = inputLscGrid.gridGr[i];
        *dstLscGridRGGB++ = inputLscGrid.gridGb[i];
        *dstLscGridRGGB++ = inputLscGrid.gridB[i];
    }

    return OK;
}

int AiqCore::storeLensShadingMap(const LSCGrid& inputLscGrid, const LSCGrid& resizeLscGrid,
                                 float* dstLscGridRGGB) {
    CheckAndLogError(inputLscGrid.isBad() || resizeLscGrid.isBad() || !dstLscGridRGGB, BAD_VALUE,
                     "@%s, Bad input values for lens shading map storing", __func__);

    int destWidth = resizeLscGrid.width;
    int destHeight = resizeLscGrid.height;
    int width = inputLscGrid.width;
    int height = inputLscGrid.height;

    if (width != destWidth || height != destHeight) {
        // requests lensShadingMapSize must be smaller than 64*64
        // and it is a constant size.
        // Our lensShadingMapSize is dynamic based on the resolution, so need
        // to do resize for 4 channels separately

        AiqUtils::resize2dArray(inputLscGrid.gridR, width, height, resizeLscGrid.gridR, destWidth,
                                destHeight);
        AiqUtils::resize2dArray(inputLscGrid.gridGr, width, height, resizeLscGrid.gridGr, destWidth,
                                destHeight);
        AiqUtils::resize2dArray(inputLscGrid.gridGb, width, height, resizeLscGrid.gridGb, destWidth,
                                destHeight);
        AiqUtils::resize2dArray(inputLscGrid.gridB, width, height, resizeLscGrid.gridB, destWidth,
                                destHeight);

        LOG2("%s:resize lens shading map from [%d,%d] to [%d,%d]", __func__, width, height,
             destWidth, destHeight);
    } else {
        size_t size = destWidth * destHeight * sizeof(resizeLscGrid.gridR[0]);
        STDCOPY((int8_t*)resizeLscGrid.gridR, (int8_t*)inputLscGrid.gridR, size);
        STDCOPY((int8_t*)resizeLscGrid.gridGr, (int8_t*)inputLscGrid.gridGr, size);
        STDCOPY((int8_t*)resizeLscGrid.gridGb, (int8_t*)inputLscGrid.gridGb, size);
        STDCOPY((int8_t*)resizeLscGrid.gridB, (int8_t*)inputLscGrid.gridB, size);
    }

    return reFormatLensShadingMap(resizeLscGrid, dstLscGridRGGB);
}

int AiqCore::processSAResults(cca::cca_sa_results* saResults, float* lensShadingMap) {
    CheckAndLogError(!saResults || !lensShadingMap, BAD_VALUE,
                     "@%s, Bad input values, saResults %p, lensShadingMap %p", __func__, saResults,
                     lensShadingMap);
    LOG2("@%s, mLensShadingMapMode %d", __func__, mLensShadingMapMode);

    if (mLensShadingMapMode == LENS_SHADING_MAP_MODE_OFF) return OK;

    if (saResults->lsc_update) {
        ColorOrder co_ind = {};
        int ret = checkColorOrder(saResults->color_order, &co_ind);
        CheckAndLogError(ret != OK, BAD_VALUE, "Failed to checkColorOrder, ret: %d", ret);

        LSCGrid inputGrid;
        inputGrid.gridB = saResults->lsc_grid[co_ind.b[0]][co_ind.b[1]];
        inputGrid.gridR = saResults->lsc_grid[co_ind.r[0]][co_ind.r[1]];
        inputGrid.gridGr = saResults->lsc_grid[co_ind.gr[0]][co_ind.gr[1]];
        inputGrid.gridGb = saResults->lsc_grid[co_ind.gb[0]][co_ind.gb[1]];
        inputGrid.width = saResults->width;
        inputGrid.height = saResults->height;

        LSCGrid resizeGrid;
        resizeGrid.gridB = mResizeLscGridB;
        resizeGrid.gridR = mResizeLscGridR;
        resizeGrid.gridGr = mResizeLscGridGr;
        resizeGrid.gridGb = mResizeLscGridGb;
        resizeGrid.width = mLensShadingMapSize.x;
        resizeGrid.height = mLensShadingMapSize.y;

        storeLensShadingMap(inputGrid, resizeGrid, mLscGridRGGB);

        // resizeGrid's width and height should be equal to inputGrid's width and height
        mLscGridRGGBLen = resizeGrid.width * resizeGrid.height * 4;
        size_t errCount = 0;
        for (size_t i = 0; i < mLscGridRGGBLen; i++) {
            if (mLscGridRGGB[i] < 1.0f) {
                mLscGridRGGB[i] = 1.0f;
                errCount++;
            }
        }
        if (errCount) {
            LOGW("Error - SA produced too small values (%zu/%zu)!", errCount, mLscGridRGGBLen);
        }
    }

    float* lsm = (mShadingMode != SHADING_MODE_OFF) ? mLscGridRGGB : mLscOffGrid;
    for (size_t i = 0; i < mLscGridRGGBLen; i++) {
        lensShadingMap[i] = lsm[i];
    }

    return OK;
}

bool AiqCore::bypassAe(const aiq_parameter_t& param) {
    if (mAeRunTime == 0 || (mIntel3AParameter->mAeParams.ev_shift != mLastEvShift)) return false;
    if (mAeForceLock || mAeRunTime % mIntel3AParameter->mAePerTicks != 0) return true;

    if (param.aeMode != AE_MODE_AUTO || param.powerMode != CAMERA_LOW_POWER) return false;

    bool converged = mLastAeResult.exposures[0].converged;

    return skipAlgoRunning(&mAeRunRateInfo, IMAGING_ALGO_AE, converged);
}

bool AiqCore::bypassAf(const aiq_parameter_t& param) {
    if (mAfForceLock) return true;

    if (param.afMode == AF_MODE_OFF || param.powerMode != CAMERA_LOW_POWER) return false;

    bool converged = mAiqResults->af_output.status == ia_aiq_af_status_success &&
                     mAiqResults->af_output.final_lens_position_reached;

    return skipAlgoRunning(&mAfRunRateInfo, IMAGING_ALGO_AF, converged);
}

bool AiqCore::bypassAwb(const aiq_parameter_t& param) {
    if (mAwbForceLock || mAwbRunTime % mIntel3AParameter->mAwbPerTicks != 0 ||
        mIntel3AParameter->mTestPatternMode != TEST_PATTERN_OFF)
        return true;

    if (param.awbMode != AWB_MODE_AUTO || param.powerMode != CAMERA_LOW_POWER) return false;

    bool converged = mAiqResults->awb_output.distance_from_convergence < EPSILON;

    return skipAlgoRunning(&mAwbRunRateInfo, IMAGING_ALGO_AWB, converged);
}

bool AiqCore::skipAlgoRunning(RunRateInfo* info, int algo, bool converged) {
    float configRunningRate = PlatformData::getAlgoRunningRate(algo, mCameraId);
    if (configRunningRate < EPSILON) return false;
    LOG2("the running rate of type %d is %f", algo, configRunningRate);

    // if converged, skip algo if running rate is reached.
    if (converged) {
        info->runCcaTime++;
        if (checkRunRate(configRunningRate, info)) return true;

        info->runAlgoTime++;
    } else {
        info->runCcaTime = 0;
        info->runAlgoTime = 0;
    }

    return false;
}

bool AiqCore::checkRunRate(float configRunningRate, const RunRateInfo* info) {
    LOG2("configRunningRate %f, runAlgoTime %d, runCcaTime %d", configRunningRate,
         info->runAlgoTime, info->runCcaTime);
    if (info->runCcaTime == 0) return true;

    float algoRunningRate = static_cast<float>(info->runAlgoTime) / info->runCcaTime;
    if ((algoRunningRate - configRunningRate) > EPSILON) return true;

    return false;
}

IntelCca* AiqCore::getIntelCca(TuningMode tuningMode) {
    CheckAndLogError(tuningMode >= TUNING_MODE_MAX, nullptr, "@%s, wrong tuningMode:%d", __func__,
                     tuningMode);

    return IntelCca::getInstance(mCameraId, tuningMode);
}

} /* namespace icamera */
