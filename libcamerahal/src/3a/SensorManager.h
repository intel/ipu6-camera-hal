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

#pragma once

#include <map>

#include "ia_aiq.h"

#include "iutils/Thread.h"
#include "CameraEventType.h"
#include "SensorHwCtrl.h"

namespace icamera {

/*
 * \struct WdrModeSetting
 *
 * This struct is used to control wdr mode switching.
 */
typedef struct {
    int64_t sequence;
    TuningMode tuningMode;
} WdrModeSetting;

typedef struct {
    unsigned short realDigitalGain;
    ia_aiq_exposure_sensor_parameters sensorParam;
} SensorExposure;

typedef struct {
    int64_t sequence;
    uint64_t timestamp;
} SofEventInfo;

typedef std::vector <SensorExposure> SensorExpGroup;
/*
 * \class SensorManager
 *
 * This class is used to control exposure and gain synchronization mechanism
 * and get some sensor info.
 */
class SensorManager {

public:
    SensorManager(int cameraId, SensorHwCtrl *sensorHw);
    ~SensorManager();
    void reset();

    void handleSofEvent(EventData eventData);
    /* sensorExposures are exposure results, applyingSeq is the sequence to apply results */
    uint32_t updateSensorExposure(SensorExpGroup sensorExposures, int64_t applyingSeq);
    int getSensorInfo(ia_aiq_frame_params &frameParams,
                      ia_aiq_exposure_sensor_descriptor &sensorDescriptor);

    // HDR_FEATURE_S
    int setWdrMode(TuningMode tuningMode, int64_t sequence);

    int setAWB(float r_per_g, float b_per_g);
    // HDR_FEATURE_E
    // CRL_MODULE_S
    int setFrameRate(float fps);
    // CRL_MODULE_E
    int getCurrentExposureAppliedDelay();
    uint64_t getSofTimestamp(int64_t sequence);
private:
    DISALLOW_COPY_AND_ASSIGN(SensorManager);

    void handleSensorExposure();
    // HDR_FEATURE_S
    void handleSensorModeSwitch(int64_t sequence);
    int convertTuningModeToWdrMode(TuningMode tuningMode);
    // HDR_FEATURE_E
    int getSensorModeData(ia_aiq_exposure_sensor_descriptor& sensorData);

private:
    static const int kMaxSensorExposures = 10;
    static const int kMaxSofEventInfo = 10;

    int mCameraId;
    SensorHwCtrl *mSensorHwCtrl;

    // HDR_FEATURE_S
    bool    mModeSwitched;         // Whether the TuningMode get updated
    WdrModeSetting mWdrModeSetting;
    // HDR_FEATURE_E

    int64_t mLastSofSequence;

    // Guard for SensorManager public API.
    Mutex mLock;

    int mAnalogGainDelay;  // Analog gain delay comparing exposure
    int mDigitalGainDelay;  // Digital gain delay comparing exposure
    // fisrt: sequence id, second: analog gain vector
    std::map<int64_t, std::vector<int>> mAnalogGainMap;
    // fisrt: sequence id, second: digital gain vector
    std::map<int64_t, std::vector<int>> mDigitalGainMap;
    typedef struct {
        std::vector<int> coarseExposures;
        std::vector<int> fineExposures;
        int lineLengthPixels;
        int frameLengthLines;
    } ExposureData;
    // first: sequence id, second: exposure data
    std::map<int64_t, ExposureData> mExposureDataMap;

    std::vector<SofEventInfo> mSofEventInfo;
};

} /* namespace icamera */
