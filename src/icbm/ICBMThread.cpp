/*
 * Copyright (C) 2023 Intel Corporation.
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

#define LOG_TAG ICBMThread

#include "src/icbm/ICBMThread.h"

#include <memory>

#include "ICBMThread.h"
#include "CameraLog.h"

namespace icamera {

int ICBMThread::setup(ICBMInitInfo* initParams) {
    LOG1("%s, Starting up...", __func__);
    mIntelICBM = std::make_unique<IntelICBM>();

    auto ret = mIntelICBM->setup(initParams);
    CheckAndLogError(ret != OK, ret, "%s: Init failed", __func__);

    return OK;
}

void ICBMThread::shutdown(const ICBMReqInfo& request) {
    LOG1("%s, Shuting down...", __func__);
    mIntelICBM->shutdown(request);
}

int ICBMThread::processFrame(const camera_buffer_t& inBuffer, const camera_buffer_t& outBuffer,
                             ICBMReqInfo& request) {
    LOG2("%s, Processing frame", __func__);

    request.inII.width = inBuffer.s.width;
    request.inII.height = inBuffer.s.height;
    request.inII.size = inBuffer.s.size;
    request.inII.stride = inBuffer.s.stride;

    request.outII.width = outBuffer.s.width;
    request.outII.height = outBuffer.s.height;
    request.outII.size = outBuffer.s.size;
    request.outII.stride = outBuffer.s.stride;

#ifdef ENABLE_SANDBOXING
    request.inII.gfxHandle = inBuffer.dmafd;
    request.outII.gfxHandle = outBuffer.dmafd;

    auto ret = mIntelICBM->processFrame(request);
#else
    void* pInBuf = (inBuffer.s.memType == V4L2_MEMORY_DMABUF) ?
                       CameraBuffer::mapDmaBufferAddr(inBuffer.dmafd, inBuffer.s.size) :
                       inBuffer.addr;

    void* pOutBuf = (outBuffer.s.memType == V4L2_MEMORY_DMABUF) ?
                        CameraBuffer::mapDmaBufferAddr(outBuffer.dmafd, outBuffer.s.size) :
                        outBuffer.addr;

    request.inII.bufAddr = pInBuf;
    request.outII.bufAddr = pOutBuf;
    auto ret = mIntelICBM->processFrame(request);

    if (inBuffer.s.memType == V4L2_MEMORY_DMABUF) {
        CameraBuffer::unmapDmaBufferAddr(pInBuf, inBuffer.s.size);
    }

    if (outBuffer.s.memType == V4L2_MEMORY_DMABUF) {
        CameraBuffer::unmapDmaBufferAddr(pOutBuf, outBuffer.s.size);
    }
#endif

    if (ret != OK) {
        LOGE("%s Run frame fails", __func__);
        return UNKNOWN_ERROR;
    }
    return OK;
}

}  // namespace icamera
