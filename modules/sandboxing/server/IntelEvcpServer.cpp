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

#define LOG_TAG IntelEVCPServer

#include "modules/sandboxing/server/IntelEvcpServer.h"

#include "CameraLog.h"
#include "iutils/Utils.h"

namespace icamera {
int IntelEvcpServer::init(void* pData, int dataSize) {
    CheckAndLogError(pData == nullptr, UNKNOWN_ERROR, "%s, pData is nullptr", __func__);
    CheckAndLogError(dataSize < static_cast<int>(sizeof(EvcpResolution)), UNKNOWN_ERROR,
                     "%s, Buffer mismatch", __func__);

    EvcpResolution* res = reinterpret_cast<EvcpResolution*>(pData);
    mIntelEvcp = std::unique_ptr<IntelEvcp>(new IntelEvcp());

    return mIntelEvcp->init(res->width, res->height);
}

int IntelEvcpServer::deInit() {
    mIntelEvcp = nullptr;

    return OK;
}

int IntelEvcpServer::runEvcpFrame(void* bufferAddr, int size) {
    CheckAndLogError(bufferAddr == nullptr, UNKNOWN_ERROR, "%s, Buffer is nullptr", __func__);

    return mIntelEvcp->runEvcpFrame(bufferAddr, size) ? OK : UNKNOWN_ERROR;
}

int IntelEvcpServer::updateEvcpParam(EvcpParam* param) {
    CheckAndLogError(param == nullptr, UNKNOWN_ERROR, "%s, new param is nullptr", __func__);

    return mIntelEvcp->updateEvcpParam(param) ? OK : UNKNOWN_ERROR;
}

int IntelEvcpServer::getEvcpParam(EvcpParam* param) const {
    CheckAndLogError(param == nullptr, UNKNOWN_ERROR, "%s, param is nullptr", __func__);

    mIntelEvcp->getEvcpParam(param);

    return OK;
}

}  // namespace icamera
