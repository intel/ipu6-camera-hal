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

#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "HalStream.h"
#include "iutils/Errors.h"
#include "iutils/Thread.h"
#include "iutils/Utils.h"
#include "modules/algowrapper/graph/GraphConfigPipe.h"

namespace icamera {

/**
 * \enum AndroidGraphConfigKey
 * List of keys that are Android Specific used in queries of settings by
 * the GraphConfigImpl.
 *
 * The enum should not overlap with the enum of tags already predefined by the
 * parser, hence the initial offset.
 */
#define GCSS_KEY(key, str) GCSS_KEY_##key,
enum AndroidGraphConfigKey {
    GCSS_ANDROID_KEY_START = GCSS_KEY_START_CUSTOM_KEYS,
    #include "custom_gcss_keys.h"
};
#undef GCSS_KEY

/**
 * Static data for graph settings for given sensor. Used to initialize GraphConfigImpl.
 */
class GraphConfigNodes {
 public:
    GraphConfigNodes();
    ~GraphConfigNodes();

 public:
    GCSS::IGraphConfig* mDesc;
    GCSS::IGraphConfig* mSettings;

 private:
    // Disable copy constructor and assignment operator
    DISALLOW_COPY_AND_ASSIGN(GraphConfigNodes);
};

/**
 * \class GraphConfigImpl
 *
 * Class to wrap over parsing and executing queries on graph settings.
 * It creates the query rule based on graph settings type and stream list
 * from user. And filter the final result for current stream config.
 * It supports COUPLED and DISPERSED
 *
 * GraphConfigImpl maintains a GraphConfigPipe map table, each item for one
 * pipe. And it provides the public APIs to get the useful information which
 * builds PSYS pipe.
 *
 * At camera open, GraphConfigImpl object is created.
 * In stream config period, GraphConfigImpl creates the GraphConfigPipe map which is
 * current stream configuration. And HAL can get all the necessary information to
 * create PSYS pipe.
 */
class GraphConfigImpl {
 public:
    GraphConfigImpl();
    GraphConfigImpl(int32_t camId, ConfigMode mode, GraphSettingType type);
    virtual ~GraphConfigImpl();

    /*
     * Methods for XML parsing for XML parsing
     */
    void addCustomKeyMap();
    status_t parse(int cameraId, const char* graphDescFile, const char* settingsFile);
    status_t parse(int cameraId, char* graphDescData, size_t descDataSize, char* settingsData,
                   size_t settingsDataSize);
    void releaseGraphNodes();

    // These public methods called by GraphConfig
    status_t configStreams(const std::vector<HalStream*>& activeStreams);
    status_t getGraphConfigData(IGraphType::GraphConfigData* data);

    int getProgramGroup(std::string pgName, ia_isp_bxt_program_group* programGroupForPG);
    status_t getPgIdForKernel(const uint32_t streamId, const int32_t kernelId, int32_t* pgId);

    status_t pipelineGetConnections(const std::vector<std::string>& pgList,
                                    std::vector<IGraphType::ScalerInfo>* scalerInfo,
                                    std::vector<IGraphType::PipelineConnection>* confVector);

 private:
    status_t prepareGraphConfig();
    bool isVideoStream(HalStream* stream);
    status_t selectSetting(int useCase,
                           std::map<int, std::vector<GCSS::IGraphConfig*> >* queryResults);
    status_t createQueryRule(const std::vector<HalStream*>& activeStreams);
    status_t getRawInputSize(GCSS::IGraphConfig* query, camera_resolution_t* reso);

    status_t getGdcKernelSetting(uint32_t* kernelId, ia_isp_bxt_resolution_info_t* resolution);
    status_t graphGetStreamIds(std::vector<int32_t>* streamIds);
    int getStreamIdByPgName(std::string pgName);
    int getPgIdByPgName(std::string pgName);
    ia_isp_bxt_program_group* getProgramGroup(int32_t streamId);
    status_t getPgRbmValue(std::string pgName, IGraphType::StageAttr* stageAttr);
    status_t getMBRData(int32_t streamId, ia_isp_bxt_gdc_limits* data);
    status_t getPgNames(std::vector<std::string>* pgNames);

    // Debug helper
    void dumpQuery(int useCase, const std::map<GCSS::ItemUID, std::string>& query);

 private:
    static Mutex sLock;
    static std::unordered_map<int32_t, GraphConfigNodes*> mGraphNode;
    /**
     * Pair of ItemUIDs to store the width and height of a stream
     * first item is for width, second for height
     */
    typedef std::pair<GCSS::ItemUID, GCSS::ItemUID> ResolutionItem;

    int32_t mCameraId;
    std::unique_ptr<GCSS::GraphQueryManager> mGraphQueryManager;
    /*
     * The query interface uses types that are actually STL maps and vectors
     * to avoid the creation/deletion on the stack for every call we
     * have them as member variables.
     * - The first item of mQuery is stream useCase(VIDEO or STILL),
     * - and the second is an query rule map(GCSS_KEY_, VALUE).
     */
    std::map<int, std::map<GCSS::ItemUID, std::string> > mQuery;

    /**
     * Map to get the virtual sink id from a client stream pointer.
     * The uid is one of the GCSS keys defined for the virtual sinks, like
     * GCSS_KEY_VIDEO0 or GCSS_KEY_STILL1
     * From that we can derive the name using the id to string methods from
     * ItemUID class
     *  - The first item is streams useCase(VIDEO or STILL)
     *  - and the second is the stream to virtual sink map
     */
    std::map<int, std::map<HalStream*, uid_t> > mStreamToSinkIdMap;

    /*
     * This vector is used to store the first query result.
     * After that we also need to filter the results with configMode,
     * stream format, and matched isys output resolution.
     */
    std::vector<GCSS::IGraphConfig*> mFirstQueryResults;

    // The stream useCase to result map
    std::map<int, GCSS::IGraphConfig*> mQueryResult;

    // The stream useCase to GraphConfigPipe map
    std::map<int, std::shared_ptr<GraphConfigPipe> > mGraphConfigPipe;

    ConfigMode mConfigMode;
    GraphSettingType mType;
    int mMcId;

    // Disable copy constructor and assignment operator
    DISALLOW_COPY_AND_ASSIGN(GraphConfigImpl);
};

}  // namespace icamera
