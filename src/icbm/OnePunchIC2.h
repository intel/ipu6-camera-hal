/*
 * Copyright (C) 2023 Intel Corporation
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
#include <memory>
#include <mutex>
#include <unordered_map>

#include "src/iutils/Utils.h"

#include "src/icbm/ICBMTypes.h"
#include "src/icbm/OPIC2Api.h"
#include "src/icbm/MemoryChain.h"

namespace icamera {

struct IOPIC2Algorithm {
    virtual ~IOPIC2Algorithm() = default;
    virtual void linkToMemoryChain(MemoryChainDescription& memoryChain) = 0;
};

class UserFramingBuilder : public IOPIC2Algorithm {
 public:
    UserFramingBuilder() = default;
    ~UserFramingBuilder() = default;

    void linkToMemoryChain(MemoryChainDescription& memoryChain) override;
};

class BackgroundBlurBuilder : public IOPIC2Algorithm {
 public:
    BackgroundBlurBuilder() = default;
    ~BackgroundBlurBuilder() = default;

    void linkToMemoryChain(MemoryChainDescription& memoryChain) override;
};

class IntelOPIC2 {
 public:
    static IntelOPIC2* getInstance();
    static void releaseInstance();

    /**
     * \brief create level0 session according to the cameraID and request type
     *
     * \return 0 if succeed.
     */
    int setup(ICBMInitInfo* initParam, std::shared_ptr<IC2ApiHandle> handle);

    /**
     * \brief shundown level0 session according to the cameraID and request type
     *
     * \return active session count(>=0) if succeed. <0 if failed
     */
    int shutdown(const ICBMReqInfo& reqInfo);

    int processFrame(const ICBMReqInfo& reqInfo);

    /**
     * \brief process tnr frame
     *
     * \return 0 if succeed
     */
    int runTnrFrame(const ICBMReqInfo& reqInfo);

 private:
    static IntelOPIC2* sInstance;
    static std::mutex sLock;
    std::shared_ptr<IC2ApiHandle> mIC2Api;

    IntelOPIC2();
    ~IntelOPIC2();
    int loadIC2Library();
    // lock for each session, key is from getIndexKey()
    std::unordered_map<int, std::unique_ptr<std::mutex>> mLockMap;
    // session map, key is from getIndexKey()
    std::unordered_map<int, iaic_session> mSessionMap;
    // feature vector of each session
    std::unordered_map<iaic_session, std::vector<const char*>> mFeatureMap;

    // transfer cameraId and type to index of the mSessionMap and mLockMap
    int getIndexKey(int cameraId, uint32_t type) {
        return (cameraId << ICBM_REQUEST_MAX_SHIFT) + type;
    }

    static MemoryChainDescription createMemoryChain(const ICBMReqInfo& reqInfo);

    // set parameters to the session before process
    void setData(iaic_session uid, void* p, size_t size, const char* featureName,
                 const char* portName);

    DISALLOW_COPY_AND_ASSIGN(IntelOPIC2);
};

}  // namespace icamera
