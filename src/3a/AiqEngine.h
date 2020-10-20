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

#pragma once

#include "AiqSetting.h"
#include "AiqCore.h"
#include "AiqResult.h"
#include "AiqStatistics.h"
#include "AiqResultStorage.h"
#include "SensorManager.h"
#include "LensManager.h"

namespace icamera {

/*
 * \class AiqEngine
 * This class is used to parse Stats, control \class AiqEngine
 * This class is used to parse Stats, control running AIQ algorithms
 * and set result to HW layer.
 * This is sub thread class.
 */
class AiqEngine {

public:
    AiqEngine(int cameraId, SensorHwCtrl *sensorHw, LensHw *lensHw, AiqSetting *setting);
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
    int configure(const std::vector<ConfigMode>& configModes);

    /**
     * \brief Calculate and set frame and sensor info, and run 3a with default setting.
     */
    int startEngine();

    /**
     * \brief Run 3a to get new 3a settings.
     * Return 0 if the operation succeeds, and output settingSequence to
     * indicate the frame that settings are applied.
     * settingSequence -1 means uncertain frame for this settings.
     */
    int run3A(long *settingSequence);

    /**
     * \brief Stop 3a thrad and LensManager.
     */
    int stopEngine();

    /**
     * \brief Get software EventListener
     */
    EventListener *getSofEventListener();

private:
    DISALLOW_COPY_AND_ASSIGN(AiqEngine);

    int saveAfGridData(const ia_aiq_af_grid* afGrid, ia_aiq_af_grid* dst);
    int saveRgbsGridData(const ia_aiq_rgbs_grid* rgbsGrid, ia_aiq_rgbs_grid* dst);

    int prepareStats(ia_aiq_statistics_input_params_v4 *statsParami,
                     ia_aiq_gbce_results *gbceResults,
                     AiqStatistics *aiqStatistics);

    void setAiqResult(AiqResult *aiqResult, bool skip);

    int getSkippingNum(AiqResult *aiqResult);

    bool needRun3A(AiqStatistics *aiqStatistics);

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

    AiqState prepareInputParam(void);
    AiqState runAiq(AiqResult *aiqResult);
    AiqState handleAiqResult(AiqResult *aiqResult);
    AiqState done(AiqResult *aiqResult);

    int run();

    // For manual ISP settings
    int applyManualTonemaps(AiqResult *aiqResult);

private:
    static const nsecs_t kWaitDuration = 1000000000; //1000ms
    static const int kMaxStatisticsDataSize = 3;
    static const int kMaxExposureAppliedDelay = 5;

private:
    int mCameraId;
    AiqResultStorage* mAiqResultStorage;
    AiqSetting *mAiqSetting;
    AiqCore *mAiqCore;
    SensorManager *mSensorManager;
    LensManager *mLensManager;
    bool mFirstAiqRunning;
    bool mFirstExposureSetting;
    bool mAiqRunningForPerframe;

    // Guard for public API of AiqEngine.
    Mutex mEngineLock;
    Condition mStatsAvailableSignal;

    uint32_t mCurrentStatsSequence;

    const ia_aiq_rgbs_grid* mRgbsGridArray[MAX_EXPOSURES_NUM];
    const ia_aiq_af_grid* mAfGridArray[MAX_EXPOSURES_NUM];

    aiq_parameter_t mAiqParam;

    long m3ACadenceSequence;
    long mLastStatsSequence;
};

} /* namespace icamera */

