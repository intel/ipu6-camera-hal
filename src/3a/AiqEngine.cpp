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

#define LOG_TAG "AiqEngine"

#include "iutils/Utils.h"
#include "iutils/Errors.h"
#include "iutils/CameraLog.h"
#include "PlatformData.h"
#include "AiqEngine.h"
#include "FaceDetection.h"

namespace icamera {

AiqEngine::AiqEngine(int cameraId, SensorHwCtrl *sensorHw, LensHw *lensHw, AiqSetting *setting,
                     ParameterGenerator* paramGen) :
    mCameraId(cameraId),
    mAiqSetting(setting),
    mParamGen(paramGen),
    mFirstAiqRunning(true),
    mLastStatsSequence(-1)
{
    LOG1("%s, mCameraId = %d", __func__, mCameraId);

    mAiqRunningForPerframe = PlatformData::isFeatureSupported(mCameraId, PER_FRAME_CONTROL);
    mAiqCore = new AiqCore(mCameraId);
    mSensorManager = new SensorManager(mCameraId, sensorHw);
    mLensManager = new LensManager(mCameraId, lensHw);

    // Should consider better place to maintain the life cycle of AiqResultStorage
    mAiqResultStorage = AiqResultStorage::getInstance(mCameraId);
}

AiqEngine::~AiqEngine()
{
    LOG1("%s, mCameraId = %d", __func__, mCameraId);

    delete mLensManager;
    delete mSensorManager;
    delete mAiqCore;

    AiqResultStorage::releaseAiqResultStorage(mCameraId);
}

int AiqEngine::init()
{
    LOG1("%s, mCameraId = %d", __func__, mCameraId);

    AutoMutex l(mEngineLock);
    if (mAiqCore->init() != OK) {
        return UNKNOWN_ERROR;
    }

    mSensorManager->reset();

    return OK;
}

int AiqEngine::deinit()
{
    LOG1("%s, mCameraId = %d", __func__, mCameraId);

    AutoMutex l(mEngineLock);
    mSensorManager->reset();
    mAiqCore->deinit();

    return OK;
}

int AiqEngine::configure(const std::vector<ConfigMode>& configModes)
{
    LOG1("%s, mCameraId = %d", __func__, mCameraId);

    AutoMutex l(mEngineLock);
    mAiqCore->configure(configModes);

    return OK;
}

int AiqEngine::startEngine()
{
    LOG1("%s, mCameraId = %d", __func__, mCameraId);

    AutoMutex l(mEngineLock);
    mFirstAiqRunning = true;
    mSensorManager->reset();
    mLensManager->start();

    return OK;
}

int AiqEngine::stopEngine()
{
    LOG1("%s, mCameraId = %d", __func__, mCameraId);

    AutoMutex l(mEngineLock);
    mLensManager->stop();

    return OK;
}

int AiqEngine::run3A(long requestId, long applyingSeq, long* effectSeq)
{
    LOG3A("%s", __func__);

    // Run 3A in call thread
    AutoMutex l(mEngineLock);

    AiqStatistics *aiqStats =
        const_cast<AiqStatistics*>(mAiqResultStorage->getAndLockAiqStatistics());
    AiqResult *aiqResult = mAiqResultStorage->acquireAiqResult();
    AiqState state = AIQ_STATE_IDLE;

    if (!needRun3A(aiqStats, requestId)) {
        LOG3A("%s: needRun3A is false, return AIQ_STATE_WAIT", __func__);
        state = AIQ_STATE_WAIT;
    } else {
        state = prepareInputParam(aiqStats);
        aiqResult->mTuningMode = mAiqParam.tuningMode;
    }

    if (state == AIQ_STATE_RUN) {
        state = runAiq(requestId, aiqResult, applyingSeq);
    }
    if (state == AIQ_STATE_RESULT_SET) {
        state = handleAiqResult(aiqResult);
    }
    if (state == AIQ_STATE_DONE) {
        done(aiqResult);
    }

    mAiqResultStorage->unLockAiqStatistics();

    if (effectSeq) {
        *effectSeq = mAiqResultStorage->getAiqResult()->mSequence;
        LOG3A("%s, effect sequence %ld, mLastStatsSequence %ld", __func__, *effectSeq,
               mLastStatsSequence);
    }

    PlatformData::saveMakernoteData(mCameraId, mAiqParam.makernoteMode,
                                    mAiqResultStorage->getAiqResult()->mSequence,
                                    aiqResult->mTuningMode);

    return (state == AIQ_STATE_DONE || state == AIQ_STATE_WAIT) ? 0 : UNKNOWN_ERROR;
}

EventListener *AiqEngine::getSofEventListener()
{
    LOG1("%s, mCameraId = %d", __func__, mCameraId);

    AutoMutex l(mEngineLock);
    return mSensorManager->getSofEventListener();
}

int AiqEngine::prepareStatsParams(cca::cca_stats_params *statsParams, AiqStatistics *aiqStatistics)
{
    mLastStatsSequence = aiqStatistics->mSequence;
    LOG3A("%s, sequence %ld", __func__, aiqStatistics->mSequence);

    // update face detection related parameters
    if (PlatformData::isFaceAeEnabled(mCameraId)) {
        cca::cca_face_state* faceState = &statsParams->faces;
        int ret = icamera::FaceDetection::getResult(mCameraId, faceState);
        if (ret == OK && faceState->num_faces > 0) {
            ia_rectangle rect = faceState->faces[0].face_area;
            LOG3A("@%s, face number:%d, left:%d, top:%d, right:%d, bottom:%d", __func__,
                  faceState->num_faces, rect.left, rect.top, rect.right, rect.bottom);
        }
    }

    int ret = OK;
    do {

        // The statistics timestamp is incorrect. If possible, use SOF timestamp instead.
        unsigned long long timestamp = mSensorManager->getSofTimestamp(aiqStatistics->mSequence);
        if (timestamp == 0) {
            LOG2("The sof sequence was not found %ld", aiqStatistics->mSequence);
            timestamp = aiqStatistics->mTimestamp;
        }

        int64_t requestId = -1;
        mParamGen->getRequestId(aiqStatistics->mSequence, requestId);
        statsParams->frame_id = requestId;
        statsParams->frame_timestamp = timestamp;
        statsParams->camera_orientation = ia_aiq_camera_orientation_unknown;
    } while (0);

    return ret;
}

void AiqEngine::setAiqResult(AiqResult *aiqResult, bool skip)
{
    aiqResult->mSkip = skip;
    if (skip) {
        LOG3A("%s, skipping frame aiqResult->mSequence = %ld", __func__, aiqResult->mSequence);
    }

    mLensManager->setLensResult(aiqResult->mAeResults, aiqResult->mAfResults);
    aiqResult->mAiqParam = mAiqParam;
}

int AiqEngine::getSkippingNum(AiqResult *aiqResult)
{
    LOG3A("%s", __func__);
    int skipNum = 0;

    if (!mFirstAiqRunning) {
        const AiqResult *lastResult = mAiqResultStorage->getAiqResult();
        if (lastResult->mTuningMode != aiqResult->mTuningMode) {
            // Skip 3 frames when pipe switching
            skipNum = 3;
        }
    } else if (mAiqRunningForPerframe) {
        // The 1st result takes effect @ frame (initialSkip) (applied before stream on)
        skipNum = PlatformData::getInitialSkipFrame(mCameraId);
    }

    return skipNum;
}

bool AiqEngine::needRun3A(AiqStatistics *aiqStatistics, long requestId)
{
    LOG3A("%s", __func__);

    // Force to run 3a for per-frame control case
    if (mAiqRunningForPerframe) {
        return true;
    }

    // Force to run 3a for the first time running
    if (mFirstAiqRunning) {
        return true;
    }

    // Handle 3A cadence logic
    int run3ACadence = mAiqParam.run3ACadence;
    if (run3ACadence < 1) {
        LOGW("Invalid 3A cadence %d, use default 1.", run3ACadence);
        run3ACadence = 1;
    }
    LOG2("%s: run3ACadence is %d", __func__, run3ACadence);

    if (requestId % run3ACadence != 0) {
        // Skip 3A per cadence
        return false;
    }
    LOG2("%s: run 3A for requestId %ld", __func__, requestId);

    if (aiqStatistics == nullptr) {
        LOG3A("no stats and not need to re-run 3A");
        return false;
    }

    if (mLastStatsSequence == aiqStatistics->mSequence) {
        LOG3A("no new stats skip, mLastStatsSequence = %ld", mLastStatsSequence);
        return false;
    } else if (mSensorManager->getCurrentExposureAppliedDelay() > kMaxExposureAppliedDelay) {
        LOG3A("exposure setting applied delay is too larger, skip it");
        return false;
    }

    return true;
}

AiqEngine::AiqState AiqEngine::prepareInputParam(AiqStatistics* aiqStats)
{
    // set Aiq Params
    int ret = mAiqSetting->getAiqParameter(mAiqParam);
    if (ret != OK)
        return AIQ_STATE_ERROR;

    // Update sensor info for the first-run of AIQ
    if (mFirstAiqRunning) {
        // set sensor info if needed
        ia_aiq_exposure_sensor_descriptor sensorDescriptor = {};
        ia_aiq_frame_params frameParams = {};
        ret = mSensorManager->getSensorInfo(frameParams, sensorDescriptor);
        CheckError((ret != OK), AIQ_STATE_ERROR, "Get sensor info failed:%d", ret);
        mAiqCore->setSensorInfo(frameParams, sensorDescriptor);
    }

    // update lens related parameters
    mLensManager->getLensInfo(mAiqParam);

    mAiqCore->updateParameter(mAiqParam);

    if (aiqStats == nullptr) {
        LOG3A("%s: run aiq without stats data", __func__);
        return AIQ_STATE_RUN;
    }

    // set Stats
    cca::cca_stats_params statsParams = {};
    ret = prepareStatsParams(&statsParams, aiqStats);
    if (ret != OK) {
        LOG3A("%s: no useful stats", __func__);
        return AIQ_STATE_RUN;
    }

    mAiqCore->setStatsParams(statsParams);

    return AIQ_STATE_RUN;
}

AiqEngine::AiqState AiqEngine::runAiq(long requestId, AiqResult *aiqResult, long applyingSeq)
{
    int ret = mAiqCore->runAe(requestId, aiqResult);
    if (ret != OK) {
        return AIQ_STATE_ERROR;
    }
    setSensorExposure(aiqResult, applyingSeq);

    ret = mAiqCore->runAiq(requestId, aiqResult);
    if (ret != OK) {
        return AIQ_STATE_ERROR;
    }

    return AIQ_STATE_RESULT_SET;
}

void AiqEngine::setSensorExposure(AiqResult* aiqResult, long applyingSeq)
{
    SensorExpGroup sensorExposures;
    for (unsigned int i = 0; i < aiqResult->mAeResults.num_exposures; i++) {
        SensorExposure exposure;
        exposure.sensorParam = *aiqResult->mAeResults.exposures[i].sensor_exposure;
        exposure.realDigitalGain = aiqResult->mAeResults.exposures[i].exposure[0].digital_gain;
        sensorExposures.push_back(exposure);
    }
    aiqResult->mSequence = mSensorManager->updateSensorExposure(sensorExposures, applyingSeq);
}

AiqEngine::AiqState AiqEngine::handleAiqResult(AiqResult *aiqResult)
{
    LOG2("%s: aiqResult->mTuningMode = %d", __func__, aiqResult->mTuningMode);

    applyManualTonemaps(aiqResult);

    return AIQ_STATE_DONE;
}

int AiqEngine::applyManualTonemaps(AiqResult *aiqResult)
{
    /*
     * Normal use-case is the automatic modes, and we need not do anything here
     */
    if (mAiqParam.tonemapMode == TONEMAP_MODE_FAST ||
        mAiqParam.tonemapMode == TONEMAP_MODE_HIGH_QUALITY) {
        aiqResult->mGbceResults.have_manual_settings = false;
        return OK;
    }

    aiqResult->mGbceResults.have_manual_settings = true;

    if (mAiqParam.tonemapMode == TONEMAP_MODE_GAMMA_VALUE) {
        AiqUtils::applyTonemapGamma(mAiqParam.tonemapGamma, &aiqResult->mGbceResults);
    } else if (mAiqParam.tonemapMode == TONEMAP_MODE_PRESET_CURVE) {
        if (mAiqParam.tonemapPresetCurve == TONEMAP_PRESET_CURVE_SRGB) {
            AiqUtils::applyTonemapSRGB(&aiqResult->mGbceResults);
        } else if (mAiqParam.tonemapPresetCurve == TONEMAP_PRESET_CURVE_REC709) {
            AiqUtils::applyTonemapREC709(&aiqResult->mGbceResults);
        }
    }  else if (mAiqParam.tonemapMode == TONEMAP_MODE_CONTRAST_CURVE) {
        AiqUtils::applyTonemapCurve(mAiqParam.tonemapCurves, &aiqResult->mGbceResults);
        AiqUtils::applyAwbGainForTonemapCurve(mAiqParam.tonemapCurves, &aiqResult->mAwbResults);
    }

    // use unity value for tone map table
    if (aiqResult->mGbceResults.tone_map_lut_size > 0) {
        for (unsigned int i = 0; i < aiqResult->mGbceResults.tone_map_lut_size; i++) {
            aiqResult->mGbceResults.tone_map_lut[i] = 1.0;
        }
    }

    return OK;
}

AiqEngine::AiqState AiqEngine::done(AiqResult *aiqResult)
{
    int skipNum = getSkippingNum(aiqResult);
    AiqResult *tmp = aiqResult;

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
