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

#ifdef ENABLE_SANDBOXING
#include "modules/sandboxing/client/IntelCcaClient.h"
#else
#include "modules/algowrapper/IntelCca.h"
#endif

#include "CameraEvent.h"

#include "AiqSetting.h"
#include "AiqEngine.h"
// INTEL_DVS_S
#include "Dvs.h"
// INTEL_DVS_E
// LOCAL_TONEMAP_S
#include "Ltm.h"
// LOCAL_TONEMAP_E

namespace icamera {

class SensorHwCtrl;
class LensHw;

/*
 * \class AiqUnit
 * This class is used for upper layer to control 3a engine.
 */

class AiqUnitBase{

public:
    AiqUnitBase() {}
    virtual ~AiqUnitBase() {}

    virtual int init() { return OK; }
    virtual int deinit() { return OK; }
    virtual int configure(const stream_config_t * /*streamList*/) { return OK; }
    virtual int start() { return OK; }
    virtual int stop() { return OK; }
    virtual int run3A(long request, int64_t applyingSeq, int64_t * /*effectSeq*/)  { return OK; }

    virtual std::vector<EventListener*> getSofEventListener()
    {
        std::vector<EventListener*> eventListenerList;
        return eventListenerList;
    }
    virtual std::vector<EventListener*> getStatsEventListener()
    {
        std::vector<EventListener*> eventListenerList;
        return eventListenerList;
    }

    virtual int setParameters(const Parameters & /*params*/) { return OK; }

private:
    DISALLOW_COPY_AND_ASSIGN(AiqUnitBase);

};

class AiqUnit : public AiqUnitBase {

public:
    AiqUnit(int cameraId, SensorHwCtrl *sensorHw, LensHw *lensHw);
    ~AiqUnit();

    /**
     * \brief Init 3a related objects
     */
    int init();

    /**
     * \brief Deinit 3a related objects
     */
    int deinit();

    /**
     * \brief configure 3a engine with stream configuration
     */
    int configure(const stream_config_t *streamList);

    /**
     * \brief Start 3a Engine
     */
    int start();

    /**
     * \brief Stop 3a Engine
     */
    int stop();

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
     * \brief Get software EventListener
     */
    std::vector<EventListener*> getSofEventListener();

    /**
     * \brief Get stats EventListener
     */
    std::vector<EventListener*> getStatsEventListener();

    /**
     * \brief Set 3A Parameters
     *
     * \param params: the Parameters update to 3A
     */
    int setParameters(const Parameters &params);

private:
    DISALLOW_COPY_AND_ASSIGN(AiqUnit);

private:
    int initIntelCcaHandle(const std::vector<ConfigMode> &configModes);
    void deinitIntelCcaHandle();
    void dumpCcaInitParam(const cca::cca_init_params params);

private:
    int mCameraId;
    // LOCAL_TONEMAP_S
    Ltm *mLtm;
    // LOCAL_TONEMAP_E
    enum AiqUnitState {
        AIQ_UNIT_NOT_INIT = 0,
        AIQ_UNIT_INIT,
        AIQ_UNIT_CONFIGURED,
        AIQ_UNIT_START,
        AIQ_UNIT_STOP,
        AIQ_UNIT_MAX
    } mAiqUnitState;

    // INTEL_DVS_S
    Dvs *mDvs;
    // INTEL_DVS_E
    AiqEngine *mAiqEngine;
    AiqSetting *mAiqSetting;

    // Guard for AiqUnit public API.
    Mutex mAiqUnitLock;

    std::vector<TuningMode> mTuningModes;
    bool mCcaInitialized;
    size_t mActiveStreamCount;
};

} /* namespace icamera */

