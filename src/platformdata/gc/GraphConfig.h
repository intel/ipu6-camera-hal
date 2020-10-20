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
#include <utility>
#include <vector>
#include "HalStream.h"
#include "iutils/Utils.h"
#include "iutils/Errors.h"

#ifdef ENABLE_SANDBOXING
#include "modules/sandboxing/client/GraphConfigImpl.h"
#else
#include "modules/algowrapper/graph/GraphConfigImpl.h"
#endif

namespace icamera {

/**
 * \class GraphConfig
 *
 * \brief This is a wrapper of GraphConfigImpl class and it provides the
 * public APIs to get the graph config data.
 *
 * It maintains one static area and GraphConfigImpl object, user get graph
 * config data from the local structure or GraphConfigImpl object through
 * the public APIs
 */
class GraphConfig : public IGraphConfig {
public:
    GraphConfig();
    GraphConfig(int32_t camId, ConfigMode mode);
    virtual ~GraphConfig();

    void addCustomKeyMap();
    status_t parse(int cameraId, const char *settingsXmlFile);
    void releaseGraphNodes();

    // These public methods called by GraphConfigManager
    status_t configStreams(const std::vector<HalStream*> &activeStreams);
    int getSelectedMcId() { return mGraphData.mcId; }
    virtual int getGraphId(void) { return mGraphData.graphId; }
    virtual void getCSIOutputResolution(camera_resolution_t &reso) { reso = mGraphData.csiReso; }

    virtual status_t getGdcKernelSetting(uint32_t *kernelId,
                                         ia_isp_bxt_resolution_info_t *resolution);
    virtual status_t graphGetStreamIds(std::vector<int32_t> &streamIds);
    virtual int getStreamIdByPgName(std::string pgName);
    virtual int getPgIdByPgName(std::string pgName);
    virtual ia_isp_bxt_program_group *getProgramGroup(int32_t streamId);
    virtual status_t getPgRbmValue(std::string pgName, IGraphType::StageAttr *stageAttr);
    virtual status_t getMBRData(int32_t streamId, ia_isp_bxt_gdc_limits *data);
    virtual status_t getPgNames(std::vector<std::string>* pgNames);

    virtual int getProgramGroup(std::string pgName,
                                ia_isp_bxt_program_group* programGroupForPG);
    virtual status_t getPgIdForKernel(const uint32_t streamId,
                                      const int32_t kernelId, int32_t *pgId);

    virtual status_t pipelineGetConnections(
                         const std::vector<std::string> &pgList,
                         std::vector<IGraphType::PipelineConnection> *confVector);
private:
    // Disable copy constructor and assignment operator
    DISALLOW_COPY_AND_ASSIGN(GraphConfig);

private:
    int32_t mCameraId;
    IGraphType::GraphConfigData mGraphData;
    std::unique_ptr<GraphConfigImpl> mGraphConfigImpl;
};

} // namespace icamera
