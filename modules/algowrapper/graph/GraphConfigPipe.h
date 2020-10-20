/*
 * Copyright (C) 2015-2020 Intel Corporation
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
 * Stream id associated with the ISA PG that runs on Psys.
 */
static const int32_t PSYS_ISA_STREAM_ID = 60002;

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

    void init(int32_t reqId);
    int getGraphId(void);
    void setActiveSinks(const std::vector<uid_t>& activeSinks);
    void setActiveStreamId(const std::vector<uid_t>& activeSinks);
    /*
     * Convert Node to GraphConfig interface
     */
    const GCSS::IGraphConfig* getInterface(Node* node) const;
    const GCSS::IGraphConfig* getInterface() const;
    ia_isp_bxt_program_group* getProgramGroup(int32_t streamId);
    int getProgramGroup(std::string pgName, ia_isp_bxt_program_group* programGroupForPG);
    status_t getGdcKernelSetting(uint32_t* kernelId, ia_isp_bxt_resolution_info_t* resolution);
    const ia_isp_bxt_resolution_info_t* getKernelResolutionInfo(uint32_t streamId,
                                                                uint32_t kernelId);
    bool hasStreamInGraph(int streamId);
    bool isKernelInStream(uint32_t streamId, uint32_t kernelId);
    status_t getPgIdForKernel(const uint32_t streamId, const int32_t kernelId, int32_t* pgId);
    int32_t getTuningMode(int32_t streamId);
    status_t getMBRData(int32_t streamId, ia_isp_bxt_gdc_limits* data);
    status_t prepare(Node* settings, const StreamToSinkMap& streamToSinkIdMap);
    bool containPgs(std::vector<std::string> pgNames);

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
    bool isPipeEdgePort(Node* port);  // TODO: should be renamed as portIsEdgePort

    bool getSensorEmbeddedMetadataEnabled() const { return mMetaEnabled; }
    /*
     * Pipeline connection support
     */
    status_t pipelineGetConnections(const std::string& sinkName, int* streamId,
                                    std::vector<IGraphType::PipelineConnection>* confVector);
    status_t portGetOwner(Node* port, IGraphType::ConnectionConfig* connectionInfo);
    status_t pipelineGetConnections(const std::vector<std::string>& pgList,
                                    std::vector<IGraphType::ScalerInfo>* scalerInfo,
                                    std::vector<IGraphType::PipelineConnection>* confVector);

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
     * re-cycler static method
     */
    static void reset(GraphConfigPipe* me);
    void fullReset();
    /*
     * Debugging support
     */
    void dumpSettings();
    void dumpKernels(int32_t streamId);
    std::string getNodeName(Node* node);
    void getCSIOutputResolution(camera_resolution_t* reso) { *reso = mCsiOutput; }

 private:
    struct ResolutionMemPool {
        std::vector<ia_isp_bxt_resolution_info_t*> resHistorys;
        std::vector<ia_isp_bxt_resolution_info_t*> resInfos;
    };
    /* Helper structures to access Sensor Node information easily */
    class Rectangle {
     public:
        Rectangle();
        int32_t w; /*<! width */
        int32_t h; /*<! height */
        int32_t t; /*<! top */
        int32_t l; /*<! left */
    };
    class SubdevPad : public Rectangle {
     public:
        SubdevPad();
        int32_t mbusFormat;
    };
    struct BinFactor {
        int32_t h;
        int32_t v;
    };
    struct ScaleFactor {
        int32_t num;
        int32_t denom;
    };
    union RcFactor {  // Resolution Changing factor
        BinFactor bin;
        ScaleFactor scale;
    };
    struct SubdevInfo {
        std::string name;
        SubdevPad in;
        SubdevPad out;
        RcFactor factor;
        SubdevInfo() { CLEAR(factor); }
    };
    class SourceNodeInfo {
     public:
        SourceNodeInfo();
        std::string name;
        std::string i2cAddress;
        std::string modeId;
        bool metadataEnabled;
        std::string csiPort;
        std::string nativeBayer;
        SubdevInfo tpg;
        SubdevInfo pa;
        SubdevInfo binner;
        SubdevInfo scaler;
        SubdevPad output;
        int32_t interlaced;
        std::string verticalFlip;
        std::string horizontalFlip;
        std::string link_freq;
    };
    status_t analyzeSourceType();
    status_t analyzeCSIOutput();
    void calculateSinkDependencies();
    void storeTuningModes();
    HalStream* getHalStreamByVirtualId(uid_t vPortId);

    // Format options methods
    status_t getActiveOutputPorts(const StreamToSinkMap& streamToSinkIdMap);
    Node* getOutputPortForSink(const std::string& sinkName);
    status_t getSinkFormatOptions();
    status_t handleDynamicOptions();
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

 private:
    GCSS::GraphConfigNode* mSettings;
    int32_t mReqId;
    std::map<int32_t, ia_isp_bxt_program_group> mProgramGroup;
    GCSS::BxtAicUtils mGCSSAicUtil;

    bool mMetaEnabled;  // indicates if the specific sensor provides sensor
                        // embedded metadata
    enum SourceType {
        SRC_NONE = 0,
        SRC_SENSOR,
        SRC_TPG,
    };
    SourceType mSourceType;
    camera_resolution_t mCsiOutput;
    std::string mSourcePortName;  // Sensor or TPG port name

    /**
     * pre-computed state done *per request*.
     * This map holds the terminal id's of the ISA's peer ports (this is
     * the terminal id's of the input port of the video or still pipe)
     * that are required to fulfill a request.
     * Ideally this gets initialized during init() call.
     * But for now the GcManager will set it via a private method.
     * we use a map so that we can handle the case when a request has 2 buffers
     * that are generated from the same pipe.
     */
    std::map<uid_t, uid_t> mIsaActiveDestinations;
    std::set<int32_t> mActiveStreamId;
    /**
     * vector holding one structure per virtual sink that stores the stream id
     * (pipeline id) associated with it and the terminal id of the input port
     * of that stream.
     * This vector is updated once per stream config.
     */
    std::vector<SinkDependency> mSinkDependencies;
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
    std::map<std::string, int32_t> mIsaOutputPort2StreamId;
    /**
     * Map of tuning modes per stream id
     * Key: stream id
     * Value: tuning mode
     */
    std::map<int32_t, int32_t> mStream2TuningMap;
    int mPipeUseCase;

    // Disable copy constructor and assignment operator
    DISALLOW_COPY_AND_ASSIGN(GraphConfigPipe);
};

}  // namespace icamera
