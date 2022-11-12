/*
 * Copyright (C) 2019-2022 Intel Corporation
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

#define LOG_TAG PipeLiteExecutor

#include "PipeLiteExecutor.h"

#include <algorithm>

#include "PSysDAG.h"
// FRAME_SYNC_S
#include "SyncManager.h"
// FRAME_SYNC_E
#include "iutils/CameraDump.h"

// CIPF backends
extern "C" {
#include <ia_cipf_css/ia_cipf_css.h>
#include <ia_pal_types_isp_ids_autogen.h>
}

using std::map;
using std::shared_ptr;
using std::string;
using std::vector;

namespace icamera {

static const int32_t sStatKernels[] = {ia_pal_uuid_isp_bxt_awbstatistics,
                                       ia_pal_uuid_isp_awbstatistics_2_0,
                                       ia_pal_uuid_isp_bxt_dvsstatistics};

static const int32_t sSisKernels[] = {ia_pal_uuid_isp_sis_1_0_a};

PipeLiteExecutor::PipeLiteExecutor(int cameraId, const ExecutorPolicy& policy,
                                   vector<string> exclusivePGs, PSysDAG* psysDag,
                                   shared_ptr<IGraphConfig> gc)
        : ISchedulerNode(policy.exeName.c_str()),
          mCameraId(cameraId),
          mStreamId(-1),
          mName(policy.exeName),
          mPGNames(policy.pgList),
          mOpModes(policy.opModeList),
          mGraphConfig(gc),
          mIsInputEdge(false),
          mIsOutputEdge(false),
          mNotifyPolicy(POLICY_FRAME_FIRST),
          mAdaptor(nullptr),
          mPolicyManager(nullptr),
          mShareReferPool(nullptr),
          mLastStatsSequence(-1),
          mExclusivePGs(exclusivePGs),
          mPSysDag(psysDag),
          mkernelsCountWithStats(0) {
}

PipeLiteExecutor::~PipeLiteExecutor() {
    while (!mPGExecutors.empty()) {
        ExecutorUnit& unit = mPGExecutors.back();
        if (unit.pg.get()) {
            unit.pg->deInit();
        }
        mPGExecutors.pop_back();
    }

    releaseBuffers();
}

int PipeLiteExecutor::initPipe() {
    LOG1("@%s:%s", __func__, getName());
    CheckAndLogError(mGraphConfig == nullptr, BAD_VALUE, "%s, the graph config is NULL, BUG!",
                     __func__);

    NodesPtrVector programGroups;
    vector<IGraphType::PipelineConnection> connVector;
    vector<IGraphType::PrivPortFormat> tnrPortFormat;

    int ret = mGraphConfig->pipelineGetConnections(mPGNames, &connVector, &tnrPortFormat);
    CheckAndLogError(connVector.empty(), ret, "Failed to get connections for executor:%s",
                     mName.c_str());

    ret = createPGs();
    CheckAndLogError(ret != OK, ret, "Failed to create PGs for executor: %s", mName.c_str());

    ret = analyzeConnections(connVector);
    CheckAndLogError(ret != OK, ret, "Failed to analyze connections for executor: %s, ret = %d",
                     mName.c_str(), ret);

    ret = configurePGs(tnrPortFormat);
    CheckAndLogError(ret != OK, ret, "Failed to configure connections for executor: %s, ret = %d",
                     mName.c_str(), ret);

    assignDefaultPortsForTerminals();

    return OK;
}

int PipeLiteExecutor::analyzeConnections(const vector<IGraphType::PipelineConnection>& connVector) {
    ia_uid firstStageId = mPGExecutors.front().stageId;
    ia_uid lastStageId = mPGExecutors.back().stageId;

    for (auto const& connection : connVector) {
        LOG2("%s: terminal %d (%d): %dx%d, 0x%x(%s)", getName(),
             connection.portFormatSettings.terminalId, connection.portFormatSettings.enabled,
             connection.portFormatSettings.width, connection.portFormatSettings.height,
             connection.portFormatSettings.fourcc,
             CameraUtils::fourcc2String(connection.portFormatSettings.fourcc).c_str());
        LOG2("%s:     connection source %d, %d, %d, has edge %d", getName(),
             connection.connectionConfig.mSourceStage, connection.connectionConfig.mSourceTerminal,
             connection.connectionConfig.mSourceIteration, connection.hasEdgePort);
        LOG2("%s:     connection sink %d, %d, %d, type %d", getName(),
             connection.connectionConfig.mSinkStage, connection.connectionConfig.mSinkTerminal,
             connection.connectionConfig.mSinkIteration,
             connection.connectionConfig.mConnectionType);

        storeTerminalInfo(connection);

        if (connection.portFormatSettings.enabled == 0) {
            // No actions are needed for the disabled connections.
            continue;
        }

        // If the connection's sink stage is same as the first stage/pg id in this executor,
        // then it means the connection belongs to input terminal pairs.
        if (connection.connectionConfig.mSinkStage == firstStageId && connection.hasEdgePort) {
            mIsInputEdge = true;
        }

        // If the connection's source stage is same as the last stage/pg id in this executor,
        // then it means the connection belongs to output terminal pairs.
        // SIS is output terminal but it doesn't belong to any stream, so it is not real edge
        // output.
        if (connection.connectionConfig.mSourceStage == lastStageId && connection.hasEdgePort &&
            connection.connectionConfig.mSourceTerminal !=
                connection.connectionConfig.mSinkTerminal) {
            mIsOutputEdge = true;
        }
    }

    return OK;
}

int PipeLiteExecutor::storeTerminalInfo(const IGraphType::PipelineConnection& connection) {
    FrameInfo info;
    info.mWidth = connection.portFormatSettings.width;
    info.mHeight = connection.portFormatSettings.height;
    info.mFormat = connection.portFormatSettings.fourcc;

    ia_uid curTerminal = connection.portFormatSettings.terminalId;
    ia_uid sinkTerminal = connection.connectionConfig.mSinkTerminal;
    ia_uid sourceTerminal = connection.connectionConfig.mSourceTerminal;
    ia_uid sinkStage = connection.connectionConfig.mSinkStage;
    ia_uid sourceStage = connection.connectionConfig.mSourceStage;

    TerminalDescriptor desc;
    desc.terminal = 0;
    desc.stageId = 0;
    desc.sinkTerminal = sinkTerminal;
    desc.sourceTerminal = sourceTerminal;
    desc.sinkStage = sinkStage;
    desc.sourceStage = sourceStage;
    desc.frameDesc = info;
    desc.enabled = true;
    desc.hasConnection = true;
    desc.assignedPort = INVALID_PORT;
    desc.usrStreamId = connection.stream ? connection.stream->streamId() : -1;

    if (connection.portFormatSettings.enabled) {
        mConnectionMap[sinkTerminal] = sourceTerminal;
    }

    // Check if there is new input terminal
    if (sinkStage && mTerminalsDesc.find(sinkTerminal) == mTerminalsDesc.end()) {
        ExecutorUnit* unit = findPGExecutor(sinkStage);
        if (unit) {
            desc.terminal = sinkTerminal;
            desc.stageId = sinkStage;
            mTerminalsDesc[desc.terminal] = desc;
            unit->inputTerminals.push_back(desc.terminal);
        }
    }
    // Check if there is new output terminal
    if (sourceStage && mTerminalsDesc.find(sourceTerminal) == mTerminalsDesc.end()) {
        ExecutorUnit* unit = findPGExecutor(sourceStage);
        if (unit) {
            desc.terminal = sourceTerminal;
            desc.stageId = sourceStage;
            desc.hasConnection = (sinkTerminal != sourceTerminal);
            mTerminalsDesc[desc.terminal] = desc;
            unit->outputTerminals.push_back(desc.terminal);
        }
    }

    if (mTerminalsDesc.find(curTerminal) != mTerminalsDesc.end()) {
        mTerminalsDesc[curTerminal].enabled = connection.portFormatSettings.enabled;
    }

    return OK;
}

int PipeLiteExecutor::createPGs() {
    TuningMode tuningMode = mPSysDag->getTuningMode(-1);
    for (auto const& pgName : mPGNames) {
        int pgId = mGraphConfig->getPgIdByPgName(pgName);
        CheckAndLogError(pgId == -1, BAD_VALUE, "Cannot get PG ID for %s", pgName.c_str());

        ExecutorUnit pgUnit;
        pgUnit.pgId = pgId;
        pgUnit.stageId = psys_2600_pg_uid(pgId);
        pgUnit.pg = std::shared_ptr<PGCommon>(
            new PGCommon(mCameraId, pgId, pgName, tuningMode, pgUnit.stageId + 1));
        // Please refer to ia_cipf_css.h for terminalBaseUid
        pgUnit.pg->setShareReferPool(mShareReferPool);
        mPGExecutors.push_back(pgUnit);
        int ret = pgUnit.pg->init();
        CheckAndLogError(ret != OK, UNKNOWN_ERROR, "create PG %d error", pgId);
    }
    return OK;
}

int PipeLiteExecutor::configurePGs(const vector<IGraphType::PrivPortFormat>& tnrPortFormat) {
    FrameInfo tnrFormatInfo = {};
    for (auto& tnrFormat : tnrPortFormat) {
        if (tnrFormat.streamId == mStreamId) {
            tnrFormatInfo.mWidth = tnrFormat.formatSetting.width;
            tnrFormatInfo.mHeight = tnrFormat.formatSetting.height;
            tnrFormatInfo.mFormat = CameraUtils::getV4L2Format(tnrFormat.formatSetting.fourcc);
            break;
        }
    }

    mkernelsCountWithStats = 0;
    for (auto& unit : mPGExecutors) {
        map<ia_uid, FrameInfo> inputInfos;
        map<ia_uid, FrameInfo> outputInfos;
        vector<ia_uid> disabledTerminals;

        getTerminalFrameInfos(unit.inputTerminals, inputInfos);
        getTerminalFrameInfos(unit.outputTerminals, outputInfos);
        getDisabledTerminalsForPG(unit.stageId, disabledTerminals);

        unit.pg->setInputInfo(inputInfos, tnrFormatInfo);
        unit.pg->setOutputInfo(outputInfos);
        unit.pg->setDisabledTerminals(disabledTerminals);

        IGraphType::StageAttr stageAttr = {};
        if (mGraphConfig->getPgRbmValue(unit.pg->getName(), &stageAttr) == OK) {
            LOG1("%s: Set rbm for pgId %d, pgName: %s bytes %d", __func__, unit.pgId,
                 unit.pg->getName(), stageAttr.rbm_bytes);
            unit.pg->setRoutingBitmap(stageAttr.rbm, stageAttr.rbm_bytes);
        }

        int statsCount = getStatKernels(unit.pgId, unit.statKernelUids);
        mkernelsCountWithStats += statsCount;

        unit.pg->prepare(mAdaptor, statsCount, mStreamId);

        statsCount = getSisKernels(unit.pgId, unit.sisKernelUids);
        mkernelsCountWithStats += statsCount;
    }

    return OK;
}

/**
 * Assign ports for terminals as internal default value
 * Input ports may be overwritten with output ports of producer in setInputTerminals()
 */
int PipeLiteExecutor::assignDefaultPortsForTerminals() {
    Port portTable[] = {MAIN_PORT, SECOND_PORT, THIRD_PORT, FORTH_PORT, INVALID_PORT};
    for (auto& unit : mPGExecutors) {
        int outPortIndex = 0;
        for (auto terminal : unit.outputTerminals) {
            TerminalDescriptor& termDesc = mTerminalsDesc[terminal];
            if (termDesc.enabled && termDesc.hasConnection) {
                CheckAndLogError(portTable[outPortIndex] == INVALID_PORT, BAD_VALUE,
                                 "Port unavailable for output term %d:%d", unit.pgId, terminal);
                termDesc.assignedPort = portTable[outPortIndex];
                outPortIndex++;
            }
        }

        int inPortIndex = 0;
        for (auto terminal : unit.inputTerminals) {
            TerminalDescriptor& termDesc = mTerminalsDesc[terminal];
            if (termDesc.enabled && termDesc.hasConnection) {
                CheckAndLogError(portTable[inPortIndex] == INVALID_PORT, BAD_VALUE,
                                 "Port unavailable for input term %d", terminal);
                termDesc.assignedPort = portTable[inPortIndex];
                inPortIndex++;
            }
        }
    }

    return OK;
}

void PipeLiteExecutor::getOutputTerminalPorts(std::map<ia_uid, Port>& terminals) const {
    getTerminalPorts(mPGExecutors.back().outputTerminals, terminals);
}

void PipeLiteExecutor::getInputTerminalPorts(std::map<ia_uid, Port>& terminals) const {
    getTerminalPorts(mPGExecutors.front().inputTerminals, terminals);
}

int PipeLiteExecutor::setInputTerminals(const std::map<ia_uid, Port>& sourceTerminals) {
    // In edge PGs accepts input ports arrangement from external
    ExecutorUnit& inUnit = mPGExecutors.front();
    for (auto sinkTerminal : inUnit.inputTerminals) {
        if (mConnectionMap.find(sinkTerminal) == mConnectionMap.end()) {
            continue;
        }

        ia_uid sourceTerminal = mConnectionMap[sinkTerminal];
        if (sourceTerminals.find(sourceTerminal) != sourceTerminals.end()) {
            mTerminalsDesc[sinkTerminal].assignedPort = sourceTerminals.at(sourceTerminal);
            LOG2("pg %s get external %d -> input %d, port %d", getName(), sourceTerminal,
                 sinkTerminal, mTerminalsDesc[sinkTerminal].assignedPort);
        }
    }

    // Link internal PGs (sink PG accepts input ports arrangement from source PG (output ports)
    // source PG(output ports) -> (input ports)sink PG
    for (unsigned int i = 1; i < mPGExecutors.size(); i++) {
        for (auto sinkTerminal : mPGExecutors[i].inputTerminals) {
            if (!mTerminalsDesc[sinkTerminal].enabled) {
                continue;
            }
            if (mConnectionMap.find(sinkTerminal) != mConnectionMap.end()) {
                ia_uid sourceTerminal = mConnectionMap[sinkTerminal];
                mTerminalsDesc[sinkTerminal].assignedPort =
                    mTerminalsDesc[sourceTerminal].assignedPort;
            }
        }
    }

    // Set frame info to BufferQueue
    map<Port, stream_t> inputInfo;
    map<Port, stream_t> outputInfo;
    ExecutorUnit& outUnit = mPGExecutors.back();
    for (auto terminal : inUnit.inputTerminals) {
        if (mTerminalsDesc[terminal].assignedPort == INVALID_PORT) {
            continue;
        }

        stream_t inputConfig;
        CLEAR(inputConfig);
        inputConfig.width = mTerminalsDesc[terminal].frameDesc.mWidth;
        inputConfig.height = mTerminalsDesc[terminal].frameDesc.mHeight;
        inputConfig.format = mTerminalsDesc[terminal].frameDesc.mFormat;
        inputConfig.id = mTerminalsDesc[terminal].usrStreamId;
        inputInfo[mTerminalsDesc[terminal].assignedPort] = inputConfig;
    }
    for (auto terminal : outUnit.outputTerminals) {
        if (mTerminalsDesc[terminal].assignedPort == INVALID_PORT) {
            continue;
        }

        stream_t outputConfig;
        CLEAR(outputConfig);
        outputConfig.width = mTerminalsDesc[terminal].frameDesc.mWidth;
        outputConfig.height = mTerminalsDesc[terminal].frameDesc.mHeight;
        outputConfig.format = mTerminalsDesc[terminal].frameDesc.mFormat;
        outputConfig.id = mTerminalsDesc[terminal].usrStreamId;
        outputInfo[mTerminalsDesc[terminal].assignedPort] = outputConfig;
    }
    BufferQueue::setFrameInfo(inputInfo, outputInfo);

    return OK;
}

int PipeLiteExecutor::start() {
    LOG1("%s executor:%s", __func__, mName.c_str());
    // Need thread when PolicyManager takes responsibility. Otherwise Scheduler will handle.
    if (mPolicyManager) mProcessThread = new ProcessThread(this);
    AutoMutex l(mBufferQueueLock);

    allocBuffers();
    dumpPGs();

    mLastStatsSequence = -1;

    if (mProcessThread) {
        mThreadRunning = true;
        mProcessThread->run(mName.c_str(), PRIORITY_NORMAL);
    }

    return OK;
}

void PipeLiteExecutor::stop() {
    LOG1("%s executor:%s", __func__, mName.c_str());

    if (mProcessThread) mProcessThread->requestExitAndWait();

    // Thread is not running. It is safe to clear the Queue
    clearBufferQueues();
    if (mProcessThread) delete mProcessThread;

    // Clear the buffer pool of pg Uint
    for (auto& unit : mPGExecutors) {
        unit.inputBuffers.clear();
        unit.outputBuffers.clear();
    }
}

void PipeLiteExecutor::notifyStop() {
    LOG1("%s executor:%s", __func__, mName.c_str());
    if (!mProcessThread) return;

    mProcessThread->requestExit();
    {
        AutoMutex l(mBufferQueueLock);
        mThreadRunning = false;
        // Wakeup the thread to exit
        mFrameAvailableSignal.signal();
        mOutputAvailableSignal.signal();
    }
}

int PipeLiteExecutor::releaseStatsBuffer(const shared_ptr<CameraBuffer>& statsBuf) {
    LOG2("%s executor:%s", __func__, mName.c_str());
    AutoMutex lock(mStatsBuffersLock);

    mStatsBuffers.push(statsBuf);

    return OK;
}

bool PipeLiteExecutor::hasOutputTerminal(ia_uid sinkTerminal) {
    if (mConnectionMap.find(sinkTerminal) == mConnectionMap.end()) {
        return false;
    }

    ExecutorUnit& unit = mPGExecutors.back();
    for (auto sourceTerminal : unit.outputTerminals) {
        if (mConnectionMap[sinkTerminal] == sourceTerminal) {
            return true;
        }
    }
    return false;
}

int PipeLiteExecutor::getStatKernels(int pgId, vector<ia_uid>& kernels) {
    kernels.clear();
    for (unsigned int i = 0; i < ARRAY_SIZE(sStatKernels); i++) {
        int pgIdOfKernel = -1;
        int status = mGraphConfig->getPgIdForKernel(mStreamId, sStatKernels[i], &pgIdOfKernel);
        if (status == OK && pgIdOfKernel == pgId) {
            kernels.push_back(sStatKernels[i]);
        }
    }

    LOG1("pg %d has %lu stat kernels", pgId, kernels.size());
    return kernels.size();
}

int PipeLiteExecutor::getSisKernels(int pgId, vector<ia_uid>& kernels) {
    kernels.clear();
    for (unsigned int i = 0; i < ARRAY_SIZE(sSisKernels); i++) {
        int pgIdOfKernel = -1;
        int status = mGraphConfig->getPgIdForKernel(mStreamId, sSisKernels[i], &pgIdOfKernel);
        if (status == OK && pgIdOfKernel == pgId) {
            kernels.push_back(sSisKernels[i]);
        }
    }

    LOG1("pg %d has %lu sis kernels", pgId, kernels.size());
    return kernels.size();
}

bool PipeLiteExecutor::isSameStreamConfig(const stream_t& internal, const stream_t& external,
                                          ConfigMode configMode, bool checkStreamId) const {
    // The internal format is ia_fourcc based format, so need to convert it to V4L2 format.
    int internalFormat = CameraUtils::getV4L2Format(internal.format);
    int internalStride = CameraUtils::getStride(internalFormat, internal.width);
    int externalStride = CameraUtils::getStride(external.format, external.width);

    LOG1("%s: %s, id:%d, internal: %s(%dx%d: %d)(id %d), external: %s(%dx%d: %d) (id %d) usage:%d",
         __func__, mName.c_str(), mStreamId, CameraUtils::format2string(internalFormat).c_str(),
         internal.width, internal.height, internalStride, internal.id,
         CameraUtils::format2string(external.format).c_str(), external.width, external.height,
         externalStride, external.id, external.usage);

    if (checkStreamId && internal.id >= 0) {
        return internal.id == external.id;
    }

    /*
     * WA: PG accept GRBG format but actual input data is of RGGB format,
     *     PG use its kernel to crop to GRBG
     */
    if ((internalFormat == V4L2_PIX_FMT_SGRBG10 || internalFormat == V4L2_PIX_FMT_SGRBG12) &&
        (external.format == V4L2_PIX_FMT_SRGGB10 || external.format == V4L2_PIX_FMT_SRGGB12)) {
        return true;
    }

    bool sameHeight =
        internal.height == external.height || internal.height == ALIGN_32(external.height);
    if (internalFormat == external.format && sameHeight &&
        (internal.width == external.width || internalStride == externalStride)) {
        return true;
    }

    return false;
}

/**
 * Check if there is any valid buffer(not null) in the given port/buffer pairs.
 *
 * return true if there is at least one not null buffer.
 */
bool PipeLiteExecutor::hasValidBuffers(const CameraBufferPortMap& buffers) {
    for (const auto& item : buffers) {
        if (item.second) return true;
    }

    return false;
}

bool PipeLiteExecutor::fetchBuffersInQueue(map<Port, shared_ptr<CameraBuffer>>& cInBuffer,
                                           map<Port, shared_ptr<CameraBuffer>>& cOutBuffer) {
    for (auto& input : mInputQueue) {
        Port port = input.first;
        CameraBufQ& inputQueue = input.second;
        if (inputQueue.empty()) {
            LOG2("%s: No buffer input port %d", __func__, port);
            cInBuffer.clear();
            return false;
        }
        cInBuffer[port] = inputQueue.front();
    }

    for (auto& output : mOutputQueue) {
        Port port = output.first;
        CameraBufQ& outputQueue = output.second;
        if (outputQueue.empty()) {
            LOG2("%s: No buffer output port %d", __func__, port);
            cInBuffer.clear();
            cOutBuffer.clear();
            return false;
        }

        cOutBuffer[port] = outputQueue.front();
    }
    return true;
}

int PipeLiteExecutor::processNewFrame() {
    PERF_CAMERA_ATRACE();

    int ret = OK;
    CameraBufferPortMap inBuffers, outBuffers;
    // Wait frame buffers.
    {
        ConditionLock lock(mBufferQueueLock);
        if (mPolicyManager) {
            // Prepare frames at first, then mPolicyManager decides when to run
            ret = waitFreeBuffersInQueue(lock, inBuffers, outBuffers);
            // Already stopped
            if (!mThreadRunning) return -1;

            if (ret != OK) return OK;  // Wait frame buffer error should not involve thread exit.

            CheckAndLogError(inBuffers.empty() || outBuffers.empty(), UNKNOWN_ERROR,
                             "Failed to get input or output buffers.");
        } else {
            // Triggered by scheduler, will run if frames are ready
            if (!fetchBuffersInQueue(inBuffers, outBuffers)) return OK;
        }

        for (auto& output : mOutputQueue) {
            output.second.pop();
        }

        for (auto& input : mInputQueue) {
            input.second.pop();
        }
    }

    // Check if the executor needs to run the actual pipeline.
    // It only needs to run when there is at least one valid output buffer.
    if (!hasValidBuffers(outBuffers)) {
        // Return buffers if the executor is NOT an input edge.
        if (!mIsInputEdge) {
            for (const auto& item : inBuffers) {
                mBufferProducer->qbuf(item.first, item.second);
            }
        }
        return OK;
    }

    // Fill real buffer to run pipe
    for (auto& item : outBuffers) {
        if (item.second.get() == nullptr) {
            item.second = mInternalOutputBuffers[item.first];
        }
    }

    vector<shared_ptr<CameraBuffer>> outStatsBuffers;
    vector<EventType> eventType;
    // Should find first not none input buffer instead of always use the first one.
    shared_ptr<CameraBuffer> inBuf = inBuffers.begin()->second;
    CheckAndLogError(!inBuf, UNKNOWN_ERROR, "@%s: no valid input buffer", __func__);
    int64_t inBufSequence = inBuf->getSequence();
    v4l2_buffer_t inV4l2Buf = *inBuf->getV4L2Buffer().Get();
    TuningMode tuningMode = mPSysDag->getTuningMode(inBufSequence);

    LOG2("%s:Id:%d run pipe start for buffer:%ld", mName.c_str(), mCameraId, inBufSequence);

    // FRAME_SYNC_S
    if (PlatformData::isEnableFrameSyncCheck(mCameraId)) {
        shared_ptr<CameraBuffer> cInBuffer = inBuffers[MAIN_PORT];
        int vc = cInBuffer->getVirtualChannel();

        while ((!SyncManager::getInstance()->vcSynced(vc)) && mThreadRunning) usleep(1);

        int seq = cInBuffer->getSequence();
        SyncManager::getInstance()->printVcSyncCount();
        LOG2(
            "<seq%d> [start runPipe], CPU-timestamp:%lu, vc:%d, kernel-timestamp:%.3l", seq,
            CameraUtils::systemTime(), cInBuffer->getVirtualChannel(),
            cInBuffer->getTimestamp().tv_sec * 1000.0 + cInBuffer->getTimestamp().tv_usec / 1000.0);

        SyncManager::getInstance()->updateVcSyncCount(vc);

        // Run pipe with buffers
        ret = runPipe(inBuffers, outBuffers, outStatsBuffers, eventType);
        LOG2(
            "<seq%u> [done runPipe], CPU-timestamp:%lu, vc:%d, kernel-timestamp:%.3lf",
            cInBuffer->getSequence(), CameraUtils::systemTime(), cInBuffer->getVirtualChannel(),
            cInBuffer->getTimestamp().tv_sec * 1000.0 + cInBuffer->getTimestamp().tv_usec / 1000.0);
    } else {
        // FRAME_SYNC_E
        // Run pipe with buffers
        ret = runPipe(inBuffers, outBuffers, outStatsBuffers, eventType);
        // FRAME_SYNC_S
    }
    // FRAME_SYNC_E
    CheckAndLogError((ret != OK), UNKNOWN_ERROR, "@%s: failed to run pipe", __func__);
    LOG2("%s:Id:%d run pipe end for buffer:%ld", mName.c_str(), mCameraId, inBufSequence);

    // Remove internal output buffers
    for (auto& item : outBuffers) {
        if (item.second.get() == mInternalOutputBuffers[item.first].get()) {
            item.second = nullptr;
        }
    }

    if (mNotifyPolicy == POLICY_FRAME_FIRST) {
        // For general case, notify frame prior to stats to make sure its consumers can get
        // the frame buffers as early as possible.
        notifyFrameDone(inV4l2Buf, outBuffers);
        notifyStatsDone(tuningMode, inV4l2Buf, outStatsBuffers, eventType);
    } else if (mNotifyPolicy == POLICY_STATS_FIRST) {
        // Notify stats first and then handle frame buffers to make sure the next executor
        // can get this executor's IQ result.
        notifyStatsDone(tuningMode, inV4l2Buf, outStatsBuffers, eventType);
        notifyFrameDone(inV4l2Buf, outBuffers);
    } else {
        LOGW("Invalid notify policy:%d, should never happen.", mNotifyPolicy);
    }

    // Return buffers for the executor which is NOT an input edge
    if (!mIsInputEdge) {
        for (auto const& portBufferPair : inBuffers) {
            // Queue buffer to producer
            mBufferProducer->qbuf(portBufferPair.first, portBufferPair.second);
        }
    }

    return OK;
}

int PipeLiteExecutor::registerInBuffers(Port port, const shared_ptr<CameraBuffer>& inBuf) {
    return OK;
}

int PipeLiteExecutor::registerOutBuffers(Port port, const shared_ptr<CameraBuffer>& camBuffer) {
    return OK;
}

int PipeLiteExecutor::runPipe(map<Port, shared_ptr<CameraBuffer>>& inBuffers,
                              map<Port, shared_ptr<CameraBuffer>>& outBuffers,
                              vector<shared_ptr<CameraBuffer>>& outStatsBuffers,
                              vector<EventType>& eventType) {
    PERF_CAMERA_ATRACE();

    CheckAndLogError((inBuffers.empty() || outBuffers.empty()), BAD_VALUE,
                     "Error in pipe iteration input/output bufs");

    int ret = OK;
    int64_t sequence = inBuffers.begin()->second ? inBuffers.begin()->second->getSequence() : -1;
    if (mPolicyManager) {
        // Check if need to wait other executors.
        ret = mPolicyManager->wait(mName, sequence);
    }

    // Accept external buffers for in/out edge PGs
    getTerminalBuffersFromExternal(mPGExecutors.front().inputTerminals, inBuffers,
                                   mPGExecutors.front().inputBuffers);
    getTerminalBuffersFromExternal(mPGExecutors.back().outputTerminals, outBuffers,
                                   mPGExecutors.back().outputBuffers);

    // Get ISP parameters
    const ia_binary_data* ipuParameters = nullptr;
    TRACE_LOG_PROCESS(mName.c_str(), "runPipe", MAKE_COLOR(sequence), sequence);
    if (mAdaptor) {
        ipuParameters = mAdaptor->getIpuParameter(sequence, mStreamId);
        if (!ipuParameters) {
            LOG2("%s: <seq%ld> executor %s doesn't run due to no pal", __func__, sequence,
                 mName.c_str());
            return OK;
        }
    }

    LOG2("%s: <seq%ld> Executor %s run with input: %zu, output: %zu, ", __func__, sequence,
         mName.c_str(), inBuffers.size(), outBuffers.size());

    outStatsBuffers.clear();
    eventType.clear();
    int statTotalNum = 0;
    for (unsigned int pgIndex = 0; pgIndex < mPGExecutors.size(); pgIndex++) {
        ExecutorUnit& unit = mPGExecutors[pgIndex];

        // Prepare stats buffers for 3A/sis
        vector<ia_binary_data*> pgStatsDatas;
        // For 3A stats
        unsigned int statsCount = unit.statKernelUids.size();
        for (unsigned int counter = 0; counter < statsCount; counter++) {
            if (mStatsBuffers.empty()) {
                LOGW("No available stats buffer.");
                break;
            }
            outStatsBuffers.push_back(mStatsBuffers.front());
            eventType.push_back(EVENT_PSYS_STATS_BUF_READY);
            ia_binary_data* buffer = (ia_binary_data*)mStatsBuffers.front()->getBufferAddr();
            CheckAndLogError(buffer == nullptr, BAD_VALUE, "buffer is null pointer.");
            buffer->size = 0;  // Clear it, then the stats memory is from p2p
            buffer->data = nullptr;
            pgStatsDatas.push_back(buffer);
            mStatsBuffers.pop();
        }
        unsigned int sisCount = unit.sisKernelUids.size();
        for (unsigned int counter = 0; counter < sisCount; counter++) {
            if (mStatsBuffers.empty()) {
                LOGW("No available stats buffer.");
                break;
            }
            outStatsBuffers.push_back(mStatsBuffers.front());
            eventType.push_back(EVENT_PSYS_STATS_SIS_BUF_READY);
            ia_binary_data* buffer = (ia_binary_data*)mStatsBuffers.front()->getBufferAddr();
            pgStatsDatas.push_back(buffer);
            mStatsBuffers.pop();
        }

        // Run PGs
        // Update sequence only for the 1st input buffer currently
        unit.inputBuffers.begin()->second->setSequence(sequence);
        // Currently PG handles one stats buffer only
        ret = unit.pg->iterate(unit.inputBuffers, unit.outputBuffers,
                               (statsCount > 0) ? pgStatsDatas[0] : nullptr, ipuParameters);
        CheckAndLogError((ret != OK), ret, "%s: pipe iteration error %d", mName.c_str(), ret);

        if (CameraDump::isDumpTypeEnable(DUMP_PSYS_INTERM_BUFFER)) {
            for (auto& item : unit.outputBuffers) {
                char desc[MAX_NAME_LEN] = {'\0'};
                snprintf(desc, (MAX_NAME_LEN - 1), "-%s-%d-%ld", unit.pg->getName(),
                         item.first - unit.stageId - 1, sequence);
                CameraDump::dumpImage(mCameraId, item.second, M_NA, INVALID_PORT, desc);
            }
        }
        statTotalNum += statsCount;
        if (sisCount > 0) {
            // Currently handle one sis output only
            handleSisStats(unit.outputBuffers, outStatsBuffers[statTotalNum]);
        }
        statTotalNum += sisCount;
    }

    return OK;
}

int PipeLiteExecutor::handleSisStats(map<ia_uid, shared_ptr<CameraBuffer>>& frameBuffers,
                                     const shared_ptr<CameraBuffer>& outStatsBuffers) {
    LOG2("%s:", __func__);
    ia_binary_data* statBuf = (ia_binary_data*)outStatsBuffers->getBufferAddr();
    CheckAndLogError((statBuf == nullptr), BAD_VALUE, "Error getting buffer for sis a stats");
    statBuf->data = nullptr;
    statBuf->size = 0;

    for (auto iterm : frameBuffers) {
        ia_uid uid = iterm.first;
        if (uid == psys_ipu6_isa_lb_output_sis_a_uid) {
            statBuf->data = iterm.second->getBufferAddr();
            statBuf->size = iterm.second->getBufferSize();
            outStatsBuffers->setUserBufferInfo(-1, iterm.second->getWidth(),
                                               iterm.second->getHeight());
            return OK;
        }
    }

    return UNKNOWN_ERROR;
}

int PipeLiteExecutor::notifyFrameDone(const v4l2_buffer_t& inV4l2Buf,
                                      const CameraBufferPortMap& outBuf) {
    PERF_CAMERA_ATRACE();
    for (auto const& portBufferPair : outBuf) {
        shared_ptr<CameraBuffer> outBuf = portBufferPair.second;
        Port port = portBufferPair.first;
        // If the output buffer is nullptr, that means user doesn't request that buffer,
        // so it doesn't need to be handled here.
        if (!outBuf) continue;

        outBuf->updateV4l2Buffer(inV4l2Buf);

        // If it's output edge, the buffer should be returned to PSysDag,
        // otherwise they should be returned to its consumer.
        if (mIsOutputEdge) {
            mPSysDag->onFrameDone(port, outBuf);
        } else {
            for (auto& it : mBufferConsumerList) {
                it->onFrameAvailable(port, outBuf);
            }
        }
    }

    return OK;
}

int PipeLiteExecutor::notifyStatsDone(TuningMode tuningMode, const v4l2_buffer_t& inV4l2Buf,
                                      const vector<shared_ptr<CameraBuffer>>& outStatsBuffers,
                                      const vector<EventType>& eventType) {
    PERF_CAMERA_ATRACE();

    if (outStatsBuffers.empty()) {
        // notify event even though no stats is output
        if (mkernelsCountWithStats > 0) {
            LOG2("%s, notify stats done Stats %d", __func__, mkernelsCountWithStats);
            mPSysDag->onStatsDone(inV4l2Buf.sequence);
        }
        return OK;
    }

    /**
     * Notice for EVENT_PSYS_STATS_BUF_READY:
     * dvs stat & 3a stat come from different PG, and they are decoded separately
     * in decodeStatsData().
     * Fortunately stats data are stored in aiqResultStorage separately,
     * and user will get them from storage instead of EventData.
     * So here we can send one event after all stat buffers are decoded/stored/released.
     */
    int psysStatBufferCount = 0;
    for (auto type : eventType) {
        if (type == EVENT_PSYS_STATS_BUF_READY) {
            psysStatBufferCount++;
        }
    }

    int statsIndex = 0;
    for (auto statsBuf : outStatsBuffers) {
        if (!statsBuf) continue;

        if (mStreamId != VIDEO_STREAM_ID) {
            // DVS Zoom without STAT buffer.
            {
                EventData eventData;
                eventData.type = EVENT_DVS_READY;
                eventData.data.dvsRunReady.streamId = mStreamId;
                notifyListeners(eventData);
            }

            if (!PlatformData::isStillOnlyPipeEnabled(mCameraId)) {
                LOG2("%s: Drop still pipe statistics data", __func__);
                releaseStatsBuffer(statsBuf);
                continue;
            }
        } else if (inV4l2Buf.sequence <= mLastStatsSequence) {
            // Ignore old statistics for Raw reprocessing
            LOG2("%s: <seq%d> is less than last sequence %ld", __func__, inV4l2Buf.sequence,
                 mLastStatsSequence);
            releaseStatsBuffer(statsBuf);
            continue;
        }

        ia_binary_data* hwStatsData = (ia_binary_data*)(statsBuf->getBufferAddr());
        if (hwStatsData == nullptr || hwStatsData->data == nullptr || hwStatsData->size == 0) {
            LOGW("%s: No statistics data in buffer", __func__);
            releaseStatsBuffer(statsBuf);
            continue;
        }

        statsBuf->updateV4l2Buffer(inV4l2Buf);

        // Decode the statistics data
        if (eventType[statsIndex] == EVENT_PSYS_STATS_BUF_READY) {
            mAdaptor->decodeStatsData(tuningMode, statsBuf, mGraphConfig);
            psysStatBufferCount--;
        }

        // Notify listeners after all buffers done for type STATS_BUF_READY
        // Notify immediately for other types
        if (eventType[statsIndex] != EVENT_PSYS_STATS_BUF_READY || !psysStatBufferCount) {
            EventDataStatsReady statsReadyData;
            statsReadyData.sequence = statsBuf->getSequence();
            statsReadyData.timestamp.tv_sec = statsBuf->getTimestamp().tv_sec;
            statsReadyData.timestamp.tv_usec = statsBuf->getTimestamp().tv_usec;
            EventData eventData;
            eventData.type = eventType[statsIndex];
            eventData.buffer = statsBuf;
            eventData.data.statsReady = statsReadyData;
            eventData.pipeType = (mStreamId == VIDEO_STREAM_ID) ? VIDEO_STREAM_ID : STILL_STREAM_ID;
            notifyListeners(eventData);
        }

        releaseStatsBuffer(statsBuf);
        statsIndex++;
    }

    if (mStreamId == VIDEO_STREAM_ID && inV4l2Buf.sequence > mLastStatsSequence) {
        mLastStatsSequence = inV4l2Buf.sequence;
    }
    LOG2("%s, notify stats done", __func__);
    mPSysDag->onStatsDone(inV4l2Buf.sequence);

    return OK;
}

int PipeLiteExecutor::allocBuffers() {
    LOG1("%s executor:%s", __func__, mName.c_str());

    releaseBuffers();

    // Allocate buffer between PGs (internal)
    for (auto const& item : mTerminalsDesc) {
        const TerminalDescriptor& termDesc = item.second;
        if (!termDesc.enabled) {
            continue;
        }

        if (termDesc.assignedPort != INVALID_PORT &&
            !(findPGExecutor(termDesc.sinkStage) && findPGExecutor(termDesc.sourceStage))) {
            // Don't allocate buffer here for external connection (has valid port)
            continue;
        }

        // Allocated already
        if (mPGBuffers.find(termDesc.terminal) != mPGBuffers.end()) {
            continue;
        }

        int srcFmt = termDesc.frameDesc.mFormat;
        int srcWidth = termDesc.frameDesc.mWidth;
        int srcHeight = termDesc.frameDesc.mHeight;
        bool isCompression = PlatformData::getPSACompression(mCameraId) &&
                             PGUtils::isCompressionTerminal(termDesc.terminal);

        int size = isCompression ?
                       PGCommon::getFrameSize(srcFmt, srcWidth, srcHeight, false, true, true) :
                       PGCommon::getFrameSize(srcFmt, srcWidth, srcHeight, true);

        shared_ptr<CameraBuffer> buf =
            CameraBuffer::create(mCameraId, BUFFER_USAGE_PSYS_INPUT, V4L2_MEMORY_USERPTR, size, 0,
                                 srcFmt, srcWidth, srcHeight);
        CheckAndLogError(!buf, NO_MEMORY, "@%s: Allocate producer buffer failed", __func__);
        mPGBuffers[termDesc.sinkTerminal] = buf;
        mPGBuffers[termDesc.sourceTerminal] = buf;
    }

    int bufCount = PlatformData::getMaxRequestsInflight(mCameraId);
    for (auto& unit : mPGExecutors) {
        // Assign internal buffers for terminals of PGs according to connection
        for (auto& terminal : unit.inputTerminals) {
            if (mPGBuffers.find(terminal) != mPGBuffers.end()) {
                unit.inputBuffers[terminal] = mPGBuffers[terminal];
            }
        }
        for (auto& terminal : unit.outputTerminals) {
            if (mPGBuffers.find(terminal) != mPGBuffers.end()) {
                unit.outputBuffers[terminal] = mPGBuffers[terminal];
            }
        }

        // Allocate stats buffers if needed.
        unsigned int statsBufferCount = unit.statKernelUids.size();
        if (!statsBufferCount) {
            continue;
        }
        for (unsigned int i = 0; i < bufCount * statsBufferCount; i++) {
            shared_ptr<CameraBuffer> statsBuf = CameraBuffer::create(
                mCameraId, BUFFER_USAGE_PSYS_STATS, V4L2_MEMORY_USERPTR, sizeof(ia_binary_data), i);
            CheckAndLogError(!statsBuf, NO_MEMORY, "Executor %s: Allocate stats buffer failed",
                             mName.c_str());
            ia_binary_data* buffer = (ia_binary_data*)statsBuf->getBufferAddr();
            CheckAndLogError(buffer == nullptr, BAD_VALUE, "buffer is null pointer.");
            buffer->size = 0;
            buffer->data = nullptr;
            AutoMutex lock(mStatsBuffersLock);
            mStatsBuffers.push(statsBuf);
        }
    }

    // Allocate buffers for producer executor (external)
    // Ignore input edge due to no producer
    if (!mIsInputEdge) {
        for (auto const& terminal : mPGExecutors.front().inputTerminals) {
            Port inputPort = mTerminalsDesc[terminal].assignedPort;

            int srcFmt = mTerminalsDesc[terminal].frameDesc.mFormat;
            int srcWidth = mTerminalsDesc[terminal].frameDesc.mWidth;
            int srcHeight = mTerminalsDesc[terminal].frameDesc.mHeight;
            // Get frame size with aligned height taking in count for internal buffers.
            // To garantee PSYS kernel like GDC always get enough buffer size to process.
            bool isCompression = PlatformData::getPSACompression(mCameraId) &&
                                 PGUtils::isCompressionTerminal(terminal);

            int size = isCompression ?
                           PGCommon::getFrameSize(srcFmt, srcWidth, srcHeight, false, true, true) :
                           PGCommon::getFrameSize(srcFmt, srcWidth, srcHeight, true);

            for (int i = 0; i < bufCount; i++) {
                // Prepare internal frame buffer for its producer.
                shared_ptr<CameraBuffer> buf =
                    CameraBuffer::create(mCameraId, BUFFER_USAGE_PSYS_INPUT, V4L2_MEMORY_USERPTR,
                                         size, i, srcFmt, srcWidth, srcHeight);
                CheckAndLogError(!buf, NO_MEMORY, "@%s: Allocate producer buffer failed", __func__);
                mInternalBuffers[inputPort].push_back(buf);

                mBufferProducer->qbuf(inputPort, buf);
            }
        }
    }

    // Allocate internal output buffers to support pipe execution without user output buffer
    for (auto const& item : mOutputFrameInfo) {
        int fmt = item.second.format;
        int width = item.second.width;
        int height = item.second.height;
        int size = CameraUtils::getFrameSize(fmt, width, height, true);
        shared_ptr<CameraBuffer> buf = CameraBuffer::create(
            mCameraId, BUFFER_USAGE_PSYS_INPUT, V4L2_MEMORY_USERPTR, size, 0, fmt, width, height);
        CheckAndLogError(!buf, NO_MEMORY, "@%s: Allocate internal output buffer failed", __func__);
        mInternalOutputBuffers[item.first] = buf;
    }

    return OK;
}

void PipeLiteExecutor::releaseBuffers() {
    LOG1("%s executor:%s", __func__, mName.c_str());

    // Release internel frame buffers
    mInternalOutputBuffers.clear();
    mInternalBuffers.clear();
    mPGBuffers.clear();

    // Release stats buffers
    {
        AutoMutex lock(mStatsBuffersLock);
        while (!mStatsBuffers.empty()) mStatsBuffers.pop();
    }
}

PipeLiteExecutor::ExecutorUnit* PipeLiteExecutor::findPGExecutor(ia_uid stageId) {
    for (unsigned int i = 0; i < mPGExecutors.size(); i++) {
        if (mPGExecutors[i].stageId == stageId) {
            return &mPGExecutors[i];
        }
    }
    return nullptr;
}

void PipeLiteExecutor::getTerminalPorts(const vector<ia_uid>& terminals,
                                        map<ia_uid, Port>& terminalPortMap) const {
    terminalPortMap.clear();
    for (auto terminal : terminals) {
        const TerminalDescriptor& termDesc = mTerminalsDesc.at(terminal);
        if (termDesc.enabled && termDesc.assignedPort != INVALID_PORT) {
            terminalPortMap[terminal] = termDesc.assignedPort;
        }
    }
}

void PipeLiteExecutor::getTerminalFrameInfos(const vector<ia_uid>& terminals,
                                             map<ia_uid, FrameInfo>& infoMap) const {
    infoMap.clear();
    for (auto terminal : terminals) {
        const TerminalDescriptor& termDesc = mTerminalsDesc.at(terminal);
        if (termDesc.enabled) {
            infoMap[terminal] = termDesc.frameDesc;
        }
    }
}

void PipeLiteExecutor::getDisabledTerminalsForPG(ia_uid stageId, vector<ia_uid>& terminals) const {
    terminals.clear();
    for (auto const item : mTerminalsDesc) {
        const TerminalDescriptor& termDesc = item.second;
        if (termDesc.stageId == stageId && !termDesc.enabled) {
            terminals.push_back(termDesc.terminal);
        }
    }
}

void PipeLiteExecutor::getTerminalBuffersFromExternal(
    const vector<ia_uid>& terminals, const map<Port, shared_ptr<CameraBuffer>>& externals,
    map<ia_uid, shared_ptr<CameraBuffer>>& internals) const {
    for (auto& terminal : terminals) {
        Port port = mTerminalsDesc.at(terminal).assignedPort;
        if (externals.find(port) != externals.end()) {
            internals[terminal] = externals.at(port);
        }
    }
}

void PipeLiteExecutor::dumpPGs() const {
    if (!Log::isLogTagEnabled(GET_FILE_SHIFT(PipeLiteExecutor))) return;

    LOG3("============= dump PGs for executor %s =================", getName());
    if (mIsInputEdge) {
        LOG3("This is input edge");
    }
    if (mIsOutputEdge) {
        LOG3("This is output edge");
    }
    for (auto const& unit : mPGExecutors) {
        ia_uid stageId = unit.stageId;
        LOG3("    PG: %d: %s, stageId %d", unit.pgId, unit.pg ? unit.pg->getName() : "GPU-TNR",
             stageId);

        LOG3("        InTerms: %zu", unit.inputTerminals.size());
        for (auto const& term : unit.inputTerminals) {
            shared_ptr<CameraBuffer> buffer = nullptr;
            if (mPGBuffers.find(term) != mPGBuffers.end()) {
                buffer = mPGBuffers.at(term);
            }

            const TerminalDescriptor& termDesc = mTerminalsDesc.at(term);
            if (termDesc.enabled) {
                LOG3("            %d: %dx%d, 0x%x, port %d, buf %p",
                     termDesc.terminal - termDesc.stageId - 1, termDesc.frameDesc.mWidth,
                     termDesc.frameDesc.mHeight, termDesc.frameDesc.mFormat, termDesc.assignedPort,
                     buffer.get());
            } else {
                LOG3("            %d: %dx%d, 0x%x, disabled",
                     termDesc.terminal - termDesc.stageId - 1, termDesc.frameDesc.mWidth,
                     termDesc.frameDesc.mHeight, termDesc.frameDesc.mFormat);
            }
        }

        LOG3("        OutTerms: %zu", unit.outputTerminals.size());
        for (auto const& term : unit.outputTerminals) {
            shared_ptr<CameraBuffer> buffer = nullptr;
            if (mPGBuffers.find(term) != mPGBuffers.end()) {
                buffer = mPGBuffers.at(term);
            }

            const TerminalDescriptor& termDesc = mTerminalsDesc.at(term);
            if (termDesc.enabled) {
                LOG3("            %d: %dx%d, 0x%x, port %d, buf %p",
                     termDesc.terminal - termDesc.stageId - 1, termDesc.frameDesc.mWidth,
                     termDesc.frameDesc.mHeight, termDesc.frameDesc.mFormat, termDesc.assignedPort,
                     buffer.get());
            } else {
                LOG3("            %d: %dx%d, 0x%x, disabled",
                     termDesc.terminal - termDesc.stageId - 1, termDesc.frameDesc.mWidth,
                     termDesc.frameDesc.mHeight, termDesc.frameDesc.mFormat);
            }
        }
    }
    LOG3("============= dump done for %s =================", getName());
}

}  // namespace icamera
