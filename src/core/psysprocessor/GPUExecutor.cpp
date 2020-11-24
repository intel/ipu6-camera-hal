/*
 * Copyright (C) 2020 Intel Corporation
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

#define LOG_TAG "GPUExecutor"

#include "GPUExecutor.h"

extern "C" {
#include <ia_cipf_css/ia_cipf_css.h>
#include <ia_pal_types_isp_ids_autogen.h>
}

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "3a/AiqResult.h"
#include "3a/AiqResultStorage.h"
#include "FormatUtils.h"
#include "PSysDAG.h"
#include "SyncManager.h"
#include "ia_pal_output_data.h"
#include "iutils/CameraDump.h"

using std::map;
using std::shared_ptr;
using std::string;
using std::vector;

#define external_gpu_stage_uid(id) ia_fourcc(((id & 0xFF00) >> 8), id, 'E', 'X')

namespace icamera {

std::mutex GPUExecutor::mGPULock;

GPUExecutor::GPUExecutor(int cameraId, const ExecutorPolicy& policy, vector<string> exclusivePGs,
                         PSysDAG* psysDag, shared_ptr<IGraphConfig> gc, bool useTnrOutBuffer)
        : PipeExecutor(cameraId, policy, exclusivePGs, psysDag, gc),
          mTnr7usParam(nullptr),
          mIntelTNR(nullptr),
          mLastSequence(UINT32_MAX),
          mUseInternalTnrBuffer(useTnrOutBuffer),
          mOutBufferSize(0) {
    LOG1("@%s %s", __func__, mName.c_str());
}

GPUExecutor::~GPUExecutor() {
    LOG1("@%s %s", __func__, mName.c_str());
    mPGExecutors.clear();
}

int GPUExecutor::initPipe() {
    LOG1("@%s:%s", __func__, getName());
    CheckError(mGraphConfig == nullptr, BAD_VALUE, "%s, the graph config is NULL, BUG!", __func__);

    vector<IGraphType::PipelineConnection> connVector;

    int ret = mGraphConfig->pipelineGetConnections(mPGNames, &connVector);
    CheckError(ret != OK || connVector.empty(), ret, "Failed to get connections for executor:%s",
               mName.c_str());

    ret = createPGs();
    CheckError(ret != OK, ret, "Failed to create PGs for executor: %s", mName.c_str());

    ret = analyzeConnections(connVector);
    CheckError(ret != OK, ret, "Failed to analyze connections for executor: %s", mName.c_str());

    assignDefaultPortsForTerminals();
    return OK;
}

// GPU executor doesn't have HW pg, create pg according to graph settings
int GPUExecutor::createPGs() {
    LOG1("@%s:%s", __func__, getName());

    for (auto const& pgName : mPGNames) {
        int pgId = mGraphConfig->getPgIdByPgName(pgName);
        CheckError(pgId == -1, BAD_VALUE, "Cannot get PG ID for %s", pgName.c_str());
        ExecutorUnit pgUnit;
        pgUnit.pgId = pgId;
        // GPU Executor doesn't have pg
        pgUnit.pg = nullptr;
        pgUnit.stageId = external_gpu_stage_uid(pgId);
        mPGExecutors.push_back(pgUnit);
    }
    return OK;
}

int GPUExecutor::start() {
    LOG1("%s executor:%s", __func__, mName.c_str());

    mProcessThread = new ProcessThread(this);

    CheckError(mPGExecutors.empty(), UNKNOWN_ERROR, "PGExecutors couldn't be empty");
    ExecutorUnit* tnrUnit = &mPGExecutors[0];
    int ret = OK;
    if (!tnrUnit->inputTerminals.empty()) {
        ia_uid term = tnrUnit->inputTerminals.at(0);
        CheckError(mTerminalsDesc.find(term) == mTerminalsDesc.end(), NO_INIT,
                   "Can't find TerminalDescriptor");

        const FrameInfo& frameInfo = mTerminalsDesc[term].frameDesc;
        mIntelTNR = std::unique_ptr<IntelTNR7US>(new IntelTNR7US(mCameraId));
        TnrType type = mStreamId == VIDEO_STREAM_ID ? TNR_INSTANCE0 : TNR_INSTANCE1;
        ret = mIntelTNR->init(frameInfo.mWidth, frameInfo.mHeight, type);
        if (ret) {
            mIntelTNR = nullptr;
            // when failed to init tnr, executor will run without tnr effect
            LOGW("%s executor:%s init tnr failed", __func__, mName.c_str());
        }
    }
    AutoMutex l(mBufferQueueLock);
    ret = allocBuffers();
    CheckError(ret, UNKNOWN_ERROR, "@%s: allocBuffers failed", __func__);
    dumpPGs();

    mThreadRunning = true;
    mProcessThread->run(mName.c_str(), PRIORITY_NORMAL);

    return ret;
}

void GPUExecutor::stop() {
    LOG1("%s executor:%s", __func__, mName.c_str());

    mProcessThread->requestExitAndWait();

    // Thread is not running. It is safe to clear the Queue
    releaseBuffers();
    clearBufferQueues();

    delete mProcessThread;
    mIntelTNR = nullptr;
}

int GPUExecutor::allocBuffers() {
    LOG1("%s executor:%s", __func__, mName.c_str());

    releaseBuffers();

    // Allocate buffers for producer executor (external)
    // don't need to check mPGExecutors and inputTerminals, already check before
    ia_uid term = mPGExecutors[0].inputTerminals.at(0);
    Port inputPort = mTerminalsDesc[term].assignedPort;

    int srcFmt = mTerminalsDesc[term].frameDesc.mFormat;
    int srcWidth = mTerminalsDesc[term].frameDesc.mWidth;
    int srcHeight = mTerminalsDesc[term].frameDesc.mHeight;
    int size = PGCommon::getFrameSize(srcFmt, srcWidth, srcHeight, true);

    for (int i = 0; i < MAX_BUFFER_COUNT; i++) {
        shared_ptr<CameraBuffer> buf;
        if (mIntelTNR) {
            buf = std::make_shared<CameraBuffer>(mCameraId, BUFFER_USAGE_PSYS_INPUT,
                                                 V4L2_MEMORY_USERPTR, size, i, srcFmt);
            CheckError(!buf, NO_MEMORY, "@%s: fail to alloc CameraBuffer", __func__);
            void* buffer = mIntelTNR->allocCamBuf(size, i);
            CheckError(!buffer, NO_MEMORY, "@%s: fail to alloc shared memory", __func__);
            buf->setUserBufferInfo(srcFmt, srcWidth, srcHeight, buffer);
        } else {
            buf = CameraBuffer::create(mCameraId, BUFFER_USAGE_PSYS_INPUT, V4L2_MEMORY_USERPTR,
                                       size, i, srcFmt, srcWidth, srcHeight);
        }

        mInternalBuffers[inputPort].push_back(buf);
        mBufferProducer->qbuf(inputPort, buf);
    }

    if (mIntelTNR) {
        int ret = allocTnrOutBufs(size);
        CheckError(ret, UNKNOWN_ERROR, "@%s: alloc tnr reference buffer failed", __func__);

        mTnr7usParam = mIntelTNR->allocTnr7ParamBuf();
        CheckError(!mTnr7usParam, NO_MEMORY, "@%s: Allocate Param buffer failed", __func__);
        CLEAR(*mTnr7usParam);
    }

    // Allocate internal output buffers to support pipe execution without user output buffer
    for (auto const& item : mOutputFrameInfo) {
        int fmt = item.second.format;
        int width = item.second.width;
        int height = item.second.height;
        int size = CameraUtils::getFrameSize(fmt, width, height, true);
        shared_ptr<CameraBuffer> buf = CameraBuffer::create(
            mCameraId, BUFFER_USAGE_PSYS_INPUT, V4L2_MEMORY_USERPTR, size, 0, fmt, width, height);
        CheckError(!buf, NO_MEMORY, "@%s: Allocate internal output buffer failed", __func__);
        mInternalOutputBuffers[item.first] = buf;
    }

    return OK;
}

bool GPUExecutor::fetchTnrOutBuffer(int64_t seq, std::shared_ptr<CameraBuffer> buf) {
    if (!mUseInternalTnrBuffer) return false;

    std::unique_lock<std::mutex> lock(mTnrOutBufMapLock);
    if (mTnrOutBufMap.find(seq) != mTnrOutBufMap.end()) {
        void* pSrcBuf = (buf->getMemory() == V4L2_MEMORY_DMABUF)
                            ? CameraBuffer::mapDmaBufferAddr(buf->getFd(), buf->getBufferSize())
                            : buf->getBufferAddr();
        CheckError(!pSrcBuf, false, "%s, pSrcBuf is nullptr", __func__);
        LOG2("%s, sequence %ld is used for output", __func__, seq);
        MEMCPY_S(pSrcBuf, buf->getBufferSize(), mTnrOutBufMap[seq], mOutBufferSize);
        if (buf->getMemory() == V4L2_MEMORY_DMABUF) {
            CameraBuffer::unmapDmaBufferAddr(pSrcBuf, buf->getBufferSize());
        }

        return true;
    }

    return false;
}

int GPUExecutor::allocTnrOutBufs(uint32_t bufSize) {
    mOutBufferSize = bufSize;

    /* for yuv still stream, we use maxRaw buffer to do reprocessing, and for real still stream, 2
     * tnr buffers are enough */
    int maxTnrOutBufCount = (mStreamId == VIDEO_STREAM_ID && mUseInternalTnrBuffer)
                                ? PlatformData::getMaxRawDataNum(mCameraId)
                                : DEFAULT_TNR7US_BUFFER_COUNT;

    std::unique_lock<std::mutex> lock(mTnrOutBufMapLock);
    for (int i = 0; i < maxTnrOutBufCount; i++) {
        void* buffer = mIntelTNR->allocCamBuf(bufSize, MAX_BUFFER_COUNT + i);
        // will release all buffer in freeAllBufs
        CheckError(!buffer, UNKNOWN_ERROR, "@%s, alloc reference buffer fails", __func__);
        int index = i * (-1) - 1;  // initialize index as -1, -2, ...
        mTnrOutBufMap[index] = buffer;
    }
    return OK;
}

void GPUExecutor::releaseBuffers() {
    if (mIntelTNR) {
        mIntelTNR->freeAllBufs();
    }
    mTnrOutBufMap.clear();
    // Release internel frame buffers
    mInternalOutputBuffers.clear();
    mInternalBuffers.clear();
}

int GPUExecutor::processNewFrame() {
    PERF_CAMERA_ATRACE();

    int ret = OK;
    CameraBufferPortMap inBuffers, outBuffers;
    // Wait frame buffers.
    {
        ConditionLock lock(mBufferQueueLock);
        ret = waitFreeBuffersInQueue(lock, inBuffers, outBuffers);
        // Already stopped
        if (!mThreadRunning) return -1;

        if (ret != OK) return OK;  // Wait frame buffer error should not involve thread exit.

        CheckError(inBuffers.empty() || outBuffers.empty(), UNKNOWN_ERROR,
                   "Failed to get input or output buffers.");

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
        // As an output edge, return the inBuffer when has no outBuffer,
        for (const auto& item : inBuffers) {
            mBufferProducer->qbuf(item.first, item.second);
        }
        return OK;
    }

    // Should find first not none input buffer instead of always use the first one.
    shared_ptr<CameraBuffer> inBuf = inBuffers.begin()->second;
    CheckError(!inBuf, UNKNOWN_ERROR, "@%s: no valid input buffer", __func__);
    v4l2_buffer_t inV4l2Buf = *inBuf->getV4L2Buffer().Get();

    // Fill real buffer to run pipe
    for (auto& item : outBuffers) {
        if (item.second.get() == nullptr) {
            item.second = mInternalOutputBuffers[item.first];
        }
    }

    std::shared_ptr<CameraBuffer> outBuf = outBuffers.begin()->second;
    CheckError(!outBuf, UNKNOWN_ERROR, "@%s: no valid output buffer", __func__);

    ret = runTnrFrame(inBuf, outBuf);
    CheckError(ret != OK, ret, "@%s: run tnr failed", __func__);

    // Remove internal output buffers
    for (auto& item : outBuffers) {
        if (item.second.get() == mInternalOutputBuffers[item.first].get()) {
            item.second = nullptr;
        }
    }

    notifyFrameDone(inV4l2Buf, outBuffers);

    // Return buffers for the executor which is NOT an input edge
    for (auto const& portBufferPair : inBuffers) {
        // Queue buffer to producer
        mBufferProducer->qbuf(portBufferPair.first, portBufferPair.second);
    }

    return OK;
}

/* get the tnr7 tuning data from ISP. PalOutput can help decode IPU tnr parameters from IPU result
** The GPU tnr7 parameters struct is different from IPU tnr parameters struct, so here we copy
** the useful parameter to tnr7 parameters object
*/
int GPUExecutor::updateTnrISPConfig(Tnr7Param* pbuffer, uint32_t sequence) {
    int ret = ia_err_none;
    if (mAdaptor) {
        CLEAR(*pbuffer);
        // only video stream pipe has tnr tuning data, still pipe use same tnr tuning data as video
        ia_isp_bxt_program_group* pg = mGraphConfig->getProgramGroup(mStreamId);
        CheckError(pg == nullptr, UNKNOWN_ERROR, "Can't get IPU program group");

        // Get ISP parameters
        LOG1(" %s update tnr parameters for sequence %d", __func__, sequence);

        ia_binary_data* ipuParameters = mAdaptor->getIpuParameter(sequence, mStreamId);
        CheckError(ipuParameters == nullptr, UNKNOWN_ERROR, "Failed to find ISP parameter");

        ia_isp_bxt_program_group tmpPg = *pg;
        tmpPg.kernel_count = 0;
        int blankKernelSize = mAdaptor->getPalOutputDataSize(&tmpPg);

        PalOutputData PalOutput(pg);
        ia_binary_data tmpTnr7BinData = {0};
        tmpTnr7BinData.size = ipuParameters->size - blankKernelSize;
        tmpTnr7BinData.data = reinterpret_cast<char*>(ipuParameters->data) + blankKernelSize;

        PalOutput.setPublicOutput(&tmpTnr7BinData);
        ia_pal_isp_tnr7_bc_1_0_t* pBc;
        ia_pal_isp_tnr7_blend_1_0_t* pBlend;
        ia_pal_isp_tnr7_ims_1_0_t* pIms;
        ret |= PalOutput.getKernelPublicOutput(ia_pal_uuid_isp_tnr7_bc_1_0, (void*&)pBc);
        ret |= PalOutput.getKernelPublicOutput(ia_pal_uuid_isp_tnr7_blend_1_0, (void*&)pBlend);
        ret |= PalOutput.getKernelPublicOutput(ia_pal_uuid_isp_tnr7_ims_1_0, (void*&)pIms);
        CheckError(ret != ia_err_none, UNKNOWN_ERROR, "Can't read isp tnr7 parameters");

        tnr7_bc_1_0_t* tnr7_bc = &(pbuffer->bc);
        tnr7_blend_1_0_t* tnr7_blend = &(pbuffer->blend);
        tnr7_ims_1_0_t* tnr7_ims = &(pbuffer->ims);

        // tnr7 ims params
        tnr7_ims->enable = pIms->enable;
        tnr7_ims->update_limit = pIms->update_limit;
        tnr7_ims->update_coeff = pIms->update_coeff;
        tnr7_ims->gpu_mode = pIms->gpu_mode;
        for (int i = 0; i < sizeof(pIms->d_ml) / sizeof(int32_t); i++) {
            tnr7_ims->d_ml[i] = pIms->d_ml[i];
            tnr7_ims->d_slopes[i] = pIms->d_slopes[i];
            tnr7_ims->d_top[i] = pIms->d_top[i];
            tnr7_ims->outofbounds[i] = pIms->outofbounds[i];
        }

        // tnr7 bc params
        tnr7_bc->enable = pBc->enable;
        tnr7_bc->is_first_frame = pBc->is_first_frame;
        tnr7_bc->do_update = pBc->do_update;
        tnr7_bc->gpu_mode = pBc->gpu_mode;
        tnr7_bc->tune_sensitivity = pBc->tune_sensitivity;
        for (int i = 0; i < sizeof(pBc->coeffs) / sizeof(int32_t); i++) {
            tnr7_bc->coeffs[i] = pBc->coeffs[i];
        }

        tnr7_bc->global_protection = pBc->global_protection;
        tnr7_bc->global_protection_motion_level = pBc->global_protection_motion_level;
        for (int i = 0; i < sizeof(pBc->global_protection_sensitivity_lut_values) / sizeof(int32_t);
             i++) {
            tnr7_bc->global_protection_sensitivity_lut_values[i] =
                pBc->global_protection_sensitivity_lut_values[i];
        }
        for (int i = 0; i < sizeof(pBc->global_protection_sensitivity_lut_slopes) / sizeof(int32_t);
             i++) {
            tnr7_bc->global_protection_sensitivity_lut_slopes[i] =
                pBc->global_protection_sensitivity_lut_slopes[i];
        }

        // tnr7 blend params
        tnr7_blend->enable = pBlend->enable;
        tnr7_blend->enable_main_output = pBlend->enable_main_output;
        tnr7_blend->enable_vision_output = pBlend->enable_vision_output;
        tnr7_blend->single_output_mode = pBlend->single_output_mode;
        tnr7_blend->spatial_weight_coeff = pBlend->spatial_weight_coeff;
        tnr7_blend->max_recursive_similarity = pBlend->max_recursive_similarity;
        tnr7_blend->spatial_alpha = pBlend->spatial_alpha;
        tnr7_blend->max_recursive_similarity_vsn = pBlend->max_recursive_similarity_vsn;
        for (int i = 0; i < sizeof(pBlend->w_out_prev_LUT) / sizeof(int32_t); i++) {
            tnr7_blend->w_out_prev_LUT[i] = pBlend->w_out_prev_LUT[i];
            tnr7_blend->w_out_spl_LUT[i] = pBlend->w_out_spl_LUT[i];
            tnr7_blend->w_vsn_out_prev_LUT[i] = pBlend->w_vsn_out_prev_LUT[i];
            tnr7_blend->w_vsn_out_spl_LUT[i] = pBlend->w_vsn_out_spl_LUT[i];
        }

        for (int i = 0; i < sizeof(pBlend->output_cu_a) / sizeof(int32_t); i++) {
            tnr7_blend->output_cu_a[i] = pBlend->output_cu_a[i];
            tnr7_blend->output_cu_b[i] = pBlend->output_cu_b[i];
        }

        for (int i = 0; i < sizeof(pBlend->output_cu_x) / sizeof(int32_t); i++) {
            tnr7_blend->output_cu_x[i] = pBlend->output_cu_x[i];
        }
    }

    return ret;
}

int GPUExecutor::runTnrFrame(const std::shared_ptr<CameraBuffer>& inBuf,
                             std::shared_ptr<CameraBuffer> outBuf) {
    PERF_CAMERA_ATRACE();
    CheckError(!inBuf->getBufferAddr(), UNKNOWN_ERROR, "%s invalid input buffer", __func__);

    if (mPolicyManager) {
        // Check if need to wait other executors.
        mPolicyManager->wait(mName);
    }
    LOG2("Enter %s executor name:%s, sequence: %u", __func__, mName.c_str(), inBuf->getSequence());
    TRACE_LOG_PROCESS(mName.c_str(), __func__, MAKE_COLOR(inBuf->getSequence()),
                      inBuf->getSequence());

    uint32_t sequence = inBuf->getSequence();
    int ret = OK;
    if (mIntelTNR) {
        ret = updateTnrISPConfig(mTnr7usParam, sequence);
        CheckError(ret != OK, UNKNOWN_ERROR, " %s Failed to update TNR parameters", __func__);
    }

    int fd = outBuf->getFd();
    int memoryType = outBuf->getMemory();
    int bufferSize = outBuf->getBufferSize();
    void* outPtr = (memoryType == V4L2_MEMORY_DMABUF)
                       ? CameraBuffer::mapDmaBufferAddr(fd, bufferSize)
                       : outBuf->getBufferAddr();
    if (!outPtr) return UNKNOWN_ERROR;

    if (!mIntelTNR) {
        MEMCPY_S(outPtr, bufferSize, inBuf->getBufferAddr(), inBuf->getBufferSize());
        if (memoryType == V4L2_MEMORY_DMABUF) {
            CameraBuffer::unmapDmaBufferAddr(outPtr, bufferSize);
        }
        return OK;
    }

    if (icamera::PlatformData::isStillTnrPrior()) {
        // when running still stream tnr, should skip video tnr to decrease still capture duration.
        if (mStreamId == VIDEO_STREAM_ID && !mGPULock.try_lock()) {
            MEMCPY_S(outPtr, bufferSize, inBuf->getBufferAddr(), inBuf->getBufferSize());
            if (memoryType == V4L2_MEMORY_DMABUF) {
                CameraBuffer::unmapDmaBufferAddr(outPtr, bufferSize);
            }
            mLastSequence = UINT32_MAX;
            LOG2("%s executor name:%s, skip frame sequence: %ld", __func__, mName.c_str(),
                 inBuf->getSequence());
            return OK;
        } else if (mStreamId == STILL_STREAM_ID) {
            mGPULock.lock();
        }
    }

    if (mLastSequence == UINT32_MAX || sequence - mLastSequence >= TNR7US_RESTART_THRESHOLD) {
        mTnr7usParam->bc.is_first_frame = 1;
    } else {
        mTnr7usParam->bc.is_first_frame = 0;
    }

    void* dstBuf = outPtr;
    int dstSize = bufferSize;
    int dstFd = fd;
    std::map<int64_t, void*>::iterator tnrOutBuf;
    // use internal tnr buffer for ZSL and none APP buffer request usage
    bool useInternalBuffer = mUseInternalTnrBuffer || memoryType != V4L2_MEMORY_DMABUF;
    if (useInternalBuffer) {
        std::unique_lock<std::mutex> lock(mTnrOutBufMapLock);
        tnrOutBuf = mTnrOutBufMap.begin();
        dstBuf = tnrOutBuf->second;
        dstSize = mOutBufferSize;
        // when use internal tnr buffer, we don't need to use fd map buffer
        dstFd = -1;
    }
    ret = mIntelTNR->runTnrFrame(inBuf->getBufferAddr(), dstBuf, inBuf->getBufferSize(), dstSize,
                                 mTnr7usParam, dstFd);

    if (ret == OK) {
        if (useInternalBuffer) {
            MEMCPY_S(outPtr, bufferSize, tnrOutBuf->second, mOutBufferSize);
        }
    } else {
        LOG2("%s, Just copy source buffer if run TNR failed", __func__);
        MEMCPY_S(outPtr, bufferSize, inBuf->getBufferAddr(), inBuf->getBufferSize());
    }
    if (icamera::PlatformData::isStillTnrPrior()) {
        mGPULock.unlock();
    }

    if (useInternalBuffer) {
        std::unique_lock<std::mutex> lock(mTnrOutBufMapLock);
        mTnrOutBufMap.erase(tnrOutBuf->first);
        mTnrOutBufMap[sequence] = tnrOutBuf->second;
        LOG2("%s, outBuf->first %ld, outBuf->second %p", __func__, tnrOutBuf->first,
             tnrOutBuf->second);
    }

    if (memoryType == V4L2_MEMORY_DMABUF) {
        CameraBuffer::unmapDmaBufferAddr(outPtr, bufferSize);
    }
    CheckError(ret != OK, UNKNOWN_ERROR, " %s tnr7us run frame failed", __func__);
    mLastSequence = sequence;

    if (mStreamId != STILL_STREAM_ID) {
        // still stream will update params in tnr7us, skip async update
        AiqResult* aiqResults = const_cast<AiqResult*>(
            AiqResultStorage::getInstance(mCameraId)->getAiqResult(sequence));
        if (aiqResults == nullptr) {
            LOGW("%s: no result for sequence %ld! use the latest instead", __func__, sequence);
            aiqResults =
                const_cast<AiqResult*>(AiqResultStorage::getInstance(mCameraId)->getAiqResult());
            CheckError((aiqResults == nullptr), UNKNOWN_ERROR, "Cannot find available aiq result.");
        }

        int gain = static_cast<int>(aiqResults->mAeResults.exposures[0].exposure->analog_gain *
                                    aiqResults->mAeResults.exposures[0].exposure->digital_gain);

        // update tnr param when total gain changes, gain set to analog_gain * digital_gain.
        bool isTnrParamForceUpdate = icamera::PlatformData::isTnrParamForceUpdate();
        ret = mIntelTNR->asyncParamUpdate(gain, isTnrParamForceUpdate);
    }

    LOG2("Exit %s executor name:%s, sequence: %u", __func__, mName.c_str(), inBuf->getSequence());
    return ret;
}

int GPUExecutor::dumpTnrParameters(uint32_t sequence) {
    const int DUMP_FILE_SIZE = 0x1000;
    std::string dumpFileName =
        std::string("/home/tnr7-") + std::to_string(sequence) + std::string(".txt");

    LOG1("%s save tnr7 parameters to file %s", __func__, dumpFileName.c_str());
    tnr7_bc_1_0_t* tnr7_bc = &(mTnr7usParam->bc);
    tnr7_blend_1_0_t* tnr7_blend = &(mTnr7usParam->blend);
    tnr7_ims_1_0_t* tnr7_ims = &(mTnr7usParam->ims);
    char* dumpData = reinterpret_cast<char*>(malloc(DUMP_FILE_SIZE));
    CheckError(dumpData == nullptr, NO_MEMORY, "failed to allocate memory for dump tnr7");

    FILE* parFile = fopen(dumpFileName.c_str(), "wb");
    if (parFile) {
        int shift = 0;
        size_t length = snprintf(dumpData + shift, DUMP_FILE_SIZE - shift, "%s\n", "tnr7_bc");
        shift += length;
        for (int i = 0; i < sizeof(tnr7_bc_1_0_t) / sizeof(int32_t); i++) {
            length = snprintf(dumpData + shift, DUMP_FILE_SIZE - shift, "%u\n",
                              *(reinterpret_cast<int32_t*>(tnr7_bc) + i));
            shift += length;
        }
        length = snprintf(dumpData + shift, DUMP_FILE_SIZE - shift, "%s\n", "tnr7_blend");
        shift += length;
        for (int i = 0; i < sizeof(tnr7_blend_1_0_t) / sizeof(int32_t); i++) {
            length = snprintf(dumpData + shift, DUMP_FILE_SIZE - shift, "%u\n",
                              *(reinterpret_cast<int32_t*>(tnr7_blend) + i));
            shift += length;
        }
        length = snprintf(dumpData + shift, DUMP_FILE_SIZE - shift, "%s\n", "tnr7_ims");
        shift += length;
        for (int i = 0; i < sizeof(tnr7_ims_1_0_t) / sizeof(int32_t); i++) {
            length = snprintf(dumpData + shift, DUMP_FILE_SIZE - shift, "%u\n",
                              *(reinterpret_cast<int32_t*>(tnr7_ims) + i));
            shift += length;
        }

        fwrite(dumpData, shift, 1, parFile);
        fclose(parFile);
    } else {
        LOGW("tnr7 failed to open dump file %s", (dumpFileName + std::to_string(sequence)).c_str());
    }
    free(dumpData);
    return OK;
}
}  // namespace icamera
