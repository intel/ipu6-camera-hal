/*
 * Copyright (C) 2015-2024 Intel Corporation
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

#define LOG_TAG GraphConfigImpl

#include "modules/algowrapper/graph/GraphConfigImpl.h"

#include <GCSSParser.h>
#include <graph_query_manager.h>

#include <algorithm>
#include <unordered_map>

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
          mConfigMode(mode),
          mType(type),
          mMcId(-1) {
    AutoMutex lock(sLock);

    GraphConfigNodes* nodes = nullptr;
    if (mGraphNode.find(camId) != mGraphNode.end()) {
        nodes = mGraphNode[camId];
    }
    CheckAndLogError(!nodes, VOID_VALUE, "Failed to allocate Graph Query Manager");

    mGraphQueryManager = std::unique_ptr<GCSS::GraphQueryManager>(new GraphQueryManager());
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

    LOG2("Adding %zu custom specific keys to graph config parser", CUSTOM_GRAPH_KEYS.size());

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

    {
        AutoMutex lock(sLock);
        auto it = mGraphNode.find(cameraId);
        if (it != mGraphNode.end()) {
            LOG2("<id%d>, The graph config has been parsed", cameraId);
            return OK;
        }
    }

    GCSSParser parser;
    GraphConfigNodes* nodes = new GraphConfigNodes;
    LOG2("<id%d>, Start to parse graph config file", cameraId);

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

    {
        AutoMutex lock(sLock);
        auto it = mGraphNode.find(cameraId);
        if (it != mGraphNode.end()) {
            LOG2("<id%d>, the graph config has been parsed", cameraId);
            return OK;
        }
    }

    GCSSParser parser;
    GraphConfigNodes* nodes = new GraphConfigNodes;
    LOG2("<id%d>, Start to parse graph config data", cameraId);

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
status_t GraphConfigImpl::createQueryRule(const vector<HalStream*>& activeStreams,
                                          bool dummyStillSink) {
    mQuery.clear();
    mStreamToSinkIdMap.clear();

    int videoIndex = 0, stillIndex = 0, stillTnrIndex = 0;
    map<GCSS::ItemUID, std::string> videoQuery;
    map<GCSS::ItemUID, std::string> stillQuery;
    map<HalStream*, uid_t> videoStreamToSinkIdMap;
    map<HalStream*, uid_t> stillStreamToSinkIdMap;
    vector<AndroidGraphConfigKey> videoStreamKeys = {GCSS_KEY_VIDEO0, GCSS_KEY_VIDEO1,
                                                     GCSS_KEY_VIDEO2};
    vector<AndroidGraphConfigKey> stillStreamKeys = {GCSS_KEY_STILL0, GCSS_KEY_STILL1,
                                                     GCSS_KEY_STILL2};
    vector<AndroidGraphConfigKey> stillTnrStreamKeys = {GCSS_KEY_STILLTNR0, GCSS_KEY_STILLTNR1,
                                                        GCSS_KEY_STILLTNR2};
    // Depends on outputs numbers in GC settings
    int vOutputNum = (mType == DISPERSED) ? DISPERSED_MAX_OUTPUTS : videoStreamKeys.size();
    int sOutputNum = (mType == DISPERSED) ? DISPERSED_MAX_OUTPUTS : stillStreamKeys.size();
    for (auto& stream : activeStreams) {
        CheckAndLogError(stream->useCase() == USE_CASE_INPUT, UNKNOWN_ERROR,
                         "Error: Re-processing not supported with graph config yet.");
        /*
         * According to the usage to create the query item
         */
        CheckAndLogError(videoIndex >= vOutputNum && stillIndex >= sOutputNum, UNKNOWN_ERROR,
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
        LOG2("Adding stream %p to map %s", stream, ItemUID::key2str(key));
        if (dummyStillSink && !isVideo) {
            AndroidGraphConfigKey key = stillTnrStreamKeys[stillTnrIndex++];
            ItemUID w = {key, GCSS_KEY_WIDTH};
            ItemUID h = {key, GCSS_KEY_HEIGHT};

            query[w] = std::to_string(stream->width());
            query[h] = std::to_string(stream->height());
            LOG2("Adding dummy stream %p to %s", stream, ItemUID::key2str(key));
        }
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
            if (dummyStillSink) {
                /* User will always requests 1 still stream still0. there will be a stilltnr0
                ** stream in graph setting when dummy sink enabled, and HAL will create 2 still
                ** pipe for common still capture and for tnr still capture
                */
                LOG2("The still output number: %zu", stillStreamToSinkIdMap.size() + 1);
                stillQuery[streamCount] = std::to_string(stillStreamToSinkIdMap.size() + 1);
            } else {
                LOG2("The still output number: %zu", stillStreamToSinkIdMap.size());
                stillQuery[streamCount] = std::to_string(stillStreamToSinkIdMap.size());
            }
            mQuery[USE_CASE_STILL_CAPTURE] = stillQuery;
            dumpQuery(USE_CASE_STILL_CAPTURE, mQuery[USE_CASE_STILL_CAPTURE]);
            mStreamToSinkIdMap[USE_CASE_STILL_CAPTURE] = stillStreamToSinkIdMap;
        }
    }

    return OK;
}

status_t GraphConfigImpl::getRawInputSize(GCSS::IGraphConfig* query, camera_resolution_t* reso) {
    CheckAndLogError(!reso, UNKNOWN_ERROR, "%s, The reso is nullptr", __func__);

    GCSS::IGraphConfig* result = nullptr;
    css_err_t ret = mGraphQueryManager->createGraph(query, &result);
    std::unique_ptr<GCSS::IGraphConfig> graphResult(result);

    CheckAndLogError(!graphResult || ret != css_err_none, UNKNOWN_ERROR,
                     "%s, Failed to create the graph", __func__);

    vector<string> isysOutput = {"csi_be:output",
                                 // DOL_FEATURE_S
                                 "csi_be_dol:output",
                                 // DOL_FEATURE_E
                                 "csi_be_soc:output"};
    for (auto& item : isysOutput) {
        GCSS::IGraphConfig* isysNode = graphResult->getDescendantByString(item.c_str());
        if (isysNode != nullptr) {
            GCSS::GraphCameraUtil::getDimensions(isysNode, &(reso->width), &(reso->height));
            return OK;
        }
    }

    LOGE("Error: Couldn't get the resolution in isys output");
    return UNKNOWN_ERROR;
}

status_t GraphConfigImpl::queryAllMatchedResults(
    const std::vector<HalStream*>& activeStreams, bool dummyStillSink,
    std::map<int, std::vector<GCSS::IGraphConfig*>>* queryResults) {
    CheckAndLogError(!queryResults, UNKNOWN_ERROR, "%s, The queryResults is nullptr", __func__);

    status_t ret = createQueryRule(activeStreams, dummyStillSink);
    CheckAndLogError(ret != OK, ret, "Failed to create the query rule");

    LOG2("%s, The mQuery size: %zu", __func__, mQuery.size());
    for (auto& query : mQuery) {
        mFirstQueryResults.clear();
        mGraphQueryManager->queryGraphs(query.second, mFirstQueryResults);
        if (mFirstQueryResults.empty()) {
            LOG2("%s, Failed to query the result, please check the settings xml (0x%x)", __func__,
                 mConfigMode);
            return BAD_VALUE;
        }

        // select setting from multiple results
        ret = selectSetting(query.first, queryResults);
        if (ret != OK) {
            LOG2("%s, There is no the settings for ConfigMode (0x%x)in results", __func__,
                 mConfigMode);
            return UNKNOWN_ERROR;
        }
    }

    if ((*queryResults).empty()) {
        LOG2("%s, There isn't matched result after filtering with first query rule", __func__);
        return UNKNOWN_ERROR;
    }
    return OK;
}

bool GraphConfigImpl::queryGraphSettings(const std::vector<HalStream*>& activeStreams) {
    std::map<int, std::vector<GCSS::IGraphConfig*>> useCaseToQueryResults;
    status_t ret = queryAllMatchedResults(activeStreams, false, &useCaseToQueryResults);
    return ret == OK ? true : false;
}

void GraphConfigImpl::reorderQueryResults(std::map<int, std::vector<GCSS::IGraphConfig*>>& queryRes,
                                          SensorMode sensorMode) {
    if (sensorMode == SENSOR_MODE_UNKNOWN) return;

    for (auto& result : queryRes) {
        auto& settingVector = result.second;
        if (settingVector.size() < 2) continue;

        std::sort(settingVector.begin(), settingVector.end(),
                  [&](GCSS::IGraphConfig* a, GCSS::IGraphConfig* b) {
                      camera_resolution_t aRes, bRes;
                      if (getRawInputSize(a, &aRes) == OK && getRawInputSize(b, &bRes) == OK) {
                          if (aRes.width >= bRes.width && aRes.height >= bRes.height) {
                              if (sensorMode == SENSOR_MODE_FULL) return true;
                          } else {
                              if (sensorMode == SENSOR_MODE_BINNING) return true;
                          }
                      }
                      return false;
                  });
    }
}

/*
 * According to the stream list to query graph setting and create GraphConfigPipe
 */
status_t GraphConfigImpl::configStreams(const vector<HalStream*>& activeStreams,
                                        bool dummyStillSink, SensorMode sensorMode) {
    HAL_TRACE_CALL(CAMERA_DEBUG_LOG_LEVEL1);

    map<int, vector<GCSS::IGraphConfig*>> useCaseToQueryResults;
    status_t ret = queryAllMatchedResults(activeStreams, dummyStillSink, &useCaseToQueryResults);
    CheckAndLogError(ret != OK, UNKNOWN_ERROR, "%s, Faild to queryAllMatchedResults", __func__);

    reorderQueryResults(useCaseToQueryResults, sensorMode);

    // Filter the results with same isys output if there are
    // multiple items in useCaseToQueryResults map
    if (useCaseToQueryResults.size() > 1) {
        ret = OK;
        bool matchFound = false;

        vector<GCSS::IGraphConfig*>& videoQueryResults = useCaseToQueryResults.at(USE_CASE_VIDEO);
        vector<GCSS::IGraphConfig*>& stillQueryResults =
            useCaseToQueryResults.at(USE_CASE_STILL_CAPTURE);
        CheckAndLogError(videoQueryResults.empty() || stillQueryResults.empty(), UNKNOWN_ERROR,
                         "%s, the still or video query results is empty", __func__);

        // Filter the video and still query results with same isys ouput resolution.
        for (auto& video : videoQueryResults) {
            camera_resolution_t videoReso;
            ret = getRawInputSize(video, &videoReso);
            CheckAndLogError(ret != OK, UNKNOWN_ERROR,
                             "%s, Failed to get csi ouput resolution for video pipe", __func__);
            LOG2("Isys output resolution of video pipe: %dx%d", videoReso.width, videoReso.height);

            for (auto& still : stillQueryResults) {
                camera_resolution_t stillReso;
                ret = getRawInputSize(still, &stillReso);
                CheckAndLogError(ret != OK, UNKNOWN_ERROR,
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

        CheckAndLogError(!matchFound, UNKNOWN_ERROR,
                         "%s, Failed to find the isys ouput for video and still pipe", __func__);
    } else {
        // Use the query result with smallest isys output if there is only video pipe
        int resultIdx = 0;
        if (useCaseToQueryResults.begin()->first == USE_CASE_VIDEO &&
            sensorMode == SENSOR_MODE_UNKNOWN) {
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
    CheckAndLogError(mQueryResult.empty(), UNKNOWN_ERROR,
                     "%s, Failed to fill the map into mQueryResult", __func__);

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
    CheckAndLogError(ret != OK, ret, "%s, Failed to prepare graph config: real ConfigMode: %x",
                     __func__, mConfigMode);

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

string GraphConfigImpl::format2GraphStr(int format) {
    switch (format) {
        case V4L2_PIX_FMT_NV12:
        case V4L2_PIX_FMT_P010:
            return "Linear";
        case V4L2_PIX_FMT_YUYV:
            return "YUY2";
        default:
            LOGE("%s, unsupport the output format for graph: %s", __func__,
                 CameraUtils::format2string(format).c_str());
            return "Linear";
    }
}

string GraphConfigImpl::format2GraphBpp(int format) {
    switch (format) {
        case V4L2_PIX_FMT_NV12:
        case V4L2_PIX_FMT_YUYV:
            return "8";
        case V4L2_PIX_FMT_P010:
            return "10";
        default:
            LOGE("%s, unsupport the output format for graph: %s", __func__,
                 CameraUtils::format2string(format).c_str());
            return "8";
    }
}

/*
 * Do the secondary filter: configMode and stream format.
 */
status_t GraphConfigImpl::selectSetting(
    int useCase, std::map<int, std::vector<GCSS::IGraphConfig*>>* queryResults) {
    CheckAndLogError(!queryResults, UNKNOWN_ERROR, "%s, The queryResults is nullptr", __func__);
    string opMode;
    vector<GCSS::IGraphConfig*> internalQueryResults;

    // Firstly, filter the config mode
    for (auto& result : mFirstQueryResults) {
        vector<ConfigMode> cfgModes;
        result->getValue(GCSS_KEY_OP_MODE, opMode);
        LOG2("The operation mode str in xml: %s", opMode.c_str());

        CameraUtils::getConfigModeFromString(opMode, cfgModes);
        LOG2("The query results supports configModes size: %zu", cfgModes.size());

        for (const auto mode : cfgModes) {
            if (mConfigMode == mode) {
                internalQueryResults.push_back(result);
                break;
            }
        }
    }
    CheckAndLogError(internalQueryResults.size() == 0, UNKNOWN_ERROR,
                     "Failed to query the results for configMode: %d", mConfigMode);

    /*
     * May still have multiple graphs after config mode parsing
     * Those graphs have same resolution/configMode, but different output formats
     * Do second graph query with format/bpp as query condition
     */
    map<HalStream*, uid_t>& streamToSinkIdMap = mStreamToSinkIdMap[useCase];
    vector<GCSS::IGraphConfig*> secondQueryResults;
    if (internalQueryResults.size() > 1) {
        LOG2("There are multiple query results, use format to do new round");
        map<GCSS::ItemUID, std::string> queryItem;
        for (auto const& item : streamToSinkIdMap) {
            HalStream* s = item.first;
            ItemUID formatKey = {(ia_uid)item.second, GCSS_KEY_FORMAT};
            string fmt = format2GraphStr(s->format());
            queryItem[formatKey] = fmt;

            ItemUID bppKey = {(ia_uid)item.second, GCSS_KEY_BPP};
            string bpp = format2GraphBpp(s->format());
            queryItem[bppKey] = bpp;

            LOG2("The stream: %dx%d, format: %s, graphFmt: %s, bpp: %s", s->width(), s->height(),
                 CameraUtils::format2string(s->format()).c_str(), fmt.c_str(), bpp.c_str());
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
    LOG1("Query graphs, count %lu, 1st graph id %d", mGraphConfigPipe.size(),
         mGraphConfigPipe.begin()->second->getGraphId());
    for (auto& item : mGraphConfigPipe) {
        LOG1("    Graph usage %d, graph id %d", item.first, item.second->getGraphId());
        data->graphIds.insert(item.second->getGraphId());
    }

    mGraphConfigPipe.begin()->second->getCSIOutputResolution(&(data->csiReso));
    // DOL_FEATURE_S
    mGraphConfigPipe.begin()->second->getDolInfo(&(data->dolInfo.conversionGain),
                                                 &(data->dolInfo.dolMode));
    // DOL_FEATURE_E

    data->mcId = mMcId;
    getGdcKernelSetting(&data->gdcInfos);

    int ret = getPgNames(&(data->pgNames));
    CheckAndLogError(ret != OK, UNKNOWN_ERROR, "%s, Failed to get pg names", __func__);
    for (auto& pgName : data->pgNames) {
        IGraphType::PgInfo info;
        info.pgName = pgName;
        info.streamId = getStreamIdByPgName(pgName);
        info.pgId = getPgIdByPgName(pgName);
        getPgRbmValue(pgName, &info.rbmValue);
        data->pgInfo.push_back(info);
    }

    ret = graphGetStreamIds(&(data->streamIds));
    CheckAndLogError(ret != OK, UNKNOWN_ERROR, "%s, Failed to get streamIds", __func__);
    for (auto& streamId : data->streamIds) {
        IGraphType::MbrInfo mBr;
        mBr.streamId = streamId;
        getMBRData(streamId, &mBr.data);
        data->mbrInfo.push_back(mBr);

        IGraphType::ProgramGroupInfo info;
        info.streamId = streamId;
        info.pgPtr = getProgramGroup(streamId);
        data->programGroup.push_back(info);

        IGraphType::TuningModeInfo mode;
        mode.streamId = streamId;
        mode.tuningMode = getTuningMode(streamId);
        data->tuningModes.push_back(mode);
    }

    return OK;
}

status_t GraphConfigImpl::getGdcKernelSetting(std::vector<IGraphType::GdcInfo>* gdcInfos) {
    CheckAndLogError(mGraphConfigPipe.empty(), UNKNOWN_ERROR, "%s, the mGraphConfigPipe is empty",
                     __func__);
    CheckAndLogError(!gdcInfos, UNKNOWN_ERROR, "%s, the gdcInfos is nullptr", __func__);

    for (auto pipe : mGraphConfigPipe) {
        pipe.second->getGdcKernelResolutionInfo(gdcInfos);
    }

    LOG2("%s, %s resolution", __func__, gdcInfos->empty() ? "No gdc" : "Get gdc");

    return OK;
}

status_t GraphConfigImpl::graphGetStreamIds(std::vector<int32_t>* streamIds) {
    CheckAndLogError(mGraphConfigPipe.empty(), UNKNOWN_ERROR, "%s, the mGraphConfigPipe is empty",
                     __func__);
    CheckAndLogError(!streamIds, UNKNOWN_ERROR, "%s, The streamIds is nullptr", __func__);

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

    CheckAndLogError(streamIds->empty(), UNKNOWN_ERROR,
                     "%s, Failed to find any streamIds for all pipes", __func__);

    return OK;
}

int GraphConfigImpl::getStreamIdByPgName(std::string pgName) {
    CheckAndLogError(mGraphConfigPipe.empty(), -1, "%s, the mGraphConfigPipe is empty", __func__);

    int streamId = -1;
    for (auto& pipe : mGraphConfigPipe) {
        streamId = pipe.second->getStreamIdByPgName(pgName);
        if (streamId != -1) break;
    }
    return streamId;
}

int GraphConfigImpl::getPgIdByPgName(std::string pgName) {
    CheckAndLogError(mGraphConfigPipe.empty(), -1, "%s, the mGraphConfigPipe is empty", __func__);

    int pgId = -1;
    for (auto& pipe : mGraphConfigPipe) {
        pgId = pipe.second->getPgIdByPgName(pgName);
        if (pgId != -1) break;
    }
    return pgId;
}

ia_isp_bxt_program_group* GraphConfigImpl::getProgramGroup(int32_t streamId) {
    CheckAndLogError(mGraphConfigPipe.empty(), nullptr, "%s, the mGraphConfigPipe is empty",
                     __func__);

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

status_t GraphConfigImpl::getMBRData(int32_t streamId, ia_isp_bxt_gdc_limits* data) {
    CheckAndLogError(mGraphConfigPipe.empty(), UNKNOWN_ERROR, "%s, the mGraphConfigPipe is empty",
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
    CheckAndLogError(mGraphConfigPipe.empty(), UNKNOWN_ERROR, "%s, the mGraphConfigPipe is empty",
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

    CheckAndLogError(pgNames->empty(), UNKNOWN_ERROR,
                     "%s, Failed to get the PG's name for all pipes", __func__);

    return OK;
}

status_t GraphConfigImpl::pipelineGetConnections(
    const std::vector<std::string>& pgList, std::vector<IGraphType::ScalerInfo>* scalerInfo,
    std::vector<IGraphType::PipelineConnection>* confVector,
    std::vector<IGraphType::PrivPortFormat>* tnrPortFormat) {
    CheckAndLogError(!confVector, UNKNOWN_ERROR, "%s, the confVector is nullptr", __func__);
    CheckAndLogError(mGraphConfigPipe.empty(), UNKNOWN_ERROR, "%s, the mGraphConfigPipe is empty",
                     __func__);

    if (mGraphConfigPipe.size() == 1) {
        return mGraphConfigPipe.begin()->second->pipelineGetConnections(pgList, scalerInfo,
                                                                        confVector, tnrPortFormat);
    }

    vector<IGraphType::PipelineConnection> stillConnVector, videoConnVector;
    shared_ptr<GraphConfigPipe>& videoGraphPipe = mGraphConfigPipe.at(USE_CASE_VIDEO);
    shared_ptr<GraphConfigPipe>& stillGraphPipe = mGraphConfigPipe.at(USE_CASE_STILL_CAPTURE);

    std::vector<IGraphType::ScalerInfo> stillScalerInfo, videoScalerInfo;
    std::vector<IGraphType::PrivPortFormat> stillTnrPortFmt, videoTnrPortFmt;
    int ret = videoGraphPipe->pipelineGetConnections(pgList, &videoScalerInfo, &videoConnVector,
                                                     &videoTnrPortFmt);
    CheckAndLogError(ret != OK, UNKNOWN_ERROR, "Failed to get the connetction from video pipe");
    ret = stillGraphPipe->pipelineGetConnections(pgList, &stillScalerInfo, &stillConnVector,
                                                 &stillTnrPortFmt);
    CheckAndLogError(ret != OK, UNKNOWN_ERROR, "Failed to get the connetction from still pipe");

    LOG2("The connetction in video: %zu, in still: %zu; the scalera in video: %zu, in still: %zu",
         videoConnVector.size(), stillConnVector.size(), videoScalerInfo.size(),
         stillScalerInfo.size());

    if (!stillScalerInfo.empty()) {
        for (auto& stillScaler : stillScalerInfo) {
            videoScalerInfo.push_back(stillScaler);
        }
    }
    *scalerInfo = videoScalerInfo;

    if (tnrPortFormat) {
        for (auto& stillPort : stillTnrPortFmt) {
            videoTnrPortFmt.push_back(stillPort);
        }
        *tnrPortFormat = videoTnrPortFmt;
    }

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
    CheckAndLogError(videoConnVector.empty(), UNKNOWN_ERROR,
                     "%s, Failed to get connetctions from graph config pipe", __func__);

    LOG2("dump the final connetction");
    GraphUtils::dumpConnections(videoConnVector);
    *confVector = videoConnVector;

    return OK;
}

status_t GraphConfigImpl::getPgIdForKernel(const uint32_t streamId, const int32_t kernelId,
                                           int32_t* pgId) {
    CheckAndLogError(!pgId, UNKNOWN_ERROR, "%s, the pgId is nullptr", __func__);
    CheckAndLogError(mGraphConfigPipe.empty(), UNKNOWN_ERROR, "%s, the mGraphConfigPipe is empty",
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

int32_t GraphConfigImpl::getTuningMode(const int32_t streamId) {
    CheckAndLogError(mGraphConfigPipe.empty(), -1, "%s, the mGraphConfigPipe is empty", __func__);

    if (mGraphConfigPipe.size() == 1) {
        return mGraphConfigPipe.begin()->second->getTuningMode(streamId);
    }

    // Find the stream id from video graph pipe firstly
    int32_t tuningMode = mGraphConfigPipe.at(USE_CASE_VIDEO)->getTuningMode(streamId);
    if (tuningMode == -1) {
        tuningMode = mGraphConfigPipe.at(USE_CASE_STILL_CAPTURE)->getTuningMode(streamId);
    }

    return tuningMode;
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
