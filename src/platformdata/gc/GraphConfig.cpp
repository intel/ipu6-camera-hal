/*
 * Copyright (C) 2019-2021 Intel Corporation
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

#define LOG_TAG GraphConfig

#include "src/platformdata/gc/GraphConfig.h"

#include "PlatformData.h"
#include "iutils/CameraLog.h"

using std::map;
using std::string;
using std::vector;

namespace icamera {

GraphConfig::GraphConfig(int32_t camId, ConfigMode mode) : mCameraId(camId) {
    mGraphConfigImpl = std::unique_ptr<GraphConfigImpl>(
        new GraphConfigImpl(camId, mode, PlatformData::getGraphSettingsType(camId)));
}

GraphConfig::GraphConfig() : mCameraId(-1) {
    mGraphConfigImpl = std::unique_ptr<GraphConfigImpl>(new GraphConfigImpl());
}

GraphConfig::~GraphConfig() {}

void GraphConfig::addCustomKeyMap() {
    mGraphConfigImpl->addCustomKeyMap();
}

status_t GraphConfig::parse(int cameraId, const char* settingsXmlFile) {
    string graphDescFile = PlatformData::getGraphDescFilePath();
    string settingsFile = PlatformData::getGraphSettingFilePath() + settingsXmlFile;
    return mGraphConfigImpl->parse(cameraId, graphDescFile.c_str(), settingsFile.c_str());
}

void GraphConfig::releaseGraphNodes() {
    mGraphConfigImpl->releaseGraphNodes();
}

/*
 * Query graph setting according to activeStreams
 */
status_t GraphConfig::queryGraphSettings(const std::vector<HalStream*>& activeStreams) {
    bool ret = mGraphConfigImpl->queryGraphSettings(activeStreams);

    return ret ? OK : NO_ENTRY;
}

status_t GraphConfig::configStreams(const vector<HalStream*>& activeStreams) {
    LOG1("@%s", __func__);

    bool dummyStillSink = PlatformData::isDummyStillSink(mCameraId);
    int ret = mGraphConfigImpl->configStreams(activeStreams, dummyStillSink);
    CheckAndLogError(ret != OK, UNKNOWN_ERROR, "%s, Failed to config streams", __func__);

    ret = mGraphConfigImpl->getGraphConfigData(&mGraphData);
    CheckAndLogError(ret != OK, UNKNOWN_ERROR, "%s, Failed to get the static graph config data",
                     __func__);

    return OK;
}

status_t GraphConfig::getGdcKernelSetting(uint32_t* kernelId,
                                          ia_isp_bxt_resolution_info_t* resolution) {
    CheckAndLogError(!kernelId || !resolution, UNKNOWN_ERROR, "kernelId or resolution is nullptr");

    if ((mGraphData.gdcReso.input_width == 0) || (mGraphData.gdcReso.input_height == 0) ||
        (mGraphData.gdcReso.output_width == 0) || (mGraphData.gdcReso.output_height == 0)) {
        LOG2("%s, Failed to get gdc InReso: w: %d, h: %d; OutReso: w: %d, h: %d; ", __func__,
             mGraphData.gdcReso.input_width, mGraphData.gdcReso.input_height,
             mGraphData.gdcReso.output_width, mGraphData.gdcReso.output_height);
        return NO_ENTRY;
    }

    *kernelId = mGraphData.gdcKernelId;
    *resolution = mGraphData.gdcReso;

    return OK;
}

status_t GraphConfig::graphGetStreamIds(vector<int32_t>& streamIds) {
    CheckAndLogError(mGraphData.streamIds.empty(), UNKNOWN_ERROR,
                     "%s, The streamIds vector is empty", __func__);

    streamIds = mGraphData.streamIds;
    return OK;
}

int GraphConfig::getStreamIdByPgName(string pgName) {
    CheckAndLogError(mGraphData.pgInfo.empty(), -1, "%s, The pgInfo vector is empty", __func__);

    for (auto& info : mGraphData.pgInfo) {
        if (info.pgName == pgName) {
            return info.streamId;
        }
    }

    LOG2("%s, There is not stream id for pgName: %s", __func__, pgName.c_str());
    return -1;
}

int GraphConfig::getPgIdByPgName(string pgName) {
    CheckAndLogError(mGraphData.pgInfo.empty(), -1, "%s, The pgInfo vector is empty", __func__);

    for (auto& info : mGraphData.pgInfo) {
        if (info.pgName == pgName) {
            return info.pgId;
        }
    }

    LOG2("%s, There is not pg id for pgName: %s", __func__, pgName.c_str());
    return -1;
}

ia_isp_bxt_program_group* GraphConfig::getProgramGroup(int32_t streamId) {
    CheckAndLogError(mGraphData.programGroup.empty(), nullptr,
                     "%s, The programGroup vector is empty", __func__);

    for (auto& info : mGraphData.programGroup) {
        if (info.streamId == streamId && info.pgPtr != nullptr) {
            return info.pgPtr;
        }
    }

    LOGE("%s, Failed to get programGroup for streamId %d", __func__, streamId);
    return nullptr;
}

status_t GraphConfig::getMBRData(int32_t streamId, ia_isp_bxt_gdc_limits* data) {
    for (auto& info : mGraphData.mbrInfo) {
        if (streamId == info.streamId) {
            data = &info.data;
            return OK;
        }
    }

    return BAD_VALUE;
}

status_t GraphConfig::getPgNames(vector<string>* pgNames) {
    CheckAndLogError(mGraphData.pgNames.empty(), UNKNOWN_ERROR, "%s, The pgNames vector is empty",
                     __func__);

    *pgNames = mGraphData.pgNames;
    return OK;
}

status_t GraphConfig::getPgRbmValue(string pgName, IGraphType::StageAttr* stageAttr) {
    CheckAndLogError(mGraphData.pgInfo.empty(), UNKNOWN_ERROR, "%s, The pgInfo vector is empty",
                     __func__);

    for (auto& info : mGraphData.pgInfo) {
        if (info.pgName == pgName && info.rbmValue.rbm_bytes > 0) {
            stageAttr->rbm_bytes = info.rbmValue.rbm_bytes;
            MEMCPY_S(stageAttr->rbm, MAX_RBM_STR_SIZE, info.rbmValue.rbm, stageAttr->rbm_bytes);
            return OK;
        }
    }

    return BAD_VALUE;
}

status_t GraphConfig::pipelineGetConnections(
    const vector<string>& pgList,
    vector<IGraphType::PipelineConnection>* confVector,
    std::vector<IGraphType::PrivPortFormat>* tnrPortFormat) {
    CheckAndLogError(!confVector, UNKNOWN_ERROR, "%s, The confVector is nullptr", __func__);

    status_t ret;
    std::vector<IGraphType::ScalerInfo> scalerInfo;

    ret = mGraphConfigImpl->pipelineGetConnections(pgList, &scalerInfo, confVector, tnrPortFormat);
    CheckAndLogError(ret != OK, ret, "%s, Failed to pipelineGetConnections", __func__);

    CheckAndLogError(mCameraId == -1, UNKNOWN_ERROR, "%s: mCameraId is -1", __func__);
    PlatformData::setScalerInfo(mCameraId, scalerInfo);
    return OK;
}

status_t GraphConfig::getPgIdForKernel(const uint32_t streamId, const int32_t kernelId,
                                       int32_t* pgId) {
    CheckAndLogError(!pgId, UNKNOWN_ERROR, "%s, the pgId is nullptr", __func__);
    return mGraphConfigImpl->getPgIdForKernel(streamId, kernelId, pgId);
}
}  // namespace icamera
