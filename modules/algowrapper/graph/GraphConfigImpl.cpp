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

#define LOG_TAG "GraphConfigImpl"

#include "modules/algowrapper/graph/GraphConfigImpl.h"

#include <GCSSParser.h>
#include <graph_query_manager.h>

#include <algorithm>
#include <unordered_map>

#include "FormatUtils.h"
#include "GraphUtils.h"
#include "iutils/CameraLog.h"

using GCSS::GCSSParser;
using GCSS::GraphConfigNode;
using GCSS::GraphQueryManager;
using GCSS::ItemUID;
using std::map;
using std::shared_ptr;
using std::string;
using std::vector;

namespace icamera {

#define DISPERSED_MAX_OUTPUTS 2

Mutex GraphConfigImpl::sLock;
std::unordered_map<int32_t, GraphConfigNodes*> GraphConfigImpl::mGraphNode;

GraphConfigNodes::GraphConfigNodes() : mDesc(nullptr), mSettings(nullptr) {}

GraphConfigNodes::~GraphConfigNodes() {
    delete mDesc;
    delete mSettings;
}

GraphConfigImpl::GraphConfigImpl()
        : mCameraId(-1),
          mGraphQueryManager(nullptr),
          mConfigMode(CAMERA_STREAM_CONFIGURATION_MODE_NORMAL),
          mType(COUPLED),
          mMcId(-1) {}

GraphConfigImpl::GraphConfigImpl(int32_t camId, ConfigMode mode, GraphSettingType type)
        : mCameraId(camId),
          mGraphQueryManager(new GraphQueryManager()),
          mConfigMode(mode),
          mType(type),
          mMcId(-1) {
    AutoMutex lock(sLock);

    GraphConfigNodes* nodes = nullptr;
    if (mGraphNode.find(camId) != mGraphNode.end()) {
        nodes = mGraphNode[camId];
    }
    CheckError(!nodes, VOID_VALUE, "Failed to allocate Graph Query Manager");
    mGraphQueryManager->setGraphDescriptor(nodes->mDesc);
    mGraphQueryManager->setGraphSettings(nodes->mSettings);
}

GraphConfigImpl::~GraphConfigImpl() {}

/**
 * Add predefined keys to the map used by the graph config parser.
 *
 * This method should only be called once.
 *
 * We do this so that the keys we will use in the queries are already defined
 * and we can create the query objects in a more compact way, by using the
 * ItemUID initializers.
 */
void GraphConfigImpl::addCustomKeyMap() {
    /**
     * Initialize the map with custom specific tags found in the
     * Graph Config XML's
     */
#define GCSS_KEY(key, str) std::make_pair(#str, GCSS_KEY_##key),
    map<string, ia_uid> CUSTOM_GRAPH_KEYS = {
    #include "custom_gcss_keys.h"
    };
#undef GCSS_KEY

    LOG1("Adding %zu custom specific keys to graph config parser", CUSTOM_GRAPH_KEYS.size());

    /*
     * add custom specific tags so parser can use them
     */
    ItemUID::addCustomKeyMap(CUSTOM_GRAPH_KEYS);
}

/**
 * Method to parse the XML graph configurations and settings
 *
 * Provide the file name with absolute path to this method, and
 * save the GraphConfigNodes pointer to static area.
 * This method is currently called once per camera
 *
 * \param[in] graphDescFile: name of the graph descriptor file
 * \param[in] settingsFile: name of the graph settings file
 */
status_t GraphConfigImpl::parse(int cameraId, const char* graphDescFile, const char* settingsFile) {
    HAL_TRACE_CALL(CAMERA_DEBUG_LOG_LEVEL1);

    GCSSParser parser;
    GraphConfigNodes* nodes = new GraphConfigNodes;

    parser.parseGCSSXmlFile(graphDescFile, &nodes->mDesc);
    if (!nodes->mDesc) {
        LOGE("Failed to parse graph descriptor from %s", graphDescFile);
        delete nodes;
        return UNKNOWN_ERROR;
    }

    parser.parseGCSSXmlFile(settingsFile, &nodes->mSettings);
    if (!nodes->mSettings) {
        LOGE("Failed to parse graph settings from %s", settingsFile);
        delete nodes;
        return UNKNOWN_ERROR;
    }

    AutoMutex lock(sLock);
    // Destory the old item
    auto it = mGraphNode.find(cameraId);
    if (it != mGraphNode.end()) {
        delete it->second;
        mGraphNode.erase(it);
    }
    mGraphNode[cameraId] = nodes;

    return OK;
}

/**
 * Method to parse the XML graph configurations and settings
 *
 * Provide the memory address and size of files to this method,
 * and save the GraphConfigNodes pointer to static area.
 * This method is currently called once per camera
 *
 * \param[in] graphDescData: the memory address for graph descriptor
 * \param[in] descDataSize: the memory size for graph descriptor
 * \param[in] settingsData: the memory address for graph settings
 * \param[in] settingsDataSize: the memory size for graph settings
 */
status_t GraphConfigImpl::parse(int cameraId, char* graphDescData, size_t descDataSize,
                                char* settingsData, size_t settingsDataSize) {
    HAL_TRACE_CALL(CAMERA_DEBUG_LOG_LEVEL1);

    GCSSParser parser;
    GraphConfigNodes* nodes = new GraphConfigNodes;

    parser.parseGCSSXmlData(graphDescData, descDataSize, &nodes->mDesc);
    if (!nodes->mDesc) {
        LOGE("Failed to parse graph descriptor addr: %p, size: %zu", graphDescData, descDataSize);
        delete nodes;
        return UNKNOWN_ERROR;
    }

    parser.parseGCSSXmlData(settingsData, settingsDataSize, &nodes->mSettings);
    if (!nodes->mSettings) {
        LOGE("Failed to parse graph settings addr: %p, size: %zu", settingsData, settingsDataSize);
        delete nodes;
        return UNKNOWN_ERROR;
    }

    AutoMutex lock(sLock);
    // Destory the old item
    auto it = mGraphNode.find(cameraId);
    if (it != mGraphNode.end()) {
        delete it->second;
        mGraphNode.erase(it);
    }
    mGraphNode[cameraId] = nodes;

    return OK;
}

/*
 * Release the graph config nodes of all cameras
 */
void GraphConfigImpl::releaseGraphNodes() {
    AutoMutex lock(sLock);

    for (auto& nodes : mGraphNode) {
        delete nodes.second;
    }
    mGraphNode.clear();
}

/**
 * Create the query rule for current stream configuration based
 * on stream list and graph setting type.
 */
status_t GraphConfigImpl::createQueryRule(const vector<HalStream*>& activeStreams) {
    mQuery.clear();
    mStreamToSinkIdMap.clear();

    int videoIndex = 0, stillIndex = 0;
    map<GCSS::ItemUID, std::string> videoQuery;
    map<GCSS::ItemUID, std::string> stillQuery;
    map<HalStream*, uid_t> videoStreamToSinkIdMap;
    map<HalStream*, uid_t> stillStreamToSinkIdMap;
    vector<AndroidGraphConfigKey> videoStreamKeys = {GCSS_KEY_VIDEO0, GCSS_KEY_VIDEO1,
                                                     GCSS_KEY_VIDEO2};
    vector<AndroidGraphConfigKey> stillStreamKeys = {GCSS_KEY_STILL0, GCSS_KEY_STILL1,
                                                     GCSS_KEY_STILL2};

    // Depends on outputs numbers in GC settings
    int vOutputNum = (mType == DISPERSED) ? DISPERSED_MAX_OUTPUTS : videoStreamKeys.size();
    int sOutputNum = (mType == DISPERSED) ? DISPERSED_MAX_OUTPUTS : stillStreamKeys.size();
    for (auto& stream : activeStreams) {
        CheckError(stream->useCase() == USE_CASE_INPUT, UNKNOWN_ERROR,
                   "Error: Re-processing not supported with graph config yet.");
        /*
         * According to the usage to create the query item
         */
        CheckError(videoIndex >= vOutputNum && stillIndex >= sOutputNum, UNKNOWN_ERROR,
                   "%s: no output for new stream! video %d, still %d", __func__, videoIndex,
                   stillIndex);
        bool isVideo = isVideoStream(stream) ? true : false;
        if (videoIndex < vOutputNum) {
            isVideo = (isVideo || stillIndex >= sOutputNum);
        } else {
            isVideo = false;
        }
        AndroidGraphConfigKey key =
            (isVideo) ? videoStreamKeys[videoIndex++] : stillStreamKeys[stillIndex++];
        map<HalStream*, uid_t>& streamToSinkId =
            isVideo ? videoStreamToSinkIdMap : stillStreamToSinkIdMap;
        map<GCSS::ItemUID, std::string>& query = isVideo ? videoQuery : stillQuery;
        ItemUID w = {key, GCSS_KEY_WIDTH};
        ItemUID h = {key, GCSS_KEY_HEIGHT};

        query[w] = std::to_string(stream->width());
        query[h] = std::to_string(stream->height());
        streamToSinkId[stream] = key;
        LOG1("Adding stream %p to map %s", stream, ItemUID::key2str(key));
    }

    if (mType == COUPLED) {
        LOG2("Merge the query rule if graph settings type is COUPLED");
        /*
         * In this case(graph settings type is COUPLED), we need to merge still and
         * video together, so there is only one item in mQuery and mStreamToSinkIdMap
         * and it is used for both still and video pipe
         */

        // Merge the query rule
        for (auto& still : stillQuery) {
            videoQuery.insert(still);
        }

        // Merge the stream to sink key map
        for (auto& stillKey : stillStreamToSinkIdMap) {
            videoStreamToSinkIdMap.insert(stillKey);
        }

        /*
         * Add to the query the number of active outputs
         * The number of active outputs is video + still
         */
        ItemUID streamCount = {GCSS_KEY_ACTIVE_OUTPUTS};
        videoQuery[streamCount] = std::to_string(videoStreamToSinkIdMap.size());

        int useCase = videoQuery.empty() ? 0 : USE_CASE_VIDEO;
        useCase |= stillQuery.empty() ? 0 : USE_CASE_STILL_CAPTURE;
        mQuery[useCase] = videoQuery;
        dumpQuery(useCase, mQuery[useCase]);
        mStreamToSinkIdMap[useCase] = videoStreamToSinkIdMap;
    } else {
        LOG2("Fill each query rule if graph settings type is DISPERSED");
        /*
         * In this case(graph settings type is DISPERSED), the query item for still
         * and video is dispersed, so there are two items in mQuery and mStreamToSinkIdMap
         * one is for still pipe and the other is for video pipe
         */
        ItemUID streamCount = {GCSS_KEY_ACTIVE_OUTPUTS};

        // Add active outputs for video
        if (!videoStreamToSinkIdMap.empty()) {
            LOG2("The video output number: %zu", videoStreamToSinkIdMap.size());
            videoQuery[streamCount] = std::to_string(videoStreamToSinkIdMap.size());
            mQuery[USE_CASE_VIDEO] = videoQuery;
            dumpQuery(USE_CASE_VIDEO, mQuery[USE_CASE_VIDEO]);
            mStreamToSinkIdMap[USE_CASE_VIDEO] = videoStreamToSinkIdMap;
        }

        // Add active outputs for still
        if (!stillStreamToSinkIdMap.empty()) {
            LOG2("The still output number: %zu", stillStreamToSinkIdMap.size());
            stillQuery[streamCount] = std::to_string(stillStreamToSinkIdMap.size());
            mQuery[USE_CASE_STILL_CAPTURE] = stillQuery;
            dumpQuery(USE_CASE_STILL_CAPTURE, mQuery[USE_CASE_STILL_CAPTURE]);
            mStreamToSinkIdMap[USE_CASE_STILL_CAPTURE] = stillStreamToSinkIdMap;
        }
    }

    return OK;
}

status_t GraphConfigImpl::getRawInputSize(GCSS::IGraphConfig* query, camera_resolution_t* reso) {
    CheckError(!reso, UNKNOWN_ERROR, "%s, The reso is nullptr", __func__);
    GCSS::IGraphConfig* result = nullptr;
    css_err_t ret = mGraphQueryManager->createGraph(query, &result);
    if (ret != css_err_none) {
        delete result;
        return UNKNOWN_ERROR;
    }
    CheckError(!result, UNKNOWN_ERROR, "%s, Failed to create the graph", __func__);

    vector<string> isysOutput = {"csi_be:output",
                                 "csi_be_soc:output"};
    for (auto& item : isysOutput) {
        GCSS::IGraphConfig* isysNode = result->getDescendantByString(item.c_str());
        if (isysNode != nullptr) {
            GCSS::GraphCameraUtil::getDimensions(isysNode, &(reso->width), &(reso->height));
            return OK;
        }
    }

    LOGE("Error: Couldn't get the resolution in isys output");
    return UNKNOWN_ERROR;
}

/*
 * According to the stream list to query graph setting and create GraphConfigPipe
 */
status_t GraphConfigImpl::configStreams(const vector<HalStream*>& activeStreams) {
    HAL_TRACE_CALL(CAMERA_DEBUG_LOG_LEVEL1);

    status_t ret = createQueryRule(activeStreams);
    CheckError(ret != OK, ret, "Failed to create the query rule");

    mQueryResult.clear();
    map<int, vector<GCSS::IGraphConfig*> > useCaseToQueryResults;

    LOG2("The mQuery size: %zu", mQuery.size());
    for (auto& query : mQuery) {
        mFirstQueryResults.clear();
        dumpQuery(query.first, query.second);
        mGraphQueryManager->queryGraphs(query.second, mFirstQueryResults);
        CheckError(mFirstQueryResults.empty(), BAD_VALUE,
                   "Failed to query the result, please check the settings xml");

        // select setting from multiple results
        ret = selectSetting(query.first, &useCaseToQueryResults);
        CheckError(ret != OK, BAD_VALUE,
                   "Failed to select the settings for ConfigMode (0x%x)in results", mConfigMode);
    }

    CheckError(useCaseToQueryResults.empty(), UNKNOWN_ERROR,
               "%s, There isn't matched result after filtering with first query rule", __func__);
    // Filter the results with same isys output if there are
    // multiple items in useCaseToQueryResults map
    if (useCaseToQueryResults.size() > 1) {
        ret = OK;
        bool matchFound = false;

        vector<GCSS::IGraphConfig*>& videoQueryResults = useCaseToQueryResults.at(USE_CASE_VIDEO);
        vector<GCSS::IGraphConfig*>& stillQueryResults =
            useCaseToQueryResults.at(USE_CASE_STILL_CAPTURE);
        CheckError(videoQueryResults.empty() || stillQueryResults.empty(), UNKNOWN_ERROR,
                   "%s, the still or video query results is empty", __func__);

        // Filter the video and still query results with same isys ouput resolution.
        for (auto& video : videoQueryResults) {
            camera_resolution_t videoReso;
            ret = getRawInputSize(video, &videoReso);
            CheckError(ret != OK, UNKNOWN_ERROR,
                       "%s, Failed to get csi ouput resolution for video pipe", __func__);
            LOG2("Isys output resolution of video pipe: %dx%d", videoReso.width, videoReso.height);

            for (auto& still : stillQueryResults) {
                camera_resolution_t stillReso;
                ret = getRawInputSize(still, &stillReso);
                CheckError(ret != OK, UNKNOWN_ERROR,
                           "%s, Failed to get csi ouput resolution for still pipe", __func__);
                LOG2("Isys output resolution for still pipe: %dx%d", stillReso.width,
                     stillReso.height);

                if (videoReso.width == stillReso.width && videoReso.height == stillReso.height) {
                    matchFound = true;
                    mQueryResult[USE_CASE_VIDEO] = video;
                    mQueryResult[USE_CASE_STILL_CAPTURE] = still;
                    break;
                }
            }
            if (matchFound) break;
        }

        CheckError(!matchFound, UNKNOWN_ERROR,
                   "%s, Failed to find the isys ouput for video and still pipe", __func__);
    } else {
        // Use the query result with smallest isys output if there is only video pipe
        int resultIdx = 0;
        if (useCaseToQueryResults.begin()->first == USE_CASE_VIDEO) {
            camera_resolution_t resultReso;
            getRawInputSize((useCaseToQueryResults.begin()->second)[0], &resultReso);
            for (size_t idx = 0; idx < (useCaseToQueryResults.begin()->second).size(); idx++) {
                camera_resolution_t reso;
                getRawInputSize((useCaseToQueryResults.begin()->second)[idx], &reso);
                if (resultReso.width > reso.width && resultReso.height > reso.height) {
                    resultIdx = idx;
                    resultReso = reso;
                }
            }
        }
        mQueryResult[useCaseToQueryResults.begin()->first] =
            useCaseToQueryResults.begin()->second[resultIdx];
    }
    CheckError(mQueryResult.empty(), UNKNOWN_ERROR, "%s, Failed to fill the map into mQueryResult",
               __func__);

    int key = -1;
    string mcId, opMode;
    mQueryResult.begin()->second->getValue(GCSS_KEY_KEY, key);
    mQueryResult.begin()->second->getValue(GCSS_KEY_MC_ID, mcId);
    mQueryResult.begin()->second->getValue(GCSS_KEY_OP_MODE, opMode);
    LOG1("CAM[%d]Graph config for pipe: %d SUCCESS, settings id %d, operation mode: %s", mCameraId,
         mQueryResult.begin()->first, key, opMode.c_str());

    if (mQueryResult.size() > 1) {
        mQueryResult.rbegin()->second->getValue(GCSS_KEY_KEY, key);
        mQueryResult.rbegin()->second->getValue(GCSS_KEY_MC_ID, mcId);
        mQueryResult.rbegin()->second->getValue(GCSS_KEY_OP_MODE, opMode);
        LOG1("CAM[%d]Graph config for pipe: %d SUCCESS, settings id %d, operation mode: %s",
             mCameraId, mQueryResult.rbegin()->first, key, opMode.c_str());
    }
    mMcId = mcId.empty() ? -1 : stoi(mcId);
    ret = prepareGraphConfig();
    CheckError(ret != OK, ret, "%s, Failed to prepare graph config: real ConfigMode: %x", __func__,
               mConfigMode);

    return OK;
}

/**
 * Prepare graph config object
 *
 * Use graph query results as a parameter to getGraph. The result will be given
 * to graph config object.
 */
status_t GraphConfigImpl::prepareGraphConfig() {
    status_t status = OK;
    mGraphConfigPipe.clear();

    for (auto& query : mQueryResult) {
        shared_ptr<GraphConfigPipe> graphConfigPipe =
            std::make_shared<GraphConfigPipe>(query.first);
        GCSS::IGraphConfig* result = nullptr;
        css_err_t ret = mGraphQueryManager->createGraph(query.second, &result);
        if (ret != css_err_none) {
            delete result;
            return UNKNOWN_ERROR;
        }
        status = graphConfigPipe->prepare(static_cast<GraphConfigNode*>(result),
                                          mStreamToSinkIdMap[query.first]);
        if (status != OK) {
            delete result;
            LOGE("Failed to prepare the GraphConfigPipe for pipe: %d", query.first);
            return UNKNOWN_ERROR;
        }
        mGraphConfigPipe[query.first] = graphConfigPipe;
        LOG1("Graph config object prepared");
    }

    return OK;
}

/*
 * Do the secondary filter: configMode and stream format.
 */
status_t GraphConfigImpl::selectSetting(
    int useCase, std::map<int, std::vector<GCSS::IGraphConfig*> >* queryResults) {
    CheckError(!queryResults, UNKNOWN_ERROR, "%s, The queryResults is nullptr", __func__);
    string opMode;
    vector<GCSS::IGraphConfig*> internalQueryResults;

    // Firstly, filter the config mode
    for (auto& result : mFirstQueryResults) {
        vector<ConfigMode> cfgModes;
        result->getValue(GCSS_KEY_OP_MODE, opMode);
        LOG1("The operation mode str in xml: %s", opMode.c_str());

        CameraUtils::getConfigModeFromString(opMode, cfgModes);
        LOG1("The query results supports configModes size: %zu", cfgModes.size());

        for (const auto mode : cfgModes) {
            if (mConfigMode == mode) {
                internalQueryResults.push_back(result);
                break;
            }
        }
    }
    CheckError(internalQueryResults.size() == 0, UNKNOWN_ERROR,
               "Failed to query the results for configMode: %d", mConfigMode);

    /*
     * May still have multiple graphs after config mode parsing
     * Those graphs have same resolution/configMode, but different output formats
     * Do second graph query with format/bpp as query condition
     */
    map<HalStream*, uid_t>& streamToSinkIdMap = mStreamToSinkIdMap[useCase];
    vector<GCSS::IGraphConfig*> secondQueryResults;
    if (internalQueryResults.size() > 1) {
        map<GCSS::ItemUID, std::string> queryItem;
        for (auto const& item : streamToSinkIdMap) {
            HalStream* s = item.first;
            ItemUID formatKey = {(ia_uid)item.second, GCSS_KEY_FORMAT};
            string fmt = graphconfig::utils::format2string(s->format());
            queryItem[formatKey] = fmt;
        }

        LOG1("dumpQuery with format condition");
        dumpQuery(useCase, queryItem);

        /*
         * Note: In some projects, there isn't format item in graph setting,
         * So the result of this query may be empty, and ignore it.
         */
        mGraphQueryManager->queryGraphs(queryItem, internalQueryResults, secondQueryResults);
        LOG2("The query results size: %zu after filtering format", secondQueryResults.size());
    }

    // Firstly, select the results with fully filtering
    if (secondQueryResults.size() > 0) {
        (*queryResults)[useCase] = secondQueryResults;
    } else {
        (*queryResults)[useCase] = internalQueryResults;
    }

    return OK;
}

status_t GraphConfigImpl::getGraphConfigData(IGraphType::GraphConfigData* data) {
    // The graph id, csi output and sensor mode must be same if there are two graph config pipes
    data->graphId = mGraphConfigPipe.begin()->second->getGraphId();
    mGraphConfigPipe.begin()->second->getCSIOutputResolution(&(data->csiReso));

    data->mcId = mMcId;
    getGdcKernelSetting(&(data->gdcKernelId), &(data->gdcReso));

    int ret = getPgNames(&(data->pgNames));
    CheckError(ret != OK, UNKNOWN_ERROR, "%s, Failed to get pg names", __func__);
    for (auto& pgName : data->pgNames) {
        IGraphType::PgInfo info;
        info.pgName = pgName;
        info.streamId = getStreamIdByPgName(pgName);
        info.pgId = getPgIdByPgName(pgName);
        getPgRbmValue(pgName, &info.rbmValue);
        data->pgInfo.push_back(info);
    }

    ret = graphGetStreamIds(&(data->streamIds));
    CheckError(ret != OK, UNKNOWN_ERROR, "%s, Failed to get streamIds", __func__);
    for (auto& streamId : data->streamIds) {
        IGraphType::MbrInfo mBr;
        mBr.streamId = streamId;
        getMBRData(streamId, &mBr.data);
        data->mbrInfo.push_back(mBr);

        IGraphType::ProgramGroupInfo info;
        info.streamId = streamId;
        info.pgPtr = getProgramGroup(streamId);
        data->programGroup.push_back(info);
    }

    return OK;
}

status_t GraphConfigImpl::getGdcKernelSetting(uint32_t* kernelId,
                                              ia_isp_bxt_resolution_info_t* resolution) {
    CheckError(mGraphConfigPipe.empty(), UNKNOWN_ERROR, "%s, the mGraphConfigPipe is empty",
               __func__);
    CheckError(!kernelId || !resolution, UNKNOWN_ERROR, "%s, the kernelId or resolution is nullptr",
               __func__);

    int ret = OK;
    if (mGraphConfigPipe.size() == 1) {
        ret = mGraphConfigPipe.begin()->second->getGdcKernelSetting(kernelId, resolution);
    } else {
        // Get the information from video pipe firstly
        shared_ptr<GraphConfigPipe>& videoGraphPipe = mGraphConfigPipe.at(USE_CASE_VIDEO);
        ret = videoGraphPipe->getGdcKernelSetting(kernelId, resolution);
        if (ret != OK) {
            shared_ptr<GraphConfigPipe>& stillGraphPipe =
                mGraphConfigPipe.at(USE_CASE_STILL_CAPTURE);
            ret = stillGraphPipe->getGdcKernelSetting(kernelId, resolution);
        }
    }
    LOG2("%s, %s", __func__, ret != OK ? "No gdc resolution" : "Get gdc resolution successfully");

    return OK;
}

status_t GraphConfigImpl::graphGetStreamIds(std::vector<int32_t>* streamIds) {
    CheckError(mGraphConfigPipe.empty(), UNKNOWN_ERROR, "%s, the mGraphConfigPipe is empty",
               __func__);
    CheckError(!streamIds, UNKNOWN_ERROR, "%s, The streamIds is nullptr", __func__);

    if (mGraphConfigPipe.size() == 1) {
        mGraphConfigPipe.begin()->second->graphGetStreamIds(streamIds);
    } else {
        vector<int32_t> stillStreamIds;
        shared_ptr<GraphConfigPipe>& videoGraphPipe = mGraphConfigPipe.at(USE_CASE_VIDEO);
        shared_ptr<GraphConfigPipe>& stillGraphPipe = mGraphConfigPipe.at(USE_CASE_STILL_CAPTURE);

        videoGraphPipe->graphGetStreamIds(streamIds);
        stillGraphPipe->graphGetStreamIds(&stillStreamIds);

        for (auto& id : stillStreamIds) {
            if (std::find(streamIds->begin(), streamIds->end(), id) == streamIds->end())
                streamIds->push_back(id);
        }
    }

    CheckError(streamIds->empty(), UNKNOWN_ERROR, "%s, Failed to find any streamIds for all pipes",
               __func__);

    return OK;
}

int GraphConfigImpl::getStreamIdByPgName(std::string pgName) {
    CheckError(mGraphConfigPipe.empty(), -1, "%s, the mGraphConfigPipe is empty", __func__);

    int streamId = -1;
    for (auto& pipe : mGraphConfigPipe) {
        streamId = pipe.second->getStreamIdByPgName(pgName);
        if (streamId != -1) break;
    }
    return streamId;
}

int GraphConfigImpl::getPgIdByPgName(std::string pgName) {
    CheckError(mGraphConfigPipe.empty(), -1, "%s, the mGraphConfigPipe is empty", __func__);

    int pgId = -1;
    for (auto& pipe : mGraphConfigPipe) {
        pgId = pipe.second->getPgIdByPgName(pgName);
        if (pgId != -1) break;
    }
    return pgId;
}

ia_isp_bxt_program_group* GraphConfigImpl::getProgramGroup(int32_t streamId) {
    CheckError(mGraphConfigPipe.empty(), nullptr, "%s, the mGraphConfigPipe is empty", __func__);

    if (mGraphConfigPipe.size() == 1) {
        return mGraphConfigPipe.begin()->second->getProgramGroup(streamId);
    }

    // Find the streamd id from video graph pipe firstly
    vector<int32_t> streamIds;
    shared_ptr<GraphConfigPipe>& videoGraphPipe = mGraphConfigPipe.at(USE_CASE_VIDEO);
    videoGraphPipe->graphGetStreamIds(&streamIds);
    if (std::find(streamIds.begin(), streamIds.end(), streamId) != streamIds.end()) {
        return videoGraphPipe->getProgramGroup(streamId);
    }

    shared_ptr<GraphConfigPipe>& stillGraphPipe = mGraphConfigPipe.at(USE_CASE_STILL_CAPTURE);
    return stillGraphPipe->getProgramGroup(streamId);
}

int GraphConfigImpl::getProgramGroup(std::string pgName,
                                     ia_isp_bxt_program_group* programGroupForPG) {
    for (auto& graph : mGraphConfigPipe) {
        vector<string> pgNames;
        graph.second->getPgNames(&pgNames);
        if (std::find(pgNames.begin(), pgNames.end(), pgName) != pgNames.end()) {
            return graph.second->getProgramGroup(pgName, programGroupForPG);
        }
    }

    LOGE("There isn't this pg: %s in all graph config pipes", pgName.c_str());
    return UNKNOWN_ERROR;
}

status_t GraphConfigImpl::getMBRData(int32_t streamId, ia_isp_bxt_gdc_limits* data) {
    CheckError(mGraphConfigPipe.empty(), UNKNOWN_ERROR, "%s, the mGraphConfigPipe is empty",
               __func__);

    if (mGraphConfigPipe.size() == 1) {
        return mGraphConfigPipe.begin()->second->getMBRData(streamId, data);
    }

    // Find the streamd id from video graph pipe firstly
    vector<int32_t> streamIds;
    shared_ptr<GraphConfigPipe>& videoGraphPipe = mGraphConfigPipe.at(USE_CASE_VIDEO);
    videoGraphPipe->graphGetStreamIds(&streamIds);
    if (std::find(streamIds.begin(), streamIds.end(), streamId) != streamIds.end()) {
        return videoGraphPipe->getMBRData(streamId, data);
    }

    shared_ptr<GraphConfigPipe>& stillGraphPipe = mGraphConfigPipe.at(USE_CASE_STILL_CAPTURE);
    return stillGraphPipe->getMBRData(streamId, data);
}

status_t GraphConfigImpl::getPgNames(std::vector<std::string>* pgNames) {
    CheckError(mGraphConfigPipe.empty(), UNKNOWN_ERROR, "%s, the mGraphConfigPipe is empty",
               __func__);

    if (mGraphConfigPipe.size() == 1) {
        mGraphConfigPipe.begin()->second->getPgNames(pgNames);
    } else {
        vector<string> stillPgNames;
        shared_ptr<GraphConfigPipe>& videoGraphPipe = mGraphConfigPipe.at(USE_CASE_VIDEO);
        shared_ptr<GraphConfigPipe>& stillGraphPipe = mGraphConfigPipe.at(USE_CASE_STILL_CAPTURE);

        videoGraphPipe->getPgNames(pgNames);
        stillGraphPipe->getPgNames(&stillPgNames);

        for (auto& pg : stillPgNames) {
            if (std::find(pgNames->begin(), pgNames->end(), pg.c_str()) == pgNames->end())
                pgNames->push_back(pg);
        }
    }

    CheckError(pgNames->empty(), UNKNOWN_ERROR, "%s, Failed to get the PG's name for all pipes",
               __func__);

    return OK;
}

status_t GraphConfigImpl::pipelineGetConnections(
    const std::vector<std::string>& pgList, std::vector<IGraphType::ScalerInfo>* scalerInfo,
    std::vector<IGraphType::PipelineConnection>* confVector) {
    CheckError(!confVector, UNKNOWN_ERROR, "%s, the confVector is nullptr", __func__);
    CheckError(mGraphConfigPipe.empty(), UNKNOWN_ERROR, "%s, the mGraphConfigPipe is empty",
               __func__);

    if (mGraphConfigPipe.size() == 1) {
        return mGraphConfigPipe.begin()->second->pipelineGetConnections(pgList, scalerInfo,
                                                                        confVector);
    }

    vector<IGraphType::PipelineConnection> stillConnVector, videoConnVector;
    shared_ptr<GraphConfigPipe>& videoGraphPipe = mGraphConfigPipe.at(USE_CASE_VIDEO);
    shared_ptr<GraphConfigPipe>& stillGraphPipe = mGraphConfigPipe.at(USE_CASE_STILL_CAPTURE);

    std::vector<IGraphType::ScalerInfo> stillScalerInfo, videoScalerInfo;
    int ret = videoGraphPipe->pipelineGetConnections(pgList, &videoScalerInfo, &videoConnVector);
    CheckError(ret != OK, UNKNOWN_ERROR, "Failed to get the connetction from video pipe");
    ret = stillGraphPipe->pipelineGetConnections(pgList, &stillScalerInfo, &stillConnVector);
    CheckError(ret != OK, UNKNOWN_ERROR, "Failed to get the connetction from still pipe");

    LOG2("The connetction in video: %zu, in still: %zu; the scalera in video: %zu, in still: %uz",
         videoConnVector.size(), stillConnVector.size(), videoScalerInfo.size(),
         stillScalerInfo.size());

    if (!stillScalerInfo.empty()) {
        for (auto& stillScaler : stillScalerInfo) {
            videoScalerInfo.push_back(stillScaler);
        }
    }
    *scalerInfo = videoScalerInfo;

    if (videoConnVector.empty()) {
        videoConnVector = stillConnVector;
    } else {
        if (stillConnVector.size() > 0) {
            LOG1("Need to merge the two connetction vector: %zu", stillConnVector.size());
        }
        for (auto& stillConn : stillConnVector) {
            bool sameTerminalId = false;
            for (auto& conn : videoConnVector) {
                if (conn.portFormatSettings.terminalId == stillConn.portFormatSettings.terminalId) {
                    sameTerminalId = true;
                    if (conn.portFormatSettings.enabled == 0 &&
                        stillConn.portFormatSettings.enabled == 1)
                        conn = stillConn;
                    break;
                }
            }
            if (!sameTerminalId) videoConnVector.push_back(stillConn);
        }
    }
    CheckError(videoConnVector.empty(), UNKNOWN_ERROR,
               "%s, Failed to get connetctions from graph config pipe", __func__);

    LOG2("dump the final connetction");
    GraphUtils::dumpConnections(videoConnVector);
    *confVector = videoConnVector;

    return OK;
}

status_t GraphConfigImpl::getPgIdForKernel(const uint32_t streamId, const int32_t kernelId,
                                           int32_t* pgId) {
    CheckError(!pgId, UNKNOWN_ERROR, "%s, the pgId is nullptr", __func__);
    CheckError(mGraphConfigPipe.empty(), UNKNOWN_ERROR, "%s, the mGraphConfigPipe is empty",
               __func__);

    if (mGraphConfigPipe.size() == 1) {
        return mGraphConfigPipe.begin()->second->getPgIdForKernel(streamId, kernelId, pgId);
    }

    vector<int32_t> streamIds;
    shared_ptr<GraphConfigPipe>& videoGraphPipe = mGraphConfigPipe.at(USE_CASE_VIDEO);
    videoGraphPipe->graphGetStreamIds(&streamIds);
    if (std::find(streamIds.begin(), streamIds.end(), streamId) != streamIds.end())
        return videoGraphPipe->getPgIdForKernel(streamId, kernelId, pgId);

    shared_ptr<GraphConfigPipe>& stillGraphPipe = mGraphConfigPipe.at(USE_CASE_STILL_CAPTURE);
    return stillGraphPipe->getPgIdForKernel(streamId, kernelId, pgId);
}

status_t GraphConfigImpl::getPgRbmValue(string pgName, IGraphType::StageAttr* stageAttr) {
    for (auto& graph : mGraphConfigPipe) {
        vector<string> pgNames;
        graph.second->getPgNames(&pgNames);
        if (std::find(pgNames.begin(), pgNames.end(), pgName) != pgNames.end()) {
            return graph.second->getPgRbmValue(pgName, stageAttr);
        }
    }

    LOGE("There isn't this pg: %s in all graph config pipes", pgName.c_str());
    return UNKNOWN_ERROR;
}

/******************************************************************************
 *  HELPER METHODS
 ******************************************************************************/
/**
 * Check the gralloc hint flags and decide whether this stream should be served
 * by Video Pipe or Still Pipe
 */
bool GraphConfigImpl::isVideoStream(HalStream* stream) {
    if (stream->useCase() == USE_CASE_PREVIEW || stream->useCase() == USE_CASE_VIDEO) return true;

    return false;
}

void GraphConfigImpl::dumpQuery(int useCase, const map<GCSS::ItemUID, std::string>& query) {
    map<GCSS::ItemUID, std::string>::const_iterator it;
    it = query.begin();
    LOG1("Query Dump --- %d --- Start", useCase);
    for (; it != query.end(); ++it) {
        LOG1("item: %s value %s", it->first.toString().c_str(), it->second.c_str());
    }
    LOG1("Query Dump --- %d --- End", useCase);
}
}  // namespace icamera
