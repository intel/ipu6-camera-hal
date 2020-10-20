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
    virtual int run3A(long * /*settingSequence*/)  { return OK; }

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
     * Return 0 if the operation succeeds, and output settingSequence to
     * indicate the frame that settings are applied.
     * settingSequence -1 means uncertain frame for this settings.
     */
    int run3A(long *settingSequence);

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
};

} /* namespace icamera */

