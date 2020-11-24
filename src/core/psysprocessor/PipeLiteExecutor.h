/*
 * Copyright (C) 2019-2020 Intel Corporation
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
#include <vector>
#include <memory>
#include <string>
#include <utility> // For std::pair, std::make_pair

#include "Parameters.h"
#include "CameraBuffer.h"
#include "BufferQueue.h"
#include "psysprocessor/PGCommon.h"
#include "PolicyManager.h"
#include "ShareReferBufferPool.h"
#include "IspParamAdaptor.h"
#include "GraphConfig.h"

namespace icamera {

class PSysDAG;

typedef std::map<Port, std::shared_ptr<CameraBuffer>> CameraBufferPortMap;

class PipeLiteExecutor : public BufferQueue {
public:
    PipeLiteExecutor(int cameraId, const ExecutorPolicy &policy,
                     std::vector<std::string> exclusivePGs,
                     PSysDAG *psysDag, std::shared_ptr<IGraphConfig> gc);
    virtual ~PipeLiteExecutor();
    virtual int start();
    virtual void stop();
    virtual int initPipe();
    void notifyStop();

    virtual bool fetchTnrOutBuffer(int64_t seq, std::shared_ptr<CameraBuffer> buf) { return false; }
    int releaseStatsBuffer(const std::shared_ptr<CameraBuffer> &statsBuf);

    void setStreamId(int streamId) { mStreamId = streamId; }
    void setIspParamAdaptor(IspParamAdaptor* adaptor) { mAdaptor = adaptor; }
    void setPolicyManager(PolicyManager* policyManager) { mPolicyManager = policyManager; }
    void setNotifyPolicy(ExecutorNotifyPolicy notifyPolicy) { mNotifyPolicy = notifyPolicy; }
    void setShareReferPool(std::shared_ptr<ShareReferBufferPool> referPool) {
            mShareReferPool = referPool;
    }

    void getOutputTerminalPorts(std::map<ia_uid, Port>& outputTerminals) const;
    void getInputTerminalPorts(std::map<ia_uid, Port>& terminals) const;
    bool hasOutputTerminal(ia_uid sinkTerminal);

    // Link output terminals of producer to its input terminals
    int setInputTerminals(const std::map<ia_uid, Port>& sourceTerminals);
    int registerOutBuffers(Port port, const std::shared_ptr<CameraBuffer> &camBuffer);
    int registerInBuffers(Port port, const std::shared_ptr<CameraBuffer> &inBuf);

    /**
     * Check if the two given stream configs are the same.
     */
    bool isSameStreamConfig(const stream_t& internal, const stream_t& external,
                            ConfigMode configMode, bool checkStreamId) const;

    bool isInputEdge() { return mIsInputEdge; }
    bool isOutputEdge() { return mIsOutputEdge; }

    const char* getName() const { return mName.c_str(); }

 private:
    DISALLOW_COPY_AND_ASSIGN(PipeLiteExecutor);

 protected:
    struct TerminalDescriptor{
        ia_uid terminal;
        ia_uid stageId;

        ia_uid sourceTerminal;
        ia_uid sinkTerminal;
        ia_uid sourceStage;
        ia_uid sinkStage;

        FrameInfo frameDesc;

        bool enabled;
        bool hasConnection; // has related sink or source
                            // expection: sis output, sink = source
        Port assignedPort;  // INVALID_PORT for terminal without connection
        int usrStreamId;
    };

    struct ExecutorUnit {
        // Initialized during creation/configuration
        int pgId;
        ia_uid stageId;
        std::shared_ptr<PGCommon> pg;
        std::vector<ia_uid> statKernelUids;
        std::vector<ia_uid> sisKernelUids;

        // Initialized during connection analysis
        std::vector<ia_uid> inputTerminals; // including disabled terminals
        std::vector<ia_uid> outputTerminals;

        // Initialized during buffer allocation
        std::map<ia_uid, std::shared_ptr<CameraBuffer>> inputBuffers;
        std::map<ia_uid, std::shared_ptr<CameraBuffer>> outputBuffers;
    };

 protected:
    int analyzeConnections(const std::vector<IGraphType::PipelineConnection>& connVector);
    int assignDefaultPortsForTerminals();
    int notifyFrameDone(const v4l2_buffer_t& inV4l2Buf, const CameraBufferPortMap& outBuf);

    /**
     * Check if there is any valid buffer(not null) in the given port/buffer pairs.
     */
    bool hasValidBuffers(const CameraBufferPortMap& buffers);
    void dumpPGs() const;

 private:
    int processNewFrame();
    int runPipe(std::map<Port, std::shared_ptr<CameraBuffer>> &inBuffers,
                std::map<Port, std::shared_ptr<CameraBuffer>> &outBuffers,
                std::vector<std::shared_ptr<CameraBuffer>> &outStatsBuffers,
                std::vector<EventType> &eventType);

    int notifyStatsDone(TuningMode tuningMode, const v4l2_buffer_t& inV4l2Buf,
                        const std::vector<std::shared_ptr<CameraBuffer>> &outStatsBuffers,
                        const std::vector<EventType> &eventType);

    int createPGs();
    int configurePGs();
    int allocBuffers();
    void releaseBuffers();
    int storeTerminalInfo(const IGraphType::PipelineConnection& connection);
    int getStatKernels(int pgId, std::vector<ia_uid>& kernels);
    int getSisKernels(int pgId, std::vector<ia_uid>& kernels);
    ExecutorUnit* findPGExecutor(ia_uid stageId);
    void getDisabledTerminalsForPG(ia_uid stageId, std::vector<ia_uid>& terminals) const;
    void getTerminalFrameInfos(const std::vector<ia_uid>& terminals,
                               std::map<ia_uid, FrameInfo>& infos) const;
    void getTerminalPorts(const std::vector<ia_uid>& terminals,
                          std:: map<ia_uid, Port>& terminalPortMap) const;
    void getTerminalBuffersFromExternal(
                        const std::vector<ia_uid>& terminals,
                        const std::map<Port, std::shared_ptr<CameraBuffer> >& externals,
                        std::map<ia_uid, std::shared_ptr<CameraBuffer> >& internals) const;

    int handleSisStats(std::map<ia_uid, std::shared_ptr<CameraBuffer>>& frameBuffers,
                       const std::shared_ptr<CameraBuffer> &outStatsBuffers);

 protected:
    int mCameraId;
    int mStreamId;
    std::string mName;
    std::vector<std::string> mPGNames;
    std::vector<int> mOpModes;
    std::shared_ptr<IGraphConfig> mGraphConfig;
    bool mIsInputEdge;
    bool mIsOutputEdge;
    ExecutorNotifyPolicy mNotifyPolicy;

    std::vector<ExecutorUnit> mPGExecutors;
    IspParamAdaptor* mAdaptor;

    PolicyManager* mPolicyManager;
    std::shared_ptr<ShareReferBufferPool> mShareReferPool;

    // For internal connections (between PGs)
    std::map<ia_uid, std::shared_ptr<CameraBuffer> > mPGBuffers; // Buffers between PGs
    std::map<ia_uid, ia_uid> mConnectionMap; // <sink, source>
    std::map<ia_uid, TerminalDescriptor> mTerminalsDesc;

    int64_t mLastStatsSequence;
    CameraBufQ mStatsBuffers;
    Mutex mStatsBuffersLock;
    std::vector<std::string> mExclusivePGs;
    PSysDAG *mPSysDag;

    CameraBufferPortMap mInternalOutputBuffers;
    int mkernelsCountWithStats;
};

typedef PipeLiteExecutor PipeExecutor;
}
