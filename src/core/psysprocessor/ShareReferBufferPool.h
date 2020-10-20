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

#pragma once

#include <string>
#include <vector>

#include "iutils/Thread.h"
#include "iutils/Utils.h"
#include "modules/ia_cipr/include/Buffer.h"

namespace icamera {

/**
 * \class ShareReferBufferPool
 *
 * \brief This is a version reference buffer/payload memory sharing between PGs, which is used to
 *        copy tnr reference frame/parameter from video pipe to still pipe.
 */
class ShareReferBufferPool {
 public:
    /**
     * Construct share refer ID with stream id, pg id and (input) port id.
     * Share refer ID is an unique identification for one pair of refer in/out terminals.
     */
    static int64_t constructReferId(int32_t streamId, int32_t pgId, int32_t portId);

 public:
    explicit ShareReferBufferPool(int32_t cameraId) { mCameraId = cameraId; }
    virtual ~ShareReferBufferPool();

    int32_t setReferPair(const std::string& producerPgName, int64_t producerId,
                         const std::string& consumerPgName, int64_t consumerId);
    int32_t clearReferPair(int64_t id);
    int32_t getMinBufferNum(int64_t id);

    int32_t registerReferBuffers(int64_t id, CIPR::Buffer* buffer);

    /**
     * Cosumer can identify out sequence in acquireBuffer()
     * to copy from producer queue in case it can't find refer-in buffer in its own queue.
     */
    int32_t acquireBuffer(int64_t id, CIPR::Buffer** referIn, CIPR::Buffer** referOut,
                          long outSequence = -1);
    int32_t releaseBuffer(int64_t owner, CIPR::Buffer* referIn, CIPR::Buffer* referOut,
                          long outSequence);

 private:
    struct ReferBuffer {
        long sequence;
        CIPR::Buffer* buffer;

        ReferBuffer(long seq = -1, CIPR::Buffer* buf = nullptr) {
            sequence = seq;
            buffer = buf;
        }
    };

    struct UserPair {
        std::string producerPgName;  // for debug
        std::string consumerPgName;
        int64_t producerId;
        int64_t consumerId;

        Mutex bufferLock;
        Condition bufferSignal;
        bool busy;

        // Sort sequence in ascending order
        std::vector<ReferBuffer> mProducerBuffers;
        std::vector<ReferBuffer> mConsumerBuffers;
    };

 private:
    UserPair* findUserPair(int64_t id);
    int findReferBuffer(std::vector<ReferBuffer>* bufV, long sequence, CIPR::Buffer** out);

 private:
    static const nsecs_t kWaitDuration = 33000000;  // 33ms

    int32_t mCameraId;
    Mutex mPairLock;
    std::vector<UserPair*> mUserPairs;

 private:
    DISALLOW_COPY_AND_ASSIGN(ShareReferBufferPool);
};

}  // namespace icamera
