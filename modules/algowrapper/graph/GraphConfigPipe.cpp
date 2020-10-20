/*
 * Copyright (C) 2018-2020 Intel Corporation
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

#define LOG_TAG "GraphConfigPipe"

#include "modules/algowrapper/graph/GraphConfigPipe.h"

#include <GCSSParser.h>
#include <gcss.h>
#include <gcss_utils.h>
#include <ia_pal_types_isp_ids_autogen.h>
#include <v4l2_device.h>

#include <algorithm>

#include "FormatUtils.h"
#include "GraphUtils.h"
#include "iutils/CameraLog.h"
#include "iutils/Errors.h"
#include "iutils/Utils.h"
#include "linux/media-bus-format.h"

using GCSS::GraphConfigNode;
using std::string;
using std::vector;
namespace gcu = graphconfig::utils;

namespace icamera {
#define STRINGIFY1(x) #x
#define STRINGIFY(x) STRINGIFY1(x)

// TODO: Change the format attribute natively as integer attribute
#ifndef VIDEO_RECORDING_FORMAT
#define VIDEO_RECORDING_FORMAT TILE
#endif

const char* SENSOR_PORT_NAME = "sensor:port_0";
const char* TPG_PORT_NAME = "tpg:port_0";

#define DB_KERNEL_SIZE 2
#define PPP_KERNEL_SIZE 2
#define DS_KERNEL_SIZE 2
uint32_t dpKernel[DB_KERNEL_SIZE] = {ia_pal_uuid_isp_sc_outputscaler_dp,
                                     ia_pal_uuid_isp_sc_outputscaler_dp_1_1};
uint32_t pppKernel[PPP_KERNEL_SIZE] = {ia_pal_uuid_isp_sc_outputscaler_ppp,
                                       ia_pal_uuid_isp_sc_outputscaler_ppp_1_1};
uint32_t dsKernel[DS_KERNEL_SIZE] = {ia_pal_uuid_isp_b2i_ds_1_0_0, ia_pal_uuid_isp_b2i_ds_1_0_1};

GraphConfigPipe::GraphConfigPipe(int pipeUseCase)
        : mSettings(nullptr),
          mReqId(0),
          mMetaEnabled(false),
          mSourceType(SRC_NONE),
          mPipeUseCase(pipeUseCase) {
    mCsiOutput = {0, 0};
}

GraphConfigPipe::~GraphConfigPipe() {
    fullReset();
}
/*
 * Full reset
 * This is called whenever we want to reset the whole object. Currently that
 * is only, when GraphConfigPipe object is destroyed.
 */
void GraphConfigPipe::fullReset() {
    mSourcePortName.clear();
    mSinkPeerPort.clear();
    mStreamToSinkIdMap.clear();
    delete mSettings;
    mSettings = nullptr;
    mReqId = 0;
    mStream2TuningMap.clear();
    mProgramGroup.clear();
}
/*
 * Reset
 * This is called per frame
 */
void GraphConfigPipe::reset(GraphConfigPipe* me) {
    if (me != nullptr) {
        me->mReqId = 0;
    } else {
        LOGE("Trying to reset a null GraphConfigPipe - BUG!");
    }
}

const GCSS::IGraphConfig* GraphConfigPipe::getInterface(Node* node) const {
    if (!node) return nullptr;

    return node;
}

const GCSS::IGraphConfig* GraphConfigPipe::getInterface() const {
    return mSettings;
}

int GraphConfigPipe::getGraphId(void) {
    int graphId = -1;
    int ret = mSettings->getValue(GCSS_KEY_ID, graphId);
    if (ret != css_err_none) {
        LOGE("Failed to get graphId");
        return BAD_VALUE;
    }

    LOGG("%s: graphId %d", __func__, graphId);
    return graphId;
}

/**
 * Per frame initialization of graph config.
 * Updates request id
 * \param[in] reqId
 */
void GraphConfigPipe::init(int32_t reqId) {
    mReqId = reqId;
}

/**
 * Prepare graph config once per stream config.
 * \param[in] manager
 * \param[in] settings
 * \param[in] streamToSinkIdMap
 * \param[in] active
 */
status_t GraphConfigPipe::prepare(Node* settings, const StreamToSinkMap& streamToSinkIdMap) {
    mSettings = settings;
    status_t ret = OK;

    if (settings == nullptr) {
        LOGW("Settings is NULL!! - BUG?");
        return UNKNOWN_ERROR;
    }

    ret = analyzeSourceType();
    if (ret != OK) {
        LOGE("Failed to analyze source type");
        return ret;
    }

    ret = getActiveOutputPorts(streamToSinkIdMap);
    if (ret != OK) {
        LOGE("Failed to get output ports");
        return ret;
    }
    // Options should be updated before kernel list generation
    ret = handleDynamicOptions();
    if (ret != OK) {
        LOGE("Failed to update options");
        return ret;
    }

    const GCSS::IGraphConfig* gcHandle = getInterface(mSettings);
    css_err_t status = mGCSSAicUtil.initAicUtils(gcHandle);
    if (status != css_err_none) {
        LOGE("Failed to generate kernel list");
        return UNKNOWN_ERROR;
    }

    calculateSinkDependencies();
    storeTuningModes();
    analyzeCSIOutput();

    return ret;
}

/**
 * Store the tuning modes for each stream id into a map that can be used on a
 * per frame basis.
 * This method is executed once per stream configuration.
 * The tuning mode is used by AIC to find the correct tuning tables in CPF.
 *
 */
void GraphConfigPipe::storeTuningModes() {
    GraphConfigNode::const_iterator it = mSettings->begin();
    css_err_t ret = css_err_none;
    GraphConfigNode* result = nullptr;
    int32_t tuningMode = 0;
    int32_t streamId = 0;
    mStream2TuningMap.clear();

    while (it != mSettings->end()) {
        ret = mSettings->getDescendant(GCSS_KEY_TYPE, "program_group", it, &result);
        if (ret == css_err_none) {
            ret = result->getValue(GCSS_KEY_STREAM_ID, streamId);
            if (ret != css_err_none) {
                string pgName;
                // This should  not fail
                ret = result->getValue(GCSS_KEY_NAME, pgName);
                LOGW("Failed to find stream id for PG %s", pgName.c_str());
                continue;
            }
            tuningMode = 0;  // default value in case it is not found
            ret = result->getValue(GCSS_KEY_TUNING_MODE, tuningMode);
            if (ret != css_err_none) {
                string pgName;
                // This should  not fail
                ret = result->getValue(GCSS_KEY_NAME, pgName);
                LOG2("There isn't tuning mode for PG %s, defaulting to %d", pgName.c_str(),
                     tuningMode);
            }
            mStream2TuningMap[streamId] = tuningMode;
        }
    }
}
/**
 * Retrieve the tuning mode associated with a given stream id.
 *
 * The tuning mode is defined by IQ-studio and represent and index to different
 * set of tuning parameters in the AIQB (a.k.a CPF)
 *
 * The tuning mode is an input parameter for AIC.
 * \param [in] streamId Identifier for the branch (video/still/isa)
 * \return tuning mode, if stream id is not found defaults to 0
 */
int32_t GraphConfigPipe::getTuningMode(int32_t streamId) {
    auto item = mStream2TuningMap.find(streamId);
    if (item != mStream2TuningMap.end()) {
        return item->second;
    }
    LOGW("Could not find tuning mode for requested stream id %d", streamId);
    return 0;
}

/*
 * According to the node, analyze the source type:
 * TPG or sensor
 */
status_t GraphConfigPipe::analyzeSourceType() {
    bool hasSensor = false, hasTPG = false;
    Node* inputDevNode = nullptr;
    css_err_t ret = mSettings->getDescendant(GCSS_KEY_SENSOR, &inputDevNode);
    if (ret == css_err_none) {
        mSourceType = SRC_SENSOR;
        mSourcePortName = SENSOR_PORT_NAME;
        hasSensor = true;
    } else {
        LOG1("No sensor node from the graph");
    }

    ret = mSettings->getDescendant(GCSS_KEY_TPG, &inputDevNode);
    if (ret == css_err_none) {
        mSourceType = SRC_TPG;
        mSourcePortName = TPG_PORT_NAME;
        hasTPG = true;
    } else {
        LOG1("No TPG node from the graph");
    }

    if (hasTPG == hasSensor) {
        // failed to analyze source type, reset them
        mSourceType = SRC_NONE;
        mSourcePortName.clear();
        LOGE("Error: Both TPG/Sensor exist or none of TPG/Sensor");
        return UNKNOWN_ERROR;
    }

    return OK;
}

status_t GraphConfigPipe::analyzeCSIOutput() {
    vector<string> csiBeOutput = {"csi_be:output",
                                  "csi_be_soc:output"};
    for (auto& item : csiBeOutput) {
        GCSS::IGraphConfig* csiBeNode =
            static_cast<GCSS::IGraphConfig*>(mSettings)->getDescendantByString(item.c_str());
        if (csiBeNode != nullptr) {
            GCSS::GraphCameraUtil::getDimensions(csiBeNode, &mCsiOutput.width, &mCsiOutput.height);
            return OK;
        }
    }

    LOGE("Error: Couldn't get CSI-BE node");
    return UNKNOWN_ERROR;
}

status_t GraphConfigPipe::getMBRData(int32_t streamId, ia_isp_bxt_gdc_limits* data) {
    css_err_t ret = mGCSSAicUtil.getMbrData(streamId, data);
    if (ret == css_err_none) {
        return OK;
    } else {
        LOG2("fail to getMBRData for stream id:%d", streamId);
        return UNKNOWN_ERROR;
    }
}
/**
 * Finds the sink nodes and the output port peer. Use streamToSinkIdMap
 * since we are intrested only in sinks that serve a stream. Takes an
 * internal copy of streamToSinkIdMap to be used later.
 *
 * \param[in] streamToSinkIdMap to get the virtual sink id from a client stream pointer
 * \return OK in case of success.
 * \return UNKNOWN_ERROR or BAD_VALUE in case of fail.
 */
status_t GraphConfigPipe::getActiveOutputPorts(const StreamToSinkMap& streamToSinkIdMap) {
    status_t status = OK;
    css_err_t ret = css_err_none;
    std::vector<GCSS::IGraphConfig*> sinks;

    mStreamToSinkIdMap.clear();
    mStreamToSinkIdMap = streamToSinkIdMap;
    mSinkPeerPort.clear();

    StreamToSinkMap::const_iterator it;
    it = streamToSinkIdMap.begin();

    for (; it != streamToSinkIdMap.end(); ++it) {
        sinks.clear();
        status = GCSS::GraphCameraUtil::graphGetSinksByName(GCSS::ItemUID::key2str(it->second),
                                                            mSettings, sinks);
        if (status != OK || sinks.empty()) {
            string sinkName = GCSS::ItemUID::key2str(it->second);
            LOGE("Found %zu sinks, expecting 1 for sink %s", sinks.size(), sinkName.c_str());
            return BAD_VALUE;
        }

        Node* sink = static_cast<GraphConfigNode*>(sinks[0]);
        Node* outputPort = nullptr;

        // Get the sinkname for getting the output port
        string sinkName;
        ret = sink->getValue(GCSS_KEY_NAME, sinkName);
        if (ret != css_err_none) {
            LOGE("Failed to get sink name");
            return BAD_VALUE;
        }
        LOG2("sink name %s", sinkName.c_str());

        int32_t streamId = -1;
        ret = sink->getValue(GCSS_KEY_STREAM_ID, streamId);
        if (ret != css_err_none) {
            LOGE("Failed to get stream id");
            return BAD_VALUE;
        }
        LOG2("stream id %d", streamId);

        outputPort = getOutputPortForSink(sinkName);
        if (outputPort == nullptr) {
            LOGE("No output port found for sink");
            return UNKNOWN_ERROR;
        }

        LOG2("output port name %s", NODE_NAME(outputPort));
        mSinkPeerPort[sink] = outputPort;
    }

    return OK;
}

string GraphConfigPipe::getNodeName(Node* node) {
    string nodeName("");
    if (node == nullptr) {
        LOGE("Node is NULL");
        return nodeName;
    }

    node->getValue(GCSS_KEY_NAME, nodeName);
    return nodeName;
}

/**
 * Finds the output port which is the peer to the sink node.
 *
 * Gets root node, and finds the sink with the given name. Use portGetPeer()
 * to find the output port.
 * \return GraphConfigNode in case of success.
 * \return nullptr in case of fail.
 */
Node* GraphConfigPipe::getOutputPortForSink(const string& sinkName) {
    css_err_t ret = css_err_none;
    status_t retErr = OK;
    Node* rootNode = nullptr;
    Node* portNode = nullptr;
    Node* peerNode = nullptr;

    rootNode = mSettings->getRootNode();
    if (rootNode == nullptr) {
        LOGE("Couldn't get root node, BUG!");
        return nullptr;
    }
    ret = rootNode->getDescendantByString(sinkName, &portNode);
    if (ret != css_err_none) {
        LOGE("Error getting sink");
        return nullptr;
    }
    retErr = portGetPeer(portNode, &peerNode);
    if (retErr != OK) {
        LOGE("Error getting peer");
        return nullptr;
    }
    return peerNode;
}

/**
 * Update the option-list to the graph tree.
 * TODO: Add more options.
 * \return OK in case of success
 * \return UNKNOWN_ERROR if graph update failed.
 */
status_t GraphConfigPipe::handleDynamicOptions() {
    status_t status = setPortFormats();
    if (status != OK) {
        LOGE("Failed to update metadata");
        return UNKNOWN_ERROR;
    }

    // TODO add other options
    return status;
}

/**
 * Returns true if the given node is used to output a video record
 * stream. The sink name is found and used to find client stream from the
 * mStreamToSinkIdMap.
 * Then the video encoder gralloc flag is checked from the stream flags of the
 * client stream.
 * \param[in] peer output port to find the sink node of.
 * \return true if sink port serves a video record stream.
 * \return false if sink port does not serve a video record stream.
 */
bool GraphConfigPipe::isVideoRecordPort(Node* sink) {
    css_err_t ret = css_err_none;
    string sinkName;
    HalStream* clientStream = nullptr;

    if (sink == nullptr) {
        LOGE("No sink node provided");
        return false;
    }

    ret = sink->getValue(GCSS_KEY_NAME, sinkName);
    if (ret != css_err_none) {
        LOGE("Failed to get sink name");
        return false;
    }

    // Find the client stream for the sink port
    StreamToSinkMap::iterator it1;
    it1 = mStreamToSinkIdMap.begin();

    for (; it1 != mStreamToSinkIdMap.end(); ++it1) {
        if (GCSS::ItemUID::key2str(it1->second) == sinkName) {
            clientStream = it1->first;
            break;
        }
    }

    if (clientStream == nullptr) {
        LOGE("Failed to find client stream");
        return false;
    }

    if (clientStream->useCase() == USE_CASE_VIDEO) {
        LOG2("%s is video record port", NODE_NAME(sink));
        return true;
    }

    return false;
}

/**
 * Takes a stream id, and checks if it exists in the graph.
 *
 * \param[in] streamId
 * \return true if found, false otherwise
 */
bool GraphConfigPipe::hasStreamInGraph(int streamId) {
    status_t status;
    StreamsVector streamsFound;

    status = graphGetStreamIds(&streamsFound);
    if (status != OK) return false;

    for (auto id : streamsFound) {
        if (id == streamId) return true;
    }
    return false;
}

/**
 * Apply the video recording format for the video record stream handling
 * output port.
 * \return OK in case of success
 * \return UNKNOWN_ERROR if option list apply failed.
 */
status_t GraphConfigPipe::setPortFormats() {
    css_err_t ret = css_err_none;
    std::map<Node*, Node*>::iterator it;
    it = mSinkPeerPort.begin();

    for (; it != mSinkPeerPort.end(); ++it) {
        Node* sink = it->first;
        Node* peer = it->second;
        if (!isVideoRecordPort(sink)) continue;

        ret = peer->setValue(GCSS_KEY_FORMAT, STRINGIFY(VIDEO_RECORDING_FORMAT));
        if (ret != css_err_none) {
            // If format attribute does not exist, create it.
            ret = peer->addValue(GCSS_KEY_FORMAT, STRINGIFY(VIDEO_RECORDING_FORMAT));
            if (ret != css_err_none) {
                LOGE("Failed to update options for video record port");
                return UNKNOWN_ERROR;
            }
        }
    }

    return OK;
}

/**
 * Returns pointer to kernel list based on given stream id
 * \param[in] streamId Return kernel list for this stream id
 * \return ia_isp_bxt_program_group
 */
ia_isp_bxt_program_group* GraphConfigPipe::getProgramGroup(int32_t streamId) {
    if (mProgramGroup.find(streamId) == mProgramGroup.end()) {
        ia_isp_bxt_program_group programGroup;
        CLEAR(programGroup);
        mGCSSAicUtil.getProgramGroup(streamId, programGroup);
        mProgramGroup[streamId] = programGroup;
    }
    LOGG("Dump kernel info for stream %d", streamId);
    GraphUtils::dumpKernelInfo(mProgramGroup[streamId]);

    return &(mProgramGroup[streamId]);
}

int GraphConfigPipe::getProgramGroup(string pgName, ia_isp_bxt_program_group* programGroupForPG) {
    GCSS::NodeIterator iter(mSettings);
    GCSS::IGraphConfig* pg = iter.iterateByType(GCSS_KEY_PROGRAM_GROUP);
    std::string name;
    for (; pg != nullptr; pg = iter.iterateByType(GCSS_KEY_PROGRAM_GROUP)) {
        css_err_t ret = pg->getValue(GCSS_KEY_NAME, name);
        CheckError(ret != css_err_none, BAD_VALUE,
                   "Failed to get the name of an existing PG node, BUG");
        if (name == pgName) {
            break;
        }
    }
    CheckError(pg == nullptr, UNKNOWN_ERROR, "Failed to get program groups, BUG");

    mGCSSAicUtil.getProgramGroup(pg, *programGroupForPG);

    LOGG("Dump kernel info for %s", pgName.c_str());
    GraphUtils::dumpKernelInfo(*programGroupForPG);

    return OK;
}

status_t GraphConfigPipe::getPgRbmValue(string pgName, IGraphType::StageAttr* stageAttr) {
    std::string name;
    GCSS::NodeIterator iter(mSettings);
    GCSS::IGraphConfig* pg = iter.iterateByType(GCSS_KEY_PROGRAM_GROUP);

    for (; pg != nullptr; pg = iter.iterateByType(GCSS_KEY_PROGRAM_GROUP)) {
        css_err_t ret = pg->getValue(GCSS_KEY_NAME, name);
        CheckError(ret != css_err_none, BAD_VALUE, "Failed to get the name of PG node");
        if (name == pgName) {
            break;
        }
    }
    CheckError(pg == nullptr, UNKNOWN_ERROR, "Failed to get program groups for PG: %s",
               pgName.c_str());

    pg = pg->getDescendant(GCSS_KEY_CIPF);
    if (pg == nullptr) return NAME_NOT_FOUND;

    string rbmString;
    css_err_t ret = pg->getValue(GCSS_KEY_RBM, rbmString);
    if (ret != css_err_none) return NAME_NOT_FOUND;

    GCSS::GraphCameraUtil mGCSSCameraUtil;
    stageAttr->rbm = mGCSSCameraUtil.numString2binary(rbmString, &stageAttr->rbm_bytes);
    CheckError(!stageAttr->rbm, NO_MEMORY, "%s get rbm value: %s", __func__, rbmString.c_str());

    return OK;
}

status_t GraphConfigPipe::getScalerKernelResolutionRatio(uint32_t* kenerArray, uint32_t sizeArray,
                                                         float* widthRatio, float* heightRatio) {
    CheckError(!kenerArray, UNKNOWN_ERROR, "%s the array is null", __func__);
    CheckError(!widthRatio || !heightRatio, UNKNOWN_ERROR, "%s widthRatio or heightRatio is null",
               __func__);

    const ia_isp_bxt_resolution_info_t* resolutionInfo;
    resolutionInfo = getScalerKernelResolutionInfo(kenerArray, sizeArray);
    CheckError(!resolutionInfo, UNKNOWN_ERROR, "%s getScalerKernelResolutionInfo fails", __func__);

    *widthRatio = 1.0;
    *heightRatio = 1.0;
    const ia_rectangle* input_crop = &resolutionInfo->input_crop;
    const ia_rectangle* output_crop = &resolutionInfo->output_crop;
    if (((resolutionInfo->input_width != resolutionInfo->output_width) ||
         (resolutionInfo->input_height != resolutionInfo->output_height)) &&
        ((input_crop->left == 0) && (input_crop->top == 0) && (input_crop->right == 0) &&
         (input_crop->bottom == 0)) &&
        ((output_crop->left == 0) && (output_crop->top == 0) && (output_crop->right == 0) &&
         (output_crop->bottom == 0))) {
        *widthRatio = static_cast<float>(resolutionInfo->input_width) /
                      static_cast<float>(resolutionInfo->output_width);
        *heightRatio = static_cast<float>(resolutionInfo->input_height) /
                       static_cast<float>(resolutionInfo->output_height);
        LOG2("%s, width:%d-%d; height:%d-%d", __func__, resolutionInfo->input_width,
             resolutionInfo->output_width, resolutionInfo->input_height,
             resolutionInfo->output_height);
    }
    return OK;
}

const ia_isp_bxt_resolution_info_t* GraphConfigPipe::getScalerKernelResolutionInfo(
    uint32_t* kenerArray, uint32_t sizeArray) {
    CheckError(!kenerArray, nullptr, "%s the array is null", __func__);

    std::vector<int32_t> streamIds;
    // Get all stream IDs
    status_t ret = graphGetStreamIds(&streamIds);
    CheckWarning((ret != OK || streamIds.empty()), nullptr, "Failed to get the PG streamIds");

    uint32_t kernelId = kenerArray[0];
    int32_t streamIdTmp = VIDEO_STREAM_ID;

    bool hasVideo = false;
    for (auto streamId : streamIds) {
        for (uint32_t i = 0; i < sizeArray; i++) {
            if (isKernelInStream(streamId, kenerArray[i])) {
                LOG2("%s, found outputscaler %u from stream %d", __func__, kenerArray[i], streamId);
                streamIdTmp = streamId;
                kernelId = kenerArray[i];
                if (streamId == VIDEO_STREAM_ID) hasVideo = true;

                break;
            }
        }
        if (hasVideo) break;
    }

    // Get resolution as per above kernel and stream
    return getKernelResolutionInfo(streamIdTmp, kernelId);
}

const ia_isp_bxt_resolution_info_t* GraphConfigPipe::getGdcKernelResolutionInfo(
    uint32_t* kernelId) {
    CheckError(!kernelId, nullptr, "%s the kernelId is nullptr", __func__);

    std::vector<int32_t> streamIds;
    // Get all stream IDs
    status_t ret = graphGetStreamIds(&streamIds);
    CheckWarning((ret != OK || streamIds.empty()), nullptr, "Failed to get the PG streamIds");

    *kernelId = ia_pal_uuid_isp_gdc3;
    LOG1("%s, initalize gdc version 3 as default", __func__);
    int32_t gdcStreamId = VIDEO_STREAM_ID;
    LOG1("%s, initalize gdc video stream as default", __func__);

    // Check video stream for gdc version firstly,
    // in case more than one stream contain gdc kernel.
    bool hasVideoGdc = false;
    for (auto streamId : streamIds) {
        if (isKernelInStream(streamId, ia_pal_uuid_isp_gdc3_1)) {
            LOG1("%s, found gdc3_1 from stream %d", __func__, streamId);
            gdcStreamId = streamId;
            *kernelId = ia_pal_uuid_isp_gdc3_1;
            if (streamId == VIDEO_STREAM_ID) hasVideoGdc = true;
        } else if (isKernelInStream(streamId, ia_pal_uuid_isp_gdc3)) {
            LOG1("%s, found gdc3 from stream %d", __func__, streamId);
            gdcStreamId = streamId;
            *kernelId = ia_pal_uuid_isp_gdc3;
            if (streamId == VIDEO_STREAM_ID) hasVideoGdc = true;
        } else if (isKernelInStream(streamId, ia_pal_uuid_isp_gdc3_1_1)) {
            LOG1("%s, found gdc3_1_1 from stream %d", __func__, streamId);
            gdcStreamId = streamId;
            *kernelId = ia_pal_uuid_isp_gdc3_1_1;
            if (streamId == VIDEO_STREAM_ID) hasVideoGdc = true;
        } else if (isKernelInStream(streamId, ia_pal_uuid_isp_gdc5)) {
            LOG1("%s, found gdc5 from stream %d", __func__, streamId);
            gdcStreamId = streamId;
            *kernelId = ia_pal_uuid_isp_gdc5;
            if (streamId == VIDEO_STREAM_ID) hasVideoGdc = true;
        }
        if (hasVideoGdc) break;
    }

    // Get resolution as per above kernel and stream
    return getKernelResolutionInfo(gdcStreamId, *kernelId);
}

status_t GraphConfigPipe::getGdcKernelSetting(uint32_t* kernelId,
                                              ia_isp_bxt_resolution_info_t* resolution) {
    CheckError(!kernelId || !resolution, UNKNOWN_ERROR, "%s, the kernelId or resolution is nullptr",
               __func__);

    // Get resolution as per above kernel and stream
    const ia_isp_bxt_resolution_info_t* gdcResolution = getGdcKernelResolutionInfo(kernelId);
    CheckWarning(!gdcResolution, NO_ENTRY, "Couldn't get the GDC resolution in current pipe: %d",
                 mPipeUseCase);

    *resolution = *gdcResolution;

    LOGG("%s: kernel %d, inResolution %dx%d, outResolution %dx%d", __func__, *kernelId,
         resolution->input_width, resolution->input_height, resolution->output_width,
         resolution->output_height);

    LOGG("%s: kernel %d, inputCrop %d,%d,%d,%d; outputCrop %d,%d,%d,%d", __func__, *kernelId,
         resolution->input_crop.left, resolution->input_crop.top, resolution->input_crop.right,
         resolution->input_crop.bottom, resolution->output_crop.left, resolution->output_crop.top,
         resolution->output_crop.right, resolution->output_crop.bottom);

    return OK;
}

const ia_isp_bxt_resolution_info_t* GraphConfigPipe::getKernelResolutionInfo(uint32_t streamId,
                                                                             uint32_t kernelId) {
    ia_isp_bxt_program_group* programGroup = getProgramGroup(streamId);
    if (programGroup == nullptr) {
        return nullptr;
    }

    for (unsigned int i = 0; i < programGroup->kernel_count; i++) {
        if (programGroup->run_kernels[i].stream_id == streamId &&
            programGroup->run_kernels[i].kernel_uuid == kernelId) {
            return programGroup->run_kernels[i].resolution_info;
        }
    }

    return nullptr;
}

/**
 * check whether the kernel is in this stream
 *
 * \param[in] streamId stream id.
 * \param[in] kernelId kernel id.
 * \param[out] whether the kernel in this stream
 * \return true the kernel is in this stream
 * \return false the kernel isn't in this stream.
 *
 */
bool GraphConfigPipe::isKernelInStream(uint32_t streamId, uint32_t kernelId) {
    ia_isp_bxt_program_group* programGroup = getProgramGroup(streamId);
    if (programGroup == nullptr) {
        return false;
    }
    for (unsigned int i = 0; i < programGroup->kernel_count; i++) {
        if (programGroup->run_kernels[i].kernel_uuid == kernelId) {
            return true;
        }
    }

    return false;
}

/**
 * get program group id for some kernel
 *
 * \param[in] streamId stream id.
 * \param[in] kernelId kernel pal uuid.
 * \param[out] program group id that contain this kernel with the same stream id
 * \return error if can't find the kernel id in any ot the PGs in this stream
 */
status_t GraphConfigPipe::getPgIdForKernel(const uint32_t streamId, const int32_t kernelId,
                                           int32_t* pgId) {
    CheckError(!pgId, UNKNOWN_ERROR, "%s, the pgId is nullptr", __func__);
    css_err_t ret = css_err_none;
    status_t retErr;
    NodesPtrVector programGroups;

    // Get all program groups with the stream id
    retErr = streamGetProgramGroups(streamId, &programGroups);
    if (retErr != OK) {
        LOGE("ERROR: couldn't get program groups");
        return retErr;
    }

    // Go through all the program groups with the selected streamID
    for (auto& ndVec : programGroups) {
        /* Iterate through program group nodes, find kernel and get the PG id */
        GCSS::GraphConfigItem::const_iterator it = ndVec->begin();
        while (it != ndVec->end()) {
            Node* kernelNode = nullptr;
            // Look for kernel with the requested uuid
            ret = ndVec->getDescendant(GCSS_KEY_PAL_UUID, kernelId, it, &kernelNode);
            if (ret != css_err_none) continue;

            ret = ndVec->getValue(GCSS_KEY_PG_ID, *pgId);
            if (ret == css_err_none) {
                LOG2("got the pgid:%d for kernel id:%d in stream:%d", *pgId, kernelId, streamId);
                return OK;
            }
            LOGE("ERROR: Couldn't get pg id for kernel %d", kernelId);
            return BAD_VALUE;
        }
    }
    LOG2("%s: kernel id %d is not found in stream %d", __func__, kernelId, streamId);
    return BAD_VALUE;
}

/**
 * This method creates SinkDependency structure for every active sink found in
 * the graph. These structs allow quick access to information that is required
 * by other methods.
 * Active sinks are the ones that have a connection to an active port.
 * This list of active sinks(mSinkPeerPort) has to be filled before this method
 * is executed.
 * For every virtual sink we store the name (as a key) and the terminal id of
 * the input port of the stream associated with that stream. This input port
 * will be the destination of the buffers from the capture unit.
 *
 * This method is used during init()
 * If we would have different settings per frame then this would be enough
 * to detect the active ISA nodes, but we are not there yet. we are still using
 * the base graph settings every frame.
 */
void GraphConfigPipe::calculateSinkDependencies() {
    status_t status = OK;
    GCSS::IGraphConfig* streamInputPort = nullptr;
    std::string sinkName;
    SinkDependency aSinkDependency;
    ia_uid stageId;  // not needed
    mSinkDependencies.clear();
    mIsaOutputPort2StreamId.clear();
    std::map<Node*, Node*>::iterator sinkIter = mSinkPeerPort.begin();

    for (; sinkIter != mSinkPeerPort.end(); ++sinkIter) {
        sinkIter->first->getValue(GCSS_KEY_NAME, sinkName);
        aSinkDependency.sinkGCKey = GCSS::ItemUID::str2key(sinkName);
        sinkIter->first->getValue(GCSS_KEY_STREAM_ID, aSinkDependency.streamId);
        status = GCSS::GraphCameraUtil::getInputPort(GCSS_KEY_STREAM_ID, aSinkDependency.streamId,
                                                     mSettings, &streamInputPort);
        if (status != OK) {
            LOGE("Failed to get input port for stream %d associated to sink %s",
                 aSinkDependency.streamId, sinkName.c_str());
            continue;
        }
        status = GCSS::GraphCameraUtil::portGetFourCCInfo(streamInputPort, stageId,
                                                          aSinkDependency.streamInputPortId);
        if (status != OK) {
            LOGE("Failed to get stream %d input port 4CC code", aSinkDependency.streamId);
            continue;
        }
        GCSS::IGraphConfig* temp = nullptr;
        status = GCSS::GraphCameraUtil::portGetPeer(streamInputPort, &temp);
        if (status != OK) {
            LOGE("fail to get peer for the port(%s)",
                 GCSS::GraphCameraUtil::print(streamInputPort).c_str());
            continue;
        }
        aSinkDependency.peer = static_cast<Node*>(temp);
        LOG2("Adding dependency %s stream id %d", sinkName.c_str(), aSinkDependency.streamId);
        mSinkDependencies.push_back(aSinkDependency);

        // get the output port of capture unit
        status = GCSS::GraphCameraUtil::portGetPeer(streamInputPort, &temp);
        if (status != OK) {
            LOGE("Fail to get isa output port for sink %s", sinkName.c_str());
            continue;
        }
        Node* isaOutPutPort = static_cast<Node*>(temp);
        std::string fullName;
        status = portGetFullName(isaOutPutPort, &fullName);
        if (status != OK) {
            LOGE("Fail to get isa output port name");
            continue;
        }
        int32_t streamId = portGetStreamId(isaOutPutPort);
        if (streamId != -1 &&
            mIsaOutputPort2StreamId.find(fullName) == mIsaOutputPort2StreamId.end())
            mIsaOutputPort2StreamId[fullName] = streamId;
    }
}

/**
 * This method is used by the GC Manager that has access to the request
 * to inform us of what are the active sinks.
 * Using the sink dependency information we can then know which ISA ports
 * are active for this GC.
 *
 * Once we have different settings per request then we can incorporate this
 * method into calculateSinkDependencies.
 *
 * \param[in] activeSinks Vector with GCSS_KEY's of the active sinks in a
 *                        request
 */
void GraphConfigPipe::setActiveSinks(const std::vector<uid_t>& activeSinks) {
    mIsaActiveDestinations.clear();
    uid_t activeDest = 0;

    for (auto sink : activeSinks) {
        for (auto& dependency : mSinkDependencies) {
            if (dependency.sinkGCKey == sink) {
                activeDest = dependency.streamInputPortId;
                mIsaActiveDestinations[activeDest] = activeDest;
            }
        }
    }
}

/**
 * This method is used by the GC Manager that has access to the request
 * to inform us of what will the stream id be used.
 * Using the sink dependency information we can then know which stream ids
 * are active for this GC.
 *
 * Once we have different settings per request then we can incorporate this
 * method into calculateSinkDependencies.
 *
 * \param[in] activeSinks Vector with GCSS_KEY's of the active sinks in a
 *                        request
 */
void GraphConfigPipe::setActiveStreamId(const std::vector<uid_t>& activeSinks) {
    mActiveStreamId.clear();
    int32_t activeStreamId = 0;

    for (auto sink : activeSinks) {
        for (auto& dependency : mSinkDependencies) {
            if (dependency.sinkGCKey == sink) {
                activeStreamId = dependency.streamId;
                mActiveStreamId.insert(activeStreamId);

                // get peer's stream Id
                activeStreamId = portGetStreamId(dependency.peer);
                if (activeStreamId == -1) {
                    LOGE("fail to get the stream id for peer port");
                    continue;
                }
                if (mActiveStreamId.find(activeStreamId) == mActiveStreamId.end())
                    mActiveStreamId.insert(activeStreamId);
            }
        }
    }
}

int GraphConfigPipe::getStreamIdByPgName(string pgName) {
    css_err_t ret = ia_err_none;
    string foundPgName = "invalid";
    GraphConfigNode* programGroup = nullptr;
    GraphConfigNode::const_iterator it = mSettings->begin();

    while (it != mSettings->end()) {
        programGroup = nullptr;
        ret = mSettings->getDescendant(GCSS_KEY_TYPE, "program_group", it, &programGroup);
        if (ret != ia_err_none || programGroup == nullptr) {
            continue;
        }

        ret = programGroup->getValue(GCSS_KEY_NAME, foundPgName);
        if (ret != ia_err_none) {
            LOGW("%s, failed to get pg name in program group", __func__);
            continue;
        }

        if (foundPgName == pgName) break;
    }

    if (foundPgName != pgName || !programGroup) {
        LOG2("No matched PG found in pipeUseCase: %d", mPipeUseCase);
        return -1;
    }

    int streamId = -1;
    ret = programGroup->getValue(GCSS_KEY_STREAM_ID, streamId);
    CheckError(ret != ia_err_none, -1, "Get streamId failed by name:%s, pipeUseCase: %d",
               pgName.c_str(), mPipeUseCase);

    LOGG("%s: streamId %d", __func__, streamId);
    return streamId;
}

status_t GraphConfigPipe::getPgNames(std::vector<string>* pgNames) {
    css_err_t ret = css_err_none;
    GraphConfigNode::const_iterator it = mSettings->begin();
    GraphConfigNode* programGroup = nullptr;

    while (it != mSettings->end()) {
        programGroup = nullptr;
        ret = mSettings->getDescendant(GCSS_KEY_TYPE, "program_group", it, &programGroup);
        if (ret != css_err_none || programGroup == nullptr) {
            continue;
        }

        string foundPgName;
        ret = programGroup->getValue(GCSS_KEY_NAME, foundPgName);
        if (ret != css_err_none) {
            LOGW("%s, failed to get pg name in program group", __func__);
            continue;
        }

        pgNames->push_back(foundPgName);
    }

    return OK;
}

bool GraphConfigPipe::containPgs(std::vector<string> pgNames) {
    std::vector<string> allPgNames;
    getPgNames(&allPgNames);

    for (auto& name : pgNames) {
        if (std::find(allPgNames.begin(), allPgNames.end(), name.c_str()) == allPgNames.end())
            return false;
    }
    return true;
}

int GraphConfigPipe::getPgIdByPgName(string pgName) {
    css_err_t ret = css_err_none;
    GraphConfigNode::const_iterator it = mSettings->begin();
    GraphConfigNode* programGroup = nullptr;
    bool foundMatchedPg = false;

    while (it != mSettings->end()) {
        programGroup = nullptr;
        ret = mSettings->getDescendant(GCSS_KEY_TYPE, "program_group", it, &programGroup);
        if (ret != css_err_none || programGroup == nullptr) {
            continue;
        }

        string foundPgName;
        ret = programGroup->getValue(GCSS_KEY_NAME, foundPgName);
        if (ret != css_err_none) {
            LOGW("%s, failed to get pg name in program group", __func__);
            continue;
        }

        if (foundPgName == pgName) {
            foundMatchedPg = true;
            break;
        }
    }

    if (!foundMatchedPg) {
        LOG2("No matched PG found, pgName: %s, pipeUseCase: %d", pgName.c_str(), mPipeUseCase);
        return -1;
    }

    const GCSS::IGraphConfig* gc = getInterface(programGroup);
    CheckError(gc == nullptr, -1, "%s, Failed to get graph config interface", __func__);

    int pgId = -1;
    ret = gc->getValue(GCSS_KEY_PG_ID, pgId);
    CheckError(ret != css_err_none, -1, "Get PG ID failed with:%d", ret);

    LOGG("%s: pgName %s, pgId %d", __func__, pgName.c_str(), pgId);
    return pgId;
}

status_t GraphConfigPipe::getProgramGroupsByName(const std::vector<std::string>& pgNames,
                                                 NodesPtrVector* programGroups) {
    CheckError(!programGroups, UNKNOWN_ERROR, "%s, The programGroups is nullptr", __func__);
    css_err_t ret = css_err_none;
    GraphConfigNode* result;
    NodesPtrVector allProgramGroups;
    string foundPgName;

    GraphConfigNode::const_iterator it = mSettings->begin();

    while (it != mSettings->end()) {
        ret = mSettings->getDescendant(GCSS_KEY_TYPE, "program_group", it, &result);
        if (ret == css_err_none) allProgramGroups.push_back(result);
    }

    CheckError(allProgramGroups.empty(), UNKNOWN_ERROR,
               "%s, doesn't find any PG in current pipe: %d", __func__, mPipeUseCase);
    for (auto& ndVec : allProgramGroups) {
        ret = ndVec->getValue(GCSS_KEY_NAME, foundPgName);
        if (ret != css_err_none) {
            LOGE("%s, failed to get pg name in program group", __func__);
            continue;
        }

        for (auto& name : pgNames) {
            if (foundPgName.find(name) != string::npos) {
                programGroups->push_back(ndVec);
            }
        }
    }
    if (programGroups->empty()) {
        LOG2("%s, doesn't find the matched pg in current pipe: %d", __func__, mPipeUseCase);
    }

    return OK;
}

status_t GraphConfigPipe::pipelineGetConnections(
    const std::vector<std::string>& pgList, std::vector<IGraphType::ScalerInfo>* scalerInfo,
    std::vector<IGraphType::PipelineConnection>* confVector) {
    CheckError(!confVector, UNKNOWN_ERROR, "%s, the confVector is nullptr", __func__);

    NodesPtrVector programGroups;
    NodesPtrVector alreadyConnectedPorts;
    Node* peerPort = nullptr;
    Node* port = nullptr;
    IGraphType::PipelineConnection aConnection;
    std::map<Node*, IGraphType::PipelineConnection> edgePort2Connection;

    status_t status = getProgramGroupsByName(pgList, &programGroups);
    CheckError(status != OK, status, "%s, failed to get program groups, BUG", __func__);

    for (size_t i = 0; i < programGroups.size(); i++) {
        Node::const_iterator it = programGroups[i]->begin();

        while (it != programGroups[i]->end()) {
            css_err_t ret = programGroups[i]->getDescendant(GCSS_KEY_TYPE, "port", it, &port);
            if (ret != css_err_none) continue;

            // port for private terminal, no need to connect
            int priv = 0;
            ret = port->getValue(GCSS_KEY_PRIVATE, priv);
            if (ret == css_err_none && priv) continue;

            /*
             * Since we are iterating through the ports
             * check if this port is already connected to avoid setting
             * the connection twice
             */
            if (std::find(alreadyConnectedPorts.begin(), alreadyConnectedPorts.end(), port) !=
                alreadyConnectedPorts.end()) {
                continue;
            }
            LOG1("Configuring Port from PG[%zu] in line:%d", i, __LINE__);

            string contentType;
            ret = port->getValue(GCSS_KEY_CONTENT_TYPE, contentType);
            if (ret == css_err_none && contentType != "pixel_data") {
                LOG2("%s skipped content type %s", NODE_NAME(port), contentType.c_str());
                continue;
            }

            status = portGetFormat(port, &(aConnection.portFormatSettings));
            if (status != OK) {
                LOGE("Failed to get port format info in port from PG[%zu]", i);
                return BAD_VALUE;
            }
            if (aConnection.portFormatSettings.enabled == 0) {
                LOG1("Port from PG[%zu] disabled", i);
                status = portGetOwner(port, &(aConnection.connectionConfig));
                CheckError((status != OK), BAD_VALUE, "Failed to get ownerfor port from PG[%zu]",
                           i);
                confVector->push_back(aConnection);
                continue;
            } else {
                LOG1("Port: 0x%x format(%dx%d)fourcc: %s bpl: %d bpp: %d",
                     aConnection.portFormatSettings.terminalId,
                     aConnection.portFormatSettings.width, aConnection.portFormatSettings.height,
                     CameraUtils::fourcc2String(aConnection.portFormatSettings.fourcc).c_str(),
                     aConnection.portFormatSettings.bpl, aConnection.portFormatSettings.bpp);
            }

            /*
             * for each port get the connection info and pass it
             * to the pipeline object
             */
            status = portGetConnection(port, &(aConnection.connectionConfig), &peerPort);
            if (status != OK) {
                LOGE("Failed to create connection info in port from PG[%zu]", i);
                return BAD_VALUE;
            }

            aConnection.hasEdgePort = false;
            if (isPipeEdgePort(port)) {
                int32_t direction = portGetDirection(port);
                if (direction == GraphConfigPipe::PORT_DIRECTION_INPUT) {
                    aConnection.connectionConfig.mConnectionType = IGraphType::connection_type_push;
                } else {
                    HalStream* clientStream = nullptr;
                    status = portGetClientStream(peerPort, &clientStream);
                    CheckError(status != OK, UNKNOWN_ERROR,
                               "Failed to find client stream for v-sink");
                    aConnection.stream = clientStream;
                    if (clientStream != nullptr) {
                        edgePort2Connection[port] = aConnection;
                    }
                }
                aConnection.hasEdgePort = true;
            }
            confVector->push_back(aConnection);
            alreadyConnectedPorts.push_back(port);
            alreadyConnectedPorts.push_back(peerPort);
        }
    }

    getScalerByStreamId(edgePort2Connection, scalerInfo);
    GraphUtils::dumpConnections(*confVector);

    return status;
}

status_t GraphConfigPipe::getScalerByStreamId(
    std::map<Node*, IGraphType::PipelineConnection> edgePort2Connection,
    std::vector<IGraphType::ScalerInfo>* scalerInfo) {
    if (edgePort2Connection.empty()) {
        return OK;
    }
    CheckError(!scalerInfo, UNKNOWN_ERROR, "%s, scalerInfo is nullptr", __func__);

    for (auto it = edgePort2Connection.begin(); it != edgePort2Connection.end(); ++it) {
        const char* portName;
        bool mpFLag = false;
        bool dpFlag = false;
        bool pppFlag = false;
        float scalerW = 1;
        float scalerH = 1;

        IGraphType::PipelineConnection connection = it->second;
        portName = NODE_NAME(it->first);
        CheckError(!connection.stream, UNKNOWN_ERROR, "%s, connection.stream is null.", __func__);
        int32_t streamId = connection.stream->streamId();
        LOG2("%s, streamId:%d, portName:%s", __func__, streamId, portName);

        if (!strcmp("main", portName)) {
            mpFLag = true;
        } else if (!strcmp("display", portName)) {
            dpFlag = true;
        } else if (!strcmp("postproc", portName)) {
            pppFlag = true;
        }
        if (!mpFLag && !dpFlag && !pppFlag) continue;

        // if port name is main, the value of osW and osH are 1.
        float osW = 1;
        float osH = 1;
        if (dpFlag) {
            (void)getScalerKernelResolutionRatio(dpKernel, DB_KERNEL_SIZE, &osW, &osH);
            LOG2("%s, dp ratio, osW:%f, osH:%f", __func__, osW, osH);
        } else if (pppFlag) {
            (void)getScalerKernelResolutionRatio(pppKernel, PPP_KERNEL_SIZE, &osW, &osH);
            LOG2("%s, ppp ratio, osW:%f, osH:%f", __func__, osW, osH);
        }

        uint32_t kernelId;
        float gdcScalerW = 1;
        float gdcScalerH = 1;
        const ia_isp_bxt_resolution_info_t* gdcResolution = getGdcKernelResolutionInfo(&kernelId);
        if ((gdcResolution) && ((gdcResolution->input_width != gdcResolution->output_width) ||
                                (gdcResolution->input_height != gdcResolution->output_height))) {
            const ia_rectangle* input_crop = &gdcResolution->input_crop;
            const ia_rectangle* output_crop = &gdcResolution->output_crop;
            if (((input_crop->left == 0) && (input_crop->top == 0) && (input_crop->right == 0) &&
                 (input_crop->bottom == 0)) &&
                ((output_crop->left == 0) && (output_crop->top == 0) && (output_crop->right == 0) &&
                 (output_crop->bottom == 0))) {
                gdcScalerW = static_cast<float>(gdcResolution->input_width) /
                             static_cast<float>(gdcResolution->output_width);
                gdcScalerH = static_cast<float>(gdcResolution->input_height) /
                             static_cast<float>(gdcResolution->output_height);
            }
        }
        LOG2("%s, gdc ratio, gdcScalerW:%f, gdcScalerH:%f", __func__, gdcScalerW, gdcScalerH);

        float b2iDsW = 1;
        float b2iDsH = 1;
        (void)getScalerKernelResolutionRatio(dsKernel, DS_KERNEL_SIZE, &b2iDsW, &b2iDsH);
        LOG2("%s, b2iDs ratio, b2iDsW:%f, b2iDsH:%f", __func__, b2iDsW, b2iDsH);

        scalerW = osW * gdcScalerW * b2iDsW;
        scalerH = osH * gdcScalerH * b2iDsH;
        scalerInfo->push_back({streamId, scalerW, scalerH});
        LOG2("%s, streamId:%d, scalerW:%f, scalerH:%f", __func__, streamId, scalerW, scalerH);
    }

    return OK;
}

status_t GraphConfigPipe::portGetOwner(Node* port, IGraphType::ConnectionConfig* connectionInfo) {
    int32_t direction = PORT_DIRECTION_INPUT;
    css_err_t ret = port->getValue(GCSS_KEY_DIRECTION, direction);
    CheckError((ret != css_err_none), BAD_VALUE, "Failed to get port direction");

    /*
     * Default to pull, it will be amended later,
     * Iterations are not used
     */
    connectionInfo->mConnectionType = IGraphType::connection_type_pull;
    connectionInfo->mSinkIteration = 0;
    connectionInfo->mSourceIteration = 0;

    status_t status = OK;
    if (direction == PORT_DIRECTION_INPUT) {
        // input port is the sink in a connection
        status = GCSS::GraphCameraUtil::portGetFourCCInfo(*port, connectionInfo->mSinkStage,
                                                          connectionInfo->mSinkTerminal);
        CheckError((status != OK), BAD_VALUE, "Failed to create fourcc info for sink port");
    } else {
        // output port is the source in a connection
        status = GCSS::GraphCameraUtil::portGetFourCCInfo(*port, connectionInfo->mSourceStage,
                                                          connectionInfo->mSourceTerminal);
        CheckError((status != OK), BAD_VALUE, "Failed to create fourcc info for source port");
    }
    return status;
}

/**
 * Query the connection info structs for a given pipeline defined by
 * stream id.
 *
 * \param[in] sinkName to be used as key to get pipeline connections
 * \param[out] stream id connect with sink
 * \param[out] connections for pipeline configuation
 * \return OK in case of success.
 * \return UNKNOWN_ERROR or BAD_VALUE in case of fail.
 * \if sinkName is not supported, NAME_NOT_FOUND is returned.
 * \sink name support list as below defined in graph_descriptor.xml
 * \<sink name="video0"/>
 * \<sink name="video1"/>
 * \<sink name="video2"/>
 * \<sink name="still0"/>
 * \<sink name="still1"/>
 * \<sink name="still2"/>
 * \<sink name="raw"/>
 */
status_t GraphConfigPipe::pipelineGetConnections(
    const std::string& sinkName, int* streamId,
    std::vector<IGraphType::PipelineConnection>* confVector) {
    CheckError(!streamId, UNKNOWN_ERROR, "the streamId is nullptr");
    HAL_TRACE_CALL(CAMERA_DEBUG_LOG_LEVEL2);

    std::vector<GCSS::IGraphConfig*> sinks;
    NodesPtrVector programGroups;
    NodesPtrVector alreadyConnectedPorts;
    Node* peerPort = nullptr;
    Node* port = nullptr;
    IGraphType::PipelineConnection aConnection;

    alreadyConnectedPorts.clear();
    status_t status = GCSS::GraphCameraUtil::graphGetSinksByName(sinkName, mSettings, sinks);
    if (status != OK || sinks.empty()) {
        LOGD("No %s sinks in graph", sinkName.c_str());
        return NAME_NOT_FOUND;
    }

    status = sinks[0]->getValue(GCSS_KEY_STREAM_ID, *streamId);
    if (status != css_err_none) {
        LOGE("Sink node lacks stream id attribute - fix your config");
        return BAD_VALUE;
    }

    status = streamGetProgramGroups(*streamId, &programGroups);
    if (status != OK || programGroups.empty()) {
        LOGE("No Program groups associated with stream id %d", *streamId);
        return BAD_VALUE;
    }

    for (size_t i = 0; i < programGroups.size(); i++) {
        Node::const_iterator it = programGroups[i]->begin();

        while (it != programGroups[i]->end()) {
            css_err_t ret = programGroups[i]->getDescendant(GCSS_KEY_TYPE, "port", it, &port);
            if (ret != css_err_none) continue;

            // port for private terminal, no need to connect
            int priv = 0;
            ret = port->getValue(GCSS_KEY_PRIVATE, priv);
            if (ret == css_err_none && priv) continue;

            /*
             * Since we are iterating through the ports
             * check if this port is already connected to avoid setting
             * the connection twice
             */
            if (std::find(alreadyConnectedPorts.begin(), alreadyConnectedPorts.end(), port) !=
                alreadyConnectedPorts.end()) {
                continue;
            }
            LOG1("Configuring Port from PG[%zu] in line:%d", i, __LINE__);

            string contentType;
            ret = port->getValue(GCSS_KEY_CONTENT_TYPE, contentType);
            if (ret == css_err_none && contentType != "pixel_data") {
                LOG2("%s skipped content type %s", NODE_NAME(port), contentType.c_str());
                continue;
            }

            status = portGetFormat(port, &(aConnection.portFormatSettings));
            if (status != OK) {
                LOGE(
                    "Failed to get port format info in port from PG[%zu] "
                    "from stream id %d",
                    i, *streamId);
                return BAD_VALUE;
            }
            if (aConnection.portFormatSettings.enabled == 0) {
                LOG1("Port from PG[%zu] from stream id %d disabled", i, *streamId);
                confVector->push_back(aConnection);
                continue;
            } else {
                LOG1("Port: 0x%x format(%dx%d)fourcc: %s bpl: %d bpp: %d",
                     aConnection.portFormatSettings.terminalId,
                     aConnection.portFormatSettings.width, aConnection.portFormatSettings.height,
                     CameraUtils::fourcc2String(aConnection.portFormatSettings.fourcc).c_str(),
                     aConnection.portFormatSettings.bpl, aConnection.portFormatSettings.bpp);
            }

            /*
             * for each port get the connection info and pass it
             * to the pipeline object
             */
            status = portGetConnection(port, &(aConnection.connectionConfig), &peerPort);
            if (status != OK) {
                LOGE(
                    "Failed to create connection info in port from PG[%zu]"
                    "from stream id %d",
                    i, *streamId);
                return BAD_VALUE;
            }

            aConnection.hasEdgePort = false;
            if (isPipeEdgePort(port)) {
                int32_t direction = portGetDirection(port);
                if (direction == GraphConfigPipe::PORT_DIRECTION_INPUT) {
                    aConnection.connectionConfig.mConnectionType = IGraphType::connection_type_push;
                } else {
                    HalStream* clientStream = nullptr;
                    status = portGetClientStream(peerPort, &clientStream);
                    if (status != OK) {
                        LOGE("Failed to find client stream for v-sink");
                        return UNKNOWN_ERROR;
                    }
                    aConnection.stream = clientStream;
                }
                aConnection.hasEdgePort = true;
            }
            confVector->push_back(aConnection);
            alreadyConnectedPorts.push_back(port);
            alreadyConnectedPorts.push_back(peerPort);
        }
    }

    return OK;
}

/**
 * Find distinct stream ids from the graph and return them in a vector.
 * \param streamIds Vector to be populated with stream ids.
 */
status_t GraphConfigPipe::graphGetStreamIds(StreamsVector* streamIds) {
    CheckError(!streamIds, UNKNOWN_ERROR, "%s, The streamIds is nullptr", __func__);
    GraphConfigNode* result;
    int32_t streamId = -1;
    css_err_t ret;

    GraphConfigNode::const_iterator it = mSettings->begin();
    while (it != mSettings->end()) {
        bool found = false;
        // Find all program groups
        ret = mSettings->getDescendant(GCSS_KEY_TYPE, "program_group", it, &result);
        if (ret != css_err_none) continue;

        ret = result->getValue(GCSS_KEY_STREAM_ID, streamId);
        if (ret != css_err_none) continue;

        // If stream id is not yet in vector, add it
        StreamsVector::iterator ite = streamIds->begin();
        for (; ite != streamIds->end(); ++ite) {
            if (streamId == *ite) {
                found = true;
                break;
            }
        }
        if (found) continue;

        streamIds->push_back(streamId);
    }

    LOGG("%s: stream IDs size %d", __func__, streamIds->size());
    return OK;
}

int32_t GraphConfigPipe::portGetStreamId(Node* port) {
    css_err_t ret = css_err_none;
    Node* ancestor = nullptr;
    int32_t streamId = -1;

    if (port == nullptr) {
        LOGE("Invalid Node, cannot get the port stream id");
        return -1;
    }
    ret = port->getAncestor(&ancestor);
    if (ret != css_err_none) {
        LOGE("Failed to get port's ancestor");
        return -1;
    }

    ret = ancestor->getValue(GCSS_KEY_STREAM_ID, streamId);
    if (ret != css_err_none) {
        LOGE("Failed to get stream ID");
        return -1;
    }
    return streamId;
}

/**
 * Retrieve a list of program groups that belong to a  given stream id.
 * Iterates through the graph configuration storing the program groups
 * that match this stream id into the provided vector.
 *
 * \param[in] streamId Id of the stream to match.
 * \param[out] programGroups Vector with the nodes that match the criteria.
 */
status_t GraphConfigPipe::streamGetProgramGroups(int32_t streamId, NodesPtrVector* programGroups) {
    CheckError(!programGroups, UNKNOWN_ERROR, "%s, The programGroups is nullptr", __func__);
    css_err_t ret = css_err_none;
    GraphConfigNode* result;
    NodesPtrVector allProgramGroups;
    int32_t streamIdFound = -1;

    GraphConfigNode::const_iterator it = mSettings->begin();

    while (it != mSettings->end()) {
        ret = mSettings->getDescendant(GCSS_KEY_TYPE, "program_group", it, &result);
        if (ret == css_err_none) allProgramGroups.push_back(result);
    }

    if (allProgramGroups.empty()) {
        LOGE(
            "Failed to find any PG's for stream id %d"
            " BUG(check graph config file)",
            streamId);
        return UNKNOWN_ERROR;
    }

    for (auto& pg : allProgramGroups) {
        ret = pg->getValue(GCSS_KEY_STREAM_ID, streamIdFound);
        if ((ret == css_err_none) && (streamIdFound == streamId)) {
            programGroups->push_back(pg);
        }
    }

    return OK;
}

/**
 * Retrieve the graph config node of the port that is connected to a given port.
 *
 * \param[in] port Node with the info of the port that we want to find its peer.
 * \param[out] peer Pointer to a node where the peer node reference will be
 *                  stored
 * \return OK
 * \return INVALID_OPERATION if the port is disabled.
 * \return BAD_VALUE if any of the graph settings is incorrect.
 * \return NO_ENTRY if including "private" attribute which is used for private
 *         terminal.
 */
status_t GraphConfigPipe::portGetPeer(Node* port, Node** peer) {
    css_err_t ret = css_err_none;
    int32_t enabled = 1, priv = 0;
    string peerName;

    if (port == nullptr || peer == nullptr) {
        LOGE("Invalid Node, cannot get the peer port");
        return BAD_VALUE;
    }
    ret = port->getValue(GCSS_KEY_ENABLED, enabled);
    if (ret == css_err_none && !enabled) {
        LOG1("This port is disabled, keep on getting the connection");
        return INVALID_OPERATION;
    }

    // port for private terminal, no connection needed
    ret = port->getValue(GCSS_KEY_PRIVATE, priv);
    if (ret == css_err_none && priv) {
        LOG2("NO_ENTRY due to key PRIVATE:%d", priv);
        return NO_ENTRY;
    }

    ret = port->getValue(GCSS_KEY_PEER, peerName);
    if (ret != css_err_none) {
        LOGE("Error getting peer attribute");
        return BAD_VALUE;
    }
    ret = mSettings->getDescendantByString(peerName, peer);
    if (ret != css_err_none) {
        LOGE("Failed to find peer by name %s", peerName.c_str());
        return BAD_VALUE;
    }
    return OK;
}

/**
 * Generate the connection configuration information for a given port.
 *
 * This connection configuration  information is required by CIPF to build
 * the pipeline
 *
 * \param[in] port Pointer to the port node
 * \param[out] connectionInfo point to the connection info object
 * \param[out] peerPort Reference to the peer port
 * \return OK in case of success
 * \return BAD_VALUE in case of error while retrieving the information.
 * \return INVALID_OPERATION in case of the port being disabled.
 */
status_t GraphConfigPipe::portGetConnection(Node* port,
                                            IGraphType::ConnectionConfig* connectionInfo,
                                            Node** peerPort) {
    int32_t direction = PORT_DIRECTION_INPUT;

    status_t status = portGetPeer(port, peerPort);
    if (status == NO_ENTRY) {
        LOG2("port for private terminal, no peer");
        *peerPort = nullptr;
    } else if (status != OK) {
        if (status == INVALID_OPERATION) {
            LOGE("Port %s disabled, cannot get the connection", getNodeName(port).c_str());
        } else {
            LOGE("Failed to get the peer port for port %s", getNodeName(port).c_str());
        }
        return status;
    }

    css_err_t ret = port->getValue(GCSS_KEY_DIRECTION, direction);
    if (ret != css_err_none) {
        LOGE("Failed to get port direction");
        return BAD_VALUE;
    }

    /*
     * Default to pull, it will be amended later,
     * Iterations are not used
     */
    connectionInfo->mConnectionType = IGraphType::connection_type_pull;
    connectionInfo->mSinkIteration = 0;
    connectionInfo->mSourceIteration = 0;

    if (direction == PORT_DIRECTION_INPUT) {
        // input port is the sink in a connection
        status = GCSS::GraphCameraUtil::portGetFourCCInfo(port, connectionInfo->mSinkStage,
                                                          connectionInfo->mSinkTerminal);
        if (status != OK) {
            LOGE("Failed to create fourcc info for sink port");
            return BAD_VALUE;
        }
        if (*peerPort != nullptr && !portIsVirtual(*peerPort)) {
            status = GCSS::GraphCameraUtil::portGetFourCCInfo(
                *peerPort, connectionInfo->mSourceStage, connectionInfo->mSourceTerminal);
            if (status != OK) {
                LOGE("Failed to create fourcc info for source port");
                return BAD_VALUE;
            }
        } else {
            connectionInfo->mSourceStage = 0;
            connectionInfo->mSourceTerminal = 0;
        }
    } else {
        // output port is the source in a connection
        status = GCSS::GraphCameraUtil::portGetFourCCInfo(port, connectionInfo->mSourceStage,
                                                          connectionInfo->mSourceTerminal);
        if (status != OK) {
            LOGE("Failed to create fourcc info for source port");
            return BAD_VALUE;
        }

        if (*peerPort != nullptr && !portIsVirtual(*peerPort)) {
            status = GCSS::GraphCameraUtil::portGetFourCCInfo(*peerPort, connectionInfo->mSinkStage,
                                                              connectionInfo->mSinkTerminal);
            if (status != OK) {
                LOGE("Failed to create fourcc info for sink port");
                return BAD_VALUE;
            }
            /**
             * Because all the connections are used for frame flow , so
             * create one implicit rule which sets the mSinkTerminal as
             * same as mSourceTerminal to handle the parameter or
             * hanging port. And then exclude this connection when binding
             * the port of executor.
             */
        } else if (*peerPort != nullptr && portIsVirtual(*peerPort) &&
                   getNodeName(*peerPort).find(getNodeName(port)) != string::npos) {
            LOG2("%s, found one hanging port: %s, peer port: %s", __func__,
                 getNodeName(port).c_str(), getNodeName(*peerPort).c_str());
            connectionInfo->mSinkStage = 0;
            connectionInfo->mSinkTerminal = connectionInfo->mSourceTerminal;
        } else {
            connectionInfo->mSinkStage = 0;
            connectionInfo->mSinkTerminal = 0;
        }
    }

    return status;
}

/**
 * Retrieve the format information of a port
 * if the port doesn't have any format set, it gets the format from the peer
 * port (i.e. the port connected to this one)
 *
 * \param[in] port Port to query the format.
 * \param[out] format Format settings for this port.
 */
status_t GraphConfigPipe::portGetFormat(Node* port, IGraphType::PortFormatSettings* format) {
    GraphConfigNode* peerNode = nullptr;  // The peer port node
    GraphConfigNode* tmpNode = port;      // The port node node we are interrogating
    css_err_t ret = css_err_none;
    ia_uid stageId;  // ignored

    if (port == nullptr) {
        LOGE("Invalid parameter, could not get port format");
        return BAD_VALUE;
    }

    ret = port->getValue(GCSS_KEY_ENABLED, format->enabled);
    if (ret != css_err_none) {
        // if not present by default is enabled
        format->enabled = 1;
    }

    status_t status =
        GCSS::GraphCameraUtil::portGetFourCCInfo(tmpNode, stageId, format->terminalId);
    if (status != OK) {
        LOGE("Could not get port uid");
        return INVALID_OPERATION;
    }

    // if disabled there is no need to query the format
    if (format->enabled == 0) {
        return OK;
    }

    format->width = 0;
    format->height = 0;

    ret = port->getValue(GCSS_KEY_WIDTH, format->width);
    if (ret != css_err_none) {
        /*
         * It could be the port configuration is not in settings, that is normal
         * it means that we need to ask the format from the peer.
         */
        status = portGetPeer(port, &peerNode);
        if (status != OK) {
            LOGE("Could not find peer port - Fix your graph");
            return BAD_VALUE;
        }

        tmpNode = peerNode;

        ret = tmpNode->getValue(GCSS_KEY_WIDTH, format->width);
        if (ret != css_err_none) {
            LOGE("Could not find port format info: width (from peer)");
            return BAD_VALUE;
        }
    }

    ret = tmpNode->getValue(GCSS_KEY_HEIGHT, format->height);
    if (ret != css_err_none) {
        LOGE("Could not find port format info: height");
        return BAD_VALUE;
    }

    string fourccFormat;
    ret = tmpNode->getValue(GCSS_KEY_FORMAT, fourccFormat);
    if (ret != css_err_none) {
        LOGE("Could not find port format info: fourcc");
        return BAD_VALUE;
    }

    const char* pFormat = fourccFormat.c_str();
    format->fourcc = CameraUtils::string2IaFourccCode(pFormat);
    format->bpl = gcu::getBpl(format->fourcc, format->width);
    LOG2("bpl set to %d for %s", format->bpl, fourccFormat.c_str());

    // if settings are specifying bpl, owerwrite the calculated one
    int bplFromSettings = 0;
    ret = tmpNode->getValue(GCSS_KEY_BYTES_PER_LINE, bplFromSettings);
    if (ret == css_err_none) {
        LOG2("Overwriting bpl(%d) from settings %d", format->bpl, bplFromSettings);
        format->bpl = bplFromSettings;
    }

    format->bpp = gcu::getBppFromCommon(format->fourcc);

    return OK;
}

/**
 * Return the port direction
 *
 * \param[in] port Reference to port Graph node
 * \return 0 if it is an input port
 * \return 1 if it is an output port
 */
int32_t GraphConfigPipe::portGetDirection(Node* port) {
    int32_t direction = 0;
    css_err_t ret = port->getValue(GCSS_KEY_DIRECTION, direction);
    if (ret != css_err_none) {
        LOGE("Failed to retrieve port direction, default to input");
    }

    return direction;
}

/**
 * Return the port full name
 * The port full name is made out from:
 * - the name program group it belongs to
 * - the name of the port
 * separated by ":"
 *
 * \param[in] port Reference to port Graph node
 * \param[out] fullName reference to a string to store the full name
 *
 * \return OK if everything went fine.
 * \return BAD_VALUE if any of the graph queries failed.
 */
status_t GraphConfigPipe::portGetFullName(Node* port, string* fullName) {
    CheckError(!fullName, UNKNOWN_ERROR, "%s, the fullName is nullptr", __func__);
    string portName, ancestorName;
    Node* ancestor;
    css_err_t ret = css_err_none;

    if (port == nullptr) {
        LOGE("Invalid parameter, could not get port full name");
        return BAD_VALUE;
    }

    ret = port->getAncestor(&ancestor);
    if (ret != css_err_none) {
        LOGE("Failed to retrieve port ancestor");
        return BAD_VALUE;
    }
    ret = ancestor->getValue(GCSS_KEY_NAME, ancestorName);
    if (ret != css_err_none) {
        LOGE("Failed to get ancestor name for port");
        port->dumpNodeTree(port, 1);
        return BAD_VALUE;
    }

    ret = port->getValue(GCSS_KEY_NAME, portName);
    if (ret != css_err_none) {
        LOGE("Failed to retrieve port name");
        return BAD_VALUE;
    }

    *fullName = ancestorName + ":" + portName;
    return OK;
}

/**
 * Perform a reverse lookup on the map that associates client streams to
 * virtual sinks.
 *
 * This method is used during pipeline configuration to find a stream associated
 * with the id (GCSS key) of the virtual sink
 *
 * \param[in] vPortId GCSS key representing one of the virtual sinks in the
 *                    graph, like GCSS_KEY_VIDEO1
 * \return nullptr if not found
 * \return pointer to the client stream associated with that virtual sink.
 */
HalStream* GraphConfigPipe::getHalStreamByVirtualId(uid_t vPortId) {
    for (auto& it : mStreamToSinkIdMap) {
        if (it.second == vPortId) {
            return it.first;
        }
    }

    return nullptr;
}

/**
 * Return true if the port is a virtual port, this is the end point
 * of the graph.
 * Virtual ports are the nodes of type sink.
 *
 * \param[in] port Reference to port Graph node
 * \return true if it is a virtual port
 * \return false if it is not a virtual port
 */
bool GraphConfigPipe::portIsVirtual(Node* port) {
    string type;
    css_err_t ret = port->getValue(GCSS_KEY_TYPE, type);
    if (ret != css_err_none) {
        LOGE("Failed to retrieve port type, default to input");
    }

    return (type == string("sink"));
}

/**
 * retrieve the pointer to the client stream associated with a virtual sink
 *
 * I.e. access the mapping done at stream config time between the pointers
 * to camera3_stream_t and the names video0, video1, still0 etc...
 *
 * \param[in] port Node to the virtual sink (with name videoX or stillX etc..)
 * \param[out] stream Pointer to the client stream associated with that virtual
 *                    sink.
 * \return OK
 * \return BAD_VALUE in case of invalid parameters (null pointers)
 * \return INVALID_OPERATION in case the Node is not a virtual sink.
 */
status_t GraphConfigPipe::portGetClientStream(Node* port, HalStream** stream) {
    if (!port || !stream) {
        LOGE("Could not get client stream - bad parameters");
        return BAD_VALUE;
    }

    if (!portIsVirtual(port)) {
        LOGE("Trying to find the client stream from a non virtual port");
        return INVALID_OPERATION;
    }

    string portName;
    css_err_t ret = port->getValue(GCSS_KEY_NAME, portName);
    if (ret != css_err_none) {
        LOGE("Failed to get name for port");
        port->dumpNodeTree(port, 1);
        return BAD_VALUE;
    }

    uid_t vPortId = GCSS::ItemUID::str2key(portName);
    *stream = getHalStreamByVirtualId(vPortId);

    return OK;
}

/**
 * A port is at the edge of the video stream (pipeline) if its peer's stream id is 0 or -1,
 * or if its peer is a virtual sink.
 *
 * Here we check for both conditions and return true if this port is at either
 * edge of a pipeline
 */
bool GraphConfigPipe::isPipeEdgePort(Node* port) {
    CheckError(!port, false, "%s, the port is nullptr", __func__);
    Node* peer = nullptr;
    Node* peerAncestor = nullptr;
    int32_t streamId = -1;
    int32_t peerStreamId = -1;
    string peerType;

    int32_t portDirection = portGetDirection(port);

    status_t status = portGetPeer(port, &peer);
    if (status == INVALID_OPERATION) {
        LOG1("port is disabled, so it is an edge port");
        return true;
    }
    if (status != OK) {
        LOGE("Failed to create fourcc info for source port");
        return false;
    }

    streamId = portGetStreamId(port);
    if (streamId < 0) return false;
    /*
     * get the stream id of the peer port
     * we also check the ancestor for that. If the peer is a virtual sink then
     * it does not have ancestor.
     */
    if (!portIsVirtual(peer)) {
        css_err_t ret = peer->getAncestor(&peerAncestor);
        if (ret != css_err_none) {
            LOGE("Failed to get peer's ancestor");
            return false;
        }
        ret = peerAncestor->getValue(GCSS_KEY_STREAM_ID, peerStreamId);
        if (ret != css_err_none) {
            LOGE("Failed to get stream ID of peer PG");
            return false;
        }
        /*
         * Retrieve the type of node the peer ancestor is. It could be is not a
         * program group node but a sink or hw block
         */
        peerAncestor->getValue(GCSS_KEY_TYPE, peerType);
    }

    LOG1("%s port direction: %d, port stream id:%d, peer stream id:%d", __func__, portDirection,
         streamId, peerStreamId);

    bool isEdge = false;
    if (portDirection == GraphConfigPipe::PORT_DIRECTION_INPUT) {
        /*
         *  input port,
         *  if the peer is a source or hw block then it is on the edge,
         *  or its stream id is 0 or -1.
         */
        isEdge = (peerType == string("hw") || peerStreamId == 0 || peerStreamId == -1);
    } else {
        /*
         *  output port,
         *  if the peer is a virtual port, or its stream id is 0 or -1,
         *  then it is on the edge,
         */
        isEdge = (portIsVirtual(peer) || peerStreamId == 0 || peerStreamId == -1);
    }

    return isEdge;
}

void GraphConfigPipe::dumpSettings() {
    mSettings->dumpNodeTree(mSettings, 2);
}

GraphConfigPipe::Rectangle::Rectangle() : w(0), h(0), t(0), l(0) {}
GraphConfigPipe::SubdevPad::SubdevPad() : Rectangle(), mbusFormat(0) {}
GraphConfigPipe::SourceNodeInfo::SourceNodeInfo() : metadataEnabled(false), interlaced(0) {}
}  // namespace icamera
