/*
 * Copyright (C) 2021 Intel Corporation
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

#ifdef ENABLE_SANDBOXING
#include "modules/sandboxing/client/IntelEvcpClient.h"
#else
#include "modules/algowrapper/IntelEvcp.h"
#endif

#include <memory>
#include <queue>
#include <unordered_map>

#include "EvcpCommon.h"
#include "Parameters.h"
#include "iutils/Errors.h"
#include "iutils/Thread.h"
#include "iutils/Utils.h"

namespace icamera {
class IntelECC;

class EvcpManager {
 public:
    EvcpManager(int cameraId, int width, int height, EvcpParam* evcpParam);
    virtual ~EvcpManager();

    static bool createInstance(int cameraId, int width, int height);
    static EvcpManager* getInstance(int cameraId);
    static void destoryInstance(int cameraId);

    void runEvcp(const camera_buffer_t& buffer, icamera::Parameters* param);
    bool updateEvcpParam(EvcpParam evcpParam);
    EvcpParam getEvcpParam() const;

 private:
    void runEvcpL(const camera_buffer_t& buffer);
    bool init();
    bool checkingStatus();
    void prepare4Param(icamera::Parameters* param);
    EvcpParam getParamFromExp(const icamera::Parameters* param);
    bool saveNV12Img(int fd, int width, int height, char* filename);

    static Mutex sLock;
    static std::unordered_map<int, EvcpManager*> sInstances;

    static Mutex sParamLock;
    static std::unordered_map<int, EvcpParam> mLatestParam;

    int mCameraId;

    std::unique_ptr<IntelEvcp> mEvcp;

    int mWidth;
    int mHeight;

    DISALLOW_COPY_AND_ASSIGN(EvcpManager);
};

}  // namespace icamera
