/*
 * Copyright (C) 2015-2024 Intel Corporation.
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

#define LOG_TAG RequestThread

#include "iutils/Errors.h"
#include "iutils/CameraLog.h"

#include "RequestThread.h"

using std::shared_ptr;
using std::vector;

namespace icamera {

RequestThread::RequestThread(int cameraId, AiqUnitBase* a3AControl, ParameterGenerator* aParamGen)
        : mCameraId(cameraId),
          m3AControl(a3AControl),
          mParamGenerator(aParamGen),
          mPerframeControlSupport(false),
          mGet3AStatWithFakeRequest(false),
          mRequestsInProcessing(0),
          mFirstRequest(true),
          mActive(false),
          mRequestTriggerEvent(NONE_EVENT),
          mLastRequestId(-1),
          mLastEffectSeq(-1),
          mLastAppliedSeq(-1),
          mLastSofSeq(-1),
          mBlockRequest(true),
          mSofEnabled(false),
          mWaitFrameDurationOverride(0) {
    CLEAR(mFakeReqBuf);

    mPerframeControlSupport = PlatformData::isFeatureSupported(mCameraId, PER_FRAME_CONTROL);

    mSofEnabled = PlatformData::isIsysEnabled(cameraId);
    // FILE_SOURCE_S
    mSofEnabled = mSofEnabled || PlatformData::isFileSourceEnabled();
    // FILE_SOURCE_E
    mWaitFrameDurationOverride = PlatformData::getReqWaitTimeout(cameraId);
    LOG1("%s: Set mWaitFrameDurationOverride: %lld", __func__, mWaitFrameDurationOverride);
}

RequestThread::~RequestThread() {}

void RequestThread::requestExit() {
    clearRequests();

    Thread::requestExit();
    AutoMutex l(mPendingReqLock);
    mRequestSignal.signal();
}

void RequestThread::clearRequests() {
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
    mLastEffectSeq = -1;
    mLastAppliedSeq = -1;
    mLastSofSeq = -1;
    mFirstRequest = true;
    mBlockRequest = true;
}

int RequestThread::configure(const stream_config_t* streamList) {
    int previewIndex = -1, videoIndex = -1, stillIndex = -1;
    for (int i = 0; i < streamList->num_streams; i++) {
        if (streamList->streams[i].usage == CAMERA_STREAM_PREVIEW) {
            previewIndex = i;
        } else if (streamList->streams[i].usage == CAMERA_STREAM_VIDEO_CAPTURE) {
            videoIndex = i;
        } else if (streamList->streams[i].usage == CAMERA_STREAM_STILL_CAPTURE) {
            stillIndex = i;
        }
    }

    // Don't block request handling if no 3A stats (from video pipe)
    mBlockRequest = PlatformData::isEnableAIQ(mCameraId) && (previewIndex >= 0 || videoIndex >= 0);
    LOG1("%s: user specified Configmode: %d, blockRequest: %d", __func__,
         static_cast<ConfigMode>(streamList->operation_mode), mBlockRequest);

    mGet3AStatWithFakeRequest =
        mPerframeControlSupport ? PlatformData::isPsysContinueStats(mCameraId) : false;
    if (mGet3AStatWithFakeRequest) {
        int fakeStreamIndex =
            (previewIndex >= 0) ? previewIndex : ((videoIndex >= 0) ? videoIndex : stillIndex);
        if (fakeStreamIndex < 0) {
            LOGW("There isn't valid stream to trigger stats event");
            mGet3AStatWithFakeRequest = false;
            return OK;
        }

        CLEAR(mFakeReqBuf);
        stream_t& fakeStream = streamList->streams[fakeStreamIndex];
        LOG2("%s: create fake request with stream index %d", __func__, fakeStreamIndex);
        mFakeBuffer = CameraBuffer::create(mCameraId, BUFFER_USAGE_PSYS_INTERNAL,
                                           V4L2_MEMORY_USERPTR, fakeStream.size, 0,
                                           fakeStream.format, fakeStream.width, fakeStream.height);

        mFakeReqBuf.s = fakeStream;
        mFakeReqBuf.s.memType = V4L2_MEMORY_USERPTR;
        mFakeReqBuf.addr = mFakeBuffer->getUserBuffer()->addr;
    }

    return OK;
}

bool RequestThread::blockRequest() {
    if (mPendingRequests.empty()) return true;

    /**
     * Block request processing if:
     * 1. mBlockRequest is true (except the 1st request);
     * 2. Too many requests in flight;
     * 3. if no trigger event is available.
     */
    return ((mBlockRequest &&
             (mLastRequestId >= PlatformData::getInitialPendingFrame(mCameraId))) ||
            (mRequestsInProcessing >= PlatformData::getMaxRequestsInflight(mCameraId)) ||
            (mPerframeControlSupport && (mRequestTriggerEvent == NONE_EVENT)));
}

int RequestThread::processRequest(int bufferNum, camera_buffer_t** ubuffer,
                                  const Parameters* params) {
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

    request.mRequestParam = copyRequestParams(params);
    mPendingRequests.push_back(request);

    if (!mActive) {
        mActive = true;
    }

    if (mRequestsInProcessing == 0) {
        mRequestTriggerEvent |= NEW_REQUEST;
        mRequestSignal.signal();
    }
    return OK;
}

shared_ptr<RequestParam> RequestThread::copyRequestParams(const Parameters* srcParams) {
    if (srcParams == nullptr) return nullptr;

    std::shared_ptr<RequestParam> requestParam = mParamGenerator->getRequestParamBuf();

    requestParam->param = *srcParams;
    return requestParam;
}

int RequestThread::waitFrame(int streamId, camera_buffer_t** ubuffer) {
    FrameQueue& frameQueue = mOutputFrames[streamId];
    ConditionLock lock(frameQueue.mFrameMutex);

    if (!mActive) return NO_INIT;
    while (frameQueue.mFrameQueue.empty()) {
        int ret = frameQueue.mFrameAvailableSignal.waitRelative(
            lock, (mWaitFrameDurationOverride > 0) ? mWaitFrameDurationOverride :
                                                     (kWaitFrameDuration * SLOWLY_MULTIPLIER));
        if (!mActive) return NO_INIT;

        CheckWarning(ret == TIMED_OUT, ret, "<id%d>@%s, time out happens, wait recovery", mCameraId,
                     __func__);
    }

    shared_ptr<CameraBuffer> camBuffer = frameQueue.mFrameQueue.front();
    frameQueue.mFrameQueue.pop();
    *ubuffer = camBuffer->getUserBuffer();

    LOG2("@%s, frame returned. camera id:%d, stream id:%d", __func__, mCameraId, streamId);

    return OK;
}

int RequestThread::wait1stRequestDone() {
    int ret = OK;
    ConditionLock lock(mFirstRequestLock);
    if (mFirstRequest) {
        LOG2("%s, waiting the first request done", __func__);
        ret = mFirstRequestSignal.waitRelative(lock,
                                               kWaitFirstRequestDoneDuration * SLOWLY_MULTIPLIER);
        if (ret == TIMED_OUT) LOGE("@%s: Wait 1st request timed out", __func__);
    }

    return ret;
}

void RequestThread::handleEvent(EventData eventData) {
    if (!mActive) return;

    /* Notes:
     * There should be only one of EVENT_ISYS_FRAME
     * and EVENT_PSYS_FRAME registered.
     * There should be only one of EVENT_xx_STATS_BUF_READY
     * registered.
     */
    switch (eventData.type) {
        case EVENT_ISYS_FRAME:
        case EVENT_PSYS_FRAME: {
            AutoMutex l(mPendingReqLock);
            if (mRequestsInProcessing > 0) {
                mRequestsInProcessing--;
            }
            // Just in case too many requests are pending in mPendingRequests.
            if (!mPendingRequests.empty() && (mRequestsInProcessing == 0)) {
                mRequestTriggerEvent |= NEW_FRAME;
                mRequestSignal.signal();
            }
        } break;
        case EVENT_PSYS_STATS_BUF_READY: {
            TRACE_LOG_POINT("RequestThread", "receive the stat event");
            AutoMutex l(mPendingReqLock);
            if (mBlockRequest) {
                mBlockRequest = false;
            }
            mRequestTriggerEvent |= NEW_STATS;
            mRequestSignal.signal();
        } break;
        case EVENT_ISYS_SOF: {
            AutoMutex l(mPendingReqLock);
            mLastSofSeq = eventData.data.sync.sequence;
            if (mLastSofSeq > mLastAppliedSeq) {
                mRequestTriggerEvent |= NEW_SOF;
                mRequestSignal.signal();
            }
        } break;
        case EVENT_FRAME_AVAILABLE: {
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
                LOG2("%s: fake request return %u", __func__, eventData.buffer->getSequence());
            }

            AutoMutex l(mPendingReqLock);
            // Insert fake request if no any request in the HAL to keep 3A running
            if (mGet3AStatWithFakeRequest && eventData.buffer->getSequence() >= mLastEffectSeq &&
                mPendingRequests.empty() && (mRequestsInProcessing == 0)) {
                LOGW("No request, insert fake req after req %ld to keep 3A stats update",
                     mLastRequestId);
                CameraRequest fakeRequest;
                fakeRequest.mBufferNum = 1;
                fakeRequest.mBuffer[0] = &mFakeReqBuf;
                mFakeReqBuf.sequence = -1;
                mPendingRequests.push_back(fakeRequest);
                mRequestTriggerEvent |= NEW_REQUEST;
                mRequestSignal.signal();
            }
        } break;
        default: {
            LOGW("Unknown event type %d", eventData.type);
        } break;
    }
}

/**
 * Get the next request for processing.
 * Return false if no pending requests or it is not ready for reconfiguration.
 */
bool RequestThread::fetchNextRequest(CameraRequest& request) {
    ConditionLock lock(mPendingReqLock);
    if (mPendingRequests.empty()) {
        return false;
    }

    request = mPendingRequests.front();
    mRequestsInProcessing++;
    mPendingRequests.pop_front();
    LOG2("@%s, mRequestsInProcessing %d", __func__, mRequestsInProcessing);
    return true;
}

bool RequestThread::threadLoop() {
    int64_t applyingSeq = -1;
    {
        ConditionLock lock(mPendingReqLock);

        if (blockRequest()) {
            int ret = mRequestSignal.waitRelative(lock, kWaitDuration * SLOWLY_MULTIPLIER);
            if (ret == TIMED_OUT) {
                LOG2("wait event time out, %d requests processing, %zu requests in HAL",
                     mRequestsInProcessing, mPendingRequests.size());
                return true;
            }

            if (blockRequest()) {
                LOG2("Pending request processing, mBlockRequest %d, Req in processing %d",
                     mBlockRequest, mRequestsInProcessing);
                mRequestTriggerEvent = NONE_EVENT;
                return true;
            }
        }

        /* for perframe control cases, one request should be processed in one SOF period only.
         * 1, for new SOF, processes request for current sequence if no request processed for it;
         * 2, for new stats, processes request for next sequence;
         * 3, for new request or frame done, processes request only no buffer processed in HAL.
         */
        if (mPerframeControlSupport && mRequestTriggerEvent != NONE_EVENT) {
            if ((mRequestTriggerEvent & NEW_SOF) && (mLastSofSeq > mLastAppliedSeq)) {
                applyingSeq = mLastSofSeq;
            } else if ((mRequestTriggerEvent & NEW_STATS) && (mLastSofSeq >= mLastAppliedSeq)) {
                applyingSeq = mLastSofSeq + 1;
            } else if ((mRequestTriggerEvent & (NEW_REQUEST | NEW_FRAME)) &&
                       (mRequestsInProcessing == 0)) {
                applyingSeq = mLastSofSeq + 1;
            } else {
                mRequestTriggerEvent = NONE_EVENT;
                return true;
            }

            mLastAppliedSeq = applyingSeq;
            if ((mLastAppliedSeq + PlatformData::getExposureLag(mCameraId)) <= mLastEffectSeq) {
                mRequestTriggerEvent = NONE_EVENT;
                LOG2("%s, skip processing request for AE delay issue", __func__);
                return true;
            }
            LOG2("%s, trigger event %x, SOF %ld, predict %ld, processed %d request id %ld",
                 __func__, mRequestTriggerEvent, mLastSofSeq, mLastAppliedSeq,
                 mRequestsInProcessing, mLastRequestId);
        }
    }

    if (!mActive) {
        return false;
    }

    CameraRequest request;
    if (fetchNextRequest(request)) {
        handleRequest(request, applyingSeq);
        {
            AutoMutex l(mPendingReqLock);
            mRequestTriggerEvent = NONE_EVENT;
        }
    }
    return true;
}

void RequestThread::handleRequest(CameraRequest& request, int64_t applyingSeq) {
    int64_t effectSeq = mLastEffectSeq + 1;
    // Raw reprocessing case, don't run 3A.
    if (request.mBuffer[0]->sequence >= 0 && request.mBuffer[0]->timestamp > 0) {
        effectSeq = request.mBuffer[0]->sequence;
        if (request.mRequestParam) {
            mParamGenerator->updateParameters(effectSeq, &request.mRequestParam->param);
        }
        LOG2("%s: Reprocess request: seq %ld, out buffer %d", __func__, effectSeq,
             request.mBufferNum);
    } else {
        long requestId = -1;
        {
            AutoMutex l(mPendingReqLock);
            if (mActive) {
                requestId = ++mLastRequestId;
                if (request.mRequestParam) {
                    m3AControl->setParameters(request.mRequestParam->param);
                }
            }
        }

        if (requestId >= 0) {
            m3AControl->run3A(requestId, applyingSeq, mSofEnabled ? &effectSeq : nullptr);
        }

        {
            AutoMutex l(mPendingReqLock);
            if (!mActive) return;

            // Check the final prediction value from 3A
            if (effectSeq <= mLastEffectSeq) {
                LOG2("predict effectSeq %ld, last effect %ld", effectSeq, mLastEffectSeq);
            }

            mParamGenerator->saveParameters(effectSeq, mLastRequestId, request.mRequestParam);
            mLastEffectSeq = effectSeq;

            LOG2("%s: Process request: %ld:%ld, out buffer %d, param? %s", __func__, mLastRequestId,
                 effectSeq, request.mBufferNum, request.mRequestParam.get() ? "true" : "false");
        }
    }

    // Sent event to handle request buffers
    EventRequestData requestData;
    requestData.bufferNum = request.mBufferNum;
    requestData.buffer = request.mBuffer;
    camera_test_pattern_mode_t testPatternMode = TEST_PATTERN_OFF;
    if (request.mRequestParam) {
        request.mRequestParam->param.getTestPatternMode(testPatternMode);
    }
    requestData.testPatternMode = testPatternMode;
    requestData.settingSeq = effectSeq;
    EventData eventData;
    eventData.type = EVENT_PROCESS_REQUEST;
    eventData.data.request = requestData;
    notifyListeners(eventData);

    {
        AutoMutex l(mFirstRequestLock);
        if (mFirstRequest) {
            LOG1("%s: first request done", __func__);
            mFirstRequest = false;
            mFirstRequestSignal.signal();
        }
    }
}

}  // namespace icamera
