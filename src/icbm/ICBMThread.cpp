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

void ICBMThread::shutdown() {
    LOG1("%s, Shuting down...", __func__);
    mIntelICBM->shutdown();
}

int ICBMThread::processFrame(const camera_buffer_t& inBuffer, const camera_buffer_t& outBuffer,
                             ICBMReqInfo* param) {
    LOG2("%s, Processing frame", __func__);
    ImageInfo iii = {};
    iii.width = inBuffer.s.width;
    iii.height = inBuffer.s.height;
    iii.size = inBuffer.s.size;
    iii.stride = inBuffer.s.stride;

    ImageInfo iio = {};
    iio.width = outBuffer.s.width;
    iio.height = outBuffer.s.height;
    iio.size = outBuffer.s.size;
    iio.stride = outBuffer.s.stride;

#ifdef ENABLE_SANDBOXING
    iii.gfxHandle = inBuffer.dmafd;
    iio.gfxHandle = outBuffer.dmafd;

    auto ret = mIntelICBM->processFrame(iii, iio, *param);
#else
    void* pInBuf = (inBuffer.s.memType == V4L2_MEMORY_DMABUF) ?
                       CameraBuffer::mapDmaBufferAddr(inBuffer.dmafd, inBuffer.s.size) :
                       inBuffer.addr;

    void* pOutBuf = (outBuffer.s.memType == V4L2_MEMORY_DMABUF) ?
                        CameraBuffer::mapDmaBufferAddr(outBuffer.dmafd, outBuffer.s.size) :
                        outBuffer.addr;

    iii.bufAddr = pInBuf;
    iio.bufAddr = pOutBuf;
    auto ret = mIntelICBM->processFrame(iii, iio, *param);

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
