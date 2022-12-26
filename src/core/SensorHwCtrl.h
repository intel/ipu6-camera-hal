/*
 * Copyright (C) 2015-2021 Intel Corporation
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

#ifdef CAL_BUILD
#include <cros-camera/v4l2_device.h>
#else
#include <v4l2_device.h>
#endif

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

// CRL_MODULE_S
    virtual int configure();
// CRL_MODULE_E
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

// HDR_FEATURE_S
    /**
     * Set WDR mode to sensor which is used to select WDR sensor settings or none-WDR settings.
     * If 1 is set, WDR sensor settings will be used,
     * while if 0 is set, none-WDR sensor settings will be used.
     *
     * \param[IN] mode: WDR mode
     *
     *\return OK if successfully.
     */
    virtual int setWdrMode(int mode);

    /**
     * Set awb result to sensor for bypass awb kernel.
     *
     * \param[IN] awb result: float r_per_g, float b_per_g
     *
     *\return OK if successfully.
     */
    virtual int setAWB(float r_per_g, float b_per_g);
// HDR_FEATURE_E

// CRL_MODULE_S
    virtual int setFrameRate(float fps);
// CRL_MODULE_E
private:
    int setLineLengthPixels(int llp);
    int getLineLengthPixels(int &llp);
    int setFrameLengthLines(int fll);
    int getFrameLengthLines(int &fll);

// CRL_MODULE_S
    int setMultiExposures(const std::vector<int>& coarseExposures,
                          const std::vector<int>& fineExposures);
    int setDualExposuresDCGAndVS(const std::vector<int>& coarseExposures,
                                 const std::vector<int>& fineExposures);
    // DOL_FEATURE_S
    int setShutterAndReadoutTiming(const std::vector<int>& coarseExposures,
                                   const std::vector<int>& fineExposures);
    // DOL_FEATURE_E
    int setConversionGain(const std::vector<int>& analogGains);
    int setMultiDigitalGain(const std::vector<int>& digitalGains);
    int setMultiAnalogGain(const std::vector<int>& analogGains);
// CRL_MODULE_E

private:
    // DOL_FEATURE_S
    // DOL sensor sink pad
    static const int SENSOR_OUTPUT_PAD = 1;
    // DOL_FEATURE_E

    V4L2Subdevice* mPixelArraySubdev;
// CRL_MODULE_S
    V4L2Subdevice* mSensorOutputSubdev;
// CRL_MODULE_E
    int mCameraId;
    int mHorzBlank;
    int mVertBlank;
    int mCropWidth;
    int mCropHeight;

// HDR_FEATURE_S
    int mWdrMode;
// HDR_FEATURE_E

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
// HDR_FEATURE_S
    int setWdrMode(int mode) { return OK; }
// HDR_FEATURE_E
// CRL_MODULE_S
    int setFrameRate(float fps) { return OK; }
// CRL_MODULE_E
};

} // namespace icamera
