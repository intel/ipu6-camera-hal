/*
 * Copyright (C) 2018-2022 Intel Corporation
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

#define LOG_TAG GraphConfigPipe

#include "modules/algowrapper/graph/GraphConfigPipe.h"

#include <GCSSParser.h>
#include <gcss.h>
#include <gcss_utils.h>
#include <ia_pal_types_isp_ids_autogen.h>

#ifdef CAL_BUILD
#include <cros-camera/v4l2_device.h>
#else
#include <v4l2_device.h>
#endif

#include <algorithm>

#include "GraphUtils.h"
#include "iutils/CameraLog.h"
#include "iutils/Errors.h"
#include "iutils/Utils.h"
#include "linux/media-bus-format.h"

using GCSS::GraphConfigNode;
using std::string;
using std::vector;

namespace icamera {
#define STRINGIFY1(x) #x
#define STRINGIFY(x) STRINGIFY1(x)

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

GraphConfigPipe::GraphConfigPipe(int pipeUseCase) : mSettings(nullptr), mPipeUseCase(pipeUseCase) {
    mCsiOutput = {0, 0};
}

GraphConfigPipe::~GraphConfigPipe() {
    mSinkPeerPort.clear();
    mStreamToSinkIdMap.clear();
    delete mSettings;
    mSettings = nullptr;
    mProgramGroup.clear();
}

const GCSS::IGraphConfig* GraphConfigPipe::getInterface(Node* node) const {
    if (!node) return nullptr;

    return node;
}

int GraphConfigPipe::getGraphId(void) {
    int graphId = -1;
    int ret = mSettings->getValue(GCSS_KEY_ID, graphId);
    CheckAndLogError(ret != css_err_none, BAD_VALUE, "%s, Failed to get graphId", __func__);

    LOG2("%s: graphId %d", __func__, graphId);
    return graphId;
}

/**
 * Prepare graph config once per stream config.
 * \param[in] manager
 * \param[in] settings
 * \param[in] streamToSinkIdMap
 * \param[in] active
 */
status_t GraphConfigPipe::prepare(Node* settings, const StreamToSinkMap& streamToSinkIdMap) {
    CheckAndLogError(!settings, UNKNOWN_ERROR, "%s, settings is nullptr", __func__);

    mSettings = settings;
    status_t ret = analyzeSourceType();
    CheckAndLogError(ret != OK, ret, "%s, Analyze source type fails", __func__);

    ret = getActiveOutputPorts(streamToSinkIdMap);
    CheckAndLogError(ret != OK, ret, "%s, Get Active output port fails", __func__);

    // Options should be updated before kernel list generation
    ret = setPortFormats();
    CheckAndLogError(ret != OK, ret, "%s, Set port format fails", __func__);

    const GCSS::IGraphConfig* gcHandle = getInterface(mSettings);
    css_err_t status = mGCSSAicUtil.initAicUtils(gcHandle);
    CheckAndLogError(status != css_err_none, UNKNOWN_ERROR, "%s, Failed to generate kernel list",
                     __func__);

    calculateSinkDependencies();
    analyzeCSIOutput();

    return ret;
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
        hasSensor = true;
        LOG2("Has sensor node in the graph");
    }

    ret = mSettings->getDescendant(GCSS_KEY_TPG, &inputDevNode);
    if (ret == css_err_none) {
        hasTPG = true;
        LOG2("Has tpg node in the graph");
    }

    CheckAndLogError(hasTPG == hasSensor, UNKNOWN_ERROR,
                     "%s, Both TPG/Sensor exist or none of TPG/Sensor", __func__);

    return OK;
}

status_t GraphConfigPipe::analyzeCSIOutput() {
    vector<string> csiBeOutput = {"csi_be:output",
                                  // DOL_FEATURE_S
                                  "csi_be_dol:output",
                                  // DOL_FEATURE_E
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
        // Get the sinkname for getting the output port
        string sinkName;
        ret = sink->getValue(GCSS_KEY_NAME, sinkName);
        CheckAndLogError(ret != css_err_none, BAD_VALUE, "%s, Failed to get sink name", __func__);

        int32_t streamId = -1;
        ret = sink->getValue(GCSS_KEY_STREAM_ID, streamId);
        CheckAndLogError(ret != css_err_none, BAD_VALUE, "%s, Failed to get stream id", __func__);

        Node* outputPort = getOutputPortForSink(sinkName);
        CheckAndLogError(!outputPort, BAD_VALUE, "%s, No output port found for sink", __func__);

        LOG2("%s, sink name: %s, stream id: %d, output port name: %s", __func__, sinkName.c_str(),
             streamId, NODE_NAME(outputPort));
        mSinkPeerPort[sink] = outputPort;
    }

    return OK;
}

string GraphConfigPipe::getNodeName(Node* node) {
    string nodeName("");
    CheckAndLogError(!node, nodeName, "%s, node is nullptr", __func__);

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
    Node* rootNode = nullptr;
    Node* portNode = nullptr;
    Node* peerNode = nullptr;

    rootNode = mSettings->getRootNode();
    CheckAndLogError(!rootNode, nullptr, "%s, Couldn't get root node", __func__);

    css_err_t ret = rootNode->getDescendantByString(sinkName, &portNode);
    CheckAndLogError(ret != css_err_none, nullptr, "%s, Failed to get sink", __func__);

    status_t retErr = portGetPeer(portNode, &peerNode);
    CheckAndLogError(retErr != OK, nullptr, "%s, Failed to get peer", __func__);

    return peerNode;
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

    CheckAndLogError(!sink, false, "%s, No sink node provided", __func__);

    ret = sink->getValue(GCSS_KEY_NAME, sinkName);
    CheckAndLogError(ret != css_err_none, false, "%s, Failed to get sink name", __func__);

    // Find the client stream for the sink port
    StreamToSinkMap::iterator it1;
    it1 = mStreamToSinkIdMap.begin();

    for (; it1 != mStreamToSinkIdMap.end(); ++it1) {
        if (GCSS::ItemUID::key2str(it1->second) == sinkName) {
            clientStream = it1->first;
            break;
        }
    }
    CheckAndLogError(!clientStream, false, "%s, Failed to find client stream", __func__);

    if (clientStream->useCase() == USE_CASE_VIDEO) {
        LOG2("%s is video record port", NODE_NAME(sink));
        return true;
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
            CheckAndLogError(ret != css_err_none, UNKNOWN_ERROR,
                             "%s, Failed to update options for video record port", __func__);
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
    LOG3("Dump kernel info for stream %d", streamId);
    GraphUtils::dumpKernelInfo(mProgramGroup[streamId]);

    return &(mProgramGroup[streamId]);
}

status_t GraphConfigPipe::getPgRbmValue(string pgName, IGraphType::StageAttr* stageAttr) {
    std::string name;
    GCSS::NodeIterator iter(mSettings);
    GCSS::IGraphConfig* pg = iter.iterateByType(GCSS_KEY_PROGRAM_GROUP);

    for (; pg != nullptr; pg = iter.iterateByType(GCSS_KEY_PROGRAM_GROUP)) {
        css_err_t ret = pg->getValue(GCSS_KEY_NAME, name);
        CheckAndLogError(ret != css_err_none, BAD_VALUE, "Failed to get the name of PG node");
        if (name == pgName) {
            break;
        }
    }
    CheckAndLogError(pg == nullptr, UNKNOWN_ERROR, "Failed to get program groups for PG: %s",
                     pgName.c_str());

    pg = pg->getDescendant(GCSS_KEY_CIPF);
    if (pg == nullptr) return NAME_NOT_FOUND;

    string rbmString;
    css_err_t ret = pg->getValue(GCSS_KEY_RBM, rbmString);
    if (ret != css_err_none) return NAME_NOT_FOUND;

    GCSS::GraphCameraUtil mGCSSCameraUtil;
    void* rbmAddr = mGCSSCameraUtil.numString2binary(rbmString, &stageAttr->rbm_bytes);
    CheckAndLogError(!rbmAddr, NO_MEMORY, "%s get rbm value: %s", __func__, rbmString.c_str());

    if (stageAttr->rbm_bytes > MAX_RBM_STR_SIZE) {
        LOGE("%s, memory is too small to save rbm value: %d, %d", __func__, stageAttr->rbm_bytes,
             MAX_RBM_STR_SIZE);
        stageAttr->rbm_bytes = 0;
        return NO_MEMORY;
    }
    MEMCPY_S(stageAttr->rbm, MAX_RBM_STR_SIZE, rbmAddr, stageAttr->rbm_bytes);
    mGCSSCameraUtil.releaseBinary(rbmAddr);

    return OK;
}

status_t GraphConfigPipe::getScalerKernelResolutionRatio(uint32_t* kenerArray, uint32_t sizeArray,
                                                         float* widthRatio, float* heightRatio) {
    CheckAndLogError(!kenerArray, UNKNOWN_ERROR, "%s the array is null", __func__);
    CheckAndLogError(!widthRatio || !heightRatio, UNKNOWN_ERROR,
                     "%s widthRatio or heightRatio is null", __func__);

    const ia_isp_bxt_resolution_info_t* resolutionInfo;
    resolutionInfo = getScalerKernelResolutionInfo(kenerArray, sizeArray);
    if (!resolutionInfo) return OK;  // no scaling in current setting

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
    CheckAndLogError(!kenerArray, nullptr, "%s the array is null", __func__);

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

bool GraphConfigPipe::getGdcKernelId(uint32_t* kernelId, int32_t streamId) {
    if (isKernelInStream(streamId, ia_pal_uuid_isp_gdc3_1)) {
        LOG2("%s, found gdc3_1 from stream %d", __func__, streamId);
        *kernelId = ia_pal_uuid_isp_gdc3_1;
        return true;
    } else if (isKernelInStream(streamId, ia_pal_uuid_isp_gdc3)) {
        LOG2("%s, found gdc3 from stream %d", __func__, streamId);
        *kernelId = ia_pal_uuid_isp_gdc3;
        return true;
    } else if (isKernelInStream(streamId, ia_pal_uuid_isp_gdc3_1_1)) {
        LOG2("%s, found gdc3_1_1 from stream %d", __func__, streamId);
        *kernelId = ia_pal_uuid_isp_gdc3_1_1;
        return true;
    } else if (isKernelInStream(streamId, ia_pal_uuid_isp_gdc5)) {
        LOG2("%s, found gdc5 from stream %d", __func__, streamId);
        *kernelId = ia_pal_uuid_isp_gdc5;
        return true;
    }

    return false;
}

void GraphConfigPipe::getGdcKernelResolutionInfo(std::vector<IGraphType::GdcInfo>* gdcInfos) {
    CheckAndLogError(!gdcInfos, VOID_VALUE, "%s, the gdcInfos is nullptr", __func__);

    std::vector<int32_t> streamIds;
    // Get all stream IDs
    graphGetStreamIds(&streamIds);
    if (streamIds.empty()) return;

    uint32_t kernelId = ia_pal_uuid_isp_gdc3;
    for (auto streamId : streamIds) {
        bool hasGdcKernel = getGdcKernelId(&kernelId, streamId);
        if (!hasGdcKernel) continue;

        IGraphType::GdcInfo gdcInfo;
        auto info = getKernelResolutionInfo(streamId, kernelId);
        if (info) {
            gdcInfo.streamId = streamId;
            gdcInfo.gdcKernelId = kernelId;
            gdcInfo.gdcReso = *info;
            // push video stream as the first one item
            if (streamId == VIDEO_STREAM_ID) {
                gdcInfos->insert(gdcInfos->begin(), gdcInfo);
            } else {
                gdcInfos->push_back(gdcInfo);
            }

            LOG2("%s: kernel %d, inResolution %dx%d, outResolution %dx%d", __func__, kernelId,
                 info->input_width, info->input_height, info->output_width, info->output_height);
            LOG2("%s: kernel %d, inCrop %d,%d,%d,%d; outCrop %d,%d,%d,%d", __func__, kernelId,
                 info->input_crop.left, info->input_crop.top, info->input_crop.right,
                 info->input_crop.bottom, info->output_crop.left, info->output_crop.top,
                 info->output_crop.right, info->output_crop.bottom);
        }
    }
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
bool GraphConfigPipe::isKernelInStream(int32_t streamId, uint32_t kernelId) {
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
    CheckAndLogError(!pgId, UNKNOWN_ERROR, "%s, the pgId is nullptr", __func__);
    css_err_t ret = css_err_none;
    status_t retErr;
    NodesPtrVector programGroups;

    // Get all program groups with the stream id
    retErr = streamGetProgramGroups(streamId, &programGroups);
    CheckAndLogError(retErr != OK, retErr, "%s, Couldn't get program groups", __func__);

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
            CheckAndLogError(ret != css_err_none, BAD_VALUE, "Couldn't get pg id for kernel: %d",
                             kernelId);

            LOG2("got the pgid:%d for kernel id:%d in stream:%d", *pgId, kernelId, streamId);
            return OK;
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
        LOG2("The stream id: %d for isa output port: %s", streamId, fullName.c_str());
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
    CheckAndLogError(ret != ia_err_none, -1, "Get streamId failed by name:%s, pipeUseCase: %d",
                     pgName.c_str(), mPipeUseCase);

    LOG2("%s: streamId %d, pgName: %s", __func__, streamId, pgName.c_str());
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
    CheckAndLogError(gc == nullptr, -1, "%s, Failed to get graph config interface", __func__);

    int pgId = -1;
    ret = gc->getValue(GCSS_KEY_PG_ID, pgId);
    CheckAndLogError(ret != css_err_none, -1, "Get PG ID failed with:%d", ret);

    LOG2("%s: pgName %s, pgId %d", __func__, pgName.c_str(), pgId);
    return pgId;
}

status_t GraphConfigPipe::getProgramGroupsByName(const std::vector<std::string>& pgNames,
                                                 NodesPtrVector* programGroups) {
    CheckAndLogError(!programGroups, UNKNOWN_ERROR, "%s, The programGroups is nullptr", __func__);
    css_err_t ret = css_err_none;
    GraphConfigNode* result;
    NodesPtrVector allProgramGroups;
    string foundPgName;

    GraphConfigNode::const_iterator it = mSettings->begin();

    while (it != mSettings->end()) {
        ret = mSettings->getDescendant(GCSS_KEY_TYPE, "program_group", it, &result);
        if (ret == css_err_none) allProgramGroups.push_back(result);
    }

    CheckAndLogError(allProgramGroups.empty(), UNKNOWN_ERROR,
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

status_t GraphConfigPipe::getPrivatePortFormat(Node* port,
                                               vector<IGraphType::PrivPortFormat>* tnrPortFormat) {
    if (!tnrPortFormat) return OK;

    string portName;
    css_err_t ret = port->getValue(GCSS_KEY_NAME, portName);
    LOG2("%s, port name: %s", __func__, portName.c_str());
    // Currently, only get the output format for tnr_ref_out
    if (ret != css_err_none || portName.compare("tnr_ref_out")) return OK;

    IGraphType::PrivPortFormat format = {};
    format.streamId = portGetStreamId(port);
    ret = port->getValue(GCSS_KEY_ENABLED, format.formatSetting.enabled);
    if (ret != css_err_none) {
        // if not present by default is enabled
        format.formatSetting.enabled = 1;
    }

    ia_uid stageId;
    status_t status =
        GCSS::GraphCameraUtil::portGetFourCCInfo(port, stageId, format.formatSetting.terminalId);
    CheckAndLogError(status != OK, INVALID_OPERATION, "Failed to get port uid", __func__);
    ret = port->getValue(GCSS_KEY_WIDTH, format.formatSetting.width);
    CheckAndLogError(ret != css_err_none, BAD_VALUE, "Failed to get port width", __func__);
    ret = port->getValue(GCSS_KEY_HEIGHT, format.formatSetting.height);
    CheckAndLogError(ret != css_err_none, BAD_VALUE, "Failed to get port height", __func__);

    string fourccFormat;
    ret = port->getValue(GCSS_KEY_FORMAT, fourccFormat);
    CheckAndLogError(ret != css_err_none, BAD_VALUE, "Failed to find port fourcc", __func__);

    format.formatSetting.fourcc = CameraUtils::string2IaFourccCode(fourccFormat.c_str());
    format.formatSetting.bpl =
        CameraUtils::getBpl(format.formatSetting.fourcc, format.formatSetting.width);
    format.formatSetting.bpp = CameraUtils::getBpp(format.formatSetting.fourcc);

    LOG2("%s, Tnr ref out: streamId: %d, %dx%d, terminalId: %d, fmt: %s, bpp: %d, bpl: %d",
         __func__, format.streamId, format.formatSetting.width, format.formatSetting.height,
         format.formatSetting.terminalId, fourccFormat.c_str(), format.formatSetting.bpp,
         format.formatSetting.bpl);

    tnrPortFormat->push_back(format);

    return OK;
}

status_t GraphConfigPipe::pipelineGetConnections(
    const std::vector<std::string>& pgList, std::vector<IGraphType::ScalerInfo>* scalerInfo,
    std::vector<IGraphType::PipelineConnection>* confVector,
    std::vector<IGraphType::PrivPortFormat>* tnrPortFormat) {
    CheckAndLogError(!confVector, UNKNOWN_ERROR, "%s, the confVector is nullptr", __func__);

    NodesPtrVector programGroups;
    NodesPtrVector alreadyConnectedPorts;
    Node* peerPort = nullptr;
    Node* port = nullptr;
    IGraphType::PipelineConnection aConnection;
    std::map<Node*, IGraphType::PipelineConnection> edgePort2Connection;

    status_t status = getProgramGroupsByName(pgList, &programGroups);
    CheckAndLogError(status != OK, status, "%s, failed to get program groups, BUG", __func__);

    for (size_t i = 0; i < programGroups.size(); i++) {
        Node::const_iterator it = programGroups[i]->begin();

        while (it != programGroups[i]->end()) {
            css_err_t ret = programGroups[i]->getDescendant(GCSS_KEY_TYPE, "port", it, &port);
            if (ret != css_err_none) continue;

            // port for private terminal, no need to connect
            int priv = 0;
            ret = port->getValue(GCSS_KEY_PRIVATE, priv);
            if (ret == css_err_none && priv) {
                status = getPrivatePortFormat(port, tnrPortFormat);
                CheckAndLogError(status != OK, status, "%s, failed to get private port format",
                                 __func__);
                continue;
            }

            /*
             * Since we are iterating through the ports
             * check if this port is already connected to avoid setting
             * the connection twice
             */
            if (std::find(alreadyConnectedPorts.begin(), alreadyConnectedPorts.end(), port) !=
                alreadyConnectedPorts.end()) {
                continue;
            }
            LOG2("Configuring Port from PG[%zu] in line:%d", i, __LINE__);

            string contentType;
            ret = port->getValue(GCSS_KEY_CONTENT_TYPE, contentType);
            if (ret == css_err_none && contentType != "pixel_data") {
                LOG2("%s skipped content type %s", NODE_NAME(port), contentType.c_str());
                continue;
            }

            status = portGetFormat(port, &(aConnection.portFormatSettings));
            CheckAndLogError(status != OK, BAD_VALUE, "Failed to get format info, PG[%zu]", i);

            if (aConnection.portFormatSettings.enabled == 0) {
                LOG2("Port from PG[%zu] disabled", i);
                status = portGetOwner(port, &(aConnection.connectionConfig));
                CheckAndLogError((status != OK), BAD_VALUE,
                                 "Failed to get ownerfor port from PG[%zu]", i);
                confVector->push_back(aConnection);
                continue;
            } else {
                LOG2("Port: 0x%x format(%dx%d)fourcc: %s bpl: %d bpp: %d",
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
            CheckAndLogError(status != OK, BAD_VALUE,
                             "Failed to create connection info in port from PG[%zu]", i);

            aConnection.hasEdgePort = false;
            if (portIsEdgePort(port)) {
                int32_t direction = portGetDirection(port);
                if (direction == GraphConfigPipe::PORT_DIRECTION_INPUT) {
                    aConnection.connectionConfig.mConnectionType = IGraphType::connection_type_push;
                } else {
                    HalStream* clientStream = nullptr;
                    status = portGetClientStream(peerPort, &clientStream);
                    CheckAndLogError(status != OK, UNKNOWN_ERROR,
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
    CheckAndLogError(!scalerInfo, UNKNOWN_ERROR, "%s, scalerInfo is nullptr", __func__);

    for (auto it = edgePort2Connection.begin(); it != edgePort2Connection.end(); ++it) {
        const char* portName;
        bool mpFLag = false;
        bool dpFlag = false;
        bool pppFlag = false;
        float scalerW = 1;
        float scalerH = 1;

        IGraphType::PipelineConnection connection = it->second;
        portName = NODE_NAME(it->first);
        CheckAndLogError(!connection.stream, UNKNOWN_ERROR, "%s, connection.stream is null.",
                         __func__);
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

        float gdcScalerW = 1;
        float gdcScalerH = 1;
        std::vector<IGraphType::GdcInfo> gdcInfos;
        getGdcKernelResolutionInfo(&gdcInfos);
        if (!gdcInfos.empty()) {
            auto& gdcResolution = gdcInfos.begin()->gdcReso;
            if ((gdcResolution.input_width != gdcResolution.output_width) ||
                (gdcResolution.input_height != gdcResolution.output_height)) {
                const ia_rectangle* input_crop = &gdcResolution.input_crop;
                const ia_rectangle* output_crop = &gdcResolution.output_crop;
                if (((input_crop->left == 0) && (input_crop->top == 0) &&
                     (input_crop->right == 0) && (input_crop->bottom == 0)) &&
                    ((output_crop->left == 0) && (output_crop->top == 0) &&
                     (output_crop->right == 0) &&(output_crop->bottom == 0))) {
                    gdcScalerW = static_cast<float>(gdcResolution.input_width) /
                                 static_cast<float>(gdcResolution.output_width);
                    gdcScalerH = static_cast<float>(gdcResolution.input_height) /
                                 static_cast<float>(gdcResolution.output_height);
                }
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
    CheckAndLogError((ret != css_err_none), BAD_VALUE, "Failed to get port direction");

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
        CheckAndLogError((status != OK), BAD_VALUE, "Failed to create fourcc info for sink port");
    } else {
        // output port is the source in a connection
        status = GCSS::GraphCameraUtil::portGetFourCCInfo(*port, connectionInfo->mSourceStage,
                                                          connectionInfo->mSourceTerminal);
        CheckAndLogError((status != OK), BAD_VALUE, "Failed to create fourcc info for source port");
    }
    return status;
}

/**
 * Find distinct stream ids from the graph and return them in a vector.
 * \param streamIds Vector to be populated with stream ids.
 */
status_t GraphConfigPipe::graphGetStreamIds(StreamsVector* streamIds) {
    CheckAndLogError(!streamIds, UNKNOWN_ERROR, "%s, The streamIds is nullptr", __func__);
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

    LOG2("%s: stream IDs size %zu", __func__, streamIds->size());
    return OK;
}

int32_t GraphConfigPipe::getTuningMode(const int32_t streamId) {
    GraphConfigNode* result = nullptr;
    int32_t tuningMode = -1, graphStreamId = -1;
    GraphConfigNode::const_iterator it = mSettings->begin();

    while (it != mSettings->end()) {
        // Get the tuning mode by psys streamId
        css_err_t ret = mSettings->getDescendant(GCSS_KEY_TYPE, "program_group", it, &result);
        if (ret != css_err_none) continue;

        ret = result->getValue(GCSS_KEY_STREAM_ID, graphStreamId);
        if (ret == css_err_none && graphStreamId == streamId && graphStreamId != -1) {
            GraphConfigNode* tuningModeNode = nullptr;
            ret = result->getDescendant(GCSS_KEY_TUNING_MODE, &tuningModeNode);
            if (ret == css_err_none && tuningModeNode) {
                string tuningModeStr;
                ret = tuningModeNode->getValue(GCSS_KEY_VALUE, tuningModeStr);
                if (ret == css_err_none && !tuningModeStr.empty()) {
                    tuningMode = atoi(tuningModeStr.c_str());
                    LOG2("%s, streamId: %d, tuningMode: %d", __func__, streamId, tuningMode);
                    break;
                }
            }
        }
    }

    return tuningMode;
}

int32_t GraphConfigPipe::portGetStreamId(Node* port) {
    CheckAndLogError(!port, -1, "Invalid Node, cannot get the port stream id");

    Node* ancestor = nullptr;
    css_err_t ret = port->getAncestor(&ancestor);
    CheckAndLogError(ret != css_err_none, -1, "%s, Failed to get port's ancestor", __func__);

    int32_t streamId = -1;
    ret = ancestor->getValue(GCSS_KEY_STREAM_ID, streamId);
    CheckAndLogError(ret != css_err_none, -1, "%s, Failed to get stream id", __func__);

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
    CheckAndLogError(!programGroups, UNKNOWN_ERROR, "%s, The programGroups is nullptr", __func__);
    css_err_t ret = css_err_none;
    GraphConfigNode* result;
    NodesPtrVector allProgramGroups;
    int32_t streamIdFound = -1;

    GraphConfigNode::const_iterator it = mSettings->begin();

    while (it != mSettings->end()) {
        ret = mSettings->getDescendant(GCSS_KEY_TYPE, "program_group", it, &result);
        if (ret == css_err_none) allProgramGroups.push_back(result);
    }

    CheckAndLogError(allProgramGroups.empty(), UNKNOWN_ERROR,
                     "Failed to find any PG's for stream id: %d", streamId);

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

    CheckAndLogError(port == nullptr || peer == nullptr, BAD_VALUE,
                     "%s, Invalid Node, cannot get the peer port", __func__);

    ret = port->getValue(GCSS_KEY_ENABLED, enabled);
    if (ret == css_err_none && !enabled) {
        LOG2("This port is disabled, keep on getting the connection");
        return INVALID_OPERATION;
    }

    // port for private terminal, no connection needed
    ret = port->getValue(GCSS_KEY_PRIVATE, priv);
    if (ret == css_err_none && priv) {
        LOG2("NO_ENTRY due to key PRIVATE:%d", priv);
        return NO_ENTRY;
    }

    ret = port->getValue(GCSS_KEY_PEER, peerName);
    CheckAndLogError(ret != css_err_none, BAD_VALUE, "%s, Failed to get peer attribute", __func__);

    ret = mSettings->getDescendantByString(peerName, peer);
    CheckAndLogError(ret != css_err_none, BAD_VALUE, "%s, Failed to find peer by name %s", __func__,
                     peerName.c_str());

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
        LOGE("Cannot get the connection for port: %s", getNodeName(port).c_str());
        return status;
    }

    css_err_t ret = port->getValue(GCSS_KEY_DIRECTION, direction);
    CheckAndLogError(ret != css_err_none, BAD_VALUE, "%s, Failed to get port direction", __func__);

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
        CheckAndLogError(status != OK, BAD_VALUE, "%s, Failed to create fourcc info for sink port",
                         __func__);

        if (*peerPort != nullptr && !portIsVirtual(*peerPort)) {
            status = GCSS::GraphCameraUtil::portGetFourCCInfo(
                *peerPort, connectionInfo->mSourceStage, connectionInfo->mSourceTerminal);
            CheckAndLogError(status != OK, BAD_VALUE,
                             "%s, Failed to create fourcc info for source port", __func__);
        } else {
            connectionInfo->mSourceStage = 0;
            connectionInfo->mSourceTerminal = 0;
        }
    } else {
        // output port is the source in a connection
        status = GCSS::GraphCameraUtil::portGetFourCCInfo(port, connectionInfo->mSourceStage,
                                                          connectionInfo->mSourceTerminal);
        CheckAndLogError(status != OK, BAD_VALUE, "%s, Failed to create fourcc info for sink port",
                         __func__);

        if (*peerPort != nullptr && !portIsVirtual(*peerPort)) {
            status = GCSS::GraphCameraUtil::portGetFourCCInfo(*peerPort, connectionInfo->mSinkStage,
                                                              connectionInfo->mSinkTerminal);
            CheckAndLogError(status != OK, BAD_VALUE,
                             "%s, Failed to create fourcc info for source port", __func__);
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

    CheckAndLogError(!port, BAD_VALUE, "%s, Could not get port format", __func__);
    ret = port->getValue(GCSS_KEY_ENABLED, format->enabled);
    if (ret != css_err_none) {
        // if not present by default is enabled
        format->enabled = 1;
    }

    status_t status =
        GCSS::GraphCameraUtil::portGetFourCCInfo(tmpNode, stageId, format->terminalId);
    CheckAndLogError(status != OK, INVALID_OPERATION, "Could not get port uid", __func__);

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
        CheckAndLogError(status != OK, BAD_VALUE, "Could not find peer port", __func__);
        tmpNode = peerNode;

        ret = tmpNode->getValue(GCSS_KEY_WIDTH, format->width);
        CheckAndLogError(ret != css_err_none, BAD_VALUE, "Could not find port width", __func__);
    }

    ret = tmpNode->getValue(GCSS_KEY_HEIGHT, format->height);
    CheckAndLogError(ret != css_err_none, BAD_VALUE, "Could not find port height", __func__);

    string fourccFormat;
    ret = tmpNode->getValue(GCSS_KEY_FORMAT, fourccFormat);
    CheckAndLogError(ret != css_err_none, BAD_VALUE, "Could not find port fourcc", __func__);

    const char* pFormat = fourccFormat.c_str();
    format->fourcc = CameraUtils::string2IaFourccCode(pFormat);
    format->bpl = CameraUtils::getBpl(format->fourcc, format->width);
    LOG2("bpl set to %d for %s", format->bpl, fourccFormat.c_str());

    // if settings are specifying bpl, owerwrite the calculated one
    int bplFromSettings = 0;
    ret = tmpNode->getValue(GCSS_KEY_BYTES_PER_LINE, bplFromSettings);
    if (ret == css_err_none) {
        LOG2("Overwriting bpl(%d) from settings %d", format->bpl, bplFromSettings);
        format->bpl = bplFromSettings;
    }

    format->bpp = CameraUtils::getBpp(format->fourcc);

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
    CheckAndLogError(!fullName || !port, UNKNOWN_ERROR, "%s, the fullName or port is nullptr",
                     __func__);
    string portName, ancestorName;
    Node* ancestor;
    css_err_t ret = css_err_none;

    ret = port->getAncestor(&ancestor);
    CheckAndLogError(ret != css_err_none, BAD_VALUE, "%s, Failed to retrieve port ancestor",
                     __func__);

    ret = ancestor->getValue(GCSS_KEY_NAME, ancestorName);
    if (ret != css_err_none) {
        LOGE("Failed to get ancestor name for port");
        port->dumpNodeTree(port, 1);
        return BAD_VALUE;
    }

    ret = port->getValue(GCSS_KEY_NAME, portName);
    CheckAndLogError(ret != css_err_none, BAD_VALUE, "%s, Failed to retrieve port name", __func__);

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
    CheckAndLogError(!port || !stream, BAD_VALUE, "%s, Invalid parameters", __func__);
    CheckAndLogError(!portIsVirtual(port), INVALID_OPERATION, "%s, port is not a virtual port",
                     __func__);

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
bool GraphConfigPipe::portIsEdgePort(Node* port) {
    CheckAndLogError(!port, false, "%s, the port is nullptr", __func__);
    Node* peer = nullptr;
    Node* peerAncestor = nullptr;
    int32_t streamId = -1;
    int32_t peerStreamId = -1;
    string peerType;

    int32_t portDirection = portGetDirection(port);

    status_t status = portGetPeer(port, &peer);
    if (status == INVALID_OPERATION) {
        LOG2("port is disabled, so it is an edge port");
        return true;
    }
    CheckAndLogError(status != OK, false, "%s, Failed to create fourcc info for source port",
                     __func__);

    streamId = portGetStreamId(port);
    if (streamId < 0) return false;
    /*
     * get the stream id of the peer port
     * we also check the ancestor for that. If the peer is a virtual sink then
     * it does not have ancestor.
     */
    if (!portIsVirtual(peer)) {
        css_err_t ret = peer->getAncestor(&peerAncestor);
        CheckAndLogError(ret != css_err_none, false, "%s, Failed to get ancestor", __func__);

        ret = peerAncestor->getValue(GCSS_KEY_STREAM_ID, peerStreamId);
        CheckAndLogError(ret != css_err_none, false, "%s, Failed to get stream id", __func__);

        /*
         * Retrieve the type of node the peer ancestor is. It could be is not a
         * program group node but a sink or hw block
         */
        peerAncestor->getValue(GCSS_KEY_TYPE, peerType);
    }

    LOG2("%s port direction: %d, port stream id:%d, peer stream id:%d", __func__, portDirection,
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

// DOL_FEATURE_S
int GraphConfigPipe::getDolInfo(float* gain, string* mode) {
    CheckAndLogError(!gain || !mode, UNKNOWN_ERROR, "%s, the gain or mode is nullptr", __func__);

    css_err_t status = mGCSSAicUtil.getDolInfo(*gain, *mode);
    CheckAndLogError(status != css_err_none, UNKNOWN_ERROR, "%s, Get DOL info fails", __func__);

    return OK;
}
// DOL_FEATURE_E

void GraphConfigPipe::dumpSettings() {
    mSettings->dumpNodeTree(mSettings, 2);
}

}  // namespace icamera
