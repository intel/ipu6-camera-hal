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

#define LOG_TAG "GraphConfig"

#include "src/platformdata/gc/GraphConfig.h"

#include "PlatformData.h"
#include "iutils/CameraLog.h"

using std::vector;
using std::map;
using std::string;

namespace icamera {

GraphConfig::GraphConfig(int32_t camId, ConfigMode mode) :
        mCameraId(camId) {
    mGraphConfigImpl = std::unique_ptr<GraphConfigImpl>(new GraphConfigImpl(camId, mode,
                                                        PlatformData::getGraphSettingsType(camId)));
}

GraphConfig::GraphConfig() :
        mCameraId(-1) {
    mGraphConfigImpl = std::unique_ptr<GraphConfigImpl>(new GraphConfigImpl());
}

GraphConfig::~GraphConfig() {
}

void GraphConfig::addCustomKeyMap() {
    mGraphConfigImpl->addCustomKeyMap();
}

status_t GraphConfig::parse(int cameraId, const char *settingsXmlFile) {
    string graphDescFile = PlatformData::getGraphDescFilePath();
    string settingsFile = PlatformData::getGraphSettingFilePath() + settingsXmlFile;
    return mGraphConfigImpl->parse(cameraId, graphDescFile.c_str(), settingsFile.c_str());
}

void GraphConfig::releaseGraphNodes() {
    mGraphConfigImpl->releaseGraphNodes();
}

status_t GraphConfig::configStreams(const vector<HalStream*> &activeStreams) {
    LOG1("@%s", __func__);

    int ret = mGraphConfigImpl->configStreams(activeStreams);
    CheckError(ret != OK, UNKNOWN_ERROR, "%s, Failed to config streams", __func__);

    ret = mGraphConfigImpl->getGraphConfigData(&mGraphData);
    CheckError(ret != OK, UNKNOWN_ERROR, "%s, Failed to get the static graph config data", __func__);

    return OK;
}

status_t GraphConfig::getGdcKernelSetting(uint32_t *kernelId,
                                          ia_isp_bxt_resolution_info_t *resolution) {
    LOG1("@%s", __func__);
    CheckError(!kernelId || !resolution, UNKNOWN_ERROR, "kernelId or resolution is nullptr");

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

status_t GraphConfig::graphGetStreamIds(vector<int32_t> &streamIds) {
    LOG1("@%s", __func__);
    CheckError(mGraphData.streamIds.empty(), UNKNOWN_ERROR, "%s, The streamIds vector is empty", __func__);

    streamIds = mGraphData.streamIds;
    return OK;
}

int GraphConfig::getStreamIdByPgName(string pgName) {
    LOG1("@%s", __func__);
    CheckError(mGraphData.pgInfo.empty(), -1, "%s, The pgInfo vector is empty", __func__);

    for (auto &info : mGraphData.pgInfo) {
        if (info.pgName == pgName) {
            return info.streamId;
        }
    }

    LOGE("%s, Failed to get stream id for pgName: %s", __func__, pgName.c_str());
    return -1;
}

int GraphConfig::getPgIdByPgName(string pgName) {
    LOG1("@%s", __func__);
    CheckError(mGraphData.pgInfo.empty(), -1, "%s, The pgInfo vector is empty", __func__);

    for (auto &info : mGraphData.pgInfo) {
        if (info.pgName == pgName) {
            return info.pgId;
        }
    }

    LOGE("%s, Failed to get pg id for pgName: %s", __func__, pgName.c_str());
    return -1;
}

ia_isp_bxt_program_group* GraphConfig::getProgramGroup(int32_t streamId) {
    LOG1("@%s", __func__);
    CheckError(mGraphData.programGroup.empty(), nullptr, "%s, The programGroup vector is empty", __func__);

    for (auto &info : mGraphData.programGroup) {
        if (info.streamId == streamId && info.pgPtr != nullptr) {
            return info.pgPtr;
        }
    }

    LOGE("%s, Failed to get programGroup for streamId", __func__, streamId);
    return nullptr;
}

status_t GraphConfig::getMBRData(int32_t streamId, ia_isp_bxt_gdc_limits *data) {
    LOG1("@%s", __func__);
    for (auto &info : mGraphData.mbrInfo) {
        if (streamId == info.streamId) {
            data = &info.data;
            return OK;
        }
    }

    return BAD_VALUE;
}

status_t GraphConfig::getPgNames(vector<string>* pgNames) {
    LOG1("@%s", __func__);
    CheckError(mGraphData.pgNames.empty(), UNKNOWN_ERROR, "%s, The pgNames vector is empty", __func__);

    *pgNames = mGraphData.pgNames;
    return OK;
}

status_t GraphConfig::getPgRbmValue(string pgName, IGraphType::StageAttr *stageAttr) {
    LOG1("@%s", __func__);
    CheckError(mGraphData.pgInfo.empty(), UNKNOWN_ERROR, "%s, The pgInfo vector is empty", __func__);

    for (auto &info : mGraphData.pgInfo) {
        if (info.pgName == pgName && info.rbmValue.rbm != nullptr) {
            *stageAttr = info.rbmValue;
            return OK;
        }
    }

    return BAD_VALUE;
}

int GraphConfig::getProgramGroup(string pgName, ia_isp_bxt_program_group* programGroupForPG) {
    LOG1("@%s", __func__);
    return mGraphConfigImpl->getProgramGroup(pgName, programGroupForPG);
}

status_t GraphConfig::pipelineGetConnections(const vector<string>& pgList,
                                             vector<IGraphType::PipelineConnection> *confVector) {
    LOG1("@%s", __func__);
    CheckError(!confVector, UNKNOWN_ERROR, "%s, The confVector is nullptr", __func__);

    status_t ret;
    std::vector<IGraphType::ScalerInfo> scalerInfo;

    ret = mGraphConfigImpl->pipelineGetConnections(pgList, &scalerInfo, confVector);
    CheckError(ret != OK, ret, "%s, Failed to pipelineGetConnections", __func__);

    CheckError(mCameraId == -1, UNKNOWN_ERROR, "%s: mCameraId is -1", __func__);
    PlatformData::setScalerInfo(mCameraId, scalerInfo);
    return OK;
}

status_t GraphConfig::getPgIdForKernel(const uint32_t streamId, const int32_t kernelId, int32_t *pgId) {
    LOG1("@%s", __func__);
    CheckError(!pgId, UNKNOWN_ERROR, "%s, the pgId is nullptr", __func__);
    return mGraphConfigImpl->getPgIdForKernel(streamId, kernelId, pgId);
}
}  // icamera
