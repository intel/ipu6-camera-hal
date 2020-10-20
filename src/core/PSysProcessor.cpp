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

#define LOG_TAG "PSysProcessor"

#include <utility>

#include "iutils/Utils.h"
#include "iutils/CameraLog.h"
#include "iutils/CameraDump.h"
#include "iutils/SwImageConverter.h"

#include "PlatformData.h"
#include "3a/AiqResultStorage.h"
#include "ParameterGenerator.h"

#include "PSysProcessor.h"

/*
 * The sof event time margin is a tunning value
 * it's based on sensor vblank, psys iterating time
 * and thread scheduling
 */
#define SOF_EVENT_MARGIN (5000000)  // 5ms
#define SOF_EVENT_MAX_MARGIN (60000000)  // 60ms
// GPU TNR needs to run extra 2 iterators to converge
#define TNR_CONVERGE_COUNT 2

using std::shared_ptr;
using std::unique_ptr;

namespace icamera {
PSysProcessor::PSysProcessor(int cameraId, ParameterGenerator *pGenerator) :
        mCameraId(cameraId),
        mParameterGenerator(pGenerator),
        mCurConfigMode(CAMERA_STREAM_CONFIGURATION_MODE_NORMAL),
        mTuningMode(TUNING_MODE_MAX),
        mRawPort(INVALID_PORT),
        mSofSequence(-1),
        mOpaqueRawPort(INVALID_PORT),
        mHoldRawBuffers(false),
        mLastStillTnrSequence(-1),
        mStatus(PIPELINE_UNCREATED)
{
    LOG1("@%s camera id:%d", __func__, mCameraId);

    mProcessThread = new ProcessThread(this);
    CLEAR(mSofTimestamp);
}

PSysProcessor::~PSysProcessor()
{
    LOG1("@%s ", __func__);

    mProcessThread->join();
    delete mProcessThread;
}

int PSysProcessor::configure(const std::vector<ConfigMode>& configModes)
{
    //Create PSysDAGs actually
    LOG1("@%s ", __func__);
    CheckError(mStatus == PIPELINE_CREATED, -1, "@%s mStatus is in wrong status: PIPELINE_CREATED", __func__);
    mConfigModes = configModes;
    mSofSequence = -1;

    mLastStillTnrSequence = -1;
    mHoldRawBuffers = false;
    mOpaqueRawPort = INVALID_PORT;
    mRawPort = INVALID_PORT;

    std::map<Port, stream_t> outputFrameInfo;
    stream_t stillStream = {}, videoStream = {};
    for (auto &outFrameInfo : mOutputFrameInfo) {
        // Check if it's required to output raw image from ISYS
        if (outFrameInfo.second.format == V4L2_PIX_FMT_SGRBG12) {
            mRawPort = outFrameInfo.first;
        } else if (outFrameInfo.second.usage == CAMERA_STREAM_OPAQUE_RAW) {
            mOpaqueRawPort = outFrameInfo.first;
            mHoldRawBuffers = true;
        } else if (outFrameInfo.second.usage == CAMERA_STREAM_STILL_CAPTURE) {
            stillStream = outFrameInfo.second;
            outputFrameInfo[outFrameInfo.first] = outFrameInfo.second;
        } else {
            videoStream = outFrameInfo.second;
            outputFrameInfo[outFrameInfo.first] = outFrameInfo.second;
        }
    }

    // Hold raw for SDV case with GPU TNR enabled
    if (PlatformData::isGpuTnrEnabled() && videoStream.width > 0 && stillStream.width > 0) {
        // hold raw buffer for running TNR in still pipe
        mHoldRawBuffers = true;
    }

    int ret = OK;
    //Create PSysDAG according to real configure mode
    for (auto &cfg : mConfigModes) {
        if (mPSysDAGs.find(cfg) != mPSysDAGs.end()) {
            continue;
        }

        TuningConfig tuningConfig;
        ret = PlatformData::getTuningConfigByConfigMode(mCameraId, cfg, tuningConfig);
        CheckError(ret != OK, ret, "%s: can't get config for mode %d", __func__, cfg);

        LOG1("Create PSysDAG for ConfigMode %d", cfg);
        unique_ptr<PSysDAG> pSysDAG = unique_ptr<PSysDAG>(new PSysDAG(mCameraId, this));

        pSysDAG->setFrameInfo(mInputFrameInfo, outputFrameInfo);
        ret = pSysDAG->configure(tuningConfig.configMode, tuningConfig.tuningMode);
        CheckError(ret != OK, ret, "@%s configure psys dag failed:%d", __func__, ret);

        mPSysDAGs[tuningConfig.configMode] = std::move(pSysDAG);

        //Update default active config mode
        mCurConfigMode = tuningConfig.configMode;
        mTuningMode = tuningConfig.tuningMode;
    }

    if (ret == OK) mStatus = PIPELINE_CREATED;
    return ret;

}

int PSysProcessor::registerUserOutputBufs(Port port, const shared_ptr<CameraBuffer> &camBuffer)
{
    for (auto &psysDAGPair : mPSysDAGs) {
        if (!psysDAGPair.second) continue;
        int ret = psysDAGPair.second->registerUserOutputBufs(port, camBuffer);
        CheckError(ret != OK, BAD_VALUE, "%s, register user buffer failed, ret: %d", __func__, ret);
    }

    return OK;
}

int PSysProcessor::start()
{
    PERF_CAMERA_ATRACE();
    LOG1("%s", __func__);
    AutoMutex   l(mBufferQueueLock);
    int rawBufferNum = mHoldRawBuffers ? PlatformData::getMaxRawDataNum(mCameraId) :
                       PlatformData::getPreferredBufQSize(mCameraId);

    /* Should use MIN_BUFFER_COUNT to optimize frame latency when PSYS processing
     * time is slower than ISYS
     */
    bool needProducerBuffer = PlatformData::isIsysEnabled(mCameraId);

    if (needProducerBuffer) {
        int ret = allocProducerBuffers(mCameraId, rawBufferNum);
        CheckError(ret != OK, NO_MEMORY, "Allocating producer buffer failed:%d", ret);
    }

    mThreadRunning = true;
    mProcessThread->run("PsysProcessor", PRIORITY_NORMAL);
    for (auto &psysDAGPair : mPSysDAGs) {
        if (!psysDAGPair.second) continue;
        psysDAGPair.second->start();
        if (needProducerBuffer && PlatformData::isNeedToPreRegisterBuffer(mCameraId)) {
            psysDAGPair.second->registerInternalBufs(mInternalBuffers);
        }
    }

    return OK;
}

void PSysProcessor::stop()
{
    PERF_CAMERA_ATRACE();
    LOG1("%s", __func__);

    for (auto &psysDAGPair : mPSysDAGs) {
        if (!psysDAGPair.second) continue;
        psysDAGPair.second->stop();
    }

    mProcessThread->requestExit();
    {
        AutoMutex l(mBufferQueueLock);
        mThreadRunning = false;
        //Wakeup the thread to exit
        mFrameAvailableSignal.signal();
        mOutputAvailableSignal.signal();
        mFrameDoneSignal.signal();
        AutoMutex lMeta(mMetaQueueLock);
        mMetaAvailableSignal.signal();
    }
    {
        AutoMutex l(mSofLock);
        mSofCondition.signal();
    }

    mProcessThread->requestExitAndWait();

    // Thread is not running. It is safe to clear the Queue
    clearBufferQueues();
}

int PSysProcessor::setParameters(const Parameters& param)
{
    LOG1("%s camera id:%d", __func__, mCameraId);
    // Process image enhancement related settings.
    camera_image_enhancement_t enhancement;
    int ret = param.getImageEnhancement(enhancement);
    AutoWMutex wl(mIspSettingsLock);
    if (ret == OK) {
        mIspSettings.manualSettings.manualSharpness = (char)enhancement.sharpness;
        mIspSettings.manualSettings.manualBrightness = (char)enhancement.brightness;
        mIspSettings.manualSettings.manualContrast = (char)enhancement.contrast;
        mIspSettings.manualSettings.manualHue = (char)enhancement.hue;
        mIspSettings.manualSettings.manualSaturation = (char)enhancement.saturation;
        mIspSettings.eeSetting.strength = enhancement.sharpness;
    } else {
        mIspSettings.eeSetting.strength = 0;
    }
    LOG2("%s: manual edge strength set: %d", __func__, mIspSettings.eeSetting.strength);

    camera_edge_mode_t manualEdgeMode;
    ret = param.getEdgeMode(manualEdgeMode);
    if (ret == OK) {
        LOG2("%s: manual edge mode set: %d", __func__, manualEdgeMode);
        switch (manualEdgeMode) {
            case EDGE_MODE_OFF:
                mIspSettings.eeSetting.feature_level = ia_isp_feature_level_off;
                break;
            case EDGE_MODE_FAST:
                mIspSettings.eeSetting.feature_level = ia_isp_feature_level_high;
                break;
            case EDGE_MODE_HIGH_QUALITY:
                mIspSettings.eeSetting.feature_level = ia_isp_feature_level_high;
                break;
            case EDGE_MODE_ZERO_SHUTTER_LAGE:
                mIspSettings.eeSetting.feature_level = ia_isp_feature_level_off;
                break;
            default:
                mIspSettings.eeSetting.feature_level = ia_isp_feature_level_high;
        }
    } else {
        mIspSettings.eeSetting.feature_level = ia_isp_feature_level_high;
    }

    camera_nr_mode_t manualNrMode;
    camera_nr_level_t manualNrLevel;

    int manualNrModeSet = param.getNrMode(manualNrMode);
    int manualNrLevelSet = param.getNrLevel(manualNrLevel);

    if (manualNrModeSet == OK) {
        LOG2("%s: manual NR mode set: %d", __func__, manualNrMode);
        switch (manualNrMode) {
            case NR_MODE_OFF:
                mIspSettings.nrSetting.feature_level = ia_isp_feature_level_off;
                break;
            case NR_MODE_AUTO:
                mIspSettings.nrSetting.feature_level = ia_isp_feature_level_low;
                break;
            case NR_MODE_MANUAL_NORMAL:
                mIspSettings.nrSetting.feature_level = ia_isp_feature_level_low;
                break;
            case NR_MODE_MANUAL_EXPERT:
            case NR_MODE_HIGH_QUALITY:
                mIspSettings.nrSetting.feature_level = ia_isp_feature_level_high;
                break;
            default:
                mIspSettings.nrSetting.feature_level = ia_isp_feature_level_low;
        }
    } else {
        LOG2("%s: manual NR mode not set, default enabled", __func__);
        mIspSettings.nrSetting.feature_level = ia_isp_feature_level_high;
    }

    if (manualNrLevelSet == OK) {
        LOG2("%s: manual NR level set: %d", __func__, manualNrLevel.overall);
        mIspSettings.nrSetting.strength = (char)manualNrLevel.overall;
    } else {
        LOG2("%s: manual NR level not set, default used", __func__);
        mIspSettings.nrSetting.strength = (char)0;
    }

    LOG2("%s: ISP NR setting, level: %d, strength: %d",
            __func__, (int)mIspSettings.nrSetting.feature_level,
            (int)mIspSettings.nrSetting.strength);

    camera_video_stabilization_mode_t stabilizationMode;
    ret = param.getVideoStabilizationMode(stabilizationMode);
    if (ret == OK) {
         mIspSettings.videoStabilization = (stabilizationMode == VIDEO_STABILIZATION_MODE_ON);
    } else {
         mIspSettings.videoStabilization = false;
    }
    LOG2("%s: Video stablilization enabled:%d", __func__, mIspSettings.videoStabilization);

    return ret;
}

int PSysProcessor::getParameters(Parameters& param)
{
    LOG1("@%s ", __func__);
    AutoRMutex rl(mIspSettingsLock);
    camera_image_enhancement_t enhancement = { mIspSettings.manualSettings.manualSharpness,
                                               mIspSettings.manualSettings.manualBrightness,
                                               mIspSettings.manualSettings.manualContrast,
                                               mIspSettings.manualSettings.manualHue,
                                               mIspSettings.manualSettings.manualSaturation };
    int ret = param.setImageEnhancement(enhancement);

    ret |= mPSysDAGs[mCurConfigMode]->getParameters(param);

    return ret;
}

/**
 * Get available setting sequence from outBuf
 */
long PSysProcessor::getSettingSequence(const CameraBufferPortMap &outBuf)
{
    long settingSequence = -1;
    for (auto& output: outBuf) {
        if (output.second) {
            settingSequence = output.second->getSettingSequence();
            break;
        }
    }
    return settingSequence;
}

/**
 * Check if the input frame should be skipped
 *
 * If the corresponding mSkip of AiqResult gotten from sequence is true,
 * return true; otherwise return false.
 */
bool PSysProcessor::needSkipOutputFrame(long sequence)
{
    // Check if need to skip output frame
    const AiqResult* aiqResults = AiqResultStorage::getInstance(mCameraId)->getAiqResult(sequence);
    if (aiqResults != nullptr && aiqResults->mSkip) {
        LOG1("%s, sequence %ld", __func__, sequence);
        return true;
    }
    return false;
}

/**
 * Check if 'inBuffer' can be used for 'settingSequence' to run PSys pipe.
 *
 * If 'settingSequence' is -1, it means the output buffer doesn't require particular
 * input buffer, so it can run the pipe.
 * If 'inputSequence' larger than 'settingSequence', the pipeline needs to
 * run as well, otherwise the pipe doesn't need to run and this input buffer needs to
 * be skipped.
 */
bool PSysProcessor::needExecutePipe(long settingSequence, long inputSequence)
{
    if (settingSequence == -1 || inputSequence >= settingSequence) {
        return true;
    }

    return false;
}

/**
 * Check if the input buffer need to be reused
 *
 * If 'settingSequence' is -1, it means the output buffer doesn't require particular
 * input buffer, so the input buffer doesn't need to be reused.
 * If 'inputSequence' larger than 'settingSequence', means the input buffer
 * may be required by following output buffer, so it may be reused later.
 */
bool PSysProcessor::needHoldOnInputFrame(long settingSequence, long inputSequence)
{
    if (settingSequence == -1 || inputSequence <= settingSequence) {
        return false;
    }

    return true;
}

/**
 * Check if pipe needs to be switched according to AIQ result.
 */
bool PSysProcessor::needSwitchPipe(long sequence)
{
    const AiqResult* aiqResults = AiqResultStorage::getInstance(mCameraId)->getAiqResult(sequence);
    if (aiqResults == nullptr) {
        LOG2("%s: not found sequence %ld in AiqResultStorage, no update for active modes",
            __func__, sequence);
        return false;
    }

    TuningMode curTuningMode = aiqResults->mTuningMode;
    LOG2("%s: aiqResults->mTuningMode = %d", __func__, curTuningMode);

    if (mTuningMode == curTuningMode) {
        return false;
    }

    for (auto cfg : mConfigModes) {
        TuningMode tMode;
        int ret = PlatformData::getTuningModeByConfigMode(mCameraId, cfg, tMode);
        if (ret == OK && tMode == curTuningMode) {
            mCurConfigMode = cfg;
            mTuningMode = curTuningMode;
            return true;
        }
    }
    return false;
}

void PSysProcessor::handleEvent(EventData eventData)
{
    LOG2("%s: got event type %d", __func__, eventData.type);
    // Process registered events
    switch (eventData.type) {
        case EVENT_ISYS_SOF:
            {
                AutoMutex l(mSofLock);

                mSofSequence = eventData.data.sync.sequence;
                gettimeofday(&mSofTimestamp, nullptr);
                LOG2("%s, received SOF event sequence: %ld, timestamp: %ld",
                     __func__, eventData.data.sync.sequence, TIMEVAL2USECS(mSofTimestamp));
                mSofCondition.signal();
            }
            break;
        default:
            LOGW("Unexpected event: %d", eventData.type);
            break;
    }
}

// PSysProcessor ThreadLoop
int PSysProcessor::processNewFrame() {
    PERF_CAMERA_ATRACE();
    LOG2("@%s, mCameraId:%d", __func__, mCameraId);

    CheckError(!mBufferProducer, INVALID_OPERATION, "No available producer");

    int ret = OK;
    CameraBufferPortMap srcBuffers, dstBuffers;

    if (!PlatformData::psysAlignWithSof(mCameraId)) {
        {
            ConditionLock lock(mBufferQueueLock);
            ret = waitFreeBuffersInQueue(lock, srcBuffers, dstBuffers);
            // Already stopped
            if (!mThreadRunning) return -1;

            // Wait frame buffer time out should not involve thread exit.
            if (ret != OK) {
                LOG1("%s, cameraId: %d timeout happen, wait recovery", __func__, mCameraId);
                return OK;
            }
        }

        ret = prepareTask(&srcBuffers, &dstBuffers);
        CheckError(ret != OK, UNKNOWN_ERROR, "%s, Failed to process frame", __func__);
    } else {
        {
            ConditionLock lock(mSofLock);

            timeval curTime;
            gettimeofday(&curTime, nullptr);
            int64_t sofInterval = TIMEVAL2NSECS(curTime) - TIMEVAL2NSECS(mSofTimestamp);

            // Wait next sof event when missing last one for a long time
            if (sofInterval > SOF_EVENT_MARGIN && sofInterval < SOF_EVENT_MAX_MARGIN) {
                LOG2("%s, need to wait next sof event. sofInterval: %ld", __func__, sofInterval);
                ret = mSofCondition.waitRelative(lock, kWaitDuration * SLOWLY_MULTIPLIER);

                // Already stopped
                if (!mThreadRunning) return -1;

                // Wait sof event time out should not involve thread exit.
                if (ret != OK) {
                    LOG1("%s, cameraId: %d wait sof event timeout, recovery", __func__, mCameraId);
                    return OK;
                }
            }
        }

        // push all the pending buffers to task
        while (true) {
            {
                ConditionLock lock(mBufferQueueLock);
                ret = waitFreeBuffersInQueue(lock, srcBuffers, dstBuffers, SOF_EVENT_MARGIN);

                // Return to wait next sof event if there isn't pending buffer.
                if (ret != OK) {
                    LOG1("%s, cameraId: %d, there isn't pending buffer, recovery",
                         __func__, mCameraId);
                    return OK;
                }
            }

            {
                AutoMutex l(mSofLock);
                if (srcBuffers.begin()->second->getSequence() >= mSofSequence) {
                    LOG2("%s, run the frame in next sof: buffer sequence: %ld, sof sequence: %ld",
                         __func__, srcBuffers.begin()->second->getSequence(), mSofSequence);
                    return OK;
                }
            }

            ret = prepareTask(&srcBuffers, &dstBuffers);
            CheckError(ret != OK, UNKNOWN_ERROR, "%s, Failed to process frame", __func__);
        }
    }

    return OK;
}

void PSysProcessor::handleRawReprocessing(CameraBufferPortMap *srcBuffers,
                                          CameraBufferPortMap *dstBuffers, bool *allBufDone,
                                          bool *hasRawOutput, bool *hasRawInput)
{
    std::shared_ptr<CameraBuffer> rawOutputBuffer = nullptr;
    long settingSequence = -1;
    CameraBufferPortMap videoBuf, stillBuf;

    for (const auto& item : *dstBuffers) {
        if (item.second) {
            if (item.second->getStreamUsage() == CAMERA_STREAM_OPAQUE_RAW) {
                rawOutputBuffer = item.second;
            } else if (item.second->getStreamUsage() == CAMERA_STREAM_STILL_CAPTURE) {
                stillBuf[item.first] = item.second;
            } else {
                videoBuf[item.first] = item.second;
            }
            if (item.second->getSettingSequence() >= 0) {
                settingSequence = item.second->getSettingSequence();
            }
        }
    }

    Port defaultPort = srcBuffers->begin()->first;
    shared_ptr<CameraBuffer> mainBuf = (*srcBuffers)[defaultPort];
    long inputSequence = mainBuf->getSequence();

    if (rawOutputBuffer) {
        if (!needExecutePipe(settingSequence, inputSequence)) {
            LOG2("%s, inputSequence %ld is smaller than settingSequence %ld, skip sensor frame.",
                 __func__, inputSequence, settingSequence);
            return;
        }
        // Return opaque RAW data
        uint64_t timestamp = TIMEVAL2NSECS(mainBuf->getTimestamp());
        sensor_raw_info_t opaqueRawInfo = { inputSequence, timestamp };

        rawOutputBuffer->updateV4l2Buffer(*mainBuf->getV4L2Buffer().Get());

        MEMCPY_S(rawOutputBuffer->getBufferAddr(), rawOutputBuffer->getBufferSize(),
                 &opaqueRawInfo, sizeof(opaqueRawInfo));
        LOG2("%s, timestamp %ld, inputSequence %ld, dstBufferSize %d, addr %p", __func__,
              timestamp, inputSequence, rawOutputBuffer->getBufferSize(),
              rawOutputBuffer->getBufferAddr());

        // Return opaque RAW buffer
        for (auto &it : mBufferConsumerList) {
            it->onFrameAvailable(mOpaqueRawPort, rawOutputBuffer);
        }
        *hasRawOutput = true;

        PlatformData::updateMakernoteTimeStamp(mCameraId, settingSequence, timestamp);

        if (stillBuf.empty() && videoBuf.empty()) {
            *allBufDone = true;
        } else {
            // Remove from dstBuffers map
            dstBuffers->erase(mOpaqueRawPort);
        }
    } else if (settingSequence != -1 && inputSequence > settingSequence) {
        // handle Shutter first
        for (const auto& output : *dstBuffers) {
            if (output.second && output.second->getUsage() != BUFFER_USAGE_PSYS_INTERNAL) {
                EventData event;
                event.type = EVENT_PSYS_REQUEST_BUF_READY;
                event.buffer = nullptr;
                event.data.requestReady.timestamp = output.second->getUserBuffer()->timestamp;
                event.data.requestReady.sequence = settingSequence;
                notifyListeners(event);
                break;
            }
        }
        // only one video buffer is supported
        if (PlatformData::isGpuTnrEnabled() && videoBuf.size() == 1) {
            shared_ptr<CameraBuffer> buf = videoBuf.begin()->second;
            bool handled = mPSysDAGs[mCurConfigMode]->fetchTnrRefBuffer(settingSequence, buf);
            if (handled) {
                LOG2("%s, settingSequence %ld is found", __func__, settingSequence);
                if (stillBuf.empty()) {
                    *hasRawInput = true;
                    *allBufDone = true;
                } else {
                    (*dstBuffers)[videoBuf.begin()->first] = nullptr;
                }
                onBufferDone(settingSequence, videoBuf.begin()->first, buf);
            }
        }
        if (mHoldRawBuffers && !(*allBufDone)) {
            AutoMutex lock(mBufferMapLock);
            // Find Raw buffer in mRawBufferMap
            if (mRawBufferMap.find(settingSequence) != mRawBufferMap.end()) {
                CameraBufferPortMap &mapBuf = mRawBufferMap[settingSequence];
                // Update source buffers
                for (const auto& bufPortMap : mapBuf) {
                    (*srcBuffers)[bufPortMap.first] = bufPortMap.second;
                }
                *hasRawInput = true;
            }
        }
    }

    LOG2("%s, hasRawInput %d, hasRawOutput %d, allBufDone %d, settingSequence %ld, inputSequence %ld",
          __func__, *hasRawInput, *hasRawOutput, *allBufDone, settingSequence, inputSequence);
}

bool PSysProcessor::isBufferHoldForRawReprocess(long sequence)
{
    if (!mHoldRawBuffers) return false;

    AutoMutex lock(mBufferMapLock);
    if (mRawBufferMap.find(sequence) == mRawBufferMap.end()) return false;

    return true;
}

void PSysProcessor::saveRawBuffer(CameraBufferPortMap *srcBuffers)
{
    // Save buffer into mRawBufferMap
    CameraBufferPortMap mapBuf;
    for (const auto& src : *srcBuffers) {
        mapBuf[src.first] = src.second;
    }

    Port defaultPort = srcBuffers->begin()->first;
    shared_ptr<CameraBuffer> mainBuf = (*srcBuffers)[defaultPort];
    long inputSequence = mainBuf->getSequence();

    LOG2("@%s, mCameraId:%d, inputSequence %ld", __func__, mCameraId, inputSequence);

    AutoMutex lock(mBufferMapLock);
    mRawBufferMap[inputSequence] = mapBuf;

    // If too many buffers are holden in mRawQueue, return back to producer
    if (mRawBufferMap.size() > (PlatformData::getMaxRawDataNum(mCameraId) -
                                PlatformData::getMaxRequestsInflight(mCameraId))) {
        std::map<long, CameraBufferPortMap>::iterator it = mRawBufferMap.begin();
        CameraBufferPortMap &bufferPortMap = it->second;
        for (auto &item : bufferPortMap) {
            mBufferProducer->qbuf(item.first, item.second);
        }
        mRawBufferMap.erase(mRawBufferMap.begin());
    }
}

status_t PSysProcessor::prepareTask(CameraBufferPortMap *srcBuffers,
                                    CameraBufferPortMap *dstBuffers) {
    CheckError(srcBuffers->empty() || dstBuffers->empty(),
               UNKNOWN_ERROR, "%s, the input or output buffer is empty", __func__);
    if (mHoldRawBuffers && mOpaqueRawPort == INVALID_PORT) {
        saveRawBuffer(srcBuffers);
    }

    bool allBufDone = false;
    bool hasRawOutput = false;
    bool hasRawInput = false;
    if (mOpaqueRawPort != INVALID_PORT) {
        handleRawReprocessing(srcBuffers, dstBuffers, &allBufDone, &hasRawOutput, &hasRawInput);
        if (hasRawOutput) {
            saveRawBuffer(srcBuffers);
        }
        if (allBufDone) {
            sendPsysFrameDoneEvent(dstBuffers);
            // If all buffers are handled
            AutoMutex l(mBufferQueueLock);
            if (hasRawOutput) {
                for (auto& input: mInputQueue) {
                    input.second.pop();
                }
            }
            for (auto& output: mOutputQueue) {
                output.second.pop();
            }
            return OK;
        }
    }

    Port defaultPort = srcBuffers->begin()->first;
    shared_ptr<CameraBuffer> mainBuf = (*srcBuffers)[defaultPort];
    long inputSequence = mainBuf->getSequence();
    TRACE_LOG_POINT("PSysProcessor", "input output buffer ready", MAKE_COLOR(inputSequence),
                    inputSequence);
    uint64_t timestamp = TIMEVAL2NSECS(mainBuf->getTimestamp());
    LOG2("%s: input buffer sequence %ld timestamp %ld", __func__, inputSequence, timestamp);

    // Output raw image
    if (mRawPort != INVALID_PORT) {
        shared_ptr<CameraBuffer> dstBuf = nullptr;

        // Get output buffer and remove it from dstBuffers
        for (auto &buffer : *dstBuffers) {
            if (buffer.first == mRawPort) {
                dstBuf = buffer.second;
                CheckError(!dstBuf, UNKNOWN_ERROR, "%s, dstBuf for output raw is null", __func__);
                dstBuf->updateV4l2Buffer(*mainBuf->getV4L2Buffer().Get());
                dstBuffers->erase(mRawPort);
                break;
            }
        }
        outputRawImage(mainBuf, dstBuf);
    }

    long settingSequence = getSettingSequence(*dstBuffers);
    bool needRunPipe = needExecutePipe(settingSequence, inputSequence);
    bool holdOnInput = needHoldOnInputFrame(settingSequence, inputSequence);
    LOG2("%s: dst sequence = %ld, src sequence = %ld, needRunPipe = %d, needReuseInput = %d",
         __func__, settingSequence, inputSequence, needRunPipe, holdOnInput);

    {
        AutoMutex l(mBufferQueueLock);
        if (needRunPipe && !needSkipOutputFrame(inputSequence)) {
            for (auto& output: mOutputQueue) {
                output.second.pop();
            }
        }

        // If input buffer will be used later, don't pop it from the queue.
        if (!holdOnInput && !hasRawInput) {
            for (auto& input: mInputQueue) {
                input.second.pop();
            }
        }
    }

    if (needRunPipe) {
        // Raw output already has been returned back, and don't need to handle again.
        if (!hasRawOutput) {
            for (const auto& output : *dstBuffers) {
                if (output.second && output.second->getUsage() != BUFFER_USAGE_PSYS_INTERNAL) {
                    EventData event;
                    event.type = EVENT_PSYS_REQUEST_BUF_READY;
                    event.buffer = nullptr;
                    event.data.requestReady.timestamp = timestamp;
                    event.data.requestReady.sequence = settingSequence;
                    notifyListeners(event);
                    break;
                }
            }
        }
        if (PlatformData::isGpuTnrEnabled()) {
            handleStillPipeForTnr(inputSequence, dstBuffers);
        }
        dispatchTask(*srcBuffers, *dstBuffers);
    } else if (!holdOnInput && !isBufferHoldForRawReprocess(inputSequence)) {
        for (const auto& src : *srcBuffers) {
            mBufferProducer->qbuf(src.first, src.second);
        }
    }

    return OK;
}

void PSysProcessor::handleStillPipeForTnr(long sequence, CameraBufferPortMap *dstBuffers)
{
    bool hasStill = false;
    for (const auto& item : *dstBuffers) {
        if (item.second && item.second->getStreamUsage() == CAMERA_STREAM_STILL_CAPTURE) {
            hasStill = true;
            break;
        }
    }
    LOG2("@%s, seq %ld, hold raw %d, last still seq %ld, still %d", __func__, sequence,
         mHoldRawBuffers, mLastStillTnrSequence, hasStill);

    if (hasStill && sequence != (mLastStillTnrSequence + 1) && mHoldRawBuffers) {
        CameraBufferPortMap fakeTaskBuffers = *dstBuffers;
        for (const auto& item : fakeTaskBuffers) {
            if (item.second && item.second->getStreamUsage() != CAMERA_STREAM_STILL_CAPTURE) {
                fakeTaskBuffers[item.first] = nullptr;
            }
        }
        for (int i = TNR_CONVERGE_COUNT; i > 0; i--) {
            CameraBufferPortMap srcBuf;
            {
            AutoMutex lock(mBufferMapLock);
            if (sequence - i != mLastStillTnrSequence &&
                mRawBufferMap.find(sequence - i) != mRawBufferMap.end()) {
                for (const auto& item : mRawBufferMap[sequence - i]) {
                    srcBuf[item.first] = item.second;
                }
            }
            }
            if (!srcBuf.empty()) {
                dispatchTask(srcBuf, fakeTaskBuffers, true);
            }
        }
    }

    if (hasStill) mLastStillTnrSequence = sequence;
}

void PSysProcessor::dispatchTask(CameraBufferPortMap &inBuf, CameraBufferPortMap &outBuf,
                                 bool fakeTask)
{
    long currentSequence = inBuf.begin()->second->getSequence();
    TRACE_LOG_POINT("PSysProcessor", "start run PSYS", MAKE_COLOR(currentSequence),
                    currentSequence);

    LOG2("@%s, mCameraId:%d, seq %ld, fake task %d", __func__, mCameraId, currentSequence, fakeTask);

    {
        ConditionLock lock(mBufferQueueLock);

        ConfigMode previousMode = mCurConfigMode;
        bool needSwitch = needSwitchPipe(currentSequence);

        if (needSwitch) {
            LOG1("Switch pipe for sequence:%ld, unprocessed buffer number:%zu",
                  currentSequence, mSequenceInflight.size());

            // Deactive the PSysDag which is no longer used.
            mPSysDAGs[previousMode]->pause();

            // Before switching, need to wait all buffers in current pipe being processed.
            while (!mSequenceInflight.empty()) {
                int ret = mFrameDoneSignal.waitRelative(lock, kWaitDuration * SLOWLY_MULTIPLIER);
                if (!mThreadRunning) {
                    LOG1("@%s: Processor is not active while waiting for frame done.", __func__);
                    return;
                }

                if (ret == TIMED_OUT) {
                    LOGE("Waiting for frame done event timeout");
                    return;
                }
            }

            // Activate the current used PSysDag.
            mPSysDAGs[mCurConfigMode]->resume();
        }
        mSequenceInflight.push(currentSequence);
    } // End of lock mBufferQueueLock

    // Prepare the task input paramerters including input and output buffers, settings etc.
    PSysTaskData taskParam;
    taskParam.mTuningMode = mTuningMode;
    taskParam.mInputBuffers = inBuf;
    taskParam.mOutputBuffers = outBuf;
    taskParam.mFakeTask = fakeTask;

    long settingSequence = getSettingSequence(outBuf);
    // Handle per-frame settings if output buffer requires
    if (settingSequence > -1 && mParameterGenerator) {
        Parameters params;
        if (mParameterGenerator->getParameters(currentSequence, &params, false) == OK) {
            setParameters(params);
        }
    }
    {
        AutoRMutex rl(mIspSettingsLock);
        mIspSettings.palOverride = nullptr;
        taskParam.mIspSettings = mIspSettings;
    }

    if (!mThreadRunning) return;

    mPSysDAGs[mCurConfigMode]->addTask(taskParam);
}

void PSysProcessor::registerListener(EventType eventType, EventListener* eventListener)
{
    // Only delegate stats event registration to deeper layer DAG and PipeExecutor
    if ((eventType != EVENT_PSYS_STATS_BUF_READY) && (eventType != EVENT_PSYS_STATS_SIS_BUF_READY)) {
        BufferQueue::registerListener(eventType, eventListener);
        return;
    }

    for (auto const& realModeDAGPair: mPSysDAGs) {
        realModeDAGPair.second->registerListener(eventType, eventListener);
    }
}

void PSysProcessor::removeListener(EventType eventType, EventListener* eventListener)
{
    // Only delegate stats event unregistration to deeper layer DAG and PipeExecutor
    if ((eventType != EVENT_PSYS_STATS_BUF_READY) && (eventType != EVENT_PSYS_STATS_SIS_BUF_READY)) {
        BufferQueue::removeListener(eventType, eventListener);
        return;
    }

    for (auto const& realModeDAGPair: mPSysDAGs) {
        realModeDAGPair.second->removeListener(eventType, eventListener);
    }
}

void PSysProcessor::onBufferDone(int64_t sequence, Port port,
                                 const std::shared_ptr<CameraBuffer> &camBuffer)
{
    LOG2("@%s, mCameraId:%d, sequence %ld, port %d", __func__, mCameraId, sequence, port);

    if (CameraDump::isDumpTypeEnable(DUMP_PSYS_OUTPUT_BUFFER)) {
        CameraDump::dumpImage(mCameraId, camBuffer, M_PSYS, port);
    }

    if (!needSkipOutputFrame(sequence)) {
        for (auto &it : mBufferConsumerList) {
            it->onFrameAvailable(port, camBuffer);
        }
    }
}

void PSysProcessor::sendPsysFrameDoneEvent(const CameraBufferPortMap* dstBuffers)
{
    for (auto& dst : *dstBuffers) {
        shared_ptr<CameraBuffer> outBuf = dst.second;
        if (!outBuf || outBuf->getSequence() < 0) {
            continue;
        }

        EventData frameData;
        frameData.type = EVENT_PSYS_FRAME;
        frameData.buffer = nullptr;
        frameData.data.frame.sequence = outBuf->getSequence();
        frameData.data.frame.timestamp.tv_sec = outBuf->getTimestamp().tv_sec;
        frameData.data.frame.timestamp.tv_usec = outBuf->getTimestamp().tv_usec;
        notifyListeners(frameData);

        LOG2("%s, frame done for sequence: %ld", __func__, frameData.data.frame.sequence);
        break;
    }
}

void PSysProcessor::onFrameDone(const PSysTaskData& result)
{
    PERF_CAMERA_ATRACE();
    LOG2("@%s, mCameraId:%d", __func__, mCameraId);

    long sequence = result.mInputBuffers.begin()->second->getSequence();
    TRACE_LOG_POINT("PSysProcessor", __func__, MAKE_COLOR(sequence), sequence);

    if (!needSkipOutputFrame(sequence)) {
        sendPsysFrameDoneEvent(&result.mOutputBuffers);
    }

    long settingSequence = getSettingSequence(result.mOutputBuffers);
    bool holdOnInput = needHoldOnInputFrame(settingSequence, sequence);
    bool hasRawOutput = isBufferHoldForRawReprocess(sequence);

    // Return buffer only if the buffer is not used in the future.
    if (!holdOnInput && mBufferProducer && !hasRawOutput) {
        for (const auto& src : result.mInputBuffers) {
            mBufferProducer->qbuf(src.first, src.second);

            if (src.second->getStreamType() == CAMERA_STREAM_INPUT) {
                for (auto &it : mBufferConsumerList) {
                    it->onFrameAvailable(src.first, src.second);
                }
            }
        }
    }

    AutoMutex l(mBufferQueueLock);
    long oldest = mSequenceInflight.front();
    if (sequence != oldest) {
        // The output buffer should always be FIFO.
        LOGW("The sequence should be %ld, but it's %ld", oldest, sequence);
    }

    mSequenceInflight.pop();
    if (mSequenceInflight.empty()) {
        mFrameDoneSignal.signal();
    }
}

void PSysProcessor::outputRawImage(shared_ptr<CameraBuffer> &srcBuf, shared_ptr<CameraBuffer> &dstBuf)
{
    if ((srcBuf == nullptr) || (dstBuf == nullptr)) {
        return;
    }

    // Copy from source buffer
    int srcBufferSize = srcBuf->getBufferSize();
    int srcMemoryType = srcBuf->getMemory();
    void* pSrcBuf = (srcMemoryType == V4L2_MEMORY_DMABUF)
                    ? CameraBuffer::mapDmaBufferAddr(srcBuf->getFd(), srcBufferSize)
                    : srcBuf->getBufferAddr();

    int dstBufferSize = dstBuf->getBufferSize();
    int dstMemoryType = dstBuf->getMemory();
    void* pDstBuf = (dstMemoryType == V4L2_MEMORY_DMABUF)
                    ? CameraBuffer::mapDmaBufferAddr(dstBuf->getFd(), dstBufferSize)
                    : dstBuf->getBufferAddr();

    MEMCPY_S(pDstBuf, dstBufferSize, pSrcBuf, srcBufferSize);

    if (srcMemoryType == V4L2_MEMORY_DMABUF) {
        CameraBuffer::unmapDmaBufferAddr(pSrcBuf, srcBufferSize);
    }

    if (dstMemoryType == V4L2_MEMORY_DMABUF) {
        CameraBuffer::unmapDmaBufferAddr(pDstBuf, dstBufferSize);
    }

    // Send output buffer to its consumer
    for (auto &it : mBufferConsumerList) {
        it->onFrameAvailable(mRawPort, dstBuf);
    }
}

} //namespace icamera
