/*
 * Copyright (C) 2020 Intel Corporation.
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

#define LOG_TAG "ShareRefer"

#include "ShareReferBufferPool.h"

#include "PlatformData.h"
#include "iutils/CameraLog.h"
#include "iutils/Errors.h"

using std::map;
using std::unique_ptr;
using std::vector;

namespace icamera {

#define CONSUMER_BUFFER_NUM 2

int64_t ShareReferBufferPool::constructReferId(int32_t streamId, int32_t pgId, int32_t portId) {
    return (((int64_t)streamId << 32) + ((int64_t)pgId << 16) + portId);
}

ShareReferBufferPool::~ShareReferBufferPool() {
    AutoMutex l(mPairLock);
    while (!mUserPairs.empty()) {
        UserPair* pair = mUserPairs.back();
        mUserPairs.pop_back();
        delete pair;
    }
}

int32_t ShareReferBufferPool::setReferPair(const std::string& producerPgName, int64_t producerId,
                                           const std::string& consumerPgName, int64_t consumerId) {
    CheckError(producerId == consumerId, BAD_VALUE, "same pair for producer/consumer %lx",
               producerId);

    UserPair* pair = new UserPair;
    pair->producerPgName = producerPgName;
    pair->producerId = producerId;
    pair->consumerPgName = consumerPgName;
    pair->consumerId = consumerId;
    pair->busy = false;
    LOG1("%s: %s:%lx -> %s:%lx", __func__, producerPgName.c_str(), producerId,
         consumerPgName.c_str(), consumerId);
    AutoMutex l(mPairLock);
    mUserPairs.push_back(pair);
    return OK;
}

int32_t ShareReferBufferPool::clearReferPair(int64_t id) {
    AutoMutex l(mPairLock);
    for (auto it = mUserPairs.begin(); it != mUserPairs.end(); it++) {
        UserPair* pair = *it;
        if (pair->producerId != id && pair->consumerId != id) continue;

        pair->bufferLock.lock();
        if (pair->busy) {
            pair->bufferLock.unlock();
            LOGE("Can't clear pair %lx because Q is busy!", id);
            return UNKNOWN_ERROR;
        }

        mUserPairs.erase(it);
        pair->bufferLock.unlock();
        delete pair;
        return OK;
    }

    return BAD_VALUE;
}

int32_t ShareReferBufferPool::getMinBufferNum(int64_t id) {
    AutoMutex l(mPairLock);
    for (auto pair : mUserPairs) {
        if (pair->producerId == id)
            return PlatformData::getMaxRawDataNum(mCameraId);
        else if (pair->consumerId == id)
            return CONSUMER_BUFFER_NUM;
    }
    return 0;
}

int32_t ShareReferBufferPool::registerReferBuffers(int64_t id, CIPR::Buffer* buffer) {
    AutoMutex l(mPairLock);
    UserPair* pair = findUserPair(id);
    CheckError(!pair, UNKNOWN_ERROR, "Can't find id %lx", id);

    ReferBuffer referBuf = {-1, buffer};
    AutoMutex m(pair->bufferLock);
    std::vector<ReferBuffer>& bufV =
        (id == pair->producerId) ? pair->mProducerBuffers : pair->mConsumerBuffers;
    bufV.push_back(referBuf);
    return OK;
}

/**
 * example for buffer operations:
 * 1. sequence in buffer Q (len is 4)
 *     front->back: S6,   S7,   S8,   S9
 * 2. acquireBuffer before running for frame 10:
 *     front->back: (out),S7,   S8,   S9(in)     (pop s6 as new output)
 * 3. releaseBuffer after running:
 *     front->back:       S7,   S8,   S9,   S10   (push_back S10, new output)
 */
int32_t ShareReferBufferPool::acquireBuffer(int64_t id, CIPR::Buffer** referIn,
                                            CIPR::Buffer** referOut, long outSequence) {
    CheckError(!referIn || !referOut, BAD_VALUE, "nullptr input for refer buf pair");

    long inSequence = outSequence - 1;
    UserPair* pair = nullptr;
    {
        AutoMutex l(mPairLock);
        pair = findUserPair(id);
        CheckError(!pair, UNKNOWN_ERROR, "Can't find id %lx", id);

        AutoMutex m(pair->bufferLock);
        std::vector<ReferBuffer>& bufV =
            (id == pair->producerId) ? pair->mProducerBuffers : pair->mConsumerBuffers;
        CheckError(bufV.empty(), BAD_VALUE, "no refer buffer for id %lx", id);

        *referOut = bufV.front().buffer;
        bufV.erase(bufV.begin());  // pop front (the oldest one) as new output
        *referIn = bufV.back().buffer;
        if (bufV.back().sequence == inSequence || inSequence < 0) {
            // Return if found required buffers or it is the 1st frame.
            LOG2("%lx acquire in seq %ld, got %ld, out seq %ld", id, inSequence,
                 bufV.back().sequence, outSequence);
            return OK;
        } else if (id == pair->producerId) {
            // Find required refer in buffer for producer.
            // In general, it happens in reprocessing case that producer want to run old frame
            for (auto& item : bufV) {
                if (item.sequence == inSequence) {
                    *referIn = item.buffer;
                    LOG2("%lx acquire in seq %ld for reprocessing", id, inSequence);
                    return OK;
                }
            }
            LOGE("%lx has no refer in seq %ld", id, inSequence);
            return UNKNOWN_ERROR;
        }
        pair->busy = true;
    }

    LOG1("consumer %s try to get in seq %ld from %s", pair->consumerPgName.c_str(), inSequence,
         pair->producerPgName.c_str());
    CIPR::Buffer* srcBuf = nullptr;
    int ret = NOT_ENOUGH_DATA;
    int waitFrames = 3;  // wait 3 frames
    while (waitFrames-- && ret == NOT_ENOUGH_DATA) {
        ConditionLock lock(pair->bufferLock);
        ret = findReferBuffer(&pair->mProducerBuffers, inSequence, &srcBuf);

        if (ret == NOT_ENOUGH_DATA) {
            pair->bufferSignal.waitRelative(lock, kWaitDuration * SLOWLY_MULTIPLIER);
        }
    }

    if (ret == OK && srcBuf) {
        void* srcPtr = nullptr;
        int32_t srcSize = 0;
        srcBuf->getMemoryCpuPtr(&srcPtr);
        srcBuf->getMemorySize(&srcSize);
        void* dstPtr = nullptr;
        int32_t dstSize = 0;
        (*referIn)->getMemoryCpuPtr(&dstPtr);
        (*referIn)->getMemorySize(&dstSize);
        if (srcPtr && dstPtr) {
            MEMCPY_S(dstPtr, dstSize, srcPtr, srcSize);
        }
        LOG1("%s acquire in seq %ld (copy from %s), out seq %ld", pair->consumerPgName.c_str(),
             inSequence, pair->producerPgName.c_str(), outSequence);
    }

    AutoMutex m(pair->bufferLock);
    pair->busy = false;
    return ret;
}

int32_t ShareReferBufferPool::releaseBuffer(int64_t id, CIPR::Buffer* referIn,
                                            CIPR::Buffer* referOut, long outSequence) {
    CheckError(!referIn || !referOut, BAD_VALUE, "nullptr for refer buf pair for release");

    AutoMutex l(mPairLock);
    UserPair* pair = findUserPair(id);
    CheckError(!pair, UNKNOWN_ERROR, "Can't find id %lx", id);

    AutoMutex m(pair->bufferLock);
    std::vector<ReferBuffer>& bufV =
        (id == pair->producerId) ? pair->mProducerBuffers : pair->mConsumerBuffers;
    if (outSequence < bufV.back().sequence) {
        // Drop old data (in reprocessing case)
        ReferBuffer referBuf = {-1, referOut};
        bufV.insert(bufV.begin(), referBuf);
    } else {
        ReferBuffer referBuf = {outSequence, referOut};
        bufV.push_back(referBuf);
    }
    pair->bufferSignal.signal();

    return OK;
}

ShareReferBufferPool::UserPair* ShareReferBufferPool::findUserPair(int64_t id) {
    for (auto pair : mUserPairs) {
        if (pair->consumerId == id || pair->producerId == id) {
            return pair;
        }
    }
    return nullptr;
}

int ShareReferBufferPool::findReferBuffer(std::vector<ReferBuffer>* bufV, long sequence,
                                          CIPR::Buffer** out) {
    CheckError(!bufV, BAD_VALUE, "nullptr buffers");
    CheckError(!out, BAD_VALUE, "nullptr out buffer");

    if (bufV->empty() || bufV->back().sequence < sequence) return NOT_ENOUGH_DATA;

    for (auto item = bufV->rbegin(); item != bufV->rend(); item++) {
        if (item->sequence <= sequence) {
            *out = item->buffer;
            LOG2("%s: find seq %ld for required seq %ld", __func__, item->sequence, sequence);
            return OK;
        }
    }

    LOGE("No refer buffer with required seq %ld", sequence);
    return UNKNOWN_ERROR;
}

}  // namespace icamera
