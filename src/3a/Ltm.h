/*
 * Copyright (C) 2016-2020 Intel Corporation.
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

#include <memory>
#include <queue>

#ifdef ENABLE_SANDBOXING
#include "modules/sandboxing/client/IntelCca.h"
#else
#include "modules/algowrapper/IntelCca.h"
#endif

#include "AiqResult.h"
#include "AiqSetting.h"
#include "CameraEvent.h"
#include "iutils/CameraDump.h"
#include "iutils/Thread.h"

namespace icamera {

struct LtmInputParams {
    cca::cca_ltm_input_params ltmParams;
    long sequence;

    LtmInputParams() {
        CLEAR(ltmParams);
        sequence = -1;
    }
};

/**
 * \class Ltm
 * This class is used to run Local tone mapping (Ltm) algorithm.
 */
class Ltm : public EventListener {
 public:
    Ltm(int cameraId);
    ~Ltm();

    int init();
    int deinit();
    int start();
    void stop();

    int configure(const std::vector<ConfigMode>& configModes);

    /**
     * \brief handle statistics event
     */
    void handleEvent(EventData eventData);
    int handleSisLtm(const std::shared_ptr<CameraBuffer>& cameraBuffer);

 private:
    DISALLOW_COPY_AND_ASSIGN(Ltm);

    int runLtmAsync();
    int runLtm(const LtmInputParams& ltmInputParams);

    AiqResult* getAiqResult(long sequence);

 private:
    /**
     * \brief The ltm thread
     */
    class LtmThread : public Thread {
        Ltm* mLtmHandle;

     public:
        LtmThread(Ltm* pThis) : mLtmHandle(pThis) {}

        virtual bool threadLoop() {
            int ret = mLtmHandle->runLtmAsync();
            return (ret == 0);
        }
    };

    int mCameraId;
    TuningMode mTuningMode;

    enum LtmState { LTM_NOT_INIT = 0, LTM_INIT, LTM_CONFIGURED, LTM_MAX } mLtmState;

    // Use it to lock mInputParamIndex and mLtmParams
    // And use it for thread condition lock
    Mutex mLtmLock;
    LtmThread* mLtmThread;
    bool mThreadRunning;
    Condition mParamAvailableSignal;
    static const nsecs_t kWaitDuration = 2000000000;  // 2000ms
    static const int kMaxLtmParamsNum = 2;            // 2 ltm input params

    int mInputParamIndex;
    LtmInputParams* mLtmParams[kMaxLtmParamsNum];
    std::queue<LtmInputParams*> mLtmParamsQ;
};

} /* namespace icamera */
