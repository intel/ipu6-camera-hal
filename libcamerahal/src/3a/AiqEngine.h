/*
 * Copyright (C) 2015-2022 Intel Corporation.
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

#include "AiqCore.h"
#include "AiqResult.h"
#include "AiqResultStorage.h"
#include "AiqSetting.h"
#include "AiqStatistics.h"
#include "CameraEvent.h"
#include "LensManager.h"
#include "ParameterGenerator.h"
#include "SensorManager.h"

namespace icamera {

/*
 * \class AiqEngine
 * This class is used to parse Stats, control \class AiqEngine
 * This class is used to parse Stats, control running AIQ algorithms
 * and set result to HW layer.
 * This is sub thread class.
 */
class AiqEngine : public EventListener {
 public:
    AiqEngine(int cameraId, SensorHwCtrl* sensorHw, LensHw* lensHw, AiqSetting* setting);
    ~AiqEngine();

    /**
     * \brief Init AiqResult, AiqCore and SensorManager
     */
    int init();

    /**
     * \brief Deinit AiqResult, AiqCore and SensorManager
     */
    int deinit();

    /**
     * \brief configure with ConfigMode
     */
    int configure();

    /**
     * \brief Calculate and set frame and sensor info, and run 3a with default setting.
     */
    int startEngine();

    /**
     * \brief Run 3a to get new 3a settings.
     *
     * requestId: unique request id set by RequestThread;
     * applyingSeq: sequence id indicates which SOF sequence to set the settings,
     *             -1 means no target sequence to set the settings;
     * effectSeq: sequence id is an output parameter and indicates the settings is taken effect
     *            on the frame.
     *
     * Return 0 if the operation succeeds.
     */
    int run3A(long requestId, int64_t applyingSeq, int64_t* effectSeq);

    /**
     * \brief Stop 3a thrad and LensManager.
     */
    int stopEngine();

    /**
     * \brief Get SOF EventListener
     */
    EventListener* getSofEventListener();

    /**
     * \brief handle event
     */
    void handleEvent(EventData eventData);

    int prepareStatsParams(cca::cca_stats_params* statsParams, AiqStatistics* aiqStatistics,
                           AiqResult* aiqResult);

    // Handle AIQ results except Exposure results which are handled in setSensorExposure
    void setAiqResult(AiqResult* aiqResult, bool skip);
    void setSensorExposure(AiqResult* aiqResult, int64_t applyingSeq = -1);

    int getSkippingNum(AiqResult* aiqResult);

    bool needRun3A(AiqStatistics* aiqStatistics, long requestId);

    enum AiqState {
        AIQ_STATE_IDLE = 0,
        AIQ_STATE_WAIT,
        AIQ_STATE_INPUT_PREPARE,
        AIQ_STATE_RUN,
        AIQ_STATE_RESULT_SET,
        AIQ_STATE_DONE,
        AIQ_STATE_ERROR,
        AIQ_STATE_MAX
    };

    AiqState prepareInputParam(AiqStatistics* aiqStats, AiqResult* aiqResult);
    AiqState runAiq(long requestId, int64_t applyingSeq, AiqResult* aiqResult, bool* aiqRun);
    AiqState handleAiqResult(AiqResult* aiqResult);
    AiqState done(AiqResult* aiqResult);

    int run();

    // For manual ISP settings
    int applyManualTonemaps(AiqResult* aiqResult);

 private:
    static const int kMaxExposureAppliedDelay = 5;

 private:
    int mCameraId;
    AiqResultStorage* mAiqResultStorage;
    AiqSetting* mAiqSetting;
    AiqCore* mAiqCore;
    SensorManager* mSensorManager;
    LensManager* mLensManager;

    int mRun3ACadence;
    bool mFirstAiqRunning;
    bool mAiqRunningForPerframe;

    // Guard for public API of AiqEngine.
    Mutex mEngineLock;

    struct AiqRunningHistory {
        AiqResult* aiqResult;
        long requestId;
        int64_t statsSequnce;
    };
    AiqRunningHistory mAiqRunningHistory;

 private:
    DISALLOW_COPY_AND_ASSIGN(AiqEngine);
};

} /* namespace icamera */
