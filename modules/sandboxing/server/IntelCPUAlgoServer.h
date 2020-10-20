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

#include <base/bind.h>
#include <base/threading/thread.h>

#include <memory>
#include <queue>
#include <unordered_map>

#include "CameraLog.h"
#include "GraphConfigServer.h"
#include "IntelAiqServer.h"
#include "IntelAlgoServer.h"
#include "IntelCmcServer.h"
#include "IntelDvsServer.h"
#include "IntelFDServer.h"
#include "IntelLardServer.h"
#include "IntelLtmServer.h"
#include "IntelMknServer.h"
#include "IntelPGParamServer.h"
#include "IspParamAdaptorServer.h"
#include "cros-camera/camera_algorithm.h"
#include "iutils/Errors.h"
#include "iutils/Thread.h"
#include "modules/sandboxing/IPCCommon.h"

namespace icamera {

class IntelCPUAlgoServer : public RequestHandler {
 public:
    explicit IntelCPUAlgoServer(IntelAlgoServer* server);
    virtual ~IntelCPUAlgoServer() {}
    void handleRequest(const MsgReq& msg);

 private:
    IntelLardServer mLard;
    IntelFDServer mFaceDetection;
    GraphConfigServer mGraph;
    IntelCmcServer mCmc;
    IntelMknServer mMkn;
    IntelLtmServer mLtm;
    IntelAiqServer mAiq;
    IntelDvsServer mDvs;
    IspParamAdaptorServer mIspAdaptor;
    IntelPGParamServer mPGParam;
};
}  // namespace icamera
