/*
 * Copyright (C) 2015-2024 Intel Corporation.
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

#define LOG_TAG AiqEngine

#include "AiqEngine.h"

#include <memory>

#include "FaceDetection.h"
#include "PlatformData.h"
#include "iutils/CameraLog.h"
#include "iutils/Errors.h"
#include "iutils/Utils.h"

#include "gc/IGraphConfigManager.h"

namespace icamera {

AiqEngine::AiqEngine(int cameraId, SensorHwCtrl* sensorHw, LensHw* lensHw, AiqSetting* setting)
        : mCameraId(cameraId),
          mAiqSetting(setting),
          mRun3ACadence(1),
          mFirstAiqRunning(true) {
    LOG1("<id%d>%s", mCameraId, __func__);

    mAiqRunningForPerframe = PlatformData::isFeatureSupported(mCameraId, PER_FRAME_CONTROL);
    mAiqCore = new AiqCore(mCameraId);
    mSensorManager = new SensorManager(mCameraId, sensorHw);
    mLensManager = new LensManager(mCameraId, lensHw);

    // Should consider better place to maintain the life cycle of AiqResultStorage
    mAiqResultStorage = AiqResultStorage::getInstance(mCameraId);

    CLEAR(mAiqRunningHistory);
}

AiqEngine::~AiqEngine() {
    LOG1("<id%d>%s", mCameraId, __func__);

    delete mLensManager;
    delete mSensorManager;
    delete mAiqCore;

    AiqResultStorage::releaseAiqResultStorage(mCameraId);
}

int AiqEngine::init() {
    LOG1("<id%d>%s", mCameraId, __func__);

    AutoMutex l(mEngineLock);
    if (mAiqCore->init() != OK) {
        return UNKNOWN_ERROR;
    }

    mSensorManager->reset();

    return OK;
}

int AiqEngine::deinit() {
    LOG1("<id%d>%s", mCameraId, __func__);

    AutoMutex l(mEngineLock);
    mSensorManager->reset();
    mAiqCore->deinit();

    return OK;
}

int AiqEngine::configure() {
    AutoMutex l(mEngineLock);
    mAiqCore->configure();

    return OK;
}

int AiqEngine::startEngine() {
    LOG1("<id%d>%s", mCameraId, __func__);

    AutoMutex l(mEngineLock);
    mFirstAiqRunning = true;
    mAiqResultStorage->resetAiqStatistics();
    mSensorManager->reset();
    mLensManager->start();

    return OK;
}

int AiqEngine::stopEngine() {
    LOG1("<id%d>%s", mCameraId, __func__);

    AutoMutex l(mEngineLock);
    mLensManager->stop();

    return OK;
}

int AiqEngine::run3A(long requestId, int64_t applyingSeq, int64_t* effectSeq) {
    LOG2("<id%d:req%ld>%s: applying seq %ld", mCameraId, requestId, __func__, applyingSeq);

    // Run 3A in call thread
    AutoMutex l(mEngineLock);

    AiqStatistics* aiqStats =
        mFirstAiqRunning ? nullptr :
                           const_cast<AiqStatistics*>(mAiqResultStorage->getAndLockAiqStatistics());
    AiqState state = AIQ_STATE_IDLE;
    AiqResult* aiqResult = mAiqResultStorage->acquireAiqResult();

    if (!needRun3A(aiqStats, requestId)) {
        LOG2("%s: needRun3A is false, return AIQ_STATE_WAIT", __func__);
        state = AIQ_STATE_WAIT;
    } else {
        state = prepareInputParam(aiqStats, aiqResult);
        aiqResult->mTuningMode = aiqResult->mAiqParam.tuningMode;
    }

    bool aiqRun = false;
    if (state == AIQ_STATE_RUN) {
        state = runAiq(requestId, applyingSeq, aiqResult, &aiqRun);
    }
    if (state == AIQ_STATE_RESULT_SET) {
        state = handleAiqResult(aiqResult);
    }

    if (state == AIQ_STATE_DONE) {
        done(aiqResult);
    }

    mAiqResultStorage->unLockAiqStatistics();

    if (aiqRun) {
        mAiqRunningHistory.aiqResult = aiqResult;
        mAiqRunningHistory.requestId = requestId;
        mAiqRunningHistory.statsSequnce = aiqStats ? aiqStats->mSequence : -1;
    }

    if (effectSeq) {
        *effectSeq = mAiqResultStorage->getAiqResult()->mSequence;
        LOG2("%s, effect sequence %ld, statsSequnce %ld", __func__, *effectSeq,
             mAiqRunningHistory.statsSequnce);
    }

    PlatformData::saveMakernoteData(mCameraId, aiqResult->mAiqParam.makernoteMode,
                                    mAiqResultStorage->getAiqResult()->mSequence,
                                    aiqResult->mTuningMode);

    return (state == AIQ_STATE_DONE || state == AIQ_STATE_WAIT) ? 0 : UNKNOWN_ERROR;
}

EventListener* AiqEngine::getSofEventListener() {
    AutoMutex l(mEngineLock);
    return this;
}

void AiqEngine::handleEvent(EventData eventData) {
    AutoMutex l(mEngineLock);
    mSensorManager->handleSofEvent(eventData);
    mLensManager->handleSofEvent(eventData);
}

int AiqEngine::prepareStatsParams(cca::cca_stats_params* statsParams, AiqStatistics* aiqStatistics,
                                  AiqResult* aiqResult) {
    LOG2("%s, sequence %ld", __func__, aiqStatistics->mSequence);

    // update face detection related parameters
    if (PlatformData::isFaceAeEnabled(mCameraId)) {
        cca::cca_face_state* faceState = &statsParams->faces;
        int ret = icamera::FaceDetection::getResult(mCameraId, faceState);
        if (ret == OK && faceState->num_faces > 0) {
            ia_rectangle rect = faceState->faces[0].face_area;
            LOG2("@%s, face number:%d, left:%d, top:%d, right:%d, bottom:%d", __func__,
                 faceState->num_faces, rect.left, rect.top, rect.right, rect.bottom);
        }
    }

    int ret = OK;
    do {
        // HDR_FEATURE_S
        // Run 3A without statistics when switching pipe.
        if (aiqResult->mAiqParam.tuningMode == TUNING_MODE_VIDEO_ULL &&
            aiqStatistics->mTuningMode == TUNING_MODE_VIDEO_HDR) {
            LOG2("Switching from HDR to ULL pipe");
            ret = INVALID_OPERATION;
            break;
        } else if (aiqResult->mAiqParam.tuningMode == TUNING_MODE_VIDEO_HDR &&
                   aiqStatistics->mTuningMode == TUNING_MODE_VIDEO_ULL) {
            LOG2("Switching from ULL to HDR pipe");
            ret = INVALID_OPERATION;
            break;
        }
        // HDR_FEATURE_E

        // The statistics timestamp is incorrect. If possible, use SOF timestamp instead.
        unsigned long long timestamp = mSensorManager->getSofTimestamp(aiqStatistics->mSequence);
        if (timestamp == 0) {
            LOG2("<seq%ld>The sof sequence was not found", aiqStatistics->mSequence);
            timestamp = aiqStatistics->mTimestamp;
        }

        const AiqResult* aiqResult = mAiqResultStorage->getAiqResult(aiqStatistics->mSequence);

        if (PlatformData::isDvsSupported(mCameraId) &&
            PlatformData::getGraphConfigNodes(mCameraId)) {
            std::shared_ptr<IGraphConfig> gc = nullptr;
            IGraphConfigManager* GCM = IGraphConfigManager::getInstance(mCameraId);
            if (GCM) {
                gc = GCM->getGraphConfig(CAMERA_STREAM_CONFIGURATION_MODE_NORMAL);
            }

            CheckAndLogError(!gc, UNKNOWN_ERROR, "%s, Failed to get graph config", __func__);
            ia_isp_bxt_resolution_info_t resolution;
            uint32_t kernelId;
            int status = gc->getGdcKernelSetting(&kernelId, &resolution, aiqStatistics->mStreamId);
            CheckWarning(status != OK, UNKNOWN_ERROR, "Failed to get GDC kernel setting");

            statsParams->dvs_stats_height = resolution.output_height;
            statsParams->dvs_stats_width = resolution.output_width;
        }

        statsParams->frame_id = aiqResult ? aiqResult->mFrameId : -1;
        statsParams->frame_timestamp = timestamp;
        statsParams->camera_orientation = ia_aiq_camera_orientation_unknown;
    } while (0);

    return ret;
}

void AiqEngine::setAiqResult(AiqResult* aiqResult, bool skip) {
    aiqResult->mSkip = skip;
    if (skip) {
        LOG2("<seq%ld>%s, skipping the frame", aiqResult->mSequence, __func__);
    }

    // HDR_FEATURE_S
    int64_t sequence = aiqResult->mSequence - PlatformData::getExposureLag(mCameraId);
    mSensorManager->setWdrMode(aiqResult->mTuningMode, sequence);

    if (PlatformData::getSensorAwbEnable(mCameraId)) {
        LOG2("%s, Set sensor awb %f %f", __func__, aiqResult->mAwbResults.accurate_r_per_g,
             aiqResult->mAwbResults.accurate_b_per_g);

        mSensorManager->setAWB(aiqResult->mAwbResults.accurate_r_per_g,
                               aiqResult->mAwbResults.accurate_b_per_g);
    }
    // HDR_FEATURE_E

    mLensManager->setLensResult(aiqResult->mAfResults, aiqResult->mSequence, aiqResult->mAiqParam);
}

int AiqEngine::getSkippingNum(AiqResult* aiqResult) {
    int skipNum = 0;

    if (!mFirstAiqRunning) {
        const AiqResult* lastResult = mAiqResultStorage->getAiqResult();
        if (lastResult->mTuningMode != aiqResult->mTuningMode) {
            // Skip 3 frames when pipe switching
            skipNum = 3;
        }
    }

    return skipNum;
}

bool AiqEngine::needRun3A(AiqStatistics* aiqStatistics, long requestId) {
    // Force to run 3a for per-frame control case
    if (mAiqRunningForPerframe) {
        return true;
    }

    // Force to run 3a for the first time running
    if (mFirstAiqRunning) {
        return true;
    }

    if (requestId % mRun3ACadence != 0) {
        // Skip 3A per cadence
        return false;
    }

    if (aiqStatistics == nullptr) {
        LOG2("no stats and not need to re-run 3A");
        return false;
    }

    if (mAiqRunningHistory.statsSequnce == aiqStatistics->mSequence) {
        LOG2("no new stats skip, statsSequnce = %ld", aiqStatistics->mSequence);
        return false;
    } else if (mSensorManager->getCurrentExposureAppliedDelay() > kMaxExposureAppliedDelay) {
        LOG2("exposure setting applied delay is too larger, skip it");
        return false;
    }

    return true;
}

AiqEngine::AiqState AiqEngine::prepareInputParam(AiqStatistics* aiqStats, AiqResult* aiqResult) {
    // set Aiq Params
    int ret = mAiqSetting->getAiqParameter(aiqResult->mAiqParam);
    if (ret != OK) return AIQ_STATE_ERROR;

    mRun3ACadence = aiqResult->mAiqParam.run3ACadence;

    // Update sensor info for the first-run of AIQ
    if (mFirstAiqRunning) {
        // CRL_MODULE_S
        mSensorManager->setFrameRate(aiqResult->mAiqParam.fps);
        // CRL_MODULE_E
        // set sensor info if needed
        ia_aiq_exposure_sensor_descriptor sensorDescriptor = {};
        ia_aiq_frame_params frameParams = {};
        ret = mSensorManager->getSensorInfo(frameParams, sensorDescriptor);
        CheckAndLogError((ret != OK), AIQ_STATE_ERROR, "Get sensor info failed:%d", ret);
        mAiqCore->setSensorInfo(frameParams, sensorDescriptor);
    }

    // update lens related parameters
    mLensManager->getLensInfo(aiqResult->mAiqParam);

    ret = mAiqCore->updateParameter(aiqResult->mAiqParam);
    if (ret != OK) return AIQ_STATE_ERROR;

    if (aiqStats == nullptr) {
        LOG2("%s: run aiq without stats data", __func__);
        return AIQ_STATE_RUN;
    }

    // set Stats
    cca::cca_stats_params statsParams = {};
    ret = prepareStatsParams(&statsParams, aiqStats, aiqResult);
    if (ret != OK) {
        LOG2("%s: no useful stats", __func__);
        return AIQ_STATE_RUN;
    }

    if (PlatformData::getSensorAeEnable(mCameraId)) {
        LOG2("@%s, sensor ae is enabled", __func__);
        statsParams.using_rgbs_for_aec = true;
    }

    mAiqCore->setStatsParams(statsParams, aiqStats);

    return AIQ_STATE_RUN;
}

AiqEngine::AiqState AiqEngine::runAiq(long requestId, int64_t applyingSeq, AiqResult* aiqResult,
                                      bool* aiqRun) {
    if ((requestId % PlatformData::getAiqRunningInterval(mCameraId) == 0) || mFirstAiqRunning) {
        int ret = mAiqCore->runAe(requestId, aiqResult);
        if (ret != OK) {
            return AIQ_STATE_ERROR;
        }

        // PRIVACY_MODE_S
        if (PlatformData::getSupportPrivacy(mCameraId) == AE_BASED_PRIVACY_MODE) {
            uint32_t outMaxBin = 0;
            ret = mAiqCore->getBrightestIndex(outMaxBin);
            if (ret == OK) {
                EventData3AReady data;
                data.sequence = requestId;
                data.maxBin = outMaxBin;
                EventData eventData;
                eventData.type = EVENT_3A_READY;
                eventData.buffer = nullptr;
                eventData.data.run3AReady = data;
                notifyListeners(eventData);
            }
        }
        // PRIVACY_MODE_E

        setSensorExposure(aiqResult, applyingSeq);

        ret = mAiqCore->runAiq(requestId, aiqResult);
        if (ret != OK) {
            return AIQ_STATE_ERROR;
        }
        *aiqRun = true;
        aiqResult->mFrameId = requestId;
    } else {
        *aiqResult = *(mAiqRunningHistory.aiqResult);
        setSensorExposure(aiqResult, applyingSeq);
    }

    return AIQ_STATE_RESULT_SET;
}

void AiqEngine::setSensorExposure(AiqResult* aiqResult, int64_t applyingSeq) {
    SensorExpGroup sensorExposures;
    for (unsigned int i = 0; i < aiqResult->mAeResults.num_exposures; i++) {
        SensorExposure exposure;
        exposure.sensorParam = *aiqResult->mAeResults.exposures[i].sensor_exposure;
        exposure.realDigitalGain = aiqResult->mAeResults.exposures[i].exposure[0].digital_gain;
        sensorExposures.push_back(exposure);
    }
    aiqResult->mSequence = mSensorManager->updateSensorExposure(sensorExposures, applyingSeq);
}

AiqEngine::AiqState AiqEngine::handleAiqResult(AiqResult* aiqResult) {
    LOG2("%s: aiqResult->mTuningMode = %d", __func__, aiqResult->mTuningMode);

    // HDR_FEATURE_S
    aec_scene_t aecScene = (aiqResult->mAeResults.multiframe == ia_aiq_bracket_mode_ull) ?
                               AEC_SCENE_ULL :
                               AEC_SCENE_HDR;
    mAiqSetting->updateTuningMode(aecScene);
    // HDR_FEATURE_E

    aiqResult->mSceneMode = SCENE_MODE_AUTO;
    /* Use direct AE result to update sceneMode to reflect the actual mode AE want to have,
     * Besides needed by full pipe auto-switch, this is also necessary when user want to
     * switch pipe in user app according to AE result.
     */
    if (aiqResult->mAiqParam.sceneMode == SCENE_MODE_AUTO) {
        if (aiqResult->mAeResults.multiframe == ia_aiq_bracket_mode_hdr) {
            aiqResult->mSceneMode = SCENE_MODE_HDR;
        } else if (aiqResult->mAeResults.multiframe == ia_aiq_bracket_mode_ull) {
            aiqResult->mSceneMode = SCENE_MODE_ULL;
        }
    }
    LOG2("%s, sceneMode:%d", __func__, aiqResult->mSceneMode);

    applyManualTonemaps(aiqResult);

    return AIQ_STATE_DONE;
}

int AiqEngine::applyManualTonemaps(AiqResult* aiqResult) {
    aiqResult->mGbceResults.have_manual_settings = true;

    // Due to the tone map curve effect on image IQ, so need to apply
    // manual/fixed tone map table in manual tonemap or manual ISO/ET mode
    if (aiqResult->mAiqParam.tonemapMode == TONEMAP_MODE_FAST ||
        aiqResult->mAiqParam.tonemapMode == TONEMAP_MODE_HIGH_QUALITY) {
        aiqResult->mGbceResults.have_manual_settings = false;

        if (aiqResult->mAiqParam.aeMode != AE_MODE_AUTO && aiqResult->mAiqParam.manualIso != 0 &&
            aiqResult->mAiqParam.manualExpTimeUs != 0) {
            aiqResult->mGbceResults.have_manual_settings = true;
        }
    }
    LOG2("%s, has manual setting: %d, aeMode: %d, tonemapMode: %d", __func__,
         aiqResult->mGbceResults.have_manual_settings, aiqResult->mAiqParam.aeMode,
         aiqResult->mAiqParam.tonemapMode);

    if (!aiqResult->mGbceResults.have_manual_settings) return OK;

    // Apply user value or gamma curve for gamma table
    if (aiqResult->mAiqParam.tonemapMode == TONEMAP_MODE_GAMMA_VALUE) {
        AiqUtils::applyTonemapGamma(aiqResult->mAiqParam.tonemapGamma, &aiqResult->mGbceResults);
    } else if (aiqResult->mAiqParam.tonemapMode == TONEMAP_MODE_PRESET_CURVE) {
        if (aiqResult->mAiqParam.tonemapPresetCurve == TONEMAP_PRESET_CURVE_SRGB) {
            AiqUtils::applyTonemapSRGB(&aiqResult->mGbceResults);
        } else if (aiqResult->mAiqParam.tonemapPresetCurve == TONEMAP_PRESET_CURVE_REC709) {
            AiqUtils::applyTonemapREC709(&aiqResult->mGbceResults);
        }
    } else if (aiqResult->mAiqParam.tonemapMode == TONEMAP_MODE_CONTRAST_CURVE) {
        AiqUtils::applyTonemapCurve(aiqResult->mAiqParam.tonemapCurves, &aiqResult->mGbceResults);
        AiqUtils::applyAwbGainForTonemapCurve(aiqResult->mAiqParam.tonemapCurves,
                                              &aiqResult->mAwbResults);
    }

    // Apply the fixed unity value for tone map table
    if (aiqResult->mGbceResults.tone_map_lut_size > 0) {
        for (unsigned int i = 0; i < aiqResult->mGbceResults.tone_map_lut_size; i++) {
            aiqResult->mGbceResults.tone_map_lut[i] = 1.0;
        }
    }

    return OK;
}

AiqEngine::AiqState AiqEngine::done(AiqResult* aiqResult) {
    int skipNum = getSkippingNum(aiqResult);
    AiqResult* tmp = aiqResult;

    for (int i = 0; i < skipNum; i++) {
        // Increase the sensor settings sequence id, so for any frame that
        // its sequence id is bigger than the user expected id will be discarded.
        setAiqResult(tmp, true);
        mAiqResultStorage->updateAiqResult(tmp->mSequence);
        tmp = mAiqResultStorage->acquireAiqResult();
        *tmp = *aiqResult;
        setSensorExposure(tmp);
    }

    setAiqResult(tmp, false);
    mAiqResultStorage->updateAiqResult(tmp->mSequence);

    mFirstAiqRunning = false;
    return AIQ_STATE_WAIT;
}

} /* namespace icamera */
