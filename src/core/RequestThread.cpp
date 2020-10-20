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

#define LOG_TAG "RequestThread"

#include "iutils/Errors.h"
#include "iutils/CameraLog.h"

#include "RequestThread.h"

using std::vector;
using std::shared_ptr;

namespace icamera {

RequestThread::RequestThread(int cameraId, AiqUnitBase *a3AControl, ParameterGenerator* aParamGen) :
    mCameraId(cameraId),
    m3AControl(a3AControl),
    mParamGenerator(aParamGen),
    mPerframeControlSupport(false),
    mGet3AStatWithFakeRequest(false),
    mRequestsInProcessing(0),
    mFirstRequest(true),
    mRequestConfigMode(CAMERA_STREAM_CONFIGURATION_MODE_END),
    mUserConfigMode(CAMERA_STREAM_CONFIGURATION_MODE_END),
    mNeedReconfigPipe(false),
    mReconfigPipeScore(0),
    mActive(false),
    mLastRequestId(-1),
    mLastPredictSeq(-1),
    mLastEofSeq(-1),
    mBlockRequest(true)
{
    CLEAR(mStreamConfig);
    CLEAR(mConfiguredStreams);
    CLEAR(mFakeReqBuf);

    mStreamConfig.operation_mode = CAMERA_STREAM_CONFIGURATION_MODE_END;
    mPerframeControlSupport = PlatformData::isFeatureSupported(mCameraId, PER_FRAME_CONTROL);
}

RequestThread::~RequestThread()
{
    while (!mReqParamsPool.empty()) {
        mReqParamsPool.pop();
    }
}

void RequestThread::requestExit()
{
    clearRequests();

    Thread::requestExit();
    AutoMutex l(mPendingReqLock);
    mRequestSignal.signal();
}

void RequestThread::clearRequests()
{
    LOG1("%s", __func__);

    mActive = false;
    for (int streamId = 0; streamId < MAX_STREAM_NUMBER; streamId++) {
        FrameQueue& frameQueue = mOutputFrames[streamId];
        AutoMutex lock(frameQueue.mFrameMutex);
        while (!frameQueue.mFrameQueue.empty()) {
            frameQueue.mFrameQueue.pop();
        }
        frameQueue.mFrameAvailableSignal.broadcast();
    }

    AutoMutex l(mPendingReqLock);
    mRequestsInProcessing = 0;
    while (!mPendingRequests.empty()) {
        mPendingRequests.pop_back();
    }

    mLastRequestId = -1;
    mLastPredictSeq = -1;
    mLastEofSeq = -1;
    mFirstRequest = true;
    mBlockRequest = true;
}

void RequestThread::setConfigureModeByParam(const Parameters& param)
{
    camera_scene_mode_t sceneMode = SCENE_MODE_MAX;
    if (param.getSceneMode(sceneMode) != OK) {
        return;
    }

    ConfigMode configMode = CameraUtils::getConfigModeBySceneMode(sceneMode);
    LOG2("@%s, sceneMode %d, configMode %d", __func__, sceneMode, configMode);

    if (configMode == CAMERA_STREAM_CONFIGURATION_MODE_END) {
        LOG2("%s: no valid config mode, skip setting", __func__);
        return;
    }

    /* Reset internal mode related settings if requested mode is same as
     * the mode currently running for better stability.
     */
    if (mStreamConfig.operation_mode == configMode) {
        LOG2("%s: config mode %d keep unchanged.", __func__, configMode);
        mNeedReconfigPipe = false;
        mReconfigPipeScore = 0;
        mRequestConfigMode = configMode;
        return;
    }

    if (mRequestConfigMode != configMode) {
        if (mRequestConfigMode != CAMERA_STREAM_CONFIGURATION_MODE_END) {
            mNeedReconfigPipe = true;
            mReconfigPipeScore = 0;
            LOG2("%s: request configure mode changed, reset score %d", __func__, mReconfigPipeScore);
        }
        LOG2("%s: mRequestConfigMode updated from %d to %d", __func__, mRequestConfigMode, configMode);
        mRequestConfigMode = configMode;
    } else if (mReconfigPipeScore < PlatformData::getPipeSwitchDelayFrame(mCameraId)) {
        mReconfigPipeScore ++;
        LOG2("%s: request configure mode unchanged, current score %d", __func__, mReconfigPipeScore);
    }
}

int RequestThread::configure(const stream_config_t *streamList) {
    bool hasVideoStream = false;

    mStreamConfig.num_streams = streamList->num_streams;
    mStreamConfig.operation_mode = streamList->operation_mode;
    mUserConfigMode = (ConfigMode)streamList->operation_mode;
    int previewStreamIndex = -1;
    for (int i = 0; i < streamList->num_streams; i++) {
        mConfiguredStreams[i] = streamList->streams[i];
        if (previewStreamIndex < 0 && mConfiguredStreams[i].usage == CAMERA_STREAM_PREVIEW) {
            previewStreamIndex = i;
        }
        if (mConfiguredStreams[i].usage == CAMERA_STREAM_PREVIEW ||
            mConfiguredStreams[i].usage == CAMERA_STREAM_VIDEO_CAPTURE) {
            hasVideoStream = true;
        }
    }
    LOG1("%s: user specified Configmode: %d, hasVideoStream %d", __func__, mUserConfigMode, hasVideoStream);
    mStreamConfig.streams = mConfiguredStreams;

    // Use concrete mode in RequestThread
    if ((ConfigMode)mStreamConfig.operation_mode == CAMERA_STREAM_CONFIGURATION_MODE_AUTO) {
        vector <ConfigMode> configModes;
        int ret = PlatformData::getConfigModesByOperationMode(mCameraId,
                                                              mStreamConfig.operation_mode,
                                                              configModes);
        CheckError((ret != OK || configModes.empty()), ret,
                   "%s, get real ConfigMode failed %d", __func__, ret);

        mRequestConfigMode = configModes[0];
        LOG2("%s: use concrete mode %d as default initial mode for auto op mode",
             __func__, mRequestConfigMode);
        mStreamConfig.operation_mode = mRequestConfigMode;
    }

    // Don't block request handling if no 3A stats (from video pipe)
    mBlockRequest = PlatformData::isEnableAIQ(mCameraId) && hasVideoStream;

    LOG2("%s: mRequestConfigMode initial value: %d", __func__, mRequestConfigMode);
    return OK;
}

void RequestThread::postConfigure(const stream_config_t *streamList) {
    mGet3AStatWithFakeRequest =
        mPerframeControlSupport ? PlatformData::isPsysContinueStats(mCameraId) : false;

    if (!mGet3AStatWithFakeRequest) return;

    CLEAR(mFakeReqBuf);
    int fakeStreamIndex = -1, videoIndex = -1, stillIndex = -1;
    for (int i = 0; i < streamList->num_streams; i++) {
        // Select preview stream firstly
        if (streamList->streams[i].usage == CAMERA_STREAM_PREVIEW) {
            fakeStreamIndex = i;
            break;
        } else if (streamList->streams[i].usage == CAMERA_STREAM_VIDEO_CAPTURE) {
            videoIndex = i;
        } else if (streamList->streams[i].usage == CAMERA_STREAM_STILL_CAPTURE) {
            stillIndex = i;
        }
    }
    if (fakeStreamIndex < 0) {
        if (videoIndex >= 0) {
            fakeStreamIndex = videoIndex;
        } else if (stillIndex >=0) {
            fakeStreamIndex = stillIndex;
        }
    }

    if (fakeStreamIndex < 0) {
        LOGW("There isn't valid stream to trigger stats event");
        mGet3AStatWithFakeRequest = false;
        return;
    }

    stream_t &fakeStream = streamList->streams[fakeStreamIndex];
    LOG2("%s: create fake request with stream index %d", __func__, fakeStreamIndex);
    mFakeBuffer = CameraBuffer::create(mCameraId, BUFFER_USAGE_PSYS_INTERNAL, V4L2_MEMORY_USERPTR,
                                       fakeStream.size, 0, fakeStream.format, fakeStream.width,
                                       fakeStream.height);

    mFakeReqBuf.s = fakeStream;
    mFakeReqBuf.s.memType = V4L2_MEMORY_USERPTR;
    mFakeReqBuf.addr = mFakeBuffer->getUserBuffer()->addr;
}

bool RequestThread::blockRequest() {
    /**
     * Block request processing if:
     * 1. mBlockRequest is true (except the 1st request), or
     * 2. Too many requests in flight.
     */
    return ((mBlockRequest && (mLastRequestId >= 0)) ||
            mRequestsInProcessing >= PlatformData::getMaxRequestsInflight(mCameraId));
}

int RequestThread::processRequest(int bufferNum, camera_buffer_t **ubuffer, const Parameters* params)
{
    AutoMutex l(mPendingReqLock);
    CameraRequest request;
    request.mBufferNum = bufferNum;
    bool hasVideoBuffer = false;

    for (int id = 0; id < bufferNum; id++) {
        request.mBuffer[id] = ubuffer[id];
        if (ubuffer[id]->s.usage == CAMERA_STREAM_PREVIEW ||
            ubuffer[id]->s.usage == CAMERA_STREAM_VIDEO_CAPTURE) {
            hasVideoBuffer = true;
        }
    }

    if (mFirstRequest && !hasVideoBuffer) {
        LOG2("there is no video buffer in first request, so don't block request processing.");
        mBlockRequest = false;
    }

    request.mParams = copyRequestParams(params);
    mPendingRequests.push_back(request);

    if (!mActive) {
        mActive = true;
    }

    mRequestSignal.signal();
    return OK;
}

shared_ptr<Parameters>
RequestThread::copyRequestParams(const Parameters *srcParams)
{
    if (srcParams == nullptr)
        return nullptr;

    if (mReqParamsPool.empty()) {
        shared_ptr<Parameters> sParams = std::make_shared<Parameters>();
        CheckError(!sParams, nullptr, "%s: no memory!", __func__);
        mReqParamsPool.push(sParams);
    }

    shared_ptr<Parameters> sParams = mReqParamsPool.front();
    mReqParamsPool.pop();
    *sParams = *srcParams;
    return sParams;
}

int RequestThread::waitFrame(int streamId, camera_buffer_t **ubuffer)
{
    FrameQueue& frameQueue = mOutputFrames[streamId];
    ConditionLock lock(frameQueue.mFrameMutex);

    while (frameQueue.mFrameQueue.empty()) {
        int ret = frameQueue.mFrameAvailableSignal.waitRelative(
                      lock,
                      kWaitFrameDuration * SLOWLY_MULTIPLIER);
        if (!mActive) return INVALID_OPERATION;

        if (ret == TIMED_OUT) {
            LOGW("@%s, mCameraId:%d, time out happens, wait recovery", __func__, mCameraId);
            return ret;
        }
    }

    shared_ptr<CameraBuffer> camBuffer = frameQueue.mFrameQueue.front();
    frameQueue.mFrameQueue.pop();
    *ubuffer = camBuffer->getUserBuffer();

    LOG2("@%s, frame returned. camera id:%d, stream id:%d", __func__, mCameraId, streamId);

    return OK;
}

int RequestThread::wait1stRequestDone()
{
    LOG1("%s", __func__);
    int ret = OK;
    ConditionLock lock(mFirstRequestLock);
    if (mFirstRequest) {
        LOG1("%s, waiting the first request done", __func__);
        ret = mFirstRequestSignal.waitRelative(
                  lock,
                  kWaitFirstRequestDoneDuration * SLOWLY_MULTIPLIER);
        if (ret == TIMED_OUT)
            LOGE("@%s: Wait 1st request timed out", __func__);
    }

    return ret;
}

void RequestThread::handleEvent(EventData eventData)
{
    if (!mActive) return;

    /* Notes:
      * There should be only one of EVENT_ISYS_FRAME
      * and EVENT_PSYS_FRAME registered.
      * There should be only one of EVENT_xx_STATS_BUF_READY
      * registered.
      */
    switch (eventData.type) {
        case EVENT_ISYS_FRAME:
        case EVENT_PSYS_FRAME:
            {
                AutoMutex l(mPendingReqLock);
                if (mRequestsInProcessing > 0) {
                    mRequestsInProcessing--;
                }
                // Continue process(that maight be blocked due to many requests in flight).
                if (!mPendingRequests.empty()) {
                    mRequestSignal.signal();
                }
            }
            break;
        case EVENT_PSYS_STATS_BUF_READY:
            {
                TRACE_LOG_POINT("RequestThread", "receive the stat event");
                AutoMutex l(mPendingReqLock);
                if (mBlockRequest) {
                    mBlockRequest = false;
                    mRequestSignal.signal();
                }
            }
            break;
        case EVENT_ISYS_SOF: {
            AutoMutex l(mPendingReqLock);
            mLastEofSeq = eventData.data.sync.sequence - 1;
            break;
        }
        case EVENT_FRAME_AVAILABLE:
            {
                if (eventData.buffer->getUserBuffer() != &mFakeReqBuf) {
                    int streamId = eventData.data.frameDone.streamId;
                    FrameQueue& frameQueue = mOutputFrames[streamId];

                    AutoMutex lock(frameQueue.mFrameMutex);
                    bool needSignal = frameQueue.mFrameQueue.empty();
                    frameQueue.mFrameQueue.push(eventData.buffer);
                    if (needSignal) {
                        frameQueue.mFrameAvailableSignal.signal();
                    }
                } else {
                    LOG2("%s: fake request return %ld", __func__, eventData.buffer->getSequence());
                }

                AutoMutex l(mPendingReqLock);
                // Insert fake request if no any request in the HAL to keep 3A running
                if (mGet3AStatWithFakeRequest &&
                    eventData.buffer->getSequence() >= mLastPredictSeq &&
                    mPendingRequests.empty()) {
                    LOGW("No request, insert fake req after req %d to keep 3A stats update",
                         mLastRequestId);
                    CameraRequest fakeRequest;
                    fakeRequest.mBufferNum = 1;
                    fakeRequest.mBuffer[0] = &mFakeReqBuf;
                    mFakeReqBuf.sequence = -1;
                    mPendingRequests.push_back(fakeRequest);
                    mRequestSignal.signal();
                }
            }
            break;
        default:
            {
                LOGW("Unknown event type %d", eventData.type);
            }
            break;
    }
}

/**
 * Get the next request for processing.
 * Return false if no pending requests or it is not ready for reconfiguration.
 */
bool RequestThread::fetchNextRequest(CameraRequest& request)
{
    ConditionLock lock(mPendingReqLock);
    if (isReconfigurationNeeded() && !isReadyForReconfigure()) {
        return false;
    }

    if (mPendingRequests.empty()) {
        return false;
    }

    request = mPendingRequests.front();
    mRequestsInProcessing++;
    mPendingRequests.pop_front();
    LOG2("@%s, mRequestsInProcessing %d", __func__, mRequestsInProcessing);
    return true;
}

/**
 * Check if ConfigMode is changed or not.
 * If new ConfigMode is different with previous configured ConfigMode,
 * return true.
 */
bool RequestThread::isReconfigurationNeeded()
{
    bool needReconfig = (mUserConfigMode == CAMERA_STREAM_CONFIGURATION_MODE_AUTO &&
                         PlatformData::getAutoSwitchType(mCameraId) == AUTO_SWITCH_FULL &&
                         mNeedReconfigPipe &&
                         (mReconfigPipeScore >= PlatformData::getPipeSwitchDelayFrame(mCameraId)));
    LOG2("%s: need reconfigure %d, score %d, decision %d",
         __func__, mNeedReconfigPipe, mReconfigPipeScore, needReconfig);
    return needReconfig;
}

/**
 * If reconfiguration is needed, there are 2 extra conditions for reconfiguration:
 * 1, there is no buffer in processing; 2, there is buffer in mPendingRequests.
 * Return true if reconfiguration is ready.
 */
bool RequestThread::isReadyForReconfigure()
{
    return (!mPendingRequests.empty() && mRequestsInProcessing == 0);
}

bool RequestThread::threadLoop()
{
    bool restart = false;
    {
         ConditionLock lock(mPendingReqLock);

         bool waitProcessing = blockRequest() || mPendingRequests.empty();
         if (waitProcessing) {
            int ret = mRequestSignal.waitRelative(lock, kWaitDuration * SLOWLY_MULTIPLIER);
            waitProcessing = blockRequest() || mPendingRequests.empty();
            if (ret == TIMED_OUT || waitProcessing) {
                LOGW("%s: wait event time out", __func__);
                return true;
            }
        }

        restart = isReconfigurationNeeded();
    }

    if (!mActive) {
        return false;
    }

    CameraRequest request;
    if (fetchNextRequest(request)) {
        // Update for reconfiguration
        if (request.mParams.get()) {
            setConfigureModeByParam(*(request.mParams.get()));
        }
        // Re-check
        if (restart && isReconfigurationNeeded()) {
            handleReconfig();
        }

        handleRequest(request);
    }
    return true;
}

void RequestThread::handleReconfig()
{
    LOG1("%s, ConfigMode change from %x to %x", __func__,
            mStreamConfig.operation_mode, mRequestConfigMode);
    mStreamConfig.operation_mode = mRequestConfigMode;
    EventConfigData configData;
    configData.streamList = &mStreamConfig;
    EventData eventData;
    eventData.type = EVENT_DEVICE_RECONFIGURE;
    eventData.data.config = configData;
    notifyListeners(eventData);
    mNeedReconfigPipe = false;
    mReconfigPipeScore = 0;
    return;
}

void RequestThread::handleRequest(CameraRequest& request)
{
    long predictSequence = mLastPredictSeq + 1;
    if (mLastPredictSeq < 0) {
        predictSequence = PlatformData::getInitialSkipFrame(mCameraId);
    } else if (predictSequence < mLastEofSeq) {
        // Drop early frames
        predictSequence = mLastEofSeq;
    }

    // Raw reprocessing case, don't run 3A.
    if (request.mBuffer[0]->sequence >= 0 && request.mBuffer[0]->timestamp > 0) {
        predictSequence = request.mBuffer[0]->sequence;
        if (request.mParams.get()) {
            mParamGenerator->updateParameters(predictSequence, request.mParams.get());
        }
    } else {
        if (request.mParams.get()) {
            m3AControl->setParameters(*request.mParams);
        }
        m3AControl->run3A(mPerframeControlSupport? &predictSequence : nullptr);

        // Check the final prediction value
        if (predictSequence <= mLastPredictSeq) {
            LOGW("predict %ld error! should > %ld", predictSequence, mLastPredictSeq);
            predictSequence = mLastPredictSeq + 1;
        }

        mLastPredictSeq = predictSequence;
        mLastRequestId++;
        mParamGenerator->saveParameters(mLastPredictSeq, mLastRequestId, request.mParams.get());
    }
    LOG2("%s: Process request: %ld:%ld, out buffer %d, param? %s", __func__,
          mLastRequestId, predictSequence, request.mBufferNum,
          request.mParams.get() ? "true" : "false");

    // Sent event to handle request buffers
    EventRequestData requestData;
    requestData.bufferNum = request.mBufferNum;
    requestData.buffer = request.mBuffer;
    requestData.param = request.mParams.get();
    requestData.settingSeq = predictSequence;
    EventData eventData;
    eventData.type = EVENT_PROCESS_REQUEST;
    eventData.data.request = requestData;
    notifyListeners(eventData);

    // Recycle params ptr for re-using
    if (request.mParams) {
        AutoMutex l(mPendingReqLock);
        mReqParamsPool.push(request.mParams);
    }

    {
        AutoMutex l(mFirstRequestLock);
        if (mFirstRequest) {
            LOG1("%s: first request done", __func__);
            mFirstRequest = false;
            mFirstRequestSignal.signal();
        }
    }
}

} //namespace icamera
