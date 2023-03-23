/*
 * Copyright (C) 2017-2023 Intel Corporation.
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

#include <set>
#include <queue>

#include "BufferQueue.h"
#include "iutils/RWLock.h"

#include "IspSettings.h"
#include "psysprocessor/PSysDAG.h"
#include "CameraScheduler.h"

namespace icamera {

class ParameterGenerator;
class PSysDAG;

typedef std::map<Port, std::shared_ptr<CameraBuffer>> CameraBufferPortMap;
typedef std::map<ConfigMode, std::unique_ptr<PSysDAG>> PSysDAGConfigModeMap;

/**
 * PSysProcessor runs the Image Process Algorithm in the PSYS.
 * It implements the BufferConsumer and BufferProducer Interface
 */
class PSysProcessor : public BufferQueue, public PSysDagCallback {
 public:
    PSysProcessor(int cameraId, ParameterGenerator* pGenerator);
    virtual ~PSysProcessor();
    virtual int configure(const std::vector<ConfigMode>& configModes);
    virtual int setParameters(const Parameters& param);
    virtual int getParameters(Parameters& param);

    virtual int registerUserOutputBufs(Port port, const std::shared_ptr<CameraBuffer>& camBuffer);
    virtual void stopProcessing();

    // Overwrite event source API to delegate related functions
    void registerListener(EventType eventType, EventListener* eventListener);
    void removeListener(EventType eventType, EventListener* eventListener);

    virtual int start();
    virtual void stop();

    // Overwrite PSysDagCallback API, used for returning back buffers from PSysDAG.
    void onFrameDone(const PSysTaskData& result);
    void onBufferDone(int64_t sequence, Port port, const std::shared_ptr<CameraBuffer>& camBuffer);
    void onStatsDone(int64_t sequence, const CameraBufferPortMap& outBuf);
// INTEL_DVS_S
    void onDvsPrepare(int32_t streamId);
// INTEL_DVS_E

 private:
    DISALLOW_COPY_AND_ASSIGN(PSysProcessor);

 private:
    int processNewFrame();
    std::shared_ptr<CameraBuffer> allocStatsBuffer(int index);

    void prepareIpuForNextFrame(int64_t sequence);
    status_t prepareTask(CameraBufferPortMap* srcBuffers, CameraBufferPortMap* dstBuffers);
    void dispatchTask(CameraBufferPortMap& inBuf, CameraBufferPortMap& outBuf,
                      bool fakeTask = false, bool callbackRgbs = false);

    void handleEvent(EventData eventData);
    // DOL_FEATURE_S
    int setVbpToIspParam(int64_t sequence, timeval timestamp);
    // DOL_FEATURE_E

    int64_t getSettingSequence(const CameraBufferPortMap& outBuf);
    bool needSkipOutputFrame(int64_t sequence);
    bool needExecutePipe(int64_t settingSequence, int64_t inputSequence);
    bool needHoldOnInputFrame(int64_t settingSequence, int64_t inputSequence);
    bool needSwitchPipe(int64_t sequence);

    // ISP_CONTROL_S
    int allocPalControlBuffers();
    size_t getRequiredPalBufferSize();
    int fillPalOverrideData(const Parameters& param);
    int fillDefaultAcmData(uint8_t* overrideData);
    // ISP_CONTROL_E
    void outputRawImage(std::shared_ptr<CameraBuffer>& srcBuf,
                        std::shared_ptr<CameraBuffer>& dstBuf);

    void handleRawReprocessing(CameraBufferPortMap* srcBuffers, CameraBufferPortMap* dstBuffers,
                               bool* allBufDone, bool* hasRawOutput, bool* hasRawInput);
    bool isBufferHoldForRawReprocess(int64_t sequence);
    void saveRawBuffer(CameraBufferPortMap* srcBuffers);
    void returnRawBuffer();
    void handleStillPipeForTnr(int64_t sequence, CameraBufferPortMap* dstBuffers);
    void sendPsysFrameDoneEvent(const CameraBufferPortMap* dstBuffers);
    void sendPsysRequestEvent(const CameraBufferPortMap* dstBuffers, int64_t sequence,
                              uint64_t timestamp, EventType eventType);

 private:
    int mCameraId;
    static const nsecs_t kWaitDuration = 1000000000;  // 1000ms
    ParameterGenerator* mParameterGenerator;
    CameraScheduler* mScheduler;
    bool mRunAicAfterQTask;

    IspSettings mIspSettings;
    RWLock mIspSettingsLock;

    // Since the isp settings may be re-used in all modes, so the buffer size of
    // isp settings should be equal to frame buffer size.
    static const int IA_PAL_CONTROL_BUFFER_SIZE = 10;

    // ISP_CONTROL_S
    // Use mUpdatedIspIndex to select the buffer to store the updated param
    // and use mUsedIspIndex to select the buffer to set isp control.
    int mUpdatedIspIndex;
    int mUsedIspIndex;
    ia_binary_data mPalCtrlBuffers[IA_PAL_CONTROL_BUFFER_SIZE];
    // ISP_CONTROL_E

    Condition mFrameDoneSignal;
    // Save the sequences which are being processed.
    std::multiset<int64_t> mSequencesInflight;

    std::vector<ConfigMode> mConfigModes;
    PSysDAGConfigModeMap mPSysDAGs;
    // Active config mode and tuning mode
    ConfigMode mCurConfigMode;
    TuningMode mTuningMode;

    std::queue<EventDataMeta> mMetaQueue;
    // Guard for the metadata queue
    Mutex mMetaQueueLock;
    Condition mMetaAvailableSignal;

    Port mRawPort;

    // variables for sof alignment
    timeval mSofTimestamp;
    Mutex mSofLock;
    Condition mSofCondition;
    int64_t mSofSequence;

    // variables for opaque raw
    Port mOpaqueRawPort;
    std::mutex mBufferMapLock;
    // hold RAW buffers for raw reprocessing or GPU still TNR
    bool mHoldRawBuffers;
    std::map<int64_t, CameraBufferPortMap> mRawBufferMap;
    // Indicate the latest sequence of raw buffer used in still TNR
    int64_t mLastStillTnrSequence;

    enum { PIPELINE_UNCREATED = 0, PIPELINE_CREATED } mStatus;
};  // End of class PSysProcessor

}  // namespace icamera
