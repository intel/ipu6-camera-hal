/*
 * Copyright (C) 2015-2021 Intel Corporation
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

#include <gcss.h>
#include <gcss_aic_utils.h>
#include <ia_aiq.h>
#include <ia_isp_bxt_types.h>

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "iutils/CameraLog.h"
#include "iutils/Errors.h"
#include "iutils/Utils.h"
#include "src/platformdata/gc/IGraphConfig.h"

#define NODE_NAME(x) (getNodeName(x).c_str())

namespace icamera {

/**
 * \class SinkDependency
 *
 * This class is a container for sink dependency information for each virtual sink.
 * This information is useful to determine the connections that preceded the
 * virtual sink.
 * We do not go all the way up to the sensor (we could), we just store the
 * terminal id of the input port of the pipeline that serves a particular sink
 * (i.e. the input port of the video pipe or still pipe)
 */
class SinkDependency {
 public:
    SinkDependency() : sinkGCKey(0), streamId(-1), streamInputPortId(0), peer(nullptr) {}

    uid_t sinkGCKey;             /**< GCSS_KEY that represents a sink, like GCSS_KEY_VIDEO1 */
    int32_t streamId;            /**< (a.k.a pipeline id) linked to this sink (ex 60000) */
    uid_t streamInputPortId;     /**< 4CC code of that terminal */
    GCSS::GraphConfigNode* peer; /**< pointer to peer of this sink */
};
/**
 * \class GraphConfigPipe
 *
 * Reference and accessor to pipe configuration for specific request.
 *
 * In general case, at sream-config time there are multiple possible graphs.
 * Per each request there is additional intent that can narrow down the
 * possibilities to single graph settings: the GraphConfigPipe object.
 *
 * This class is instantiated by \class GraphConfigManager for each request,
 * and passed around HAL (control unit, capture unit, processing unit) via
 * shared pointers. The objects are read-only and owned by GCM.
 */
class GraphConfigPipe {
 public:
    typedef std::vector<int32_t> StreamsVector;
    typedef std::map<int32_t, int32_t> StreamsMap;
    typedef std::map<HalStream*, uid_t> StreamToSinkMap;
    static const int32_t PORT_DIRECTION_INPUT = 0;
    static const int32_t PORT_DIRECTION_OUTPUT = 1;

 public:
    explicit GraphConfigPipe(int pipeUseCase);
    ~GraphConfigPipe();

    int getGraphId(void);
    /*
     * Convert Node to GraphConfig interface
     */
    const GCSS::IGraphConfig* getInterface(Node* node) const;
    ia_isp_bxt_program_group* getProgramGroup(int32_t streamId);
    status_t getGdcKernelSetting(uint32_t* kernelId, ia_isp_bxt_resolution_info_t* resolution);
    const ia_isp_bxt_resolution_info_t* getKernelResolutionInfo(uint32_t streamId,
                                                                uint32_t kernelId);
    bool isKernelInStream(uint32_t streamId, uint32_t kernelId);
    status_t getPgIdForKernel(const uint32_t streamId, const int32_t kernelId, int32_t* pgId);
    status_t getMBRData(int32_t streamId, ia_isp_bxt_gdc_limits* data);
    status_t prepare(Node* settings, const StreamToSinkMap& streamToSinkIdMap);

    /*
     * Find distinct stream ids from the graph
     */
    status_t graphGetStreamIds(std::vector<int32_t>* streamIds);
    /*
     * Sink Interrogation methods
     */
    int32_t portGetStreamId(Node* port);
    /*
     * Stream Interrogation methods
     */
    status_t streamGetProgramGroups(int32_t streamId, NodesPtrVector* programGroups);
    /*
     * Port Interrogation methods
     */
    status_t portGetFullName(Node* port, std::string* fullName);
    status_t portGetPeer(Node* port, Node** peer);
    status_t portGetFormat(Node* port, IGraphType::PortFormatSettings* format);
    status_t portGetConnection(Node* port, IGraphType::ConnectionConfig* connectionInfo,
                               Node** peerPort);
    status_t portGetClientStream(Node* port, HalStream** stream);
    int32_t portGetDirection(Node* port);
    bool portIsVirtual(Node* port);
    bool portIsEdgePort(Node* port);

    /*
     * Pipeline connection support
     */
    status_t portGetOwner(Node* port, IGraphType::ConnectionConfig* connectionInfo);
    status_t pipelineGetConnections(const std::vector<std::string>& pgList,
                                    std::vector<IGraphType::ScalerInfo>* scalerInfo,
                                    std::vector<IGraphType::PipelineConnection>* confVector,
                                    std::vector<IGraphType::PrivPortFormat>* tnrPortFormat);

    status_t getPgNames(std::vector<std::string>* pgNames);
    status_t getPgRbmValue(std::string pgName, IGraphType::StageAttr* stageAttr);

    /**
     * Get PG id by given PG name.
     * -1 will be returned if cannot find the valid PG id.
     */
    int getPgIdByPgName(std::string pgName);

    /**
     * Get PG streamId by given PG name.
     * -1 will be returned if cannot find the valid PG id.
     */
    int getStreamIdByPgName(std::string pgName);

    /*
     * Debugging support
     */
    void dumpSettings();
    void dumpKernels(int32_t streamId);
    std::string getNodeName(Node* node);
    void getCSIOutputResolution(camera_resolution_t* reso) { *reso = mCsiOutput; }

 private:
    status_t analyzeSourceType();
    status_t analyzeCSIOutput();
    void calculateSinkDependencies();
    HalStream* getHalStreamByVirtualId(uid_t vPortId);

    // Format options methods
    status_t getActiveOutputPorts(const StreamToSinkMap& streamToSinkIdMap);
    Node* getOutputPortForSink(const std::string& sinkName);
    status_t getSinkFormatOptions();
    status_t setPortFormats();
    bool isVideoRecordPort(Node* sink);
    status_t getProgramGroupsByName(const std::vector<std::string>& pgNames,
                                    NodesPtrVector* programGroups);
    const ia_isp_bxt_resolution_info_t* getGdcKernelResolutionInfo(uint32_t* kernelId);
    const ia_isp_bxt_resolution_info_t* getScalerKernelResolutionInfo(uint32_t* kenerArray,
                                                                      uint32_t sizeArray);
    status_t getScalerKernelResolutionRatio(uint32_t* kenerArray, uint32_t sizeArray,
                                            float* widthRatio, float* heightRatio);
    status_t getScalerByStreamId(
        std::map<Node*, IGraphType::PipelineConnection> edgePort2Connection,
        std::vector<IGraphType::ScalerInfo>* scalerInfo);
    status_t getPrivatePortFormat(Node* port,
                                  std::vector<IGraphType::PrivPortFormat>* tnrPortFormat);

 private:
    GCSS::GraphConfigNode* mSettings;
    std::map<int32_t, ia_isp_bxt_program_group> mProgramGroup;
    GCSS::BxtAicUtils mGCSSAicUtil;
    camera_resolution_t mCsiOutput;
    /**
     * vector holding the peers to the sink nodes. Map contains pairs of
     * {sink, peer}.
     * This map is filled at stream config time.
     */
    std::map<Node*, Node*> mSinkPeerPort;
    /**
     *copy of the map provided from GraphConfigManager to be used internally.
     */
    StreamToSinkMap mStreamToSinkIdMap;
    int mPipeUseCase;

    // Disable copy constructor and assignment operator
    DISALLOW_COPY_AND_ASSIGN(GraphConfigPipe);
};

}  // namespace icamera
