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

#define LOG_TAG "AiqUnit"

#include "iutils/Errors.h"
#include "iutils/CameraLog.h"

#include "AiqUnit.h"

namespace icamera {

AiqUnit::AiqUnit(int cameraId, SensorHwCtrl *sensorHw, LensHw *lensHw) :
    mCameraId(cameraId),
    // LOCAL_TONEMAP_S
    mLtm(nullptr),
    // LOCAL_TONEMAP_E
    mAiqUnitState(AIQ_UNIT_NOT_INIT)
{
    LOG1("@%s mCameraId = %d", __func__, mCameraId);

    mAiqSetting = new AiqSetting(cameraId);

    mAiqEngine = new AiqEngine(cameraId, sensorHw, lensHw, mAiqSetting);

    // INTEL_DVS_S
    mDvs = new Dvs(cameraId, mAiqSetting);
    // INTEL_DVS_E
    // LOCAL_TONEMAP_S
    if (PlatformData::isLtmEnabled(mCameraId)) {
        mLtm = new Ltm(cameraId);
    }
    // LOCAL_TONEMAP_E
}

AiqUnit::~AiqUnit()
{
    LOG1("@%s mCameraId = %d", __func__, mCameraId);

    if (mAiqUnitState == AIQ_UNIT_START) {
        stop();
    }

    if (mAiqUnitState == AIQ_UNIT_INIT) {
        deinit();
    }

    // LOCAL_TONEMAP_S
    delete mLtm;
    // LOCAL_TONEMAP_E
    // INTEL_DVS_S
    delete mDvs;
    // INTEL_DVS_E
    delete mAiqEngine;
    delete mAiqSetting;
}

int AiqUnit::init()
{
    AutoMutex l(mAiqUnitLock);
    LOG1("@%s mCameraId = %d", __func__, mCameraId);

    int ret = mAiqSetting->init();
    if (ret != OK) {
        mAiqSetting->deinit();
        return ret;
    }

    if (mAiqUnitState == AIQ_UNIT_NOT_INIT) {
        ret = mAiqEngine->init();
        if (ret != OK) {
            mAiqEngine->deinit();
            return ret;
        }

        // INTEL_DVS_S
        mDvs->init();
        // INTEL_DVS_E
        // LOCAL_TONEMAP_S
        if (mLtm) {
            mLtm->init();
        }
        // LOCAL_TONEMAP_E
    }

    mAiqUnitState = AIQ_UNIT_INIT;

    return OK;
}

int AiqUnit::deinit()
{
    AutoMutex l(mAiqUnitLock);
    LOG1("@%s mCameraId = %d", __func__, mCameraId);

    // LOCAL_TONEMAP_S
    if (mLtm) {
        mLtm->deinit();
    }
    // LOCAL_TONEMAP_E
    // INTEL_DVS_S
    mDvs->deinit();
    // INTEL_DVS_E
    mAiqEngine->deinit();

    mAiqSetting->deinit();

    mAiqUnitState = AIQ_UNIT_NOT_INIT;

    return OK;
}

int AiqUnit::configure(const stream_config_t *streamList)
{
    CheckError(streamList == nullptr, BAD_VALUE, "streamList is nullptr");

    AutoMutex l(mAiqUnitLock);
    LOG1("@%s mCameraId = %d", __func__, mCameraId);

    if (mAiqUnitState != AIQ_UNIT_INIT && mAiqUnitState != AIQ_UNIT_STOP) {
        LOGW("%s: configure in wrong state: %d", __func__, mAiqUnitState);
        return BAD_VALUE;
    }

    std::vector<ConfigMode> configModes;
    PlatformData::getConfigModesByOperationMode(mCameraId, streamList->operation_mode, configModes);

    int ret = mAiqSetting->configure(streamList);
    CheckError(ret != OK, ret, "configure AIQ settings error: %d", ret);

    ret = mAiqEngine->configure(configModes);
    CheckError(ret != OK, ret, "configure AIQ engine error: %d", ret);
    // INTEL_DVS_S
    ret = mDvs->configure(configModes);
    CheckError(ret != OK, ret, "configure DVS engine error: %d", ret);
    // INTEL_DVS_E
    // LOCAL_TONEMAP_S
    if (mLtm) {
        ret = mLtm->configure(configModes);
        CheckError(ret != OK, ret, "configure LTM engine error: %d", ret);
    }
    // LOCAL_TONEMAP_E

    mAiqUnitState = AIQ_UNIT_CONFIGURED;
    return OK;
}

int AiqUnit::start()
{
    AutoMutex l(mAiqUnitLock);
    LOG1("@%s mCameraId = %d", __func__, mCameraId);

    if (mAiqUnitState != AIQ_UNIT_CONFIGURED && mAiqUnitState != AIQ_UNIT_STOP) {
        LOGW("%s: configure in wrong state: %d", __func__, mAiqUnitState);
        return BAD_VALUE;
    }

    // LOCAL_TONEMAP_S
    if (mLtm) {
        mLtm->start();
    }
    // LOCAL_TONEMAP_E
    int ret = mAiqEngine->startEngine();
    if (ret == OK) {
        mAiqUnitState = AIQ_UNIT_START;
    }

    return OK;
}

int AiqUnit::stop()
{
    AutoMutex l(mAiqUnitLock);
    LOG1("@%s mCameraId = %d", __func__, mCameraId);

    if (mAiqUnitState == AIQ_UNIT_START) {
        mAiqEngine->stopEngine();
        // LOCAL_TONEMAP_S
        if (mLtm) {
            mLtm->stop();
        }
        // LOCAL_TONEMAP_E
    }

    mAiqUnitState = AIQ_UNIT_STOP;

    return OK;
}

int AiqUnit::run3A(long *settingSequence)
{
    AutoMutex l(mAiqUnitLock);
    TRACE_LOG_PROCESS("AiqUnit", "run3A");

    if (settingSequence)
       *settingSequence = -1;

    if (mAiqUnitState != AIQ_UNIT_START) {
        LOGW("%s: AIQ is not started: %d", __func__, mAiqUnitState);
        return BAD_VALUE;
    }

    int ret = mAiqEngine->run3A(settingSequence);
    CheckError(ret != OK, ret, "run 3A failed.");

    return OK;
}

std::vector<EventListener*> AiqUnit::getSofEventListener()
{
    AutoMutex l(mAiqUnitLock);
    LOG1("@%s mCameraId = %d", __func__, mCameraId);

    std::vector<EventListener*> eventListenerList;
    eventListenerList.push_back(mAiqEngine->getSofEventListener());
    return eventListenerList;
}

std::vector<EventListener*> AiqUnit::getStatsEventListener()
{
    AutoMutex l(mAiqUnitLock);
    LOG1("@%s mCameraId = %d", __func__, mCameraId);

    std::vector<EventListener*> eventListenerList;
    // LOCAL_TONEMAP_S
    if (mLtm) {
        eventListenerList.push_back(mLtm);
    }
    // LOCAL_TONEMAP_E
    // INTEL_DVS_S
    eventListenerList.push_back(mDvs);
    // INTEL_DVS_E
    return eventListenerList;
}

int AiqUnit::setParameters(const Parameters &params)
{
    AutoMutex l(mAiqUnitLock);
    LOG1("@%s mCameraId = %d", __func__, mCameraId);

    return mAiqSetting->setParameters(params);
}

} /* namespace icamera */
