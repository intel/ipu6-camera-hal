/*
 * Copyright (C) 2017-2024 Intel Corporation.
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

#define LOG_TAG PSysProcessor

#include <set>
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
#define TRIGGER_MARGIN (5000000)       // 5ms
#define TRIGGER_MAX_MARGIN (60000000)  // 60ms

#define EXTREME_STRENGTH_LEVEL4 (-120)
#define EXTREME_STRENGTH_LEVEL3 (-60)
#define EXTREME_STRENGTH_LEVEL2 (0)
#define EXTREME_STRENGTH_LEVEL1 (20)

using std::shared_ptr;
using std::unique_ptr;

namespace icamera {
PSysProcessor::PSysProcessor(int cameraId, ParameterGenerator* pGenerator)
        : mCameraId(cameraId),
          mParameterGenerator(pGenerator),
          mScheduler(nullptr),
          mRunAicAfterQTask(false),
          mCurConfigMode(CAMERA_STREAM_CONFIGURATION_MODE_NORMAL),
          mTuningMode(TUNING_MODE_MAX),
          mRawPort(INVALID_PORT),
          mSofSequence(-1),
          mOpaqueRawPort(INVALID_PORT),
          mHoldRawBuffers(false),
          mLastStillTnrSequence(-1),
          mStatus(PIPELINE_UNCREATED) {
    mProcessThread = new ProcessThread(this);
    CLEAR(mSofTimestamp);

    if (PlatformData::isSchedulerEnabled(mCameraId)) mScheduler = new CameraScheduler();
}

PSysProcessor::~PSysProcessor() {
    mProcessThread->join();
    delete mProcessThread;

    /* PipeExecutor is shared between Scheduler and PSysDAG, so need to delete Scheduler
     * before delete PipeExecutor in PSysDAG.
     */
    if (mScheduler) {
        if (mPSysDAGs.find(mCurConfigMode) != mPSysDAGs.end()) {
            mPSysDAGs[mCurConfigMode]->unregisterNode();
        }
        delete mScheduler;
    }
    mPSysDAGs.clear();
}

int PSysProcessor::configure(const std::vector<ConfigMode>& configModes) {
    // Create PSysDAGs actually
    CheckAndLogError(mStatus == PIPELINE_CREATED, -1,
                     "@%s mStatus is in wrong status: PIPELINE_CREATED", __func__);

    mConfigModes = configModes;
    mSofSequence = -1;
    mLastStillTnrSequence = -1;
    mHoldRawBuffers = false;
    mOpaqueRawPort = INVALID_PORT;
    mRawPort = INVALID_PORT;
    mRunAicAfterQTask = false;

    std::map<Port, stream_t> outputFrameInfo;
    stream_t stillStream = {}, videoStream = {};
    for (auto& outFrameInfo : mOutputFrameInfo) {
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
    if (PlatformData::isGpuTnrEnabled(mCameraId) && videoStream.width > 0 &&
        stillStream.width > 0) {
        // hold raw buffer for running TNR in still pipe
        mHoldRawBuffers = true;
    }

    // Only enable psys bundle with aic when has video pipe only
    if (stillStream.width <= 0 && PlatformData::psysBundleWithAic(mCameraId)) {
        mRunAicAfterQTask = true;
    }

    int ret = OK;
    // Create PSysDAG according to real configure mode
    for (auto& cfg : mConfigModes) {
        if (mPSysDAGs.find(cfg) != mPSysDAGs.end()) {
            continue;
        }

        TuningConfig tuningConfig;
        ret = PlatformData::getTuningConfigByConfigMode(mCameraId, cfg, tuningConfig);
        CheckAndLogError(ret != OK, ret, "%s: can't get config for mode %d", __func__, cfg);

        LOG1("%s, Create PSysDAG for ConfigMode %d", __func__, cfg);
        unique_ptr<PSysDAG> pSysDAG = unique_ptr<PSysDAG>(new PSysDAG(mCameraId, mScheduler, this));

        pSysDAG->setFrameInfo(mInputFrameInfo, outputFrameInfo);
        bool useTnrOutBuffer = mOpaqueRawPort != INVALID_PORT;
        ret = pSysDAG->configure(tuningConfig.configMode, tuningConfig.tuningMode, useTnrOutBuffer);
        CheckAndLogError(ret != OK, ret, "@%s configure psys dag failed:%d", __func__, ret);

        mPSysDAGs[tuningConfig.configMode] = std::move(pSysDAG);

        // Update default active config mode
        mCurConfigMode = tuningConfig.configMode;
        mTuningMode = tuningConfig.tuningMode;
    }

    if (ret == OK) mStatus = PIPELINE_CREATED;
    return ret;
}

int PSysProcessor::registerUserOutputBufs(Port port, const shared_ptr<CameraBuffer>& camBuffer) {
    for (auto& psysDAGPair : mPSysDAGs) {
        if (!psysDAGPair.second) continue;
        int ret = psysDAGPair.second->registerUserOutputBufs(port, camBuffer);
        CheckAndLogError(ret != OK, BAD_VALUE, "%s, register user buffer failed, ret: %d", __func__,
                         ret);
    }

    return OK;
}

// Pre-release some resources when stopping stage
void PSysProcessor::stopProcessing() {
    for (auto& psysDAGPair : mPSysDAGs) {
        if (psysDAGPair.second) psysDAGPair.second->stopProcessing();
    }
}

int PSysProcessor::start() {
    PERF_CAMERA_ATRACE();
    AutoMutex l(mBufferQueueLock);
    int rawBufferNum = mHoldRawBuffers ? PlatformData::getMaxRawDataNum(mCameraId) :
                                         PlatformData::getPreferredBufQSize(mCameraId);

    /* Should use MIN_BUFFER_COUNT to optimize frame latency when PSYS processing
     * time is slower than ISYS
     */
    bool needProducerBuffer = PlatformData::isIsysEnabled(mCameraId);

    // FILE_SOURCE_S
    needProducerBuffer = needProducerBuffer || PlatformData::isFileSourceEnabled();
    // FILE_SOURCE_E

    if (needProducerBuffer) {
        int ret = allocProducerBuffers(mCameraId, rawBufferNum);
        CheckAndLogError(ret != OK, NO_MEMORY, "Allocating producer buffer failed:%d", ret);
    }

    {
        AutoMutex lock(mBufferMapLock);
        mRawBufferMap.clear();
    }

    mThreadRunning = true;
    CLEAR(mSofTimestamp);
    mProcessThread->run("PsysProcessor", PRIORITY_NORMAL);
    for (auto& psysDAGPair : mPSysDAGs) {
        if (!psysDAGPair.second) continue;
        psysDAGPair.second->start();
        if (needProducerBuffer && PlatformData::isNeedToPreRegisterBuffer(mCameraId)) {
            psysDAGPair.second->registerInternalBufs(mInternalBuffers);
        }
    }

    return OK;
}

void PSysProcessor::stop() {
    PERF_CAMERA_ATRACE();

    for (auto& psysDAGPair : mPSysDAGs) {
        if (!psysDAGPair.second) continue;
        psysDAGPair.second->stop();
    }

    mProcessThread->requestExit();
    {
        AutoMutex l(mBufferQueueLock);
        mThreadRunning = false;
        // Wakeup the thread to exit
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

int PSysProcessor::setParameters(const Parameters& param) {
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
        mIspSettings.eeSetting.strength = static_cast<char>(EXTREME_STRENGTH_LEVEL2);
    }

    mIspSettings.eeSetting.feature_level = ia_isp_feature_level_high;
    camera_edge_mode_t manualEdgeMode;
    ret = param.getEdgeMode(manualEdgeMode);
    if (ret == OK) {
        LOG2("%s: manual edge mode set: %d", __func__, manualEdgeMode);
        switch (manualEdgeMode) {
            case EDGE_MODE_LEVEL_4:
                mIspSettings.eeSetting.strength = static_cast<char>(EXTREME_STRENGTH_LEVEL4);
                break;
            case EDGE_MODE_LEVEL_3:
                mIspSettings.eeSetting.strength = static_cast<char>(EXTREME_STRENGTH_LEVEL3);
                break;
            case EDGE_MODE_LEVEL_2:
                mIspSettings.eeSetting.strength = static_cast<char>(EXTREME_STRENGTH_LEVEL2);
                break;
            case EDGE_MODE_LEVEL_1:
                mIspSettings.eeSetting.strength = static_cast<char>(EXTREME_STRENGTH_LEVEL1);
                break;
            default:
                mIspSettings.eeSetting.strength = static_cast<char>(EXTREME_STRENGTH_LEVEL2);
        }
    }

    LOG2("%s: ISP EE setting, level: %d, strength: %d", __func__,
         static_cast<int>(mIspSettings.eeSetting.feature_level),
         static_cast<int>(mIspSettings.eeSetting.strength));

    camera_nr_mode_t manualNrMode;
    camera_nr_level_t manualNrLevel;

    int manualNrModeSet = param.getNrMode(manualNrMode);
    mIspSettings.nrSetting.feature_level = ia_isp_feature_level_high;
    mIspSettings.nrSetting.strength = static_cast<char>(EXTREME_STRENGTH_LEVEL2);
    if (manualNrModeSet == OK) {
        LOG2("%s: manual NR mode set: %d", __func__, manualNrMode);
        switch (manualNrMode) {
            case NR_MODE_LEVEL_4:
                mIspSettings.nrSetting.strength = static_cast<char>(EXTREME_STRENGTH_LEVEL4);
                break;
            case NR_MODE_LEVEL_3:
                mIspSettings.nrSetting.strength = static_cast<char>(EXTREME_STRENGTH_LEVEL3);
                break;
            case NR_MODE_LEVEL_2:
                mIspSettings.nrSetting.strength = static_cast<char>(EXTREME_STRENGTH_LEVEL2);
                break;
            case NR_MODE_LEVEL_1:
                mIspSettings.nrSetting.strength = static_cast<char>(EXTREME_STRENGTH_LEVEL1);
                break;
            default:
                mIspSettings.nrSetting.strength = static_cast<char>(EXTREME_STRENGTH_LEVEL2);
        }
    }

    int manualNrLevelSet = param.getNrLevel(manualNrLevel);
    if (manualNrLevelSet == OK) {
        mIspSettings.nrSetting.strength = static_cast<char>(manualNrLevel.overall);
    }

    LOG2("%s: ISP NR setting, level: %d, strength: %d", __func__,
         static_cast<int>(mIspSettings.nrSetting.feature_level),
         static_cast<int>(mIspSettings.nrSetting.strength));

    mIspSettings.nrStillSetting = mIspSettings.nrSetting;
    mIspSettings.eeStillSetting = mIspSettings.eeSetting;

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

int PSysProcessor::getParameters(Parameters& param) {
    AutoRMutex rl(mIspSettingsLock);
    camera_image_enhancement_t enhancement = {
        mIspSettings.manualSettings.manualSharpness, mIspSettings.manualSettings.manualBrightness,
        mIspSettings.manualSettings.manualContrast, mIspSettings.manualSettings.manualHue,
        mIspSettings.manualSettings.manualSaturation};
    int ret = param.setImageEnhancement(enhancement);

    return ret;
}

/**
 * Get available setting sequence from outBuf
 */
int64_t PSysProcessor::getSettingSequence(const CameraBufferPortMap& outBuf) {
    int64_t settingSequence = -1;
    for (auto& output : outBuf) {
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
bool PSysProcessor::needSkipOutputFrame(int64_t sequence) {
    // Check if need to skip output frame
    const AiqResult* aiqResults = AiqResultStorage::getInstance(mCameraId)->getAiqResult(sequence);
    if (aiqResults != nullptr && aiqResults->mSkip) {
        LOG1("<seq:%ld>@%s", sequence, __func__);
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
bool PSysProcessor::needExecutePipe(int64_t settingSequence, int64_t inputSequence) {
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
bool PSysProcessor::needHoldOnInputFrame(int64_t settingSequence, int64_t inputSequence) {
    if (settingSequence == -1 || inputSequence <= settingSequence) {
        return false;
    }

    return true;
}

/**
 * Check if pipe needs to be switched according to AIQ result.
 */
bool PSysProcessor::needSwitchPipe(int64_t sequence) {
    const AiqResult* aiqResults = AiqResultStorage::getInstance(mCameraId)->getAiqResult(sequence);
    if (aiqResults == nullptr) {
        LOG2("%s: not found sequence %ld in AiqResultStorage, no update for active modes", __func__,
             sequence);
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

void PSysProcessor::handleEvent(EventData eventData) {
    // Process registered events
    switch (eventData.type) {
        // CSI_META_S
        case EVENT_META:
            // DOL_FEATURE_S
            if (PlatformData::needHandleVbpInMetaData(mCameraId, mCurConfigMode)) {
                AutoMutex l(mMetaQueueLock);
                mMetaQueue.push(eventData.data.meta);
                LOG2("%s: received meta data, current queue size %lu", __func__, mMetaQueue.size());
                mMetaAvailableSignal.signal();
            }
            // DOL_FEATURE_E
            break;
        // CSI_META_E
        case EVENT_ISYS_SOF: {
            AutoMutex l(mSofLock);

            mSofSequence = eventData.data.sync.sequence;
            if (PlatformData::psysAlignWithSof(mCameraId)) {
                gettimeofday(&mSofTimestamp, nullptr);
                LOG2("%s, received SOF event sequence: %ld, timestamp: %ld", __func__,
                     eventData.data.sync.sequence, TIMEVAL2USECS(mSofTimestamp));
                mSofCondition.signal();
            }
            break;
        }
        default:
            LOGW("Unexpected event: %d", eventData.type);
            break;
    }
}

// DOL_FEATURE_S
int PSysProcessor::setVbpToIspParam(int64_t sequence, timeval timestamp) {
    // Check fixed VBP firstly.
    int fixedVbp = PlatformData::getFixedVbp(mCameraId);
    if (fixedVbp >= 0) {
        AutoWMutex wl(mIspSettingsLock);
        LOG2("%s: set fixed vbp %d", __func__, fixedVbp);
        mIspSettings.vbp = fixedVbp;
        return OK;
    }

    // Check dynamic VBP.
    ConditionLock lock(mMetaQueueLock);

    // Remove all older meta data
    while (!mMetaQueue.empty() && mMetaQueue.front().sequence < sequence) {
        LOG2("%s: remove older meta data for sequence %ld", __func__, mMetaQueue.front().sequence);
        mMetaQueue.pop();
    }

    while (mMetaQueue.empty()) {
        int ret = mMetaAvailableSignal.waitRelative(lock, kWaitDuration * SLOWLY_MULTIPLIER);

        if (!mThreadRunning) {
            LOG2("@%s: Processor is not active while waiting for meta data.", __func__);
            return UNKNOWN_ERROR;
        }

        CheckAndLogError(ret == TIMED_OUT, ret, "@%s: dqbuf MetaQueue timed out", __func__);
    }

    if (mMetaQueue.front().sequence == sequence) {
        AutoWMutex l(mIspSettingsLock);
        mIspSettings.vbp = mMetaQueue.front().vbp;
        mMetaQueue.pop();
        LOG2("%s: found vbp %d for frame sequence %ld", __func__, mIspSettings.vbp, sequence);
        return OK;
    }

    LOGW("Missing meta data for seq %ld, timestamp %ld, Cur meta seq %ld, timestamp %ld", sequence,
         TIMEVAL2USECS(timestamp), mMetaQueue.front().sequence,
         TIMEVAL2USECS(mMetaQueue.front().timestamp));
    return UNKNOWN_ERROR;
}
// DOL_FEATURE_E

// PSysProcessor ThreadLoop
int PSysProcessor::processNewFrame() {
    LOG2("<id%d>@%s", mCameraId, __func__);
    CheckAndLogError(!mBufferProducer, INVALID_OPERATION, "No available producer");

    int ret = OK;
    int64_t inputSequence = -1;
    CameraBufferPortMap srcBuffers, dstBuffers;
    if (mScheduler) {
        {
            ConditionLock lock(mBufferQueueLock);
            // Wait input buffer, use TRIGGER_MAX_MARGIN to ensure Scheduler is triggered in time.
            // Reduce wait time for the first 10 frames for better performance.
            int64_t delay = mSofSequence < 10 ? TRIGGER_MARGIN : TRIGGER_MAX_MARGIN;
            bool bufReady = waitBufferQueue(lock, mInputQueue, delay);
            // Already stopped
            if (!mThreadRunning) return -1;

            if (bufReady) {
                waitFreeBuffersInQueue(lock, srcBuffers, dstBuffers, TRIGGER_MARGIN);
            }
        }

        if (!srcBuffers.empty() && !dstBuffers.empty()) {
            inputSequence = srcBuffers.begin()->second->getSequence();
            ret = prepareTask(&srcBuffers, &dstBuffers);
            CheckAndLogError(ret != OK, UNKNOWN_ERROR, "%s, Failed to process frame", __func__);
        } else {
            LOG2("<id%d>@%s, No available buffers, in %lu, out %lu", mCameraId, __func__,
                 srcBuffers.size(), dstBuffers.size());
        }

        // Trigger when there are tasks (new or existing)
        bool trigger = false;
        {
            AutoMutex l(mBufferQueueLock);
            trigger = !mSequencesInflight.empty();
        }
        if (trigger) {
            std::string source;
            mScheduler->executeNode(source, inputSequence);
        }
        prepareIpuForNextFrame(inputSequence);
    } else if (!PlatformData::psysAlignWithSof(mCameraId)) {
        {
            ConditionLock lock(mBufferQueueLock);
            ret = waitFreeBuffersInQueue(lock, srcBuffers, dstBuffers);
            // Already stopped
            if (!mThreadRunning) return -1;

            // Wait frame buffer time out should not involve thread exit.
            if (ret != OK) {
                LOG1("<id%d>@%s, timeout happen, wait recovery", mCameraId, __func__);
                return OK;
            }
        }

        ret = prepareTask(&srcBuffers, &dstBuffers);
        CheckAndLogError(ret != OK, UNKNOWN_ERROR, "%s, Failed to process frame", __func__);
        inputSequence = srcBuffers.begin()->second->getSequence();
        prepareIpuForNextFrame(inputSequence);
    } else {
        timeval curTime;
        int64_t sofInterval = 0;
        {
            ConditionLock lock(mSofLock);

            gettimeofday(&curTime, nullptr);
            sofInterval = TIMEVAL2NSECS(curTime) - TIMEVAL2NSECS(mSofTimestamp);
            // Wait next sof event when missing last one for a long time
            if (sofInterval > TRIGGER_MARGIN && sofInterval < TRIGGER_MAX_MARGIN) {
                LOG2("%s, need to wait next sof event. sofInterval: %ld", __func__, sofInterval);
                ret = mSofCondition.waitRelative(lock, TRIGGER_MAX_MARGIN * SLOWLY_MULTIPLIER);

                // Already stopped
                if (!mThreadRunning) return -1;

                // Wait sof event time out should not involve thread exit.
                if (ret != OK) {
                    LOG1("<id%d>%s, wait sof event timeout, recovery", mCameraId, __func__);
                    return OK;
                }
            }
        }

        // push all the pending buffers to task
        int64_t waitTime = TRIGGER_MARGIN;
        // Don't need to catch sof for 1st frame or sof time out
        if (TIMEVAL2NSECS(mSofTimestamp) == 0 || sofInterval >= TRIGGER_MAX_MARGIN) waitTime = 0;
        while (true) {
            {
                ConditionLock lock(mBufferQueueLock);
                ret = waitFreeBuffersInQueue(lock, srcBuffers, dstBuffers, waitTime);

                // Already stopped
                if (!mThreadRunning) return -1;

                // Return to wait next sof event if there isn't pending buffer.
                if (ret != OK) return OK;
            }

            {
                AutoMutex l(mSofLock);
                inputSequence = srcBuffers.begin()->second->getSequence();
                if (inputSequence >= mSofSequence) {
                    gettimeofday(&curTime, nullptr);
                    sofInterval = TIMEVAL2NSECS(curTime) - TIMEVAL2NSECS(mSofTimestamp);

                    // Handle the frame of sof(N) on sof(N + 1) when the sof event is continuously
                    if (sofInterval < TRIGGER_MAX_MARGIN) {
                        return OK;
                    }
                    LOG1("%s, sof event lost for long time, skip wating. sofInterval: %ld",
                         __func__, sofInterval);
                }
            }

            ret = prepareTask(&srcBuffers, &dstBuffers);
            CheckAndLogError(ret != OK, UNKNOWN_ERROR, "%s, Failed to process frame", __func__);
            prepareIpuForNextFrame(inputSequence);
        }
    }

    return OK;
}

void PSysProcessor::prepareIpuForNextFrame(int64_t sequence) {
    {
        AutoMutex l(mBufferQueueLock);
        if (sequence < 0 || !mRunAicAfterQTask ||
            mSequencesInflight.find(sequence) == mSequencesInflight.end()) return;
    }

    // HDR_FEATURE_S
    if (mTuningMode == TUNING_MODE_VIDEO_HDR || mTuningMode == TUNING_MODE_VIDEO_HDR2) return;
    // HDR_FEATURE_E

    int32_t userRequestId = -1;
    // Check if next sequence is used or not
    if (mParameterGenerator &&
        mParameterGenerator->getUserRequestId(sequence + 1, userRequestId) == OK) {
        mPSysDAGs[mCurConfigMode]->prepareIpuParams((sequence + 1), false, nullptr, true);
    }
}

void PSysProcessor::handleRawReprocessing(CameraBufferPortMap* srcBuffers,
                                          CameraBufferPortMap* dstBuffers, bool* allBufDone,
                                          bool* hasRawOutput, bool* hasRawInput) {
    std::shared_ptr<CameraBuffer> rawOutputBuffer = nullptr;
    int64_t settingSequence = -1;
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
    int64_t inputSequence = mainBuf->getSequence();
    uint64_t timestamp = TIMEVAL2NSECS(mainBuf->getTimestamp());

    if (rawOutputBuffer) {
        if (!needExecutePipe(settingSequence, inputSequence)) {
            LOG2("%s, inputSequence %ld is smaller than settingSequence %ld, skip sensor frame.",
                 __func__, inputSequence, settingSequence);
            return;
        }

        raw_data_output_t rawDataOutput = CAMERA_RAW_DATA_OUTPUT_OFF;
        if (mParameterGenerator &&
            mParameterGenerator->getRawOutputMode(inputSequence, rawDataOutput) == OK) {
            if (rawDataOutput == CAMERA_RAW_DATA_OUTPUT_ON) {
                uint32_t srcBufferSize = mainBuf->getBufferSize();

                if (srcBufferSize <= rawOutputBuffer->getBufferSize()) {
                    MEMCPY_S(rawOutputBuffer->getBufferAddr(), rawOutputBuffer->getBufferSize(),
                             mainBuf->getBufferAddr(), srcBufferSize);
                } else {
                    LOGW("%s, raw dst size %d is smaller than raw src size %d", __func__,
                         rawOutputBuffer->getBufferSize(), srcBufferSize);
                }
            }
        }

        rawOutputBuffer->updateV4l2Buffer(*mainBuf->getV4L2Buffer().Get());

        LOG2("%s, timestamp %ld, inputSequence %ld, dstBufferSize %d, addr %p", __func__, timestamp,
             inputSequence, rawOutputBuffer->getBufferSize(), rawOutputBuffer->getBufferAddr());

        // handle Shutter first if has raw output
        sendPsysRequestEvent(dstBuffers, settingSequence, timestamp, EVENT_PSYS_REQUEST_BUF_READY);

        // Return opaque RAW buffer
        for (auto& it : mBufferConsumerList) {
            it->onFrameAvailable(mOpaqueRawPort, rawOutputBuffer);
        }
        *hasRawOutput = true;

        if (stillBuf.empty() && videoBuf.empty()) {
            *allBufDone = true;
        } else {
            // Remove from dstBuffers map
            dstBuffers->erase(mOpaqueRawPort);
        }
    } else if (settingSequence != -1 && inputSequence > settingSequence) {
        timestamp = 0;
        // handle Shutter first if has raw input
        sendPsysRequestEvent(dstBuffers, settingSequence, timestamp, EVENT_PSYS_REQUEST_BUF_READY);

        // only one video buffer is supported
        if (PlatformData::isGpuTnrEnabled(mCameraId) && videoBuf.size() == 1) {
            shared_ptr<CameraBuffer> buf = videoBuf.begin()->second;
            bool handled = mPSysDAGs[mCurConfigMode]->fetchTnrOutBuffer(settingSequence, buf);
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
                CameraBufferPortMap& mapBuf = mRawBufferMap[settingSequence];
                // Update source buffers
                for (const auto& bufPortMap : mapBuf) {
                    (*srcBuffers)[bufPortMap.first] = bufPortMap.second;
                }
                *hasRawInput = true;
            }
        }
    }

    // handle Metadata ready event if all buffer done here(psys doesn't run)
    if (*allBufDone) {
        sendPsysRequestEvent(dstBuffers, settingSequence, timestamp, EVENT_REQUEST_METADATA_READY);
    }

    LOG2("hasRawInput %d, hasRawOutput %d, allBufDone %d, settingSequence %ld, inputSequence %ld",
         *hasRawInput, *hasRawOutput, *allBufDone, settingSequence, inputSequence);
}

bool PSysProcessor::isBufferHoldForRawReprocess(int64_t sequence) {
    if (!mHoldRawBuffers) return false;

    AutoMutex lock(mBufferMapLock);
    if (mRawBufferMap.find(sequence) == mRawBufferMap.end()) return false;

    return true;
}

void PSysProcessor::saveRawBuffer(CameraBufferPortMap* srcBuffers) {
    // Save buffer into mRawBufferMap
    CameraBufferPortMap mapBuf;
    for (const auto& src : *srcBuffers) {
        mapBuf[src.first] = src.second;
    }

    Port defaultPort = srcBuffers->begin()->first;
    shared_ptr<CameraBuffer> mainBuf = (*srcBuffers)[defaultPort];
    int64_t inputSequence = mainBuf->getSequence();

    LOG2("<id%d:seq%ld>@%s", mCameraId, inputSequence, __func__);
    {
        AutoMutex lock(mBufferMapLock);
        mRawBufferMap[inputSequence] = mapBuf;
    }

    returnRawBuffer();
}

void PSysProcessor::returnRawBuffer() {
    AutoMutex lock(mBufferMapLock);
    // If too many buffers are holden in mRawQueue, return back to producer
    if (mRawBufferMap.size() > (PlatformData::getMaxRawDataNum(mCameraId) -
                                PlatformData::getMaxRequestsInflight(mCameraId))) {
        auto it = mRawBufferMap.cbegin();
        {
            AutoMutex l(mBufferQueueLock);
            if (mSequencesInflight.find(it->first) != mSequencesInflight.end()) {
                // Raw buffer is in used and return it later
                return;
            }
        }

        const CameraBufferPortMap& bufferPortMap = it->second;
        for (auto& item : bufferPortMap) {
            mBufferProducer->qbuf(item.first, item.second);
        }
        LOG2("@%s, returned sequence %ld", __func__, it->first);
        mRawBufferMap.erase(it);
    }
}

status_t PSysProcessor::prepareTask(CameraBufferPortMap* srcBuffers,
                                    CameraBufferPortMap* dstBuffers) {
    CheckAndLogError(srcBuffers->empty() || dstBuffers->empty(), UNKNOWN_ERROR,
                     "%s, the input or output buffer is empty", __func__);
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
                for (auto& input : mInputQueue) {
                    input.second.pop();
                }
            }
            for (auto& output : mOutputQueue) {
                output.second.pop();
            }
            return OK;
        }
    }

    Port defaultPort = srcBuffers->begin()->first;
    shared_ptr<CameraBuffer> mainBuf = (*srcBuffers)[defaultPort];
    int64_t inputSequence = mainBuf->getSequence();
    TRACE_LOG_POINT("PSysProcessor", "input output buffer ready", MAKE_COLOR(inputSequence),
                    inputSequence);
    uint64_t timestamp = TIMEVAL2NSECS(mainBuf->getTimestamp());
    LOG2("%s: input buffer sequence %ld timestamp %ld", __func__, inputSequence, timestamp);

    // DOL_FEATURE_S
    if (PlatformData::needSetVbp(mCameraId, mCurConfigMode)) {
        int vbpStatus = setVbpToIspParam(inputSequence, mainBuf->getTimestamp());

        // Skip input frame and return buffer if no matching vbp set to ISP params
        if (vbpStatus != OK) {
            AutoMutex l(mBufferQueueLock);
            for (auto& input : mInputQueue) {
                input.second.pop();
            }

            for (const auto& item : *srcBuffers) {
                mBufferProducer->qbuf(item.first, item.second);
            }
            return OK;
        }
    }
    // DOL_FEATURE_E

    // Output raw image
    if (mRawPort != INVALID_PORT) {
        shared_ptr<CameraBuffer> dstBuf = nullptr;

        // Get output buffer and remove it from dstBuffers
        for (auto& buffer : *dstBuffers) {
            if (buffer.first == mRawPort) {
                dstBuf = buffer.second;
                CheckAndLogError(!dstBuf, UNKNOWN_ERROR, "%s, dstBuf for output raw is null",
                                 __func__);
                dstBuf->updateV4l2Buffer(*mainBuf->getV4L2Buffer().Get());
                dstBuffers->erase(mRawPort);
                break;
            }
        }
        outputRawImage(mainBuf, dstBuf);
    }

    int64_t settingSequence = getSettingSequence(*dstBuffers);
    bool needRunPipe = needExecutePipe(settingSequence, inputSequence);
    bool holdOnInput = needHoldOnInputFrame(settingSequence, inputSequence);
    LOG2("%s: dst sequence = %ld, src sequence = %ld, needRunPipe = %d, needReuseInput = %d",
         __func__, settingSequence, inputSequence, needRunPipe, holdOnInput);

    {
        AutoMutex l(mBufferQueueLock);
        if (needRunPipe && !needSkipOutputFrame(inputSequence)) {
            for (auto& output : mOutputQueue) {
                output.second.pop();
            }
        }

        // If input buffer will be used later, don't pop it from the queue.
        if (!holdOnInput && !hasRawInput) {
            for (auto& input : mInputQueue) {
                input.second.pop();
            }
        }
    }

    if (needRunPipe) {
        // Raw output already has been returned back, and don't need to handle again.
        if (!hasRawOutput) {
            // handle buffer done for normal YUV output
            sendPsysRequestEvent(dstBuffers, settingSequence, timestamp,
                                 EVENT_PSYS_REQUEST_BUF_READY);
        }
        if (PlatformData::isGpuTnrEnabled(mCameraId)) {
            handleStillPipeForTnr(inputSequence, dstBuffers);
        }

        bool callbackRgbs = false;
        AiqResultStorage* storage = AiqResultStorage::getInstance(mCameraId);
        AiqResult* aiqResult = const_cast<AiqResult*>(storage->getAiqResult(inputSequence));
        if (aiqResult && aiqResult->mAiqParam.callbackRgbs) {
            callbackRgbs = true;
        }

        dispatchTask(*srcBuffers, *dstBuffers, false, callbackRgbs);

        if (!callbackRgbs) {
            // handle metadata event after running pal(update metadata from pal result)
            sendPsysRequestEvent(dstBuffers, settingSequence, timestamp,
                                 EVENT_REQUEST_METADATA_READY);
        }
    } else if (!holdOnInput && !isBufferHoldForRawReprocess(inputSequence)) {
        for (const auto& src : *srcBuffers) {
            mBufferProducer->qbuf(src.first, src.second);
        }
    }
    return OK;
}

void PSysProcessor::handleStillPipeForTnr(int64_t sequence, CameraBufferPortMap* dstBuffers) {
    bool hasStill = false;
    for (const auto& item : *dstBuffers) {
        if (item.second && item.second->getStreamUsage() == CAMERA_STREAM_STILL_CAPTURE) {
            hasStill = true;
            break;
        }
    }
    LOG2("@%s, seq %ld, hold raw %d, last still seq %ld, still %d", __func__, sequence,
         mHoldRawBuffers, mLastStillTnrSequence, hasStill);

    bool bypass = mPSysDAGs[mCurConfigMode]->isBypassStillTnr(sequence);
    if (!bypass && hasStill && mHoldRawBuffers &&
        (sequence >= (mLastStillTnrSequence + TNR7US_RESTART_THRESHOLD) ||
         mLastStillTnrSequence < 0)) {
        CameraBufferPortMap fakeTaskBuffers = *dstBuffers;
        for (const auto& item : fakeTaskBuffers) {
            if (item.second && item.second->getStreamUsage() != CAMERA_STREAM_STILL_CAPTURE) {
                fakeTaskBuffers[item.first] = nullptr;
            }
        }
        for (int i = mPSysDAGs[mCurConfigMode]->getTnrExtraFrameCount(sequence); i > 0; i--) {
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
                dispatchTask(srcBuf, fakeTaskBuffers, true, false);
            }
        }
    }

    if (hasStill) mLastStillTnrSequence = sequence;
}

void PSysProcessor::dispatchTask(CameraBufferPortMap& inBuf, CameraBufferPortMap& outBuf,
                                 bool fakeTask, bool callbackRgbs) {
    int64_t currentSequence = inBuf.begin()->second->getSequence();
    TRACE_LOG_POINT("PSysProcessor", "start run PSYS", MAKE_COLOR(currentSequence),
                    currentSequence);
    {
        ConditionLock lock(mBufferQueueLock);

        ConfigMode previousMode = mCurConfigMode;
        bool needSwitch = needSwitchPipe(currentSequence);

        if (needSwitch) {
            LOG1("Switch pipe for sequence:%ld, unprocessed buffer number:%zu", currentSequence,
                 mSequencesInflight.size());

            // Deactive the PSysDag which is no longer used.
            mPSysDAGs[previousMode]->pause();

            // Before switching, need to wait all buffers in current pipe being processed.
            while (!mSequencesInflight.empty()) {
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
        mSequencesInflight.insert(currentSequence);
    }  // End of lock mBufferQueueLock

    int32_t userRequestId = -1;
    mParameterGenerator->getUserRequestId(currentSequence, userRequestId);
    LOG2("<id%d:seq:%ld:req:%d>@%s, fake task %d, pending task: %zu", mCameraId, currentSequence,
         userRequestId, __func__, fakeTask, mSequencesInflight.size());

    // Prepare the task input paramerters including input and output buffers, settings etc.
    PSysTaskData taskParam;
    taskParam.mTuningMode = mTuningMode;
    taskParam.mInputBuffers = inBuf;
    taskParam.mOutputBuffers = outBuf;
    taskParam.mFakeTask = fakeTask;
    taskParam.mCallbackRgbs = callbackRgbs;

    int64_t settingSequence = getSettingSequence(outBuf);
    // Handle per-frame settings if output buffer requires
    if (settingSequence > -1 && mParameterGenerator) {
        Parameters params;
        if (mParameterGenerator->getIspParameters(currentSequence, &params) == OK) {
            setParameters(params);

            // Apply HAL tuning parameters
            float hdrRatio = 0;
            EdgeNrSetting edgeNrSetting;
            CLEAR(edgeNrSetting);

            int ret = params.getHdrRatio(hdrRatio);
            if (ret == OK) {
                auto* res = AiqResultStorage::getInstance(mCameraId)->getAiqResult(currentSequence);
                if (res != nullptr) {
                    auto exposure = res->mAeResults.exposures[0].exposure[0];
                    float totalGain = exposure.analog_gain * exposure.digital_gain;
                    PlatformData::getEdgeNrSetting(mCameraId, totalGain, hdrRatio, mTuningMode,
                                                   edgeNrSetting);
                    mIspSettings.eeSetting.strength += edgeNrSetting.edgeStrength;
                    mIspSettings.nrSetting.strength += edgeNrSetting.nrStrength;
                    LOG2("edgeStrength %d, nrStrength %d", edgeNrSetting.edgeStrength,
                         edgeNrSetting.nrStrength);

                    TuningMode stillMode = (mTuningMode == TUNING_MODE_VIDEO) ?
                                           TUNING_MODE_STILL_CAPTURE : TUNING_MODE_VIDEO_ULL;
                    PlatformData::getEdgeNrSetting(mCameraId, totalGain, hdrRatio, stillMode,
                                                   edgeNrSetting);
                    mIspSettings.eeStillSetting.strength += edgeNrSetting.edgeStrength;
                    mIspSettings.nrStillSetting.strength += edgeNrSetting.nrStrength;
                    LOG2("Still edgeStrength %d, nrStrength %d", edgeNrSetting.edgeStrength,
                         edgeNrSetting.nrStrength);
                }
            }

            bool hasStill = false;
            for (const auto& item : outBuf) {
                if (item.second && item.second->getStreamUsage() == CAMERA_STREAM_STILL_CAPTURE) {
                    hasStill = true;
                    break;
                }
            }
            // Dump raw image if has STILL or fake task for IQ tune
            if ((hasStill || fakeTask) && CameraDump::isDumpTypeEnable(DUMP_JPEG_BUFFER)) {
                if (userRequestId >= 0) {
                    char desc[MAX_NAME_LEN];
                    int len = snprintf(desc, (MAX_NAME_LEN - 1), "_req#%d", userRequestId);
                    desc[len] = '\0';
                    CameraDump::dumpImage(mCameraId, inBuf[MAIN_PORT], M_PSYS, MAIN_PORT, desc);
                } else {
                    CameraDump::dumpImage(mCameraId, inBuf[MAIN_PORT], M_PSYS, MAIN_PORT);
                }
            }
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

void PSysProcessor::registerListener(EventType eventType, EventListener* eventListener) {
    // Only delegate stats event registration to deeper layer DAG and PipeExecutor
    if ((eventType != EVENT_PSYS_STATS_BUF_READY) &&
        (eventType != EVENT_PSYS_STATS_SIS_BUF_READY)
    ) {
        BufferQueue::registerListener(eventType, eventListener);
        return;
    }

    for (auto const& realModeDAGPair : mPSysDAGs) {
        realModeDAGPair.second->registerListener(eventType, eventListener);
    }
}

void PSysProcessor::removeListener(EventType eventType, EventListener* eventListener) {
    // Only delegate stats event unregistration to deeper layer DAG and PipeExecutor
    if ((eventType != EVENT_PSYS_STATS_BUF_READY) &&
        (eventType != EVENT_PSYS_STATS_SIS_BUF_READY)) {
        BufferQueue::removeListener(eventType, eventListener);
        return;
    }

    for (auto const& realModeDAGPair : mPSysDAGs) {
        realModeDAGPair.second->removeListener(eventType, eventListener);
    }
}

void PSysProcessor::onBufferDone(int64_t sequence, Port port,
                                 const std::shared_ptr<CameraBuffer>& camBuffer) {
    LOG2("<id%d:seq%ld>@%s, port %d", mCameraId, sequence, __func__, port);

    if (CameraDump::isDumpTypeEnable(DUMP_PSYS_OUTPUT_BUFFER)) {
        int32_t userRequestId = -1;
        if (mParameterGenerator->getUserRequestId(sequence, userRequestId) == OK &&
            userRequestId >= 0) {
            char desc[MAX_NAME_LEN];
            int len = snprintf(desc, (MAX_NAME_LEN - 1), "_req#%d", userRequestId);
            desc[len] = '\0';
            CameraDump::dumpImage(mCameraId, camBuffer, M_PSYS, port, desc);
        } else {
            CameraDump::dumpImage(mCameraId, camBuffer, M_PSYS, port);
        }
    }

    if (!needSkipOutputFrame(sequence)) {
        for (auto& it : mBufferConsumerList) {
            it->onFrameAvailable(port, camBuffer);
        }
    }
}

void PSysProcessor::sendPsysFrameDoneEvent(const CameraBufferPortMap* dstBuffers) {
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

void PSysProcessor::sendPsysRequestEvent(const CameraBufferPortMap* dstBuffers, int64_t sequence,
                                         uint64_t timestamp, EventType eventType) {
    for (const auto& output : *dstBuffers) {
        if (output.second && output.second->getUsage() != BUFFER_USAGE_PSYS_INTERNAL) {
            EventData event;
            event.type = eventType;
            event.buffer = nullptr;
            event.data.requestReady.timestamp =
                timestamp > 0 ? timestamp : output.second->getUserBuffer()->timestamp;
            event.data.requestReady.sequence = sequence;
            event.data.requestReady.requestId = output.second->getUserBuffer()->requestId;

            notifyListeners(event);
            break;
        }
    }
}

void PSysProcessor::onFrameDone(const PSysTaskData& result) {
    int64_t sequence = result.mInputBuffers.begin()->second->getSequence();
    LOG2("<id%d:seq%ld>@%s", mCameraId, sequence, __func__);
    TRACE_LOG_POINT("PSysProcessor", __func__, MAKE_COLOR(sequence), sequence);

    if (!result.mFakeTask) {
        if (!needSkipOutputFrame(sequence)) {
            sendPsysFrameDoneEvent(&result.mOutputBuffers);
        }

        int64_t settingSequence = getSettingSequence(result.mOutputBuffers);
        bool holdOnInput = needHoldOnInputFrame(settingSequence, sequence);
        bool hasRawOutput = isBufferHoldForRawReprocess(sequence);

        LOG2("%s, dst sequence: %ld, src sequence: %ld, hasRawOutput: %d, holdOnInput: %d",
             __func__, settingSequence, sequence, hasRawOutput, holdOnInput);
        // Return buffer only if the buffer is not used in the future.
        if (!holdOnInput && mBufferProducer && !hasRawOutput) {
            for (const auto& src : result.mInputBuffers) {
                mBufferProducer->qbuf(src.first, src.second);

                if (src.second->getStreamType() == CAMERA_STREAM_INPUT) {
                    for (auto& it : mBufferConsumerList) {
                        it->onFrameAvailable(src.first, src.second);
                    }
                }
            }
        }
    }

    {
        AutoMutex l(mBufferQueueLock);
        std::multiset<int64_t>::iterator it = mSequencesInflight.find(sequence);
        if (it != mSequencesInflight.end()) {
            mSequencesInflight.erase(it);
        }

        if (mSequencesInflight.empty()) {
            mFrameDoneSignal.signal();
        }
    }

    returnRawBuffer();
}

void PSysProcessor::onStatsDone(int64_t sequence, const CameraBufferPortMap& outBuf) {
    LOG2("<seq%ld> %s", sequence, __func__);

    // handle metadata event after decoding stats
    sendPsysRequestEvent(&outBuf, sequence, 0, EVENT_REQUEST_METADATA_READY);
}

// INTEL_DVS_S
void PSysProcessor::onDvsPrepare(int64_t sequence, int32_t streamId) {
    LOG2("%s stream Id %d", __func__, streamId);

    camera_zoom_region_t region;
    if (mParameterGenerator && mParameterGenerator->getZoomRegion(sequence, region) == OK) {
        EventData eventData;
        eventData.type = EVENT_DVS_READY;
        eventData.data.dvsRunReady.streamId = streamId;
        eventData.data.dvsRunReady.sequence = sequence;
        eventData.data.dvsRunReady.region = region;
        notifyListeners(eventData);
    }
}
// INTEL_DVS_E

void PSysProcessor::outputRawImage(shared_ptr<CameraBuffer>& srcBuf,
                                   shared_ptr<CameraBuffer>& dstBuf) {
    if ((srcBuf == nullptr) || (dstBuf == nullptr)) {
        return;
    }

    // Copy from source buffer
    int srcBufferSize = srcBuf->getBufferSize();
    ScopeMapping mapperSrc(srcBuf);
    void* pSrcBuf = mapperSrc.getUserPtr();

    int dstBufferSize = dstBuf->getBufferSize();
    ScopeMapping mapperDst(dstBuf);
    void* pDstBuf = mapperDst.getUserPtr();

    MEMCPY_S(pDstBuf, dstBufferSize, pSrcBuf, srcBufferSize);

    // Send output buffer to its consumer
    for (auto& it : mBufferConsumerList) {
        it->onFrameAvailable(mRawPort, dstBuf);
    }
}

}  // namespace icamera
