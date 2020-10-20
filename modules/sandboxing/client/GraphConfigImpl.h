/*
 * Copyright (C) 2019-2020 Intel Corporation
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

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "IntelAlgoCommon.h"
#include "iutils/Errors.h"
#include "iutils/Thread.h"
#include "iutils/Utils.h"
#include "modules/sandboxing/IPCGraphConfig.h"
#include "src/platformdata/CameraTypes.h"
#include "src/platformdata/gc/HalStream.h"
#include "src/platformdata/gc/IGraphConfig.h"

namespace icamera {

class GraphConfigImpl {
 public:
    GraphConfigImpl();
    GraphConfigImpl(int32_t camId, ConfigMode mode, GraphSettingType type);
    virtual ~GraphConfigImpl();

    void addCustomKeyMap();
    status_t parse(int cameraId, const char* graphDescFile, const char* settingsFile);
    void releaseGraphNodes();

    status_t configStreams(const std::vector<HalStream*>& activeStreams);
    status_t getGraphConfigData(IGraphType::GraphConfigData* data);

    int getProgramGroup(std::string pgName, ia_isp_bxt_program_group* programGroupForPG);
    status_t getPgIdForKernel(const uint32_t streamId, const int32_t kernelId, int32_t* pgId);

    status_t pipelineGetConnections(const std::vector<std::string>& pgList,
                                    std::vector<IGraphType::ScalerInfo>* scalerInfo,
                                    std::vector<IGraphType::PipelineConnection>* confVector);

 private:
    IPCGraphConfig mIpc;
    IntelAlgoCommon mCommon;

    ShmMemInfo mMemParse;
    ShmMemInfo mMemConfig;
    ShmMemInfo mMemGetData;
    ShmMemInfo mMemGetPgId;
    ShmMemInfo mMemGetConnection;

    std::vector<ShmMem> mMems;

    int mCameraId;
    ConfigMode mConfigMode;
    GraphSettingType mType;
    bool mInitialized;

    // Disable copy constructor and assignment operator
    DISALLOW_COPY_AND_ASSIGN(GraphConfigImpl);
};
}  // namespace icamera
