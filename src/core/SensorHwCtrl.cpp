/*
 * Copyright (C) 2015-2020 Intel Corporation
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

#define LOG_TAG "SensorHwCtrl"

#include <limits.h>

#include <linux/types.h>
#include <linux/v4l2-controls.h>

#include "iutils/CameraLog.h"

#include "SensorHwCtrl.h"
#include "V4l2DeviceFactory.h"
#include "PlatformData.h"

using std::vector;

namespace icamera {

SensorHwCtrl::SensorHwCtrl(int cameraId, V4L2Subdevice* pixelArraySubdev, V4L2Subdevice* sensorOutputSubdev):
        mPixelArraySubdev(pixelArraySubdev),
        mSensorOutputSubdev(sensorOutputSubdev),
        mCameraId(cameraId),
        mHorzBlank(0),
        mVertBlank(0),
        mCropWidth(0),
        mCropHeight(0),
        mCurFll(0),
        mCalculatingFrameDuration(true)
{
    HAL_TRACE_CALL(CAMERA_DEBUG_LOG_LEVEL1);
    LOG2("@%s, mCameraId:%d", __func__, mCameraId);
}

SensorHwCtrl::~SensorHwCtrl()
{
    HAL_TRACE_CALL(CAMERA_DEBUG_LOG_LEVEL1);
}

SensorHwCtrl* SensorHwCtrl::createSensorCtrl(int cameraId)
{
    if (!PlatformData::isIsysEnabled(cameraId)) {
        return new DummySensor(cameraId);
    }

    std::string subDevName;
    SensorHwCtrl* sensorCtrl = nullptr;
    int ret = PlatformData::getDevNameByType(cameraId, VIDEO_PIXEL_ARRAY, subDevName);
    if (ret == OK) {
        LOG1("%s ArraySubdev camera id:%d dev name:%s", __func__, cameraId, subDevName.c_str());
        V4L2Subdevice* pixelArraySubdev = V4l2DeviceFactory::getSubDev(cameraId, subDevName);

        V4L2Subdevice* pixelOutputSubdev = nullptr;
        // Binner and Scaler subdev only exits in CrlModule driver
        if (PlatformData::isUsingCrlModule(cameraId)) {
            subDevName.clear();
            ret = PlatformData::getDevNameByType(cameraId, VIDEO_PIXEL_SCALER, subDevName);
            if (ret == OK) {
                LOG1("%s ScalerSubdev camera id:%d dev name:%s", __func__, cameraId, subDevName.c_str());
                pixelOutputSubdev = V4l2DeviceFactory::getSubDev(cameraId, subDevName);
            } else {
                subDevName.clear();
                ret = PlatformData::getDevNameByType(cameraId, VIDEO_PIXEL_BINNER, subDevName);
                if (ret == OK) {
                    LOG1("%s BinnerSubdev camera id:%d dev name:%s", __func__, cameraId, subDevName.c_str());
                    pixelOutputSubdev = V4l2DeviceFactory::getSubDev(cameraId, subDevName);
                }
            }
        }

        sensorCtrl = new SensorHwCtrl(cameraId, pixelArraySubdev, pixelOutputSubdev);
    } else {
        LOG1("%s create a dummy sensor ctrl for camera id:%d", __func__, cameraId);
        sensorCtrl = new DummySensor(cameraId);
    }
    return sensorCtrl;
}

int SensorHwCtrl::getActivePixelArraySize(int &width, int &height, int &pixelCode)
{
    HAL_TRACE_CALL(CAMERA_DEBUG_LOG_LEVEL2);
    CheckError(!mPixelArraySubdev, NO_INIT, "pixel array sub device is not set");

    int status = mPixelArraySubdev->GetPadFormat(0, &width, &height, &pixelCode);
    mCropWidth = width;
    mCropHeight = height;

    LOG2("@%s, width:%d, height:%d, status:%d", __func__, width, height, status);
    return status;
}

int SensorHwCtrl::getPixelRate(int &pixelRate)
{
    HAL_TRACE_CALL(CAMERA_DEBUG_LOG_LEVEL2);
    CheckError(!mPixelArraySubdev, NO_INIT, "pixel array sub device is not set");

    int ret = mPixelArraySubdev->GetControl(V4L2_CID_PIXEL_RATE, &pixelRate);

    LOG2("@%s, pixelRate:%d, ret:%d", __func__, pixelRate, ret);

    return ret;
}

int SensorHwCtrl::setTestPatternMode(int32_t testPatternMode)
{
    HAL_TRACE_CALL(CAMERA_DEBUG_LOG_LEVEL2);
    CheckError(!mPixelArraySubdev, NO_INIT, "pixel array sub device is not set");

    LOG2("@%s, testPatternMode: %d", __func__, testPatternMode);
    return mPixelArraySubdev->SetControl(V4L2_CID_TEST_PATTERN, testPatternMode);
}

int SensorHwCtrl::setExposure(const vector<int>& coarseExposures, const vector<int>& fineExposures)
{
    HAL_TRACE_CALL(CAMERA_DEBUG_LOG_LEVEL2);
    CheckError(!mPixelArraySubdev, NO_INIT, "pixel array sub device is not set");
    CheckError((coarseExposures.empty() || fineExposures.empty()), BAD_VALUE, "No exposure data!");

    LOG2("%s coarseExposure=%d fineExposure=%d", __func__, coarseExposures[0], fineExposures[0]);
    LOG2("SENSORCTRLINFO: exposure_value=%d", coarseExposures[0]);
    return mPixelArraySubdev->SetControl(V4L2_CID_EXPOSURE, coarseExposures[0]);
}

int SensorHwCtrl::setAnalogGains(const vector<int>& analogGains)
{
    HAL_TRACE_CALL(CAMERA_DEBUG_LOG_LEVEL2);
    CheckError(!mPixelArraySubdev, NO_INIT, "pixel array sub device is not set");
    CheckError(analogGains.empty(), BAD_VALUE, "No analog gain data!");

    LOG2("%s analogGain=%d", __func__, analogGains[0]);
    return mPixelArraySubdev->SetControl(V4L2_CID_ANALOGUE_GAIN, analogGains[0]);
}

int SensorHwCtrl::setDigitalGains(const vector<int>& digitalGains)
{
    HAL_TRACE_CALL(CAMERA_DEBUG_LOG_LEVEL2);
    CheckError(!mPixelArraySubdev, NO_INIT, "pixel array sub device is not set");
    CheckError(digitalGains.empty(), BAD_VALUE, "No digital gain data!");

    LOG2("%s digitalGain=%d", __func__, digitalGains[0]);
    return mPixelArraySubdev->SetControl(V4L2_CID_DIGITAL_GAIN, digitalGains[0]);
}

int SensorHwCtrl::setLineLengthPixels(int llp)
{
    int status = OK;
    LOG2("@%s, llp:%d", __func__, llp);

    if (mCalculatingFrameDuration) {
        int horzBlank = llp - mCropWidth;
        if (mHorzBlank != horzBlank) {
            status = mPixelArraySubdev->SetControl(V4L2_CID_HBLANK, horzBlank);
        }
    }

    CheckError(status != OK, status, "failed to set llp.");

    mHorzBlank = llp - mCropWidth;
    return status;
}

int SensorHwCtrl::setFrameLengthLines(int fll)
{
    int status = OK;
    LOG2("@%s, fll:%d", __func__, fll);

    if (mCalculatingFrameDuration) {
        int vertBlank = fll - mCropHeight;
        if (mVertBlank != vertBlank) {
            status = mPixelArraySubdev->SetControl(V4L2_CID_VBLANK, vertBlank);
        }
    }

    mCurFll = fll;

    CheckError(status != OK, status, "failed to set fll.");

    mVertBlank = fll - mCropHeight;
    return status;
}

int SensorHwCtrl::setFrameDuration(int llp, int fll)
{
    HAL_TRACE_CALL(CAMERA_DEBUG_LOG_LEVEL2);
    CheckError(!mPixelArraySubdev, NO_INIT, "pixel array sub device is not set");

    int status = OK;
    LOG2("@%s, llp:%d, fll:%d", __func__, llp, fll);

    /* only set them to driver when llp or fll is not 0 */
    if (llp) {
        status = setLineLengthPixels(llp);
    }

    if (fll) {
        status |= setFrameLengthLines(fll);
    }

    return status;
}

int SensorHwCtrl::getLineLengthPixels(int &llp)
{
    int status = OK;

    if (mCalculatingFrameDuration) {
        int horzBlank = 0;
        status = mPixelArraySubdev->GetControl(V4L2_CID_HBLANK, &horzBlank);
        if (status == OK) {
            mHorzBlank = horzBlank;
            llp = horzBlank + mCropWidth;
        }
    }

    LOG2("@%s, llp:%d", __func__, llp);
    CheckError(status != OK, status, "failed to get llp.");

    return status;
}

int SensorHwCtrl::getFrameLengthLines(int &fll)
{
    int status = OK;

    if (mCalculatingFrameDuration) {
        int vertBlank = 0;
        status = mPixelArraySubdev->GetControl(V4L2_CID_VBLANK, &vertBlank);
        if (status == OK) {
            mVertBlank = vertBlank;
            fll = vertBlank + mCropHeight;
        }
    }

    LOG2("@%s, fll:%d", __func__, fll);
    CheckError(status != OK, status, "failed to get fll.");

    return status;
}

int SensorHwCtrl::getFrameDuration(int &llp, int &fll)
{
    HAL_TRACE_CALL(CAMERA_DEBUG_LOG_LEVEL2);
    CheckError(!mPixelArraySubdev, NO_INIT, "pixel array sub device is not set");

    int status = getLineLengthPixels(llp);

    status |= getFrameLengthLines(fll);
    LOG2("@%s, llp:%d, fll:%d", __func__, llp, fll);

    return status;
}

int SensorHwCtrl::getVBlank(int &vblank)
{
    HAL_TRACE_CALL(CAMERA_DEBUG_LOG_LEVEL2);
    vblank = mVertBlank;
    LOG2("@%s, vblank:%d", __func__, vblank);

    return OK;
}

/**
 * get exposure range value from sensor driver
 *
 * \param[OUT] coarse_exposure: exposure min value
 * \param[OUT] fine_exposure: exposure max value
 * \param[OUT] exposure_step: step of exposure
 * V4L2 does not support FINE_EXPOSURE setting
 *
 * \return OK if successfully.
 */
int SensorHwCtrl::getExposureRange(int &exposureMin, int &exposureMax, int &exposureStep)
{
    HAL_TRACE_CALL(CAMERA_DEBUG_LOG_LEVEL2);
    CheckError(!mPixelArraySubdev, NO_INIT, "pixel array sub device is not set");

    v4l2_queryctrl exposure = {};
    exposure.id = V4L2_CID_EXPOSURE;
    int status = mPixelArraySubdev->QueryControl(&exposure);
    CheckError(status != OK, status, "Couldn't get exposure Range status:%d", status);

    exposureMin = exposure.minimum;
    exposureMax = exposure.maximum;
    exposureStep = exposure.step;
    LOG2("@%s, exposureMin:%d, exposureMax:%d, exposureStep:%d",
        __func__, exposureMin, exposureMax, exposureStep);

    return status;
}

int SensorHwCtrl::setFrameRate(float fps)
{
    HAL_TRACE_CALL(CAMERA_DEBUG_LOG_LEVEL2);
    CheckError(!mSensorOutputSubdev, NO_INIT, "sensor output sub device is not set");

    LOG2("%s FPS is: %f", __func__, fps);

    struct v4l2_queryctrl query;
    CLEAR(query);
    query.id = V4L2_CID_LINK_FREQ;
    int status = mSensorOutputSubdev->QueryControl(&query);
    CheckError(status != OK, status, "Couldn't get V4L2_CID_LINK_FREQ, status:%d", status);

    LOG2("@%s, query V4L2_CID_LINK_FREQ:, default_value:%d, maximum:%d, minimum:%d, step:%d",
        __func__, query.default_value, query.maximum, query.minimum, query.step);

    int mode = 0;
    if (query.maximum == query.minimum) {
        mode = query.default_value;
    } else {
        /***********************************************************************************
         * WA: This heavily depends on sensor driver implementation, need to find a graceful
         * solution.
         * imx185:
         * When fps larger than 30, should switch to high speed mode, currently only
         * 0, 1, 2 are available. 0 means 720p 30fps, 1 means 2M 30fps, and 2 means 2M 60fps.
         * imx290:
         * 0 and 1 available, for 30 and higher FPS.
         ***********************************************************************************/
        mode = (fps > 30) ? query.maximum : (query.maximum - 1);
    }
    LOG2("@%s, set V4L2_CID_LINK_FREQ to %d", __func__, mode);
    return mSensorOutputSubdev->SetControl(V4L2_CID_LINK_FREQ, mode);
}

} // namespace icamera
