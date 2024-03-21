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

#define LOG_TAG SensorManager

#include "iutils/Errors.h"
#include "iutils/CameraLog.h"

#include "AiqUtils.h"
#include "SensorManager.h"
#include "PlatformData.h"

using std::vector;

namespace icamera {

SensorManager::SensorManager(int cameraId, SensorHwCtrl* sensorHw)
        : mCameraId(cameraId),
          mSensorHwCtrl(sensorHw),
          // HDR_FEATURE_S
          mModeSwitched(false),
          // HDR_FEATURE_E
          mLastSofSequence(-1),
          mAnalogGainDelay(0),
          mDigitalGainDelay(0) {
    // HDR_FEATURE_S
    CLEAR(mWdrModeSetting);
    // HDR_FEATURE_E

    if (PlatformData::getAnalogGainLag(mCameraId) > 0) {
        mAnalogGainDelay =
            PlatformData::getExposureLag(mCameraId) - PlatformData::getAnalogGainLag(mCameraId);
        mDigitalGainDelay = mAnalogGainDelay;
    }

    if (PlatformData::getDigitalGainLag(mCameraId) >= 0) {
        mDigitalGainDelay =
            PlatformData::getExposureLag(mCameraId) - PlatformData::getDigitalGainLag(mCameraId);
    }
}

SensorManager::~SensorManager() {}

void SensorManager::reset() {
    LOG1("<id%d>@%s", mCameraId, __func__);

    AutoMutex l(mLock);
    mLastSofSequence = -1;
    mExposureDataMap.clear();
    mAnalogGainMap.clear();
    mDigitalGainMap.clear();

    // HDR_FEATURE_S
    mModeSwitched = false;
    CLEAR(mWdrModeSetting);
    mWdrModeSetting.tuningMode = TUNING_MODE_MAX;
    // HDR_FEATURE_E

    mSofEventInfo.clear();
}

void SensorManager::handleSofEvent(EventData eventData) {
    AutoMutex l(mLock);
    if (eventData.type == EVENT_ISYS_SOF) {
        LOG2("<seq%ld> SOF timestamp = %ld", eventData.data.sync.sequence,
             TIMEVAL2USECS(eventData.data.sync.timestamp));
        mLastSofSequence = eventData.data.sync.sequence;
        handleSensorExposure();

        // HDR_FEATURE_S
        handleSensorModeSwitch(eventData.data.sync.sequence);
        // HDR_FEATURE_E

        SofEventInfo info;
        info.sequence = eventData.data.sync.sequence;
        info.timestamp = ((long)eventData.data.sync.timestamp.tv_sec) * 1000000 +
                         eventData.data.sync.timestamp.tv_usec;
        if (mSofEventInfo.size() >= kMaxSofEventInfo) {
            mSofEventInfo.erase(mSofEventInfo.begin());
        }
        mSofEventInfo.push_back(info);
    }
}

uint64_t SensorManager::getSofTimestamp(int64_t sequence) {
    AutoMutex l(mLock);

    for (auto info : mSofEventInfo) {
        if (info.sequence == sequence) {
            return info.timestamp;
        }
    }
    return 0;
}

// HDR_FEATURE_S
int SensorManager::convertTuningModeToWdrMode(TuningMode tuningMode) {
    return ((tuningMode == TUNING_MODE_VIDEO_HDR) || (tuningMode == TUNING_MODE_VIDEO_HDR2)) ? 1 :
                                                                                               0;
}

void SensorManager::handleSensorModeSwitch(int64_t sequence) {
    if (!PlatformData::isEnableHDR(mCameraId) || !mModeSwitched) {
        return;
    }

    if (mWdrModeSetting.sequence <= sequence) {
        int wdrMode = convertTuningModeToWdrMode(mWdrModeSetting.tuningMode);
        LOG2("<seq%ld>@%s, tunning mode %d, set wdrMode %d sequence %u", sequence, __func__,
             mWdrModeSetting.tuningMode, wdrMode, mWdrModeSetting.sequence);

        if (mSensorHwCtrl->setWdrMode(wdrMode) == OK) {
            mModeSwitched = false;
        }
    }
}

int SensorManager::setWdrMode(TuningMode tuningMode, int64_t sequence) {
    if (!PlatformData::isEnableHDR(mCameraId)) {
        return OK;
    }

    AutoMutex l(mLock);
    LOG2("@%s, tuningMode %d, sequence %ld", __func__, tuningMode, sequence);
    int ret = OK;

    // Set Wdr Mode after running AIQ first time.
    if (mWdrModeSetting.tuningMode == TUNING_MODE_MAX) {
        int wdrMode = convertTuningModeToWdrMode(tuningMode);
        ret = mSensorHwCtrl->setWdrMode(wdrMode);
        mWdrModeSetting.tuningMode = tuningMode;
        return ret;
    }

    if (mWdrModeSetting.tuningMode != tuningMode) {
        // Save WDR mode and update this mode to driver in SOF event handler.
        // So we know which frame is corrupted and we can skip the corrupted frames.
        LOG2("<seq%ld>@%s, tuningMode %d", sequence, __func__, tuningMode);
        mWdrModeSetting.tuningMode = tuningMode;
        mWdrModeSetting.sequence = sequence;
        mModeSwitched = true;
    }

    return ret;
}

int SensorManager::setAWB(float r_per_g, float b_per_g) {
    AutoMutex l(mLock);
    LOG2("@%s, r_per_g %f, b_per_g %f", __func__, r_per_g, b_per_g);

    int ret = mSensorHwCtrl->setAWB(r_per_g, b_per_g);
    return ret;
}
// HDR_FEATURE_E

void SensorManager::handleSensorExposure() {
    if (mExposureDataMap.find(mLastSofSequence) != mExposureDataMap.end()) {
        const ExposureData& exposureData = mExposureDataMap[mLastSofSequence];
        mSensorHwCtrl->setFrameDuration(exposureData.lineLengthPixels,
                                        exposureData.frameLengthLines);
        mSensorHwCtrl->setExposure(exposureData.coarseExposures, exposureData.fineExposures);
        mExposureDataMap.erase(mLastSofSequence);
    }

    if (mAnalogGainMap.find(mLastSofSequence) != mAnalogGainMap.end()) {
        mSensorHwCtrl->setAnalogGains(mAnalogGainMap[mLastSofSequence]);
        mAnalogGainMap.erase(mLastSofSequence);
    }

    if (mDigitalGainMap.find(mLastSofSequence) != mDigitalGainMap.end()) {
        mSensorHwCtrl->setDigitalGains(mDigitalGainMap[mLastSofSequence]);
        mDigitalGainMap.erase(mLastSofSequence);
    }
}

int SensorManager::getCurrentExposureAppliedDelay() {
    AutoMutex l(mLock);

    return mExposureDataMap.size() + PlatformData::getExposureLag(mCameraId);
}

uint32_t SensorManager::updateSensorExposure(SensorExpGroup sensorExposures, int64_t applyingSeq) {
    AutoMutex l(mLock);

    int64_t effectSeq = mLastSofSequence < 0 ? 0
        : mLastSofSequence + PlatformData::getExposureLag(mCameraId);

    if (sensorExposures.empty()) {
        LOGW("%s: No exposure parameter", __func__);
        return ((uint32_t)effectSeq);
    }

    ExposureData exposureData;
    vector<int> analogGains;
    vector<int> digitalGains;
    exposureData.lineLengthPixels = sensorExposures[0].sensorParam.line_length_pixels;
    exposureData.frameLengthLines = sensorExposures[0].sensorParam.frame_length_lines;
    for (auto exp : sensorExposures) {
        exposureData.coarseExposures.push_back(exp.sensorParam.coarse_integration_time);
        exposureData.fineExposures.push_back(exp.sensorParam.fine_integration_time);

        analogGains.push_back(exp.sensorParam.analog_gain_code_global);
        int digitalGain = exp.sensorParam.digital_gain_global;
        if (PlatformData::isUsingIspDigitalGain(mCameraId)) {
            digitalGain = PlatformData::getSensorDigitalGain(mCameraId, exp.realDigitalGain);
        }
        digitalGains.push_back(digitalGain);
    }

    if (effectSeq > 0) {
        int sensorSeq = mLastSofSequence + mExposureDataMap.size() + 1;
        if (applyingSeq > 0 && applyingSeq == mLastSofSequence) {
            sensorSeq = applyingSeq;
            mSensorHwCtrl->setFrameDuration(exposureData.lineLengthPixels,
                                            exposureData.frameLengthLines);
            mSensorHwCtrl->setExposure(exposureData.coarseExposures, exposureData.fineExposures);
        } else {
            mExposureDataMap[sensorSeq] = exposureData;
        }

        if ((sensorSeq + mAnalogGainDelay) == mLastSofSequence) {
            mSensorHwCtrl->setAnalogGains(analogGains);
        } else {
            mAnalogGainMap[sensorSeq + mAnalogGainDelay] = analogGains;
        }
        if ((sensorSeq + mDigitalGainDelay) == mLastSofSequence) {
            mSensorHwCtrl->setDigitalGains(digitalGains);
        } else {
            mDigitalGainMap[sensorSeq + mDigitalGainDelay] = digitalGains;
        }
        effectSeq += mExposureDataMap.size();
    } else if (PlatformData::isIsysEnabled(mCameraId)) {
        mSensorHwCtrl->setFrameDuration(exposureData.lineLengthPixels,
                                        exposureData.frameLengthLines);
        mSensorHwCtrl->setExposure(exposureData.coarseExposures, exposureData.fineExposures);
        mSensorHwCtrl->setAnalogGains(analogGains);
        mSensorHwCtrl->setDigitalGains(digitalGains);
    }

    if (effectSeq == 0) {
        effectSeq = PlatformData::getInitialSkipFrame(mCameraId);
    }

    LOG2("<seq%ld>@%s: effectSeq %ld, applyingSeq %ld", mLastSofSequence, __func__, effectSeq,
         applyingSeq);
    return ((uint32_t)effectSeq);
}
// CRL_MODULE_S
int SensorManager::setFrameRate(float fps) {
    return mSensorHwCtrl->setFrameRate(fps);
}
// CRL_MODULE_E

int SensorManager::getSensorInfo(ia_aiq_frame_params& frameParams,
                                 ia_aiq_exposure_sensor_descriptor& sensorDescriptor) {
    SensorFrameParams sensorFrameParams;
    CLEAR(sensorFrameParams);

    int ret = PlatformData::calculateFrameParams(mCameraId, sensorFrameParams);
    if (ret == OK) {
        AiqUtils::convertToAiqFrameParam(sensorFrameParams, frameParams);
    }

    if (!PlatformData::isIsysEnabled(mCameraId)) {
        vector<camera_resolution_t> res;
        PlatformData::getSupportedISysSizes(mCameraId, res);

        CheckAndLogError(res.empty(), BAD_VALUE, "Supported ISYS resolutions are not configured.");
        // In none-ISYS cases, only take 30 fps into account.
        int fps = 30;
        float freq = res[0].width * res[0].height * fps / 1000000;
        sensorDescriptor = {freq,
                            static_cast<unsigned short>(res[0].width),
                            static_cast<unsigned short>(res[0].height),
                            24,
                            0,
                            static_cast<unsigned short>(res[0].width),
                            6,
                            0};
        LOG2("freq %f, width %d, height %d", freq, res[0].width, res[0].height);
        return OK;
    }

    ret |= getSensorModeData(sensorDescriptor);

    LOG3("ia_aiq_frame_params=[%d, %d, %d, %d, %d, %d, %d, %d]", frameParams.horizontal_crop_offset,
         frameParams.vertical_crop_offset, frameParams.cropped_image_height,
         frameParams.cropped_image_width, frameParams.horizontal_scaling_numerator,
         frameParams.horizontal_scaling_denominator, frameParams.vertical_scaling_numerator,
         frameParams.vertical_scaling_denominator);

    LOG3("ia_aiq_exposure_sensor_descriptor=[%f, %d, %d, %d, %d, %d, %d, %d]",
         sensorDescriptor.pixel_clock_freq_mhz, sensorDescriptor.pixel_periods_per_line,
         sensorDescriptor.line_periods_per_field, sensorDescriptor.line_periods_vertical_blanking,
         sensorDescriptor.coarse_integration_time_min,
         sensorDescriptor.coarse_integration_time_max_margin,
         sensorDescriptor.fine_integration_time_min,
         sensorDescriptor.fine_integration_time_max_margin);

    return ret;
}

/**
 * get sensor mode data (sensor descriptor) from sensor driver
 *
 * \return OK if successfully.
 */
int SensorManager::getSensorModeData(ia_aiq_exposure_sensor_descriptor& sensorData) {
    int pixel = 0;
    int status = mSensorHwCtrl->getPixelRate(pixel);
    CheckAndLogError(status != OK, status, "Failed to get pixel clock ret:%d", status);
    sensorData.pixel_clock_freq_mhz = (float)pixel / 1000000;

    int width = 0, height = 0, pixelCode = 0;
    status = mSensorHwCtrl->getActivePixelArraySize(width, height, pixelCode);
    CheckAndLogError(status != OK, status, "Failed to get active pixel array size ret:%d", status);

    int pixel_periods_per_line, line_periods_per_field;
    status = mSensorHwCtrl->getFrameDuration(pixel_periods_per_line, line_periods_per_field);
    CheckAndLogError(status != OK, status, "Failed to get frame Durations ret:%d", status);

    sensorData.pixel_periods_per_line = CLIP(pixel_periods_per_line, USHRT_MAX, 0);
    sensorData.line_periods_per_field = CLIP(line_periods_per_field, USHRT_MAX, 0);

    int coarse_int_time_min, integration_step = 0, integration_max = 0;
    status =
        mSensorHwCtrl->getExposureRange(coarse_int_time_min, integration_max, integration_step);
    CheckAndLogError(status != OK, status, "Failed to get Exposure Range ret:%d", status);

    sensorData.coarse_integration_time_min = CLIP(coarse_int_time_min, USHRT_MAX, 0);
    sensorData.coarse_integration_time_max_margin = PlatformData::getCITMaxMargin(mCameraId);

    // fine integration is not supported by v4l2
    sensorData.fine_integration_time_min = 0;
    sensorData.fine_integration_time_max_margin = sensorData.pixel_periods_per_line;
    int vblank;
    status = mSensorHwCtrl->getVBlank(vblank);
    CheckAndLogError(status != OK, status, "Failed to get vblank ret:%d", status);
    sensorData.line_periods_vertical_blanking = CLIP(vblank, USHRT_MAX, 0);

    return OK;
}

} /* namespace icamera */
