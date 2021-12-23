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

#define LOG_TAG ParameterGenerator

#include <math.h>

#include <memory>
#include <vector>

#include "AiqResultStorage.h"
#include "AiqUtils.h"
#include "MakerNote.h"
#include "ParameterHelper.h"
#include "iutils/CameraLog.h"
#include "ParameterGenerator.h"
#include "iutils/Errors.h"
#include "iutils/Utils.h"

namespace icamera {

#define CHECK_REQUEST_ID(id) \
    CheckAndLogError((id < 0), UNKNOWN_ERROR, "%s: error request id %ld!", __func__, id);
#define CHECK_SEQUENCE(id) \
    CheckAndLogError((id < 0), UNKNOWN_ERROR, "%s: error sequence %ld!", __func__, id);

ParameterGenerator::ParameterGenerator(int cameraId)
        : mCameraId(cameraId),
          mTonemapMaxCurvePoints(0) {
    reset();

    camera_info_t info;
    CLEAR(info);
    PlatformData::getCameraInfo(mCameraId, info);
    info.capability->getTonemapMaxCurvePoints(mTonemapMaxCurvePoints);
    if (mTonemapMaxCurvePoints > 0 && mTonemapMaxCurvePoints < MIN_TONEMAP_POINTS) {
        LOGW("%s: wrong tonemap points", __func__);
        mTonemapMaxCurvePoints = 0;
    }
    if (mTonemapMaxCurvePoints) {
        mTonemapCurveRed = std::unique_ptr<float[]>(new float[mTonemapMaxCurvePoints * 2]);
        mTonemapCurveBlue = std::unique_ptr<float[]>(new float[mTonemapMaxCurvePoints * 2]);
        mTonemapCurveGreen = std::unique_ptr<float[]>(new float[mTonemapMaxCurvePoints * 2]);

        // Initialize P_IN, P_OUT values [(P_IN, P_OUT), ..]
        for (int32_t i = 0; i < mTonemapMaxCurvePoints; i++) {
            float index = static_cast<float>(i);
            mTonemapCurveRed[i * 2] = index / (mTonemapMaxCurvePoints - 1);
            mTonemapCurveRed[i * 2 + 1] = index / (mTonemapMaxCurvePoints - 1);
            mTonemapCurveBlue[i * 2] = index / (mTonemapMaxCurvePoints - 1);
            mTonemapCurveBlue[i * 2 + 1] = index / (mTonemapMaxCurvePoints - 1);
            mTonemapCurveGreen[i * 2] = index / (mTonemapMaxCurvePoints - 1);
            mTonemapCurveGreen[i * 2 + 1] = index / (mTonemapMaxCurvePoints - 1);
        }
    }
}

ParameterGenerator::~ParameterGenerator() {}

int ParameterGenerator::reset() {
    LOG1("<id%d>%s", mCameraId, __func__);
    AutoMutex l(mParamsLock);
    mRequestParamMap.clear();

    return OK;
}

int ParameterGenerator::saveParameters(int64_t sequence, long requestId, const Parameters* param) {
    CHECK_REQUEST_ID(requestId);
    CHECK_SEQUENCE(sequence);

    AutoMutex l(mParamsLock);
    if (param) mLastParam = *param;

    LOG2("<req%ld:seq%ld>%s", requestId, sequence, __func__);
    std::shared_ptr<RequestParam> requestParam = nullptr;
    if (mRequestParamMap.size() < kStorageSize) {
        requestParam = std::make_shared<RequestParam>();
    } else {
        auto it = mRequestParamMap.begin();
        requestParam = it->second;
        mRequestParamMap.erase(it->first);
    }

    requestParam->requestId = requestId;
    requestParam->param = mLastParam;

    mRequestParamMap[sequence] = requestParam;
    return OK;
}

void ParameterGenerator::updateParameters(int64_t sequence, const Parameters* param) {
    CheckAndLogError(!param, VOID_VALUE, "The param is nullptr!");

    LOG2("<seq%ld>%s, sequence %ld", sequence, __func__);

    AutoMutex l(mParamsLock);
    std::shared_ptr<RequestParam> requestParam = nullptr;
    if (mRequestParamMap.find(sequence) != mRequestParamMap.end()) {
        requestParam = mRequestParamMap[sequence];
    } else {
        auto it = mRequestParamMap.begin();
        requestParam = it->second;
        mRequestParamMap.erase(it->first);
    }
    int32_t userRequestId = 0;
    int ret = param->getUserRequestId(userRequestId);
    if (ret == OK) {
        requestParam->param.setUserRequestId(userRequestId);
    }
    // Update Jpeg related settings
    int rotation = 0;
    ret = param->getJpegRotation(rotation);
    if (ret == OK) {
        requestParam->param.setJpegRotation(rotation);
    }
    uint8_t quality = 0;
    ret = param->getJpegQuality(&quality);
    if (ret == OK) {
        requestParam->param.setJpegQuality(quality);
    }
    int64_t timestamp = 0;
    ret = param->getJpegGpsTimeStamp(timestamp);
    if (ret == OK) {
        requestParam->param.setJpegGpsTimeStamp(timestamp);
    }
    double gpsCoordinates[3] = {0};
    int retLat = param->getJpegGpsLatitude(gpsCoordinates[0]);
    int retLon = param->getJpegGpsLongitude(gpsCoordinates[1]);
    int retAlt = param->getJpegGpsAltitude(gpsCoordinates[2]);
    if (retLat == OK && retLon == OK && retAlt == OK) {
        requestParam->param.setJpegGpsCoordinates(gpsCoordinates);
    }
    camera_resolution_t res;
    ret = param->getJpegThumbnailSize(res);
    if (ret == OK) {
        requestParam->param.setJpegThumbnailSize(res);
    }
    quality = 0;
    ret = param->getJpegThumbnailQuality(&quality);
    if (ret == OK) {
        requestParam->param.setJpegThumbnailQuality(quality);
    }

    camera_edge_mode_t edgeMode;
    ret = param->getEdgeMode(edgeMode);
    if (ret == OK) {
        requestParam->param.setEdgeMode(edgeMode);
    }

    camera_nr_mode_t nrMode;
    ret = param->getNrMode(nrMode);
    if (ret == OK) {
        requestParam->param.setNrMode(nrMode);
    }

    mRequestParamMap[sequence] = requestParam;
}

int ParameterGenerator::getParameters(int64_t sequence, Parameters* param, bool setting,
                                      bool result) {
    CheckAndLogError((param == nullptr), UNKNOWN_ERROR, "nullptr to get param!");

    if (setting) {
        AutoMutex l(mParamsLock);
        if (sequence < 0) {
            *param = mLastParam;
        } else if (mRequestParamMap.find(sequence) != mRequestParamMap.end()) {
            *param = mRequestParamMap[sequence]->param;
        } else {
            // Find nearest parameter
            bool found = false;
            for (auto it = mRequestParamMap.crbegin(); it != mRequestParamMap.crend(); ++it) {
                if (it->first <= sequence) {
                    *param = mRequestParamMap[it->first]->param;
                    found = true;
                    break;
                }
            }
            if (!found) LOGE("Can't find settings for seq %ld", sequence);
        }
    }

    if (result) {
        generateParametersL(sequence, param);
    }
    return OK;
}

int ParameterGenerator::getUserRequestId(int64_t sequence, int32_t& userRequestId) {
    CHECK_SEQUENCE(sequence);

    AutoMutex l(mParamsLock);
    if (mRequestParamMap.find(sequence) != mRequestParamMap.end()) {
        return mRequestParamMap[sequence]->param.getUserRequestId(userRequestId);
    }

    return UNKNOWN_ERROR;
}

int ParameterGenerator::getRequestId(int64_t sequence, long& requestId) {
    CHECK_SEQUENCE(sequence);

    AutoMutex l(mParamsLock);
    if (mRequestParamMap.find(sequence) != mRequestParamMap.end()) {
        return mRequestParamMap[sequence]->requestId;
    }

    LOGE("<seq%ld>Can't find requestId", sequence);
    return UNKNOWN_ERROR;
}

int ParameterGenerator::generateParametersL(int64_t sequence, Parameters* params) {
    if (PlatformData::isEnableAIQ(mCameraId)) {
        updateWithAiqResultsL(sequence, params);
        updateTonemapCurve(sequence, params);
    }
    return OK;
}

int ParameterGenerator::updateWithAiqResultsL(int64_t sequence, Parameters* params) {
    const AiqResult* aiqResult = AiqResultStorage::getInstance(mCameraId)->getAiqResult(sequence);
    CheckAndLogError((aiqResult == nullptr), UNKNOWN_ERROR,
                     "%s Aiq result of sequence %ld does not exist", __func__, sequence);

    // Update AE related parameters
    camera_ae_state_t aeState =
        aiqResult->mAeResults.exposures[0].converged ? AE_STATE_CONVERGED : AE_STATE_NOT_CONVERGED;
    params->setAeState(aeState);

    if (CameraUtils::isMultiExposureCase(mCameraId, aiqResult->mTuningMode) &&
        aiqResult->mAeResults.num_exposures > 1) {
        params->setExposureTime(aiqResult->mAeResults.exposures[1].exposure[0].exposure_time_us);
    } else {
        params->setExposureTime(aiqResult->mAeResults.exposures[0].exposure[0].exposure_time_us);
    }
    params->setSensitivityIso(aiqResult->mAeResults.exposures[0].exposure[0].iso);
    float fps = 1000000.0 / aiqResult->mFrameDuration;
    params->setFrameRate(fps);

    // Update AWB related parameters
    updateAwbGainsL(params, aiqResult->mAwbResults);
    camera_color_transform_t ccm;
    MEMCPY_S(ccm.color_transform, sizeof(ccm.color_transform),
             aiqResult->mPaResults.color_conversion_matrix,
             sizeof(aiqResult->mPaResults.color_conversion_matrix));
    params->setColorTransform(ccm);

    camera_color_gains_t colorGains;
    colorGains.color_gains_rggb[0] = aiqResult->mPaResults.color_gains.r;
    colorGains.color_gains_rggb[1] = aiqResult->mPaResults.color_gains.gr;
    colorGains.color_gains_rggb[2] = aiqResult->mPaResults.color_gains.gb;
    colorGains.color_gains_rggb[3] = aiqResult->mPaResults.color_gains.b;
    params->setColorGains(colorGains);

    camera_awb_state_t awbState = (fabs(aiqResult->mAwbResults.distance_from_convergence) < 0.001)
                                      ? AWB_STATE_CONVERGED
                                      : AWB_STATE_NOT_CONVERGED;
    params->setAwbState(awbState);

    // Update AF related parameters
    camera_af_state_t afState =
        (aiqResult->mAfResults.status == ia_aiq_af_status_local_search)
            ? AF_STATE_LOCAL_SEARCH
            : (aiqResult->mAfResults.status == ia_aiq_af_status_extended_search)
                  ? AF_STATE_EXTENDED_SEARCH
                  : ((aiqResult->mAfResults.status == ia_aiq_af_status_success) &&
                     aiqResult->mAfResults.final_lens_position_reached)
                        ? AF_STATE_SUCCESS
                        : (aiqResult->mAfResults.status == ia_aiq_af_status_fail) ? AF_STATE_FAIL
                                                                                  : AF_STATE_IDLE;
    params->setAfState(afState);

    bool lensMoving = false;
    camera_af_mode_t afMode = AF_MODE_OFF;
    params->getAfMode(afMode);
    if (afState == AF_STATE_LOCAL_SEARCH || afState == AF_STATE_EXTENDED_SEARCH) {
        lensMoving = (aiqResult->mAfResults.final_lens_position_reached == false);
    } else if (afState == AF_STATE_SUCCESS && afMode == AF_MODE_OFF) {
        /* In manual focus mode, AF_STATE_SUCCESS is set immediately after running algo,
         * but lens is moving and will stop moving in next frame.
         */
        lensMoving = (aiqResult->mLensPosition != aiqResult->mAfResults.next_lens_position);
    }
    params->setLensState(lensMoving);
    params->setFocusDistance(aiqResult->mAfDistanceDiopters);
    params->setFocusRange(aiqResult->mFocusRange);

    // Update scene mode
    params->setSceneMode(aiqResult->mSceneMode);

    camera_lens_shading_map_mode_type_t lensShadingMapMode = LENS_SHADING_MAP_MODE_OFF;
    params->getLensShadingMapMode(lensShadingMapMode);
    if (lensShadingMapMode == LENS_SHADING_MAP_MODE_ON) {
        size_t size = aiqResult->mAiqParam.lensShadingMapSize.x *
                      aiqResult->mAiqParam.lensShadingMapSize.y * 4;
        params->setLensShadingMap(aiqResult->mLensShadingMap, size);
    }

    return updateCommonMetadata(params, aiqResult);
}

int ParameterGenerator::updateAwbGainsL(Parameters* params, const cca::cca_awb_results& result) {
    camera_awb_gains_t awbGains;
    CLEAR(awbGains);
    float normalizedR, normalizedG, normalizedB;

    if (params->getAwbGains(awbGains) == OK) {
        // User manual AWB gains
        awbGains.g_gain = CLIP(awbGains.g_gain, AWB_GAIN_MAX, AWB_GAIN_MIN);
        normalizedG = AiqUtils::normalizeAwbGain(awbGains.g_gain);
    } else {
        // non-manual AWB gains, try to find a proper G that makes R/G/B all in the gain range.
        normalizedG = sqrt((AWB_GAIN_NORMALIZED_START * AWB_GAIN_NORMALIZED_END) /
                           (result.accurate_r_per_g * result.accurate_b_per_g));
        awbGains.g_gain = AiqUtils::convertToUserAwbGain(normalizedG);
    }

    normalizedR = result.accurate_r_per_g * normalizedG;
    normalizedB = result.accurate_b_per_g * normalizedG;

    awbGains.r_gain = AiqUtils::convertToUserAwbGain(normalizedR);
    awbGains.b_gain = AiqUtils::convertToUserAwbGain(normalizedB);

    LOG2("awbGains [r, g, b] = [%d, %d, %d]", awbGains.r_gain, awbGains.g_gain, awbGains.b_gain);
    params->setAwbGains(awbGains);

    // Update the AWB result
    camera_awb_result_t awbResult;
    awbResult.r_per_g = result.accurate_r_per_g;
    awbResult.b_per_g = result.accurate_b_per_g;
    LOG2("awb result: %f, %f", awbResult.r_per_g, awbResult.b_per_g);
    params->setAwbResult(&awbResult);

    return OK;
}

int ParameterGenerator::updateTonemapCurve(int64_t sequence, Parameters* params) {
    if (!mTonemapMaxCurvePoints) return OK;

    const AiqResult* aiqResult = AiqResultStorage::getInstance(mCameraId)->getAiqResult(sequence);
    CheckAndLogError((aiqResult == nullptr), UNKNOWN_ERROR,
                     "%s Aiq result of sequence %ld does not exist", __func__, sequence);
    const cca::cca_gbce_params& gbceResults = aiqResult->mGbceResults;

    int multiplier = gbceResults.gamma_lut_size / mTonemapMaxCurvePoints;
    for (int32_t i = 0; i < mTonemapMaxCurvePoints; i++) {
        mTonemapCurveRed[i * 2 + 1] = gbceResults.r_gamma_lut[i * multiplier];
        mTonemapCurveBlue[i * 2 + 1] = gbceResults.g_gamma_lut[i * multiplier];
        mTonemapCurveGreen[i * 2 + 1] = gbceResults.b_gamma_lut[i * multiplier];
    }

    int count = mTonemapMaxCurvePoints * 2;
    camera_tonemap_curves_t curves = {count,
                                      count,
                                      count,
                                      mTonemapCurveRed.get(),
                                      mTonemapCurveBlue.get(),
                                      mTonemapCurveGreen.get()};
    params->setTonemapCurves(curves);
    return OK;
}

int ParameterGenerator::updateCommonMetadata(Parameters* params, const AiqResult* aiqResult) {
    icamera_metadata_ro_entry entry;
    CLEAR(entry);

    entry.tag = CAMERA_SENSOR_ROLLING_SHUTTER_SKEW;
    entry.type = ICAMERA_TYPE_INT64;
    entry.count = 1;
    entry.data.i64 = &aiqResult->mRollingShutter;
    ParameterHelper::mergeTag(entry, params);

    int64_t frameDuration = aiqResult->mFrameDuration * 1000;  // us -> ns
    entry.tag = CAMERA_SENSOR_FRAME_DURATION;
    entry.type = ICAMERA_TYPE_INT64;
    entry.count = 1;
    entry.data.i64 = &frameDuration;
    ParameterHelper::mergeTag(entry, params);

    if (aiqResult->mAiqParam.callbackRgbs) {
        int32_t width = aiqResult->mOutStats.rgbs_grid.grid_width;
        int32_t height = aiqResult->mOutStats.rgbs_grid.grid_height;
        int32_t gridSize[] = {width, height};
        entry.tag = INTEL_VENDOR_CAMERA_RGBS_GRID_SIZE;
        entry.type = ICAMERA_TYPE_INT32;
        entry.count = ARRAY_SIZE(gridSize);
        entry.data.i32 = gridSize;
        ParameterHelper::mergeTag(entry, params);

        uint8_t lscFlags = aiqResult->mOutStats.rgbs_grid.shading_correction;
        entry.tag = INTEL_VENDOR_CAMERA_SHADING_CORRECTION;
        entry.type = ICAMERA_TYPE_BYTE;
        entry.count = 1;
        entry.data.u8 = &lscFlags;
        ParameterHelper::mergeTag(entry, params);

        if (Log::isLogTagEnabled(ST_STATS)) {
            const cca::cca_out_stats *outStats = &aiqResult->mOutStats;
            const rgbs_grid_block *rgbsPtr = aiqResult->mOutStats.rgbs_blocks;
            int size = outStats->rgbs_grid.grid_width * outStats->rgbs_grid.grid_height;

            int sumLuma = 0;
            for (int j = 0; j < size; j++) {
                sumLuma += (rgbsPtr[j].avg_b + rgbsPtr[j].avg_r +
                            (rgbsPtr[j].avg_gb + rgbsPtr[j].avg_gr) / 2) / 3;
            }

            LOG2(ST_STATS, "RGB stat %dx%d, sequence %lld, y_mean %d",
                 outStats->rgbs_grid.grid_width, outStats->rgbs_grid.grid_height,
                 aiqResult->mSequence, size > 0 ? sumLuma/size : 0);
        }

        entry.tag = INTEL_VENDOR_CAMERA_RGBS_STATS_BLOCKS;
        entry.type = ICAMERA_TYPE_BYTE;
        entry.count = width * height * 5;
        entry.data.u8 = reinterpret_cast<const uint8_t*>(aiqResult->mOutStats.rgbs_blocks);
        ParameterHelper::mergeTag(entry, params);
    }

    int64_t range[] = { aiqResult->mAeResults.exposures[0].exposure[0].low_limit_total_exposure,
                        aiqResult->mAeResults.exposures[0].exposure[0].up_limit_total_exposure };
    LOG2("total et limits [%ldx%ld]", range[0], range[1]);
    entry.tag = INTEL_VENDOR_CAMERA_TOTAL_EXPOSURE_TARGET_RANGE;
    entry.type = ICAMERA_TYPE_INT64;
    entry.count = 2;
    entry.data.i64 = range;
    ParameterHelper::mergeTag(entry, params);

    if (aiqResult->mAiqParam.callbackTmCurve) {
        const cca::cca_gbce_params& gbceResults = aiqResult->mGbceResults;
        int multiplier = gbceResults.tone_map_lut_size / mTonemapMaxCurvePoints;

        std::vector<float> tmCurve(mTonemapMaxCurvePoints * 2);
        for (int32_t i = 0; i < mTonemapMaxCurvePoints; i++) {
            tmCurve[i * 2] = static_cast<float>(i) / (mTonemapMaxCurvePoints - 1);
            tmCurve[i * 2 + 1] = gbceResults.tone_map_lut[i * multiplier];
        }
        entry.tag = INTEL_VENDOR_CAMERA_TONE_MAP_CURVE;
        entry.type = ICAMERA_TYPE_FLOAT;
        entry.count = tmCurve.size();
        entry.data.f = tmCurve.data();
        ParameterHelper::mergeTag(entry, params);
    }

    return OK;
}

} /* namespace icamera */
