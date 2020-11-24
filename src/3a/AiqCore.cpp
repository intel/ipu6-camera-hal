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

#define LOG_TAG "AiqCore"

#include <math.h>

#include <memory>
#include <string>

#include "PlatformData.h"
#include "AiqUtils.h"
#include "Parameters.h"

#include "iutils/CameraLog.h"
#include "iutils/Errors.h"
#include "iutils/Utils.h"

#include "AiqCore.h"

namespace icamera {
AiqCore::AiqCore(int cameraId) :
    mCameraId(cameraId),
    mTimestamp(0),
    mSensorPixelClock(0.0),
    mAeForceLock(false),
    mAwbForceLock(false),
    mAfForceLock(false),
    mLastAeResult(nullptr),
    mLastAwbResult(nullptr),
    mLastAfResult(nullptr),
    mAeRunTime(0),
    mAwbRunTime(0),
    mAiqState(AIQ_NOT_INIT),
    mHyperFocalDistance(0.0f),
    mTuningMode(TUNING_MODE_MAX),
    mShadingMode(SHADING_MODE_FAST),
    mLensShadingMapMode(LENS_SHADING_MAP_MODE_OFF),
    mLscGridRGGBLen(0),
    mLastEvShift(0.0f) {
    LOG3A("@%s", __func__);

    mIntel3AParameter = std::unique_ptr<Intel3AParameter>(new Intel3AParameter(cameraId));

    CLEAR(mFrameParams);
    CLEAR(mGbceParams);
    CLEAR(mPaParams);
    CLEAR(mSaParams);
    CLEAR(mIntelAiqHandle);
    CLEAR(mIntelAiqHandleStatus);
    CLEAR(mPaColorGains);

    CLEAR(mResizeLscGridR);
    CLEAR(mResizeLscGridGr);
    CLEAR(mResizeLscGridGb);
    CLEAR(mResizeLscGridB);

    CLEAR(mLensShadingMapSize);
    CLEAR(mLscGridRGGB);

    // init LscOffGrid to 1.0f
    std::fill(std::begin(mLscOffGrid), std::end(mLscOffGrid), 1.0f);
}

AiqCore::~AiqCore() {
    LOG3A("@%s", __func__);

    for (int i = 0; i < TUNING_MODE_MAX; i++) {
        if (mIntelAiqHandle[i]) {
            delete mIntelAiqHandle[i];
        }
    }
}

int AiqCore::initAiqPlusParams() {
    LOG3A("@%s", __func__);

    CLEAR(mFrameParams);
    CLEAR(mGbceParams);
    CLEAR(mPaParams);
    CLEAR(mPaColorGains);
    CLEAR(mSaParams);

    mGbceParams.gbce_level = ia_aiq_gbce_level_use_tuning;
    mGbceParams.frame_use = ia_aiq_frame_use_video;
    mGbceParams.ev_shift = 0;
    mGbceParams.tone_map_level = ia_aiq_tone_map_level_use_tuning;

    mPaParams.color_gains = nullptr;

    mSaParams.sensor_frame_params = &mFrameParams;
    /* use convergence time from tunings */
    mSaParams.manual_convergence_time = -1.0;

    return OK;
}

int AiqCore::init() {
    LOG3A("@%s", __func__);

    initAiqPlusParams();

#ifndef ENABLE_SANDBOXING
    ia_env env = {&Log::ccaPrintDebug, &Log::ccaPrintError, &Log::ccaPrintInfo};
    ia_log_init(&env);
#endif

    mAiqState = AIQ_INIT;

    int ret = mIntel3AParameter->init();
    CheckError(ret != OK, ret, "@%s, Init 3a parameter failed ret: %d", __func__, ret);

    mLastAeResult = nullptr;
    mLastAwbResult = nullptr;
    mLastAfResult = nullptr;
    mAeRunTime = 0;
    mAwbRunTime = 0;

    return OK;
}

void AiqCore::deinitIntelAiqHandle() {
    LOG3A("@%s", __func__);

    for (auto mode = 0; mode < TUNING_MODE_MAX; mode++) {
        IntelAiq* aiq = mIntelAiqHandle[mode];
        if (!aiq) continue;

        if (PlatformData::isAiqdEnabled(mCameraId)) {
            ia_binary_data data = {nullptr, 0};
            ia_err iaErr = aiq->getAiqdData(&data);
            if (AiqUtils::convertError(iaErr) == OK) {
                PlatformData::saveAiqd(mCameraId, static_cast<TuningMode>(mode), data);
            } else {
                LOGW("@%s, failed to get aiqd data, iaErr %d", __func__, iaErr);
            }
        }
        aiq->deinit();
        delete aiq;
        mIntelAiqHandle[mode] = nullptr;
    }
    CLEAR(mIntelAiqHandleStatus);
}

int AiqCore::deinit() {
    LOG3A("@%s", __func__);

#ifndef ENABLE_SANDBOXING
    ia_log_deinit();
#endif

    deinitIntelAiqHandle();

    mAiqState = AIQ_NOT_INIT;

    return OK;
}

int AiqCore::initIntelAiqHandle(const std::vector<TuningMode>& tuningModes) {
    LOG3A("@%s", __func__);

    ia_mkn* mkn = static_cast<ia_mkn*>(PlatformData::getMknHandle(mCameraId));
    ia_binary_data *nvmData = PlatformData::getNvm(mCameraId);
    ia_binary_data aiqData = {nullptr, 0};
    // Initialize mIntelAiqHandle array based on different cpf data
    for (auto & mode : tuningModes) {
        uintptr_t cmcHandle = reinterpret_cast<uintptr_t>(nullptr);
        int ret = PlatformData::getCpfAndCmc(mCameraId, nullptr, &aiqData, nullptr,
                                             &cmcHandle, mode);
        CheckError(ret != OK, BAD_VALUE, "@%s, getDataAndCmc fails", __func__);

        ia_binary_data* aiqd = nullptr;
        if (PlatformData::PlatformData::isAiqdEnabled(mCameraId)) {
            aiqd = PlatformData::getAiqd(mCameraId, mode);
        }

        int statsNum = PlatformData::getExposureNum(mCameraId,
                           CameraUtils::isMultiExposureCase(mode));
        {
            IntelAiq* intelAiq = new IntelAiq();
            PERF_CAMERA_ATRACE_PARAM1_IMAGING("intelAiq->init", 1);
            ia_aiq* aiq = intelAiq->init(&(aiqData),
                                        nvmData,
                                        aiqd,
                                        MAX_STATISTICS_WIDTH,
                                        MAX_STATISTICS_HEIGHT,
                                        statsNum,
                                        reinterpret_cast<ia_cmc_t*>(cmcHandle),
                                        mkn);
            if (aiq) {
                mIntelAiqHandle[mode] = intelAiq;

                std::string aiqVersion;
                intelAiq->getVersion(&aiqVersion);
                LOGI("@%s, AIQ VERSION: %s", __func__, aiqVersion.c_str());
            } else {
                mIntelAiqHandle[mode] = nullptr;
                delete intelAiq;
            }
        }
        CheckError(!mIntelAiqHandle[mode], NO_INIT, "@%s: init aiq failed!", __func__);
        mIntelAiqHandleStatus[mode] = true;
    }

    return OK;
}

int AiqCore::configure(const std::vector<ConfigMode>& configModes) {
    LOG3A("@%s", __func__);

    int ret = OK;
    bool allTuningModeConfiged = true;
    std::vector<TuningMode> tuningModes;
    for (auto cfg : configModes) {
        TuningMode mode;
        ret = PlatformData::getTuningModeByConfigMode(mCameraId, cfg, mode);
        CheckError(ret != OK, ret, "%s: getTuningModeByConfigMode fails, cfg:%d", __func__, cfg);
        tuningModes.push_back(mode);

        if (!mIntelAiqHandle[mode]) {
            allTuningModeConfiged = false;
        }
    }

    if (mAiqState == AIQ_CONFIGURED && allTuningModeConfiged) {
        return OK;
    }

    deinitIntelAiqHandle();

    ret = initIntelAiqHandle(tuningModes);
    if (ret == OK) {
        mAiqState = AIQ_CONFIGURED;
    }

    return OK;
}

int AiqCore::setSensorInfo(const ia_aiq_frame_params &frameParams,
                           const ia_aiq_exposure_sensor_descriptor &descriptor) {
    LOG3A("@%s", __func__);

    mFrameParams = frameParams;
    mSensorPixelClock = descriptor.pixel_clock_freq_mhz;
    mIntel3AParameter->setSensorInfo(descriptor);

    return OK;
}

/**
 *  Hyperfocal distance is the closest distance at which a lens can be focused
 *  while keeping objects at infinity acceptably sharp. When the lens is focused
 *  at this distance, all objects at distances from half of the hyperfocal
 *  distance out to infinity will be acceptably sharp.
 *
 *  The equation used for this is:
 *        f*f
 *  H = -------
 *        N*c
 *
 *  where:
 *   f is the focal length
 *   N is the f-number (f/D for aperture diameter D)
 *   c is the Circle Of Confusion (COC)
 *
 *   the COC is calculated as the pixel width of 2 pixels
 *
 *  The hyperfocal distance in mm. It is ensured it will never be 0 to
 *  avoid division by 0. If any of the required CMC items is missing
 *  it will return the default value 5m
 */
int AiqCore::calculateHyperfocalDistance(TuningMode mode) {
    LOG3A("@%s, tuning mode: %d", __func__, mode);

    ia_cmc_t *cmcData = nullptr;
    int ret = PlatformData::getCpfAndCmc(mCameraId, nullptr, nullptr, nullptr,
                                         nullptr, mode, &cmcData);
    CheckError(ret != OK || !cmcData, BAD_VALUE, "@%s get cmc data failed", __func__);

    mHyperFocalDistance = AiqUtils::calculateHyperfocalDistance(*cmcData);

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
int AiqCore::calculateDepthOfField(const ia_aiq_af_results &afResults,
                                   camera_range_t *focusRange) {
    LOG3A("@%s, afResults:%p, focusRange:%p", __func__, afResults, focusRange);
    CheckError(!focusRange, BAD_VALUE, "@%s, Bad input values", __func__);

    const float DEFAULT_DOF = 5000.0f;
    focusRange->min = 1000.0f / DEFAULT_DOF;
    focusRange->max = 1000.0f / DEFAULT_DOF;

    float focusDistance = 1.0f * afResults.current_focus_distance;
    if (focusDistance == 0.0f) {
        // Not reporting error since this may be normal in fixed focus sensors
        return OK;
    }

    ia_cmc_t *cmcData = nullptr;
    cmc_optomechanics_t *optoInfo = nullptr;
    PlatformData::getCpfAndCmc(mCameraId, nullptr, nullptr, nullptr,
                               nullptr, mTuningMode, &cmcData);
    if (cmcData) {
        optoInfo = cmcData->cmc_parsed_optics.cmc_optomechanics;
    }

    float focalLengthMillis = 2.3f;
    if (optoInfo) {
        // focal length is stored in CMC in hundreds of millimeters
        focalLengthMillis = static_cast<float>(optoInfo->effect_focal_length) / 100;
    }

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

int AiqCore::updateParameter(const aiq_parameter_t &param) {
    LOG3A("@%s", __func__);

    if (mTuningMode != param.tuningMode) {
        int ret = calculateHyperfocalDistance(param.tuningMode);
        CheckError(ret != OK, ret, "%s calculateHyperfocalDistance fails", __func__);
        mTuningMode = param.tuningMode;
    }
    mShadingMode = param.shadingMode;
    mLensShadingMapMode = param.lensShadingMapMode;
    mLensShadingMapSize = param.lensShadingMapSize;

    mGbceParams.frame_use = AiqUtils::convertFrameUsageToIaFrameUsage(param.frameUsage);
    mGbceParams.ev_shift = param.evShift;

    // In still frame use force update by setting convergence time to 0.
    // in other cases use tunings.
    mSaParams.manual_convergence_time = (param.frameUsage == FRAME_USAGE_STILL) ? 0.0 : -1.0;

    mIntel3AParameter->updateParameter(param);
    mAeForceLock = param.aeForceLock;
    mAwbForceLock = param.awbForceLock;
    mAfForceLock = mIntel3AParameter->mAfForceLock;

    return OK;
}

int AiqCore::setStatistics(const ia_aiq_statistics_input_params_v4 *ispStatistics) {
    LOG3A("@%s, ispStatistics:%p", __func__, ispStatistics);
    CheckError(!ispStatistics, BAD_VALUE, "@%s, ispStatistics is nullptr", __func__);

    int ret = OK;

    CheckError(mTuningMode >= TUNING_MODE_MAX, UNKNOWN_ERROR, "mTuningMode overflow!");
    IntelAiq* intelAiq = mIntelAiqHandle[mTuningMode];
    CheckError(!intelAiq, UNKNOWN_ERROR, "%s, aiq is nullptr, mode:%d", __func__, mTuningMode);
    {
        PERF_CAMERA_ATRACE_PARAM1_IMAGING("statisticsSetV4", 1);
        ia_err iaErr = intelAiq->statisticsSetV4(ispStatistics);
        ret = AiqUtils::convertError(iaErr);
        CheckError(ret != OK, ret, "Error setting statistics, ret = %d", ret);
    }

    mTimestamp = ispStatistics->frame_timestamp;
    return ret;
}

int AiqCore::runAiq(AiqResult *aiqResult) {
    LOG3A("@%s, aiqResult:%p", __func__, aiqResult);
    CheckError(!aiqResult, BAD_VALUE, "@%s, aiqResult is nullptr", __func__);

    int ret = run3A(aiqResult);
    CheckError(ret != OK, ret, "run3A failed, ret: %d", ret);

    ret = runAiqPlus(aiqResult);
    CheckError(ret != OK, ret, "runAiqPlus failed, ret: %d", ret);

    mLastEvShift = mIntel3AParameter->mAeParams.ev_shift;
    aiqResult->mTimestamp = mTimestamp;
    return OK;
}

int AiqCore::run3A(AiqResult *aiqResult) {
    LOG3A("@%s, aiqResult:%p", __func__, aiqResult);
    CheckError(!aiqResult, BAD_VALUE, "@%s, aiqResult is nullptr", __func__);

    int ret = OK;
    int aaaType = IMAGING_ALGO_AE | IMAGING_ALGO_AWB;
    if (PlatformData::getLensHwType(mCameraId) == LENS_VCM_HW) {
        aaaType |= IMAGING_ALGO_AF;
    }

    if (aaaType & IMAGING_ALGO_AE) {
        ret |= runAe(&aiqResult->mAeResults);
    }
    if (aaaType & IMAGING_ALGO_AWB) {
        ret |= runAwb(&aiqResult->mAwbResults);
    }
    if (aaaType & IMAGING_ALGO_AF) {
        ret |= runAf(aiqResult);
    }

    uint16_t pixelInLine = aiqResult->mAeResults.exposures[0].sensor_exposure->line_length_pixels;
    uint16_t lineInFrame = aiqResult->mAeResults.exposures[0].sensor_exposure->frame_length_lines;
    aiqResult->mFrameDuration = pixelInLine * lineInFrame / mSensorPixelClock;
    aiqResult->mRollingShutter = pixelInLine * (mFrameParams.cropped_image_height - 1)
                                 / mSensorPixelClock;
    return ret;
}

int AiqCore::runAiqPlus(AiqResult *aiqResult) {
    LOG3A("@%s, aiqResult:%p", __func__, aiqResult);
    CheckError(!aiqResult, BAD_VALUE, "@%s, aiqResult is nullptr", __func__);

    int ret = runGbce(&aiqResult->mGbceResults);
    ret |= runPa(&aiqResult->mPaResults, &aiqResult->mAwbResults,
                 aiqResult->mAeResults.exposures[0].exposure,
                 &aiqResult->mPreferredAcm);
    ret |= runSa(&aiqResult->mSaResults, &aiqResult->mAwbResults, aiqResult->mLensShadingMap);

    return ret;
}

int AiqCore::runAe(ia_aiq_ae_results* aeResults) {
    LOG3A("@%s, aeResults:%p", __func__, aeResults);
    CheckError(!aeResults, BAD_VALUE, "@%s, aeResults is nullptr", __func__);
    PERF_CAMERA_ATRACE();

    int ret = OK;
    ia_aiq_ae_results *newAeResults = mLastAeResult;
    bool aeForceRun = mIntel3AParameter->mAeParams.ev_shift != mLastEvShift ||
                      (!mAeForceLock && (mAeRunTime % mIntel3AParameter->mAePerTicks == 0));

    if (aeForceRun) {
        LOG3A("AEC frame_use: %d", mIntel3AParameter->mAeParams.frame_use);

        IntelAiq* intelAiq = mIntelAiqHandle[mTuningMode];
        CheckError(!intelAiq, UNKNOWN_ERROR, "%s, aiq is nullptr, mode:%d", __func__, mTuningMode);
        {
            PERF_CAMERA_ATRACE_PARAM1_IMAGING("intelAiq->aeRun", 1);
            ia_err iaErr = intelAiq->aeRun(&mIntel3AParameter->mAeParams, &newAeResults);
            ret = AiqUtils::convertError(iaErr);
            CheckError(ret != OK || !newAeResults, ret, "Error running AE, ret: %d", ret);
        }
    }

    mIntel3AParameter->updateAeResult(newAeResults);
    if (newAeResults) {
        ret = AiqUtils::deepCopyAeResults(*newAeResults, aeResults);
    }
    mLastAeResult = aeResults;
    ++mAeRunTime;

    return ret;
}

int AiqCore::runAf(AiqResult *aiqResult) {
    LOG3A("@%s, aiqResult:%p", __func__, aiqResult);
    CheckError(!aiqResult, BAD_VALUE, "@%s, aiqResult is nullptr", __func__);
    PERF_CAMERA_ATRACE();

    ia_aiq_af_results *afResults = &aiqResult->mAfResults;
    ia_aiq_af_results *newAfResults = mLastAfResult;

    int ret = OK;
    if (!mAfForceLock) {
        IntelAiq* intelAiq = mIntelAiqHandle[mTuningMode];
        CheckError(!intelAiq, UNKNOWN_ERROR, "@%s, aiq is nullptr, mode:%d", __func__, mTuningMode);
        {
            PERF_CAMERA_ATRACE_PARAM1_IMAGING("intelAiq->afRun", 1);
            ia_err iaErr = intelAiq->afRun(&mIntel3AParameter->mAfParams, &newAfResults);
            ret = AiqUtils::convertError(iaErr);
            CheckError(ret != OK || !newAfResults, ret, "Error running AF, ret: %d", ret);
        }
    }

    focusDistanceResult(newAfResults, &aiqResult->mAfDistanceDiopters, &aiqResult->mFocusRange);
    ret = AiqUtils::deepCopyAfResults(*newAfResults, afResults);

    mLastAfResult = afResults;
    mIntel3AParameter->fillAfTriggerResult(newAfResults);
    return ret;
}

void AiqCore::focusDistanceResult(const ia_aiq_af_results *afResults,
                                  float *afDistanceDiopters,
                                  camera_range_t *focusRange) {
    LOG3A("@%s, afResults:%p, afDistanceDiopters:%p, focusRange:%p", __func__,
          afResults, afDistanceDiopters, focusRange);
    CheckError(!afResults || !afDistanceDiopters || !focusRange, VOID_VALUE,
               "@%s, Bad input values", __func__);

    *afDistanceDiopters = 1.2f;
    if (mIntel3AParameter->mAfParams.focus_mode == ia_aiq_af_operation_mode_infinity) {
        // infinity mode is special: we need to report 0.0f (1/inf = 0)
        *afDistanceDiopters = 0.0f;
    } else if (afResults->current_focus_distance != 0) {
        // In AIQ, 'current_focus_distance' is in millimeters
        // For rounding multiply by extra 100.
        // This allows the diopters to have 2 decimal values
        *afDistanceDiopters = 100 * 1000 * (1.0 / afResults->current_focus_distance);
        *afDistanceDiopters = ceil(*afDistanceDiopters);
        // Divide by 100 for final result.
        *afDistanceDiopters = *afDistanceDiopters / 100;
    }
    LOG3A("%s, Zero focus distance in AF result, reporting %f", __func__, *afDistanceDiopters);

    calculateDepthOfField(*afResults, focusRange);
    LOG3A("%s, focus distance with diopters: %f %f", __func__, focusRange->min, focusRange->max);
}

int AiqCore::runAwb(ia_aiq_awb_results* awbResults) {
    LOG3A("@%s, awbResults:%p", __func__, awbResults);
    CheckError(!awbResults, BAD_VALUE, "@%s, awbResults is nullptr", __func__);
    PERF_CAMERA_ATRACE();

    int ret = OK;
    ia_aiq_awb_results *newAwbResults = mLastAwbResult;

    if (!mAwbForceLock && (mAwbRunTime % mIntel3AParameter->mAwbPerTicks == 0)) {
        IntelAiq* intelAiq = mIntelAiqHandle[mTuningMode];
        CheckError(!intelAiq, UNKNOWN_ERROR, "%s, aiq is nullptr, mode:%d", __func__, mTuningMode);
        {
            PERF_CAMERA_ATRACE_PARAM1_IMAGING("intelAiq->awbRun", 1);
            ia_err iaErr = intelAiq->awbRun(&mIntel3AParameter->mAwbParams, &newAwbResults);
            ret = AiqUtils::convertError(iaErr);
            CheckError(ret != OK || !newAwbResults, ret, "Error running AWB, ret: %d", ret);
        }
    }

    CheckError(!newAwbResults, BAD_VALUE, "newAwbResults is nullptr");

    if (!PlatformData::isIsysEnabled(mCameraId)) {
        // Fix AWB gain to 1 for none-ISYS cases
        newAwbResults->accurate_r_per_g = 1;
        newAwbResults->accurate_b_per_g = 1;
        newAwbResults->final_r_per_g = 1;
        newAwbResults->final_b_per_g = 1;
    }

    mIntel3AParameter->updateAwbResult(newAwbResults);

    ret = AiqUtils::deepCopyAwbResults(*newAwbResults, awbResults);

    mLastAwbResult = awbResults;
    ++mAwbRunTime;

    return ret;
}

int AiqCore::runGbce(ia_aiq_gbce_results *gbceResults) {
    LOG3A("%s, gbceResults:%p", __func__, gbceResults);
    CheckError(!gbceResults, BAD_VALUE, "@%s, gbceResults is nullptr", __func__);

    // Don't run gbce if AE lock and ev shift isn't changed
    if (mAeForceLock && mGbceParams.ev_shift == mLastEvShift) return OK;

    PERF_CAMERA_ATRACE();
    ia_aiq_gbce_results *newGbceResults = nullptr;

    IntelAiq* intelAiq = mIntelAiqHandle[mTuningMode];
    CheckError(!intelAiq, UNKNOWN_ERROR, "%s, aiq is nullptr, mode:%d", __func__, mTuningMode);
    {
        PERF_CAMERA_ATRACE_PARAM1_IMAGING("intelAiq->gbceRun", 1);
        ia_err iaErr = intelAiq->gbceRun(&mGbceParams, &newGbceResults);
        int ret = AiqUtils::convertError(iaErr);
        CheckError(ret != OK || !newGbceResults, ret, "@%s, gbceRun fails, ret: %d",
                   __func__, ret);
    }

    return AiqUtils::deepCopyGbceResults(*newGbceResults, gbceResults);
}

int AiqCore::runPa(ia_aiq_pa_results_v1 *paResults,
                   ia_aiq_awb_results *awbResults,
                   ia_aiq_exposure_parameters *exposureParams,
                   ia_aiq_advanced_ccm_t *preferredAcm) {
    LOG3A("%s, paResults:%p, awbResults:%p, exposureParams:%p, preferredAcm:%p", __func__,
          paResults, awbResults, exposureParams, preferredAcm);
    CheckError(!paResults || !awbResults || !exposureParams || !preferredAcm, BAD_VALUE,
               "@%s, Bad input values", __func__);

    PERF_CAMERA_ATRACE();
    ia_aiq_pa_results_v1 *newPaResults = nullptr;

    mPaParams.awb_results = awbResults;
    mPaParams.exposure_params = exposureParams;
    mPaParams.color_gains = nullptr;

    IntelAiq* intelAiq = mIntelAiqHandle[mTuningMode];
    CheckError(!intelAiq, UNKNOWN_ERROR, "%s, aiq is nullptr, mode:%d", __func__, mTuningMode);
    {
        PERF_CAMERA_ATRACE_PARAM1_IMAGING("intelAiq->paRunV1", 1);
        ia_err iaErr = intelAiq->paRunV1(&mPaParams, &newPaResults);
        int ret = AiqUtils::convertError(iaErr);
        CheckError(ret != OK || !newPaResults, ret, "Error running PA, ret: %d", ret);
    }

    mIntel3AParameter->updatePaResult(newPaResults);
    dumpPaResult(newPaResults);

    return AiqUtils::deepCopyPaResults(*newPaResults, paResults, preferredAcm);
}

int AiqCore::checkColorOrder(cmc_bayer_order bayerOrder, ColorOrder *colorOrder) {
    LOG3A("@%s, bayerOrder = %d, colorOrder:%p", __func__, bayerOrder, colorOrder);
    CheckError(!colorOrder, BAD_VALUE, "@%s, colorOrder is nullptr", __func__);

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

int AiqCore::reFormatLensShadingMap(const LSCGrid &inputLscGrid, float *dstLscGridRGGB) {
    LOG3A("@%s, width %d, height %d", __func__, inputLscGrid.width, inputLscGrid.height);

    CheckError(inputLscGrid.isBad() || !dstLscGridRGGB, BAD_VALUE,
               "@%s, Bad input values for lens shading map reformatting", __func__);

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

int AiqCore::storeLensShadingMap(const LSCGrid &inputLscGrid,
                                 const LSCGrid &resizeLscGrid, float *dstLscGridRGGB) {
    LOG3A("@%s", __func__);
    CheckError(inputLscGrid.isBad() || resizeLscGrid.isBad() || !dstLscGridRGGB, BAD_VALUE,
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

        AiqUtils::resize2dArray(inputLscGrid.gridR,  width, height,
                      resizeLscGrid.gridR,  destWidth, destHeight);
        AiqUtils::resize2dArray(inputLscGrid.gridGr,  width, height,
                      resizeLscGrid.gridGr,  destWidth, destHeight);
        AiqUtils::resize2dArray(inputLscGrid.gridGb,  width, height,
                      resizeLscGrid.gridGb,  destWidth, destHeight);
        AiqUtils::resize2dArray(inputLscGrid.gridB,  width, height,
                      resizeLscGrid.gridB,  destWidth, destHeight);

        LOG3A("resize lens shading map from [%d,%d] to [%d,%d]",
              width, height, destWidth, destHeight);
    } else {
        size_t size = destWidth * destHeight * sizeof(resizeLscGrid.gridR[0]);
        STDCOPY((int8_t *) resizeLscGrid.gridR,  (int8_t *) inputLscGrid.gridR,  size);
        STDCOPY((int8_t *) resizeLscGrid.gridGr, (int8_t *) inputLscGrid.gridGr, size);
        STDCOPY((int8_t *) resizeLscGrid.gridGb, (int8_t *) inputLscGrid.gridGb, size);
        STDCOPY((int8_t *) resizeLscGrid.gridB,  (int8_t *) inputLscGrid.gridB,  size);
    }

    return reFormatLensShadingMap(resizeLscGrid, dstLscGridRGGB);
}

int AiqCore::processSAResults(ia_aiq_sa_results_v1 *saResults, float *lensShadingMap) {
    LOG3A("@%s, saResults:%p, lensShadingMap:%p", __func__, saResults, lensShadingMap);
    CheckError(!saResults || !lensShadingMap, BAD_VALUE, "@%s, Bad input values", __func__);

    if (mLensShadingMapMode == LENS_SHADING_MAP_MODE_OFF) return OK;

    if (saResults->lsc_update) {
        ColorOrder co_ind = {};
        int ret = checkColorOrder(saResults->color_order, &co_ind);
        CheckError(ret != OK, BAD_VALUE, "Failed to checkColorOrder, ret: %d", ret);

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
            LOGW("Error - SA produced too small values (%d/%d)!", errCount, mLscGridRGGBLen);
        }
    }

    float *lsm = (mShadingMode != SHADING_MODE_OFF) ? mLscGridRGGB : mLscOffGrid;
    for (size_t i = 0; i < mLscGridRGGBLen; i++) {
        lensShadingMap[i] = lsm[i];
    }

    return OK;
}

int AiqCore::runSa(ia_aiq_sa_results_v1 *saResults,
                   ia_aiq_awb_results *awbResults,
                   float *lensShadingMap) {
    LOG3A("%s, saResults:%p, awbResults:%p, lensShadingMap:%p", __func__,
          saResults, awbResults, lensShadingMap);
    CheckError(!saResults || !awbResults || !lensShadingMap, BAD_VALUE,
               "@%s, Bad input values", __func__);

    if (mShadingMode == SHADING_MODE_OFF) return OK;

    PERF_CAMERA_ATRACE();
    int ret = OK;
    ia_aiq_sa_results_v1 *newSaResults = nullptr;
    mSaParams.awb_results = awbResults;

    IntelAiq* intelAiq = mIntelAiqHandle[mTuningMode];
    CheckError(!intelAiq, UNKNOWN_ERROR, "%s, aiq is nullptr, mode:%d", __func__, mTuningMode);
    {
        PERF_CAMERA_ATRACE_PARAM1_IMAGING("intelAiq->saRunV2", 1);
        ia_err iaErr = intelAiq->saRunV2(&mSaParams, &newSaResults);
        ret = AiqUtils::convertError(iaErr);
        CheckError(ret != OK || !newSaResults, ret, "intelAiq->saRunV2 fails, ret: %d", ret);
    }

    dumpSaResult(newSaResults);
    ret = AiqUtils::deepCopySaResults(*newSaResults, saResults);
    CheckError(ret != OK, ret, "Error deepCopySaResults, ret: %d", ret);

    return processSAResults(saResults, lensShadingMap);
}

int AiqCore::dumpPaResult(const ia_aiq_pa_results_v1 *paResult) {
    LOG3A("%s, paResult:%p", __func__, paResult);
    CheckError(!paResult, BAD_VALUE, "@%s, paResult is nullptr", __func__);

    if (!Log::isDebugLevelEnable(CAMERA_DEBUG_LOG_AIQ)) return OK;

    LOG3A("   PA results brightness %f saturation %f",
          paResult->brightness_level,
          paResult->saturation_factor);
    LOG3A("   PA results black level row 0: %f %f %f  %f ",
          paResult->black_level_4x4[0][0],
          paResult->black_level_4x4[0][1],
          paResult->black_level_4x4[0][2],
          paResult->black_level_4x4[0][3]);
    LOG3A("   PA results black level row 1: %f %f %f  %f ",
          paResult->black_level_4x4[1][0],
          paResult->black_level_4x4[1][1],
          paResult->black_level_4x4[1][2],
          paResult->black_level_4x4[1][3]);
    LOG3A("   PA results black level row 2: %f %f %f  %f ",
          paResult->black_level_4x4[2][0],
          paResult->black_level_4x4[2][1],
          paResult->black_level_4x4[2][2],
          paResult->black_level_4x4[2][3]);
    LOG3A("   PA results black level row 3: %f %f %f  %f ",
          paResult->black_level_4x4[3][0],
          paResult->black_level_4x4[3][1],
          paResult->black_level_4x4[3][2],
          paResult->black_level_4x4[3][3]);
    LOG3A("   PA results color gains %f %f %f  %f ",
          paResult->color_gains.r,
          paResult->color_gains.gr,
          paResult->color_gains.gb,
          paResult->color_gains.b);
    LOG3A("   PA results linearization table size %d",
          paResult->linearization.size);

    for (int i = 0; i < 3; i++) {
        LOG3A("   PA results color matrix  [%.3f %.3f %.3f] ",
              paResult->color_conversion_matrix[i][0],
              paResult->color_conversion_matrix[i][1],
              paResult->color_conversion_matrix[i][2]);
    }

    if (paResult->preferred_acm) {
        LOG3A("   PA results advanced ccm sector count %d ",
              paResult->preferred_acm->sector_count);
    }
    if (paResult->ir_weight) {
        LOG3A("   PA results ir weight grid [ %d x %d ] ",
              paResult->ir_weight->width, paResult->ir_weight->height);
    }

    return OK;
}

int AiqCore::dumpSaResult(const ia_aiq_sa_results_v1 *saResult) {
    LOG3A("%s, saResult:%p", __func__, saResult);
    CheckError(!saResult, BAD_VALUE, "@%s, saResult is nullptr", __func__);

    if (!Log::isDebugLevelEnable(CAMERA_DEBUG_LOG_AIQ)) return OK;

    LOG3A("   SA results lsc Update %d size %dx%d",
          saResult->lsc_update, saResult->width,  saResult->height);

    return OK;
}
} /* namespace icamera */
