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

#define LOG_TAG "SensorManager"

#include "iutils/Errors.h"
#include "iutils/CameraLog.h"

#include "AiqUtils.h"
#include "SensorManager.h"
#include "PlatformData.h"

using std::vector;

namespace icamera {

SensorManager::SensorManager(int cameraId, SensorHwCtrl *sensorHw) :
    mCameraId(cameraId),
    mSensorHwCtrl(sensorHw),
    mModeSwitched(false),
    mLastSofSequence(-1),
    mAnalogGainDelay(0),
    mDigitalGainDelay(0)
{
    LOG1("@%s mCameraId = %d", __func__, mCameraId);

    CLEAR(mWdrModeSetting);

    if (PlatformData::getAnalogGainLag(mCameraId) > 0) {
        mAnalogGainDelay = PlatformData::getExposureLag(mCameraId)
            - PlatformData::getAnalogGainLag(mCameraId);
        mDigitalGainDelay = mAnalogGainDelay;
    }

    if (PlatformData::getDigitalGainLag(mCameraId) >= 0) {
        mDigitalGainDelay = PlatformData::getExposureLag(mCameraId)
            - PlatformData::getDigitalGainLag(mCameraId);
    }
}

SensorManager::~SensorManager()
{
    LOG1("@%s mCameraId = %d", __func__, mCameraId);
}

int SensorManager::init()
{
    AutoMutex l(mLock);
    LOG1("@%s mCameraId = %d", __func__, mCameraId);

    reset();
    return OK;
}

int SensorManager::deinit()
{
    AutoMutex l(mLock);
    LOG1("@%s mCameraId = %d", __func__, mCameraId);

    return OK;
}

void SensorManager::reset()
{
    mLastSofSequence = -1;

    mExposureDataMap.clear();
    mAnalogGainMap.clear();
    mDigitalGainMap.clear();

    mModeSwitched = false;
    CLEAR(mWdrModeSetting);
    mWdrModeSetting.tuningMode = TUNING_MODE_MAX;

    mSofEventInfo.clear();
}

EventListener *SensorManager::getSofEventListener()
{
    AutoMutex l(mLock);
    LOG1("@%s mCameraId = %d", __func__, mCameraId);

    return this;
}

void SensorManager::handleEvent(EventData eventData)
{
    AutoMutex l(mLock);
    LOG3A("@%s", __func__);

    if (eventData.type == EVENT_ISYS_SOF) {
        LOG3A("sequence = %ld, timestamp = %ld",
                eventData.data.sync.sequence,
                TIMEVAL2USECS(eventData.data.sync.timestamp));
        mLastSofSequence = eventData.data.sync.sequence;
        handleSensorExposure();

        SofEventInfo info;
        info.sequence = eventData.data.sync.sequence;
        info.timestamp = ((long)eventData.data.sync.timestamp.tv_sec) * 1000000
                         + eventData.data.sync.timestamp.tv_usec;
        if (mSofEventInfo.size() >= kMaxSofEventInfo) {
            mSofEventInfo.erase(mSofEventInfo.begin());
        }
        mSofEventInfo.push_back(info);
    }
}

uint64_t SensorManager::getSofTimestamp(long sequence)
{
    AutoMutex l(mLock);

    for (auto info : mSofEventInfo) {
        if (info.sequence == sequence) {
            return info.timestamp;
        }
    }
    return 0;
}

void SensorManager::handleSensorExposure()
{
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

int SensorManager::getCurrentExposureAppliedDelay()
{
    AutoMutex l(mLock);

    return mExposureDataMap.size() + PlatformData::getExposureLag(mCameraId);
}

uint32_t SensorManager::updateSensorExposure(SensorExpGroup sensorExposures, bool useSof)
{
    AutoMutex l(mLock);

    long appliedSeq = mLastSofSequence < 0 ? 0 : \
                      mLastSofSequence + PlatformData::getExposureLag(mCameraId);

    if (sensorExposures.empty()) {
        LOGW("%s: No exposure parameter", __func__);
        return ((uint32_t)appliedSeq);
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

    if (useSof) {
        int sensorSeq = mLastSofSequence + mExposureDataMap.size() + 1;
        mExposureDataMap[sensorSeq] = exposureData;
        mAnalogGainMap[sensorSeq + mAnalogGainDelay] = analogGains;
        mDigitalGainMap[sensorSeq + mDigitalGainDelay] = digitalGains;
        appliedSeq += mExposureDataMap.size();
    } else if (PlatformData::isIsysEnabled(mCameraId)) {
        mSensorHwCtrl->setFrameDuration(exposureData.lineLengthPixels,
                                        exposureData.frameLengthLines);
        mSensorHwCtrl->setExposure(exposureData.coarseExposures, exposureData.fineExposures);
        mSensorHwCtrl->setAnalogGains(analogGains);
        mSensorHwCtrl->setDigitalGains(digitalGains);
    }

    LOG3A("@%s, useSof:%d, mLastSofSequence:%ld, appliedSeq %ld", __func__, useSof,
           mLastSofSequence, appliedSeq);
    return ((uint32_t)appliedSeq);
}

int SensorManager::setFrameRate(float fps)
{
    return mSensorHwCtrl->setFrameRate(fps);
}

int SensorManager::getSensorInfo(ia_aiq_frame_params &frameParams,
                                 ia_aiq_exposure_sensor_descriptor &sensorDescriptor)
{
    LOG3A("@%s", __func__);
    SensorFrameParams sensorFrameParams;
    CLEAR(sensorFrameParams);

    int ret = PlatformData::calculateFrameParams(mCameraId, sensorFrameParams);
    if (ret == OK) {
        AiqUtils::convertToAiqFrameParam(sensorFrameParams, frameParams);
    }

    if (!PlatformData::isIsysEnabled(mCameraId)) {
        vector <camera_resolution_t> res;
        PlatformData::getSupportedISysSizes(mCameraId, res);

        CheckError(res.empty(), BAD_VALUE, "Supported ISYS resolutions are not configured.");
        // In none-ISYS cases, only take 30 fps into account.
        int fps = 30;
        float freq = res[0].width * res[0].height * fps / 1000000;
        sensorDescriptor = {freq, static_cast<unsigned short>(res[0].width),
                            static_cast<unsigned short>(res[0].height), 24, 0,
                            static_cast<unsigned short>(res[0].width), 6, 0};
        LOG3A("freq %f, width %d, height %d", freq, res[0].width, res[0].height);
        return OK;
    }

    ret |= getSensorModeData(sensorDescriptor);

    LOG3A("ia_aiq_frame_params=[%d, %d, %d, %d, %d, %d, %d, %d]",
        frameParams.horizontal_crop_offset,
        frameParams.vertical_crop_offset,
        frameParams.cropped_image_height,
        frameParams.cropped_image_width,
        frameParams.horizontal_scaling_numerator,
        frameParams.horizontal_scaling_denominator,
        frameParams.vertical_scaling_numerator,
        frameParams.vertical_scaling_denominator);

    LOG3A("ia_aiq_exposure_sensor_descriptor=[%f, %d, %d, %d, %d, %d, %d, %d]",
        sensorDescriptor.pixel_clock_freq_mhz,
        sensorDescriptor.pixel_periods_per_line,
        sensorDescriptor.line_periods_per_field,
        sensorDescriptor.line_periods_vertical_blanking,
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
int SensorManager::getSensorModeData(ia_aiq_exposure_sensor_descriptor& sensorData)
{
    int pixel = 0;
    int status =  mSensorHwCtrl->getPixelRate(pixel);
    CheckError(status != OK, status, "Failed to get pixel clock ret:%d", status);
    sensorData.pixel_clock_freq_mhz = (float)pixel / 1000000;

    int width = 0, height = 0, pixelCode = 0;
    status = mSensorHwCtrl->getActivePixelArraySize(width, height, pixelCode);
    CheckError(status != OK, status, "Failed to get active pixel array size ret:%d", status);

    int pixel_periods_per_line, line_periods_per_field;
    status = mSensorHwCtrl->getFrameDuration(pixel_periods_per_line, line_periods_per_field);
    CheckError(status != OK, status, "Failed to get frame Durations ret:%d", status);

    sensorData.pixel_periods_per_line = CLIP(pixel_periods_per_line, USHRT_MAX, 0);
    sensorData.line_periods_per_field = CLIP(line_periods_per_field, USHRT_MAX, 0);

    int coarse_int_time_min, integration_step = 0, integration_max = 0;
    status = mSensorHwCtrl->getExposureRange(coarse_int_time_min, integration_max, integration_step);
    CheckError(status != OK, status, "Failed to get Exposure Range ret:%d", status);

    sensorData.coarse_integration_time_min = CLIP(coarse_int_time_min, USHRT_MAX, 0);
    sensorData.coarse_integration_time_max_margin = PlatformData::getCITMaxMargin(mCameraId);

    // fine integration is not supported by v4l2
    sensorData.fine_integration_time_min = 0;
    sensorData.fine_integration_time_max_margin = sensorData.pixel_periods_per_line;
    int vblank;
    status = mSensorHwCtrl->getVBlank(vblank);
    CheckError(status != OK, status, "Failed to get vblank ret:%d", status);
    sensorData.line_periods_vertical_blanking = CLIP(vblank, USHRT_MAX, 0);

    return OK;
}

} /* namespace icamera */
