/*
 * Copyright (C) 2019-2020 Intel Corporation.
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

#include <map>
#include <memory>

#include "iutils/Errors.h"
#include "modules/algowrapper/graph/GraphConfigImpl.h"
#include "modules/sandboxing/IPCGraphConfig.h"

namespace icamera {
class GraphConfigServer {
 public:
    GraphConfigServer();
    virtual ~GraphConfigServer();

    void addCustomKeyMap();
    status_t parse(void* pData, size_t dataSize);
    void releaseGraphNodes();
    status_t configStreams(void* pData, size_t dataSize);
    status_t getGraphConfigData(void* pData, size_t dataSize);
    status_t getPgIdForKernel(void* pData, size_t dataSize);
    status_t pipelineGetConnections(void* pData, size_t dataSize);

 private:
    std::map<GraphBaseInfo, std::shared_ptr<GraphConfigImpl> > mGraphConfigMap;
    IPCGraphConfig mIpc;
};
} /* namespace icamera */
