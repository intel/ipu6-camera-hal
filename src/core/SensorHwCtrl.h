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

#pragma once

#include <v4l2_device.h>

#include <vector>

#include "iutils/Errors.h"
#include "iutils/Utils.h"

namespace icamera {

/**
 * Base class for sensor control might be inherited by ones have different sensor driver
 */
class SensorHwCtrl {

public:
    static SensorHwCtrl* createSensorCtrl(int cameraId);
    SensorHwCtrl(int cameraId, V4L2Subdevice* pixelArraySubdev, V4L2Subdevice* sensorOutputSubdev);
    virtual ~SensorHwCtrl();

    virtual int setTestPatternMode(int32_t testPatternMode);
    virtual int getPixelRate(int &pixelRate);
    virtual int setExposure(const std::vector<int>& coarseExposures,
                            const std::vector<int>& fineExposures);
    virtual int setAnalogGains(const std::vector<int>& analogGains);
    virtual int setDigitalGains(const std::vector<int>& digitalGains);
    virtual int setFrameDuration(int llp, int fll);
    virtual int getFrameDuration(int &llp, int &fll);
    virtual int getVBlank(int &vblank);
    virtual int getActivePixelArraySize(int &width, int &height, int &pixelCode);
    virtual int getExposureRange(int &exposureMin, int &exposureMax, int &exposureStep);

    virtual int setFrameRate(float fps);
private:
    int setLineLengthPixels(int llp);
    int getLineLengthPixels(int &llp);
    int setFrameLengthLines(int fll);
    int getFrameLengthLines(int &fll);

private:

    V4L2Subdevice* mPixelArraySubdev;
    V4L2Subdevice* mSensorOutputSubdev;
    int mCameraId;
    int mHorzBlank;
    int mVertBlank;
    int mCropWidth;
    int mCropHeight;

    // Current frame length lines
    int mCurFll;

    /**
     * if mCalculatingFrameDuration is true, it means sensor can't set/get llp/fll directly,
     * use HBlank/VBlank to calculate it.
     */
    bool mCalculatingFrameDuration;
}; //class SensorHwCtrl

/**
 * Dummy sensor hardware ctrl interface for those sensors cannot be controlled.
 */
class DummySensor : public SensorHwCtrl {
public:
    DummySensor(int cameraId) : SensorHwCtrl(cameraId, nullptr, nullptr) {}
    ~DummySensor() {}

    int setTestPatternMode(int32_t testPatternMode) { return OK; }
    int setDevice(V4L2Subdevice* pixelArraySubdev) { return OK; }
    int getPixelRate(int &pixelRate) { return OK; }
    int setExposure(const std::vector<int>& coarseExposures,
                    const std::vector<int>& fineExposures) { return OK; }
    int setAnalogGains(const std::vector<int>& analogGains) { return OK; }
    int setDigitalGains(const std::vector<int>& digitalGains) { return OK; }
    int setFrameDuration(int llp, int fll) { return OK; }
    int getFrameDuration(int &llp, int &fll) { return OK; }
    int getVBlank(int &vblank) { return OK; }
    int getActivePixelArraySize(int &width, int &height, int &code) { return OK; }
    int getExposureRange(int &exposureMin, int &exposureMax, int &exposureStep) { return OK; }
    int setFrameRate(float fps) { return OK; }
};

} // namespace icamera
