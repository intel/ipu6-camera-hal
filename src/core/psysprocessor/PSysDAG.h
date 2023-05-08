/*
 * Copyright (C) 2017-2023 Intel Corporation
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
#include <set>
#include <unordered_map>

#include "CameraBuffer.h"
#include "CameraScheduler.h"
#include "IspParamAdaptor.h"
#include "Parameters.h"
#include "PlatformData.h"
#ifdef USE_PG_LITE_PIPE
#include "PipeLiteExecutor.h"
#else
#include "PipeExecutor.h"
#endif
#include "PolicyManager.h"

namespace icamera {

/**
 * Encapsulation of all parameters needed by PSysExecutor to run PSYS pipeline.
 */
struct PSysTaskData {
    IspSettings mIspSettings;
    TuningMode mTuningMode;
    bool mFakeTask;
    bool mCallbackRgbs;

    CameraBufferPortMap mInputBuffers;
    CameraBufferPortMap mOutputBuffers;
    PSysTaskData() {
        mTuningMode = TUNING_MODE_MAX;
        mFakeTask = false;
        mCallbackRgbs = false;
    }
};

// Used to save all on-processing tasks.
struct TaskInfo {
    TaskInfo() : mNumOfValidBuffers(0), mNumOfReturnedBuffers(0) {}
    PSysTaskData mTaskData;
    int mNumOfValidBuffers;
    int mNumOfReturnedBuffers;
};

class PSysDagCallback {
 public:
    PSysDagCallback() {}
    virtual ~PSysDagCallback() {}
    virtual void onFrameDone(const PSysTaskData& result) {}
    virtual void onBufferDone(int64_t sequence, Port port,
                              const std::shared_ptr<CameraBuffer>& camBuffer) {}
    virtual void onStatsDone(int64_t sequence, const CameraBufferPortMap& outBuf) {}
// INTEL_DVS_S
    virtual void onDvsPrepare(int32_t streamId) {}
// INTEL_DVS_E
};

class PSysDAG {
 public:
    PSysDAG(int cameraId, CameraScheduler* scheduler, PSysDagCallback* psysDagCB);
    virtual ~PSysDAG();
    void setFrameInfo(const std::map<Port, stream_t>& inputInfo,
                      const std::map<Port, stream_t>& outputInfo);
    int configure(ConfigMode configMode, TuningMode tuningMode, bool useTnrOutBuffer);
    int start();
    int stop();

    int resume();
    int pause();

    void unregisterNode();

    int registerInternalBufs(std::map<Port, CameraBufVector>& internalBufs);
    int registerUserOutputBufs(Port port, const std::shared_ptr<CameraBuffer>& camBuffer);
    void stopProcessing();

    void addTask(PSysTaskData taskParam);

    void registerListener(EventType eventType, EventListener* eventListener);
    void removeListener(EventType eventType, EventListener* eventListener);

    TuningMode getTuningMode(int64_t sequence);
    int prepareIpuParams(int64_t sequence, bool forceUpdate = false, TaskInfo* task = nullptr,
                         bool runNext = false);

    bool fetchTnrOutBuffer(int64_t seq, std::shared_ptr<CameraBuffer> buf);
    bool isBypassStillTnr(int64_t seq);
    int getTnrExtraFrameCount(int64_t seq);

    /**
     * Use to handle the frame done event from the executors.
     */
    int onFrameDone(Port port, const std::shared_ptr<CameraBuffer>& buffer);
    void onStatsDone(int64_t sequence);

 private:
    DISALLOW_COPY_AND_ASSIGN(PSysDAG);

    void tuningReconfig(TuningMode newTuningMode);

    int createPipeExecutors(bool useTnrOutBuffer);
    int linkAndConfigExecutors();
    int bindExternalPortsToExecutor();
    void releasePipeExecutors();
#ifdef USE_PG_LITE_PIPE
    void configShareReferPool(std::shared_ptr<IGraphConfig> gc);
#endif
    PipeExecutor* findExecutorProducer(PipeExecutor* consumer);
    status_t searchStreamIdsForOutputPort(PipeExecutor* executor, Port port);

    int queueBuffers(const PSysTaskData& task);
    int returnBuffers(PSysTaskData& result);
    bool isInactiveStillStream(int streamId, const PSysTaskData* task, Port port);
    int getActiveStreamIds(const PSysTaskData& taskData, std::vector<int32_t>* activeStreamIds);

    void dumpExternalPortMap();

 private:
    int mCameraId;
    CameraScheduler* mScheduler;
    PSysDagCallback* mPSysDagCB;  // Used to callback notify frame done handling
    PolicyManager* mPolicyManager;
    ConfigMode mConfigMode;  // It is actually real config mode.
    TuningMode mTuningMode;
    IspParamAdaptor* mIspParamAdaptor;
#ifdef USE_PG_LITE_PIPE
    std::shared_ptr<ShareReferBufferPool> mShareReferPool;
#endif

    std::map<Port, stream_t> mInputFrameInfo;
    std::map<Port, stream_t> mOutputFrameInfo;
    Port mDefaultMainInputPort;

    std::vector<PipeExecutor*> mExecutorsPool;
    std::unordered_map<PipeExecutor*, int32_t> mExecutorStreamId;
    std::map<Port, std::vector<int32_t>> mOutputPortToStreamIds;
    PipeExecutor* mVideoTnrExecutor;
    PipeExecutor* mStillTnrExecutor;
    PipeExecutor* mStillExecutor;

    // A lock for protecting task data from being accessed by different threads.
    Mutex mTaskLock;
    std::vector<TaskInfo> mOngoingTasks;

    Mutex mOngoingPalMapLock;
    // first is sequence id, second is a set of stream id
    std::map<int64_t, std::set<int32_t>> mOngoingPalMap;

    /**
     * The relationship mapping between DAG's port and executors port.
     */
    struct PortMapping {
        PortMapping() : mExecutor(nullptr), mDagPort(INVALID_PORT), mExecutorPort(INVALID_PORT) {}
        PipeExecutor* mExecutor;
        Port mDagPort;
        Port mExecutorPort;
    };

    std::vector<PortMapping> mInputMaps;
    std::vector<PortMapping> mOutputMaps;
};
}  // namespace icamera
