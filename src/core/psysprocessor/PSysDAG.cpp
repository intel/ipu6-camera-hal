/*
 * Copyright (C) 2017-2020 Intel Corporation.
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

#define LOG_TAG "PSysDAG"

#include <algorithm>
#include "iutils/Utils.h"
#include "iutils/CameraLog.h"
#include "PSysDAG.h"
#ifdef TNR7_CM
#include "GPUExecutor.h"
#endif

namespace icamera {
PSysDAG::PSysDAG(int cameraId, PSysDagCallback* psysDagCB) :
    mCameraId(cameraId),
    mPSysDagCB(psysDagCB),
    mConfigMode(CAMERA_STREAM_CONFIGURATION_MODE_AUTO),
    mTuningMode(TUNING_MODE_MAX),
#ifdef USE_PG_LITE_PIPE
    mShareReferPool(nullptr),
#endif
    mDefaultMainInputPort(MAIN_PORT),
    mVideoTnrExecutor(nullptr),
    mRunAicAfterQbuf(false)
{
    LOG1("@%s, mCameraId:%d", __func__, mCameraId);

    mPolicyManager = new PolicyManager(mCameraId);
    mIspParamAdaptor = new IspParamAdaptor(mCameraId, PG_PARAM_PSYS_ISA);
}

PSysDAG::~PSysDAG()
{
    LOG1("@%s, mCameraId:%d", __func__, mCameraId);

    releasePipeExecutors();

    mIspParamAdaptor->deinit();
    delete mIspParamAdaptor;
    delete mPolicyManager;
}

void PSysDAG::setFrameInfo(const std::map<Port, stream_t>& inputInfo,
                           const std::map<Port, stream_t>& outputInfo) {
    mInputFrameInfo = inputInfo;
    mOutputFrameInfo = outputInfo;

    mDefaultMainInputPort = inputInfo.begin()->first;
    // Select default main input port in priority
    Port availablePorts[] = {MAIN_PORT, SECOND_PORT, THIRD_PORT, FORTH_PORT, INVALID_PORT};
    for (unsigned int i = 0; i < ARRAY_SIZE(availablePorts); i++) {
        if (mInputFrameInfo.find(availablePorts[i]) != mInputFrameInfo.end()) {
            mDefaultMainInputPort = availablePorts[i];
            break;
        }
    }
}

void PSysDAG::releasePipeExecutors()
{
    for (auto &executor : mExecutorsPool) {
        delete executor;
    }
    mExecutorsPool.clear();
    mExecutorStreamId.clear();
}

/*
 * According to the policy config to create the executors,
 * and use the graph config data to configure the executors.
 */
int PSysDAG::createPipeExecutors(bool useTnrOutBuffer)
{
    LOG1("@%s, mCameraId:%d", __func__, mCameraId);

    releasePipeExecutors();

    mOngoingPalMap.clear();

    IGraphConfigManager *GCM = IGraphConfigManager::getInstance(mCameraId);
    CheckError(!GCM, UNKNOWN_ERROR, "Failed to get GC manager in PSysDAG!");

    std::shared_ptr<IGraphConfig> gc = GCM->getGraphConfig(mConfigMode);
    CheckError(!gc, UNKNOWN_ERROR, "Failed to get GraphConfig in PSysDAG!");

    int graphId = gc->getGraphId();
    PolicyConfig* cfg = PlatformData::getExecutorPolicyConfig(graphId);
    CheckError(!cfg, UNKNOWN_ERROR, "Failed to get PolicyConfig in PSysDAG!");

#ifdef USE_PG_LITE_PIPE
    configShareReferPool(gc);
#endif

    std::vector<std::string> pgNames;
    gc->getPgNames(&pgNames);
    bool hasVideoPipe = false, hasStillPipe = false;

    for (auto &item : cfg->pipeExecutorVec) {
        int streamId = -1;
        bool pgFound = true;

        // Not support multiple streamId in one executor,
        // so need to the check the streamId of pgList.
        for (auto &pgName : item.pgList) {
            if (std::find(pgNames.begin(), pgNames.end(), pgName) == pgNames.end()) {
                pgFound = false;
                break;
            }
            int tmpId = gc->getStreamIdByPgName(pgName);
            CheckError(tmpId == -1, BAD_VALUE, "Cannot get streamId for %s", pgName.c_str());
            CheckError(((streamId != -1) && (tmpId != streamId)), BAD_VALUE,
                    "the streamId: %d for pgName(%s) is different with previous: %d",
                    tmpId, pgName.c_str(), streamId);
            streamId = tmpId;
            LOG1("%s executor:%s pg name:%s streamId: %d",
                  __func__, item.exeName.c_str(), pgName.c_str(), streamId);
        }
        if (!pgFound)
            continue;

        if (!hasVideoPipe)
            hasVideoPipe = (streamId == VIDEO_STREAM_ID);
        if (!hasStillPipe)
            hasStillPipe = (streamId == STILL_STREAM_ID);
#ifdef TNR7_CM
        PipeExecutor *executor;
        if (strstr(item.exeName.c_str(), "gputnr") != nullptr) {
            executor = new GPUExecutor(mCameraId, item, cfg->exclusivePgs, this, gc,
                                       useTnrOutBuffer);
            if (streamId == VIDEO_STREAM_ID) mVideoTnrExecutor = executor;
        } else {
            executor = new PipeExecutor(mCameraId, item, cfg->exclusivePgs, this, gc);
        }
#else
        PipeExecutor *executor = new PipeExecutor(mCameraId, item, cfg->exclusivePgs, this, gc);
#endif
        executor->setIspParamAdaptor(mIspParamAdaptor);
        executor->setStreamId(streamId);
        executor->setPolicyManager(mPolicyManager);
        executor->setNotifyPolicy(item.notifyPolicy);
#ifdef USE_PG_LITE_PIPE
        executor->setShareReferPool(mShareReferPool);
#endif
        int ret = executor->initPipe();
        if (ret != OK) {
            LOGE("Failed to create pipe for executor:%s", executor->getName());
            delete executor;
            return ret;
        }

        mExecutorsPool.push_back(executor);
        mExecutorStreamId[executor] = streamId;
    }
    LOG2("%s, hasVideoPipe: %d, hasStillPipe: %d, enableBundleInSdv: %d",
         __func__, hasVideoPipe, hasStillPipe, cfg->enableBundleInSdv);

    // Only enable psys bundle with aic when has video pipe only
    if (!hasStillPipe && PlatformData::psysBundleWithAic(mCameraId)) {
        mRunAicAfterQbuf = true;
    }

    if (hasStillPipe && hasVideoPipe && !cfg->enableBundleInSdv) return OK;

    for (auto &bundle : cfg->bundledExecutorDepths) {
        bool foundExecutor = true;
        for (auto &executor : bundle.bundledExecutors) {
            std::vector<PipeExecutor*>::iterator it = std::find_if(mExecutorsPool.begin(),
                    mExecutorsPool.end(), [executor](PipeExecutor* exec) {
                        return exec->getName() == executor;
                    });
            if (it == mExecutorsPool.end()) {
                foundExecutor = false;
                break;
            }
        }

        if (foundExecutor)
            mPolicyManager->addExecutorBundle(bundle.bundledExecutors, bundle.depths);
    }

    return OK;
}

bool PSysDAG::fetchTnrOutBuffer(int64_t seq, std::shared_ptr<CameraBuffer> buf)
{
    return mVideoTnrExecutor != nullptr ? mVideoTnrExecutor->fetchTnrOutBuffer(seq, buf) : false;
}

int PSysDAG::linkAndConfigExecutors()
{
    for (auto& consumer : mExecutorsPool) {
        std::map<ia_uid, Port> input;

        if (consumer->isInputEdge()) {
            // Use its own input info due to no executor as producer
            consumer->getInputTerminalPorts(input);
        } else {
            PipeExecutor* producer = findExecutorProducer(consumer);
            CheckError(producer == nullptr, BAD_VALUE, "no producer for executor %s!", consumer->getName());
            producer->getOutputTerminalPorts(input);

            consumer->setBufferProducer(producer);
            LOG1("%s: link consumer %s to %s", __func__, consumer->getName(), producer->getName());
        }

        // Link producer (output) to consumer (input) by terminal
        consumer->setInputTerminals(input);

        std::vector<ConfigMode> configModes;
        configModes.push_back(mConfigMode);
        consumer->configure(configModes);
    }

    return OK;
}

PipeExecutor* PSysDAG::findExecutorProducer(PipeExecutor* consumer)
{
    std::map<ia_uid, Port> inputTerminals;
    consumer->getInputTerminalPorts(inputTerminals);

    for (auto& executor : mExecutorsPool) {
        if (executor == consumer) {
            continue;
        }

        for (auto& inputTerminal : inputTerminals) {
            // Return if one is matched, because only one producer is supported now.
            if (executor->hasOutputTerminal(inputTerminal.first)) {
                return executor;
            }
        }
    }

    return nullptr;
}

/*
 * Search all the stream Ids in one pipe which provides frame buffer to the output port
 * mOutputPortToStreamIds: store all the different stream Ids to output port mapping table
 */
status_t PSysDAG::searchStreamIdsForOutputPort(PipeExecutor *executor, Port port) {
    LOG2("@%s, mCameraId:%d", __func__, mCameraId);

    CheckError(!executor || !executor->isOutputEdge(), BAD_VALUE,
               "%s, the executor is nullptr or is not output edge", __func__);

    if (mOutputPortToStreamIds.find(port) != mOutputPortToStreamIds.end()) {
        return OK;
    }

    std::vector<int32_t> streamIds;
    PipeExecutor *tmpExecutor = executor;
    // Loop to find the producer executor's stream id
    do {
        int32_t streamId = mExecutorStreamId[tmpExecutor];
        if (std::find(streamIds.begin(), streamIds.end(), streamId) == streamIds.end()) {
            streamIds.push_back(streamId);
            LOG2("%s, store the streamId: %d for output port: %d", __func__, streamId, port);
        }

        tmpExecutor = findExecutorProducer(tmpExecutor);
    } while (tmpExecutor);

    mOutputPortToStreamIds[port] = streamIds;

    return OK;
}

/**
 * Bind the port between DAG and its edge executors.
 * After the binding we'll know where the task buffer should be queued to.
 */
int PSysDAG::bindExternalPortsToExecutor()
{
    LOG1("@%s, mCameraId:%d", __func__, mCameraId);

    mInputMaps.clear();
    mOutputMaps.clear();
    mOutputPortToStreamIds.clear();

    std::map<Port, stream_t> outputInfo;
    std::map<Port, stream_t> inputInfo;

    // Bind the input ports first.
    LOG2("%s, start to bind the input port", __func__);
    for (auto& executor : mExecutorsPool) {
        if (!executor->isInputEdge()) {
            continue;
        }
        executor->getFrameInfo(inputInfo, outputInfo);

        for (auto& frameInfo : mInputFrameInfo) {
            for (auto& portInfo : inputInfo) {
                // Check if it has been cleared (bound already).
                if (!portInfo.second.format) {
                    continue;
                }
                if (executor->isSameStreamConfig(portInfo.second, frameInfo.second, mConfigMode, false)) {
                    PortMapping portMap;
                    portMap.mExecutor = executor;
                    portMap.mDagPort = frameInfo.first;
                    portMap.mExecutorPort = portInfo.first;
                    mInputMaps.push_back(portMap);
                    // Clear the stream of executor to avoid binding it again.
                    CLEAR(portInfo.second);
                    LOG2("%s, inputMap executor %p, dagPort %d, execPort %d", __func__,
                        executor, frameInfo.first, portInfo.first);
                    break;
                }
            }
        }
    }

    // Then bind the output ports.
    LOG2("%s, start to bind the output port", __func__);
    for (auto& executor : mExecutorsPool) {
        if (!executor->isOutputEdge()) {
            continue;
        }

        executor->getFrameInfo(inputInfo, outputInfo);
        for (auto& frameInfo : mOutputFrameInfo) {
            for (auto& portInfo : outputInfo) {
                // Check if it has been cleared (bound already).
                if (!portInfo.second.format) {
                    continue;
                }
                if (executor->isSameStreamConfig(portInfo.second, frameInfo.second, mConfigMode, true)) {
                    PortMapping portMap;
                    portMap.mExecutor = executor;
                    portMap.mDagPort = frameInfo.first;
                    portMap.mExecutorPort = portInfo.first;
                    mOutputMaps.push_back(portMap);
                    searchStreamIdsForOutputPort(executor, frameInfo.first);
                    // Clear the stream of executor to avoid binding it again.
                    CLEAR(portInfo.second);
                    break;
                }
            }
        }
    }

    // Each required port must be mapped to one of (edge) executor's port.
    // One input port may be mapped to more of (edge) executor's ports.
    CheckError(mInputMaps.size() < mInputFrameInfo.size(), BAD_VALUE, "Failed to bind input ports");
    CheckError(mOutputMaps.size() != mOutputFrameInfo.size(), BAD_VALUE, "Failed to bind output ports");

    return OK;
}

int PSysDAG::registerUserOutputBufs(Port port, const std::shared_ptr<CameraBuffer> &camBuffer)
{
    for (auto& outputMap : mOutputMaps) {
        if (port == outputMap.mDagPort) {
            outputMap.mExecutor->registerOutBuffers(outputMap.mExecutorPort, camBuffer);
            break;
        }
    }

    return OK;
}

int PSysDAG::registerInternalBufs(std::map<Port, CameraBufVector> &internalBufs)
{
    for (auto& portToBuffers : internalBufs) {
        for (auto& inputMap : mInputMaps) {
            if (inputMap.mDagPort == portToBuffers.first) {
                for (auto& inputBuf : portToBuffers.second) {
                    inputMap.mExecutor->registerInBuffers(inputMap.mExecutorPort, inputBuf);
                }
                break;
            }
        }
    }

    return OK;
}

/**
 * Queue the buffers in PSysTaskData to the cooresponding executors.
 */
int PSysDAG::queueBuffers(const PSysTaskData& task)
{
    LOG2("@%s, mCameraId:%d", __func__, mCameraId);

    // Provide the input buffers for the input edge executor.
    for (auto& inputFrame : task.mInputBuffers) {
        for (auto& inputMap : mInputMaps) {
            if (inputMap.mDagPort == inputFrame.first) {
                inputMap.mExecutor->onFrameAvailable(inputMap.mExecutorPort, inputFrame.second);
                LOG2("%s, queue input buffer: dagPort: %d, executorPort: %d, name: %s", __func__,
                     inputMap.mDagPort, inputMap.mExecutorPort, inputMap.mExecutor->getName());
            }
        }
    }

    // Provide the output buffers for the output edge executor.
    for (auto& outputFrame : task.mOutputBuffers) {
        for (auto& outputMap : mOutputMaps) {
            if (outputMap.mDagPort == outputFrame.first) {
                outputMap.mExecutor->qbuf(outputMap.mExecutorPort, outputFrame.second);
                LOG2("%s, queue output buffer, dagPort: %d, executorPort: %d, name: %s", __func__,
                     outputMap.mDagPort, outputMap.mExecutorPort, outputMap.mExecutor->getName());
                if (outputFrame.second) {
                    LOG2("outputFrame.second %p, outputFrame.first %d", outputFrame.second.get(),
                          outputFrame.first);
                }
                break;
            }
        }
    }

    return OK;
}

#ifdef USE_PG_LITE_PIPE
void PSysDAG::configShareReferPool(std::shared_ptr<IGraphConfig> gc) {
    PolicyConfig* cfg = PlatformData::getExecutorPolicyConfig(gc->getGraphId());
    if (!cfg || cfg->shareReferPairList.empty()) {
        return;
    }

    CheckWarning(cfg->shareReferPairList.size() % 2 != 0, VOID_VALUE,
                 "bad share refer pair count");

    if (!mShareReferPool.get())
        mShareReferPool = std::make_shared<ShareReferBufferPool>(mCameraId);

    for (size_t i = 0; i < cfg->shareReferPairList.size(); i += 2) {
        ShareReferIdDesc& pDesc = cfg->shareReferPairList[i];
        int32_t pStream = gc->getStreamIdByPgName(pDesc.first);
        int32_t pPgId = gc->getPgIdByPgName(pDesc.first);
        int32_t pPort = pDesc.second;
        ShareReferIdDesc& cDesc = cfg->shareReferPairList[i + 1];
        int32_t cStream = gc->getStreamIdByPgName(cDesc.first);
        int32_t cPgId = gc->getPgIdByPgName(cDesc.first);
        int32_t cPort = cDesc.second;
        // Check producer only because sometimes there is no consumer (such as still pipe)
        if (pPgId > 0) {
           int64_t pReferId = ShareReferBufferPool::constructReferId(pStream, pPgId, pPort);
           int64_t cReferId = ShareReferBufferPool::constructReferId(cStream, cPgId, cPort);
           mShareReferPool->setReferPair(pDesc.first, pReferId, cDesc.first, cReferId);
        }
    }
}
#endif

int PSysDAG::configure(ConfigMode configMode, TuningMode tuningMode, bool useTnrOutBuffer)
{
    LOG1("@%s, mCameraId:%d", __func__, mCameraId);

    mConfigMode = configMode;
    mTuningMode = tuningMode;
    mRunAicAfterQbuf = false;

    // Configure IspParamAdaptor
    int ret = mIspParamAdaptor->init();
    CheckError(ret != OK, ret, "Init isp Adaptor failed, tuningMode %d", mTuningMode);

    ret = mIspParamAdaptor->configure(mInputFrameInfo[mDefaultMainInputPort], mConfigMode, mTuningMode);
    CheckError(ret != OK, ret, "Configure isp Adaptor failed, tuningMode %d", mTuningMode);

    ret = createPipeExecutors(useTnrOutBuffer);
    CheckError(ret != OK, ret, "@%s, create psys executors failed", __func__);

    ret = linkAndConfigExecutors();
    CheckError(ret != OK, ret, "Link executors failed");

    ret = bindExternalPortsToExecutor();
    CheckError(ret != OK, ret, "Bind ports failed");

    return OK;
}

int PSysDAG::start()
{
    LOG1("@%s, mCameraId:%d", __func__, mCameraId);

    mPolicyManager->setActive(true);

    for (auto& executors : mExecutorsPool) {
        executors->start();
    }
    return OK;
}

int PSysDAG::stop()
{
    LOG1("@%s, mCameraId:%d", __func__, mCameraId);

    mPolicyManager->setActive(false);

    for (auto& executors : mExecutorsPool) {
        executors->notifyStop();
    }

    for (auto& executors : mExecutorsPool) {
        executors->stop();
    }
    return OK;
}

int PSysDAG::resume()
{
    mPolicyManager->setActive(true);
    return OK;
}

int PSysDAG::pause()
{
    mPolicyManager->setActive(false);
    return OK;
}

void PSysDAG::addTask(PSysTaskData taskParam)
{
    LOG2("@%s, mCameraId:%d", __func__, mCameraId);

    if (taskParam.mTuningMode != mTuningMode) {
        tuningReconfig(taskParam.mTuningMode);
    }

    TaskInfo task = {};
    {
        // Save the task data into mOngoingTasks
        task.mTaskData = taskParam;
        // Count how many valid output buffers need to be returned.
        for (auto& outBuf : taskParam.mOutputBuffers) {
            if (outBuf.second) {
                task.mNumOfValidBuffers++;
            }
        }
        LOG2("%s:Id:%d push task with %d output buffers, sequence: %ld",
                __func__, mCameraId, task.mNumOfValidBuffers,
                taskParam.mInputBuffers.at(mDefaultMainInputPort)->getSequence());
        AutoMutex taskLock(mTaskLock);
        mOngoingTasks.push_back(task);
    }

    // It's too early to runIspAdapt here, and the ipu parameters
    // may be incorrect when runPipe.
    bool runIspAdaptor = true;

    long sequence = taskParam.mInputBuffers.at(mDefaultMainInputPort)->getSequence();
    if (runIspAdaptor) {
        LOG2("%s, run AIC before execute psys for sequence: %ld", __func__, sequence);
        prepareIpuParams(sequence, false, &task);
    }

    queueBuffers(taskParam);

    if (runIspAdaptor && mRunAicAfterQbuf) {
        LOG2("%s, run AIC bundle with execute psys for sequence: %ld", __func__, sequence + 1);
        // if running psys bundle with aic, current aic result is for next sequence
        prepareIpuParams((sequence + 1), false, &task);
    }
}

int PSysDAG::getParameters(Parameters& param)
{
    return mIspParamAdaptor->getParameters(param);
}

TuningMode PSysDAG::getTuningMode(long sequence)
{
    AutoMutex taskLock(mTaskLock);

    TuningMode taskTuningMode = mTuningMode;
    bool taskTuningModeFound = false;

    for (auto const& task : mOngoingTasks) {
        if (sequence == task.mTaskData.mInputBuffers.at(mDefaultMainInputPort)->getSequence()) {
            taskTuningMode = task.mTaskData.mTuningMode;
            taskTuningModeFound = true;
            break;
        }
    }

    if (!taskTuningModeFound) {
        LOGW("No task tuning mode found for sequence:%ld, use current DAG tuning mode.", sequence);
    }

    return taskTuningMode;
}

/**
 * Use to handle the frame done event from the executors.
 *
 * This is for returning output buffers to PSysDAG. And it'll check if all the valid
 * output buffer returned, if so, then it'll return the whole corresponding task data to
 * PSysProcessor.
 */
int PSysDAG::onFrameDone(Port port, const std::shared_ptr<CameraBuffer>& buffer)
{
    LOG2("@%s, mCameraId:%d buffer=%p", __func__, mCameraId, buffer.get());

    if (!buffer) return OK; // No need to handle if the buffer is nullptr.

    long sequence = buffer->getSequence();
    bool needReturn = false;
    Port outputPort = INVALID_PORT;
    bool fakeTask = false;
    PSysTaskData result;
    {
        AutoMutex taskLock(mTaskLock);
        for (auto it = mOngoingTasks.begin(); it != mOngoingTasks.end(); it++) {
            // Check if the returned buffer belong to the task.
            if (sequence != it->mTaskData.mInputBuffers.at(mDefaultMainInputPort)->getSequence()) {
                continue;
            }

            // Check if buffer belongs to the task because input buffer maybe reused
            for (auto& buf : it->mTaskData.mOutputBuffers) {
                if (buf.second && (buffer->getUserBuffer() == buf.second->getUserBuffer())) {
                    outputPort = buf.first;
                }
            }
            if (outputPort == INVALID_PORT) continue;

            fakeTask = it->mTaskData.mFakeTask;
            it->mNumOfReturnedBuffers++;
            if (it->mNumOfReturnedBuffers >= it->mNumOfValidBuffers) {
                result = it->mTaskData;
                needReturn = true;
                LOG2("%s:Id:%d finish task with %d returned output buffers, sequence: %ld",
                     __func__, mCameraId, it->mNumOfReturnedBuffers, sequence);
                // Remove the task data from mOngoingTasks since it's already processed.
                mOngoingTasks.erase(it);

                // Remove the sequence when finish to process the task
                AutoMutex l(mOngoingPalMapLock);
                mOngoingPalMap.erase(sequence);
            }
            // No need check other if other tasks are matched with the returned buffer since
            // we already found one.
            break;
        }
    }

    // Don't handle buffer done if it is a fake task
    if (!fakeTask) {
        CheckError(outputPort == INVALID_PORT, INVALID_OPERATION, "outputPort is invalid");
        // Return buffer
        mPSysDagCB->onBufferDone(sequence, outputPort, buffer);
    }

    if (needReturn) {
        returnBuffers(result);
    }

    return OK;
}

int PSysDAG::prepareIpuParams(long sequence, bool forceUpdate, TaskInfo *task)
{
    TRACE_LOG_PROCESS("PSysDAG", __func__, MAKE_COLOR(sequence), sequence);
    if (task == nullptr) {
        AutoMutex taskLock(mTaskLock);
        for (size_t i = 0; i < mOngoingTasks.size(); i++) {
            if (sequence ==
                mOngoingTasks[i].mTaskData.mInputBuffers.at(mDefaultMainInputPort)->getSequence()) {
                task = &mOngoingTasks[i];
                break;
            }
        }
    }
    CheckError(!task, UNKNOWN_ERROR,
               "%s, Failed to find the task for sequence: %ld", __func__, sequence);

    // According to the output port to filter the valid executor stream Ids, and then run AIC
    std::vector<int32_t> activeStreamIds;
    for (auto& outputFrame : task->mTaskData.mOutputBuffers) {
        if (outputFrame.second.get() == nullptr)
            continue;

        std::map<Port, std::vector<int32_t> >::iterator
            it = mOutputPortToStreamIds.find(outputFrame.first);
        CheckError(it == mOutputPortToStreamIds.end(), UNKNOWN_ERROR,
                   "%s, failed to find streamIds for output port: %d",
                   __func__, outputFrame.first);

        for (auto& streamId : it->second) {
            if (std::find(activeStreamIds.begin(), activeStreamIds.end(), streamId)
                    == activeStreamIds.end()) {
                activeStreamIds.push_back(streamId);
            }
        }
    }
    LOG2("%s, the active streamId size for aic(sequence: %ld) is %zu",
         __func__, sequence, activeStreamIds.size());

    int ret = OK;
    for (auto& id : activeStreamIds) {
        // Make sure the AIC is executed once.
        if (!forceUpdate) {
            AutoMutex l(mOngoingPalMapLock);

            if (mOngoingPalMap.find(sequence) != mOngoingPalMap.end()) {
                // Check if stream id is available.
                if (mOngoingPalMap[sequence].find(id) != mOngoingPalMap[sequence].end()) {
                    // This means aic for the sequence has been executed.
                    continue;
                }
            }
        }

        ret = mIspParamAdaptor->runIspAdapt(&task->mTaskData.mIspSettings, sequence, id);
        CheckError(ret != OK, UNKNOWN_ERROR,
                   "%s, Failed to run AIC: sequence: %ld streamId: %d", __func__, sequence, id);

        // Store the new sequence.
        AutoMutex l(mOngoingPalMapLock);
        mOngoingPalMap[sequence].insert(id);
    }

    return OK;
}

int PSysDAG::returnBuffers(PSysTaskData& result)
{
    LOG2("@%s, mCameraId:%d", __func__, mCameraId);

    CheckError(!mPSysDagCB, INVALID_OPERATION, "Invalid PSysProcessor");

    mPSysDagCB->onFrameDone(result);
    return OK;
}

void PSysDAG::registerListener(EventType eventType, EventListener* eventListener)
{
    //Pass through event registration to PipeExecutor
    for (auto const& executor : mExecutorsPool) {
        executor->registerListener(eventType, eventListener);
    }
}

void PSysDAG::removeListener(EventType eventType, EventListener* eventListener)
{
    //Pass through event unregistration to PipeExecutor
    for (auto const& executor : mExecutorsPool) {
        executor->removeListener(eventType, eventListener);
    }
}

void PSysDAG::tuningReconfig(TuningMode newTuningMode)
{
    LOG1("@%s ", __func__);

    if (mIspParamAdaptor) {
        mIspParamAdaptor->deinit();
    } else {
        mIspParamAdaptor = new IspParamAdaptor(mCameraId, PG_PARAM_PSYS_ISA);
    }

    int ret = mIspParamAdaptor->init();
    CheckError(ret != OK, VOID_VALUE, "Init isp Adaptor failed, tuningMode %d", newTuningMode);

    ret = mIspParamAdaptor->configure(mInputFrameInfo[mDefaultMainInputPort], mConfigMode, newTuningMode);
    CheckError(ret != OK, VOID_VALUE, "Failed to reconfig isp Adaptor.");

    mTuningMode = newTuningMode;
}

void PSysDAG::dumpExternalPortMap()
{
    for (auto& inputMap : mInputMaps) {
        if (inputMap.mExecutor) {
            LOG2("@%s: Input port %d, executor: %s:%d", __func__, inputMap.mDagPort,
                 inputMap.mExecutor->getName(), inputMap.mExecutorPort);
        } else {
            LOGE("%s: no executro for input port %d!", __func__, inputMap.mDagPort);
        }
    }
    for (auto& outputMap : mOutputMaps) {
        if (outputMap.mExecutor) {
            LOG2("@%s: Output port %d, executor: %s:%d", __func__, outputMap.mDagPort,
                 outputMap.mExecutor->getName(), outputMap.mExecutorPort);
        } else {
            LOGE("%s: no executro for output port %d!", __func__, outputMap.mDagPort);
        }
    }
}

}
