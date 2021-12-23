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

#define LOG_TAG GraphConfigServer

#include "modules/sandboxing/server/GraphConfigServer.h"

#include <string>
#include <vector>

#include "CameraLog.h"
#include "iutils/Utils.h"

namespace icamera {
GraphConfigServer::GraphConfigServer() {
    LOG1("@%s", __func__);
}

GraphConfigServer::~GraphConfigServer() {
    LOG1("@%s", __func__);
}

void GraphConfigServer::addCustomKeyMap() {
    std::shared_ptr<GraphConfigImpl> graphConfigImpl = std::make_shared<GraphConfigImpl>();
    graphConfigImpl->addCustomKeyMap();
}

status_t GraphConfigServer::parse(void* pData, size_t dataSize) {
    CheckAndLogError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);

    GraphParseParams* parseParam = nullptr;
    bool ret = mIpc.serverUnflattenParse(pData, dataSize, &parseParam);
    CheckAndLogError(ret == false, UNKNOWN_ERROR, "@%s, serverUnflattenParse fails", __func__);

    std::shared_ptr<GraphConfigImpl> graphConfigImpl = std::make_shared<GraphConfigImpl>();
    status_t rt = graphConfigImpl->parse(parseParam->cameraId, parseParam->GD, parseParam->gdSize,
                                         parseParam->GS, parseParam->gsSize);
    CheckAndLogError(rt != OK, UNKNOWN_ERROR, "@%s, Failed to parse the graph xml data", __func__);

    return OK;
}

void GraphConfigServer::releaseGraphNodes() {
    if (mGraphConfigMap.empty()) return;
    mGraphConfigMap.begin()->second->releaseGraphNodes();
}

status_t GraphConfigServer::queryGraphSettings(void* pData, size_t dataSize) {
    CheckAndLogError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);

    GraphBaseInfo info;
    GraphSettingType type;
    std::vector<HalStream*> streams;
    bool dummyStillSink;
    bool ret = mIpc.serverUnflattenConfigStreams(pData, dataSize, &info, &type, &dummyStillSink,
                                                 &streams);
    CheckAndLogError(ret == false, UNKNOWN_ERROR, "@%s, clientFlattenConfigStreams fails",
                     __func__);

    GraphQueryGraphParams* params = static_cast<GraphQueryGraphParams*>(pData);
    std::shared_ptr<GraphConfigImpl> graphConfigImpl =
        std::make_shared<GraphConfigImpl>(info.cameraId, info.configMode, type);
    params->isHasGraphSettings = graphConfigImpl->queryGraphSettings(streams);

    return OK;
}

status_t GraphConfigServer::configStreams(void* pData, size_t dataSize) {
    CheckAndLogError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);

    GraphBaseInfo info;
    GraphSettingType type;
    std::vector<HalStream*> streams;
    bool dummyStillSink;
    bool ret = mIpc.serverUnflattenConfigStreams(pData, dataSize, &info, &type,
                                                 &dummyStillSink, &streams);
    CheckAndLogError(ret == false, UNKNOWN_ERROR, "@%s, serverUnflattenConfigStreams fails",
                     __func__);

    // release the old item
    auto it = mGraphConfigMap.find(info);
    if (it != mGraphConfigMap.end()) {
        mGraphConfigMap.erase(it);
    }
    LOG1("<id%d> @%s configMode: %d, settingType: %d, dummyStillSink: %d",
         info.cameraId, __func__, info.configMode, type, dummyStillSink);
    std::shared_ptr<GraphConfigImpl> graphConfigImpl =
        std::make_shared<GraphConfigImpl>(info.cameraId, info.configMode, type);
    status_t rt = graphConfigImpl->configStreams(streams, dummyStillSink);
    CheckAndLogError(rt != OK, ret, "<id%d> @%s, Failed to configStreams, configMode: %d",
                     info.cameraId, __func__, info.configMode);

    mGraphConfigMap[info] = graphConfigImpl;

    return OK;
}

status_t GraphConfigServer::getGraphConfigData(void* pData, size_t dataSize) {
    CheckAndLogError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);

    GraphBaseInfo info;
    bool ret = mIpc.serverUnflattenGetGraphData(pData, dataSize, &info);
    CheckAndLogError(ret == false, UNKNOWN_ERROR, "@%s, serverUnflattenGetGraphData fails",
                     __func__);

    auto it = mGraphConfigMap.find(info);
    CheckAndLogError(it == mGraphConfigMap.end(), UNKNOWN_ERROR,
                     "<id%d> @%s, Failed to find the graph config", info.cameraId, __func__);

    IGraphType::GraphConfigData graphData;
    status_t rt = it->second->getGraphConfigData(&graphData);
    CheckAndLogError(rt != OK, UNKNOWN_ERROR, "<id%d> @%s, Failed to getGraphConfigData",
                     info.cameraId, __func__);

    ret = mIpc.serverFlattenGetGraphData(pData, dataSize, graphData);
    CheckAndLogError(ret == false, UNKNOWN_ERROR, "@%s, serverFlattenGetGraphData fails", __func__);

    return OK;
}

status_t GraphConfigServer::getPgIdForKernel(void* pData, size_t dataSize) {
    CheckAndLogError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);

    uint32_t streamId = -1;
    int32_t kernelId = 0;
    GraphBaseInfo info;
    bool ret = mIpc.serverUnFlattenGetPgId(pData, dataSize, &info, &streamId, &kernelId);
    CheckAndLogError(ret == false, UNKNOWN_ERROR, "@%s, serverUnFlattenGetPgId fails", __func__);

    auto it = mGraphConfigMap.find(info);
    CheckAndLogError(it == mGraphConfigMap.end(), UNKNOWN_ERROR,
                     "<id%d> @%s, Failed to find the graph config", info.cameraId, __func__);

    int32_t pgId = -1;
    it->second->getPgIdForKernel(streamId, kernelId, &pgId);

    ret = mIpc.serverFlattenGetPgId(pData, dataSize, pgId);
    CheckAndLogError(ret == false, UNKNOWN_ERROR, "@%s, serverFlattenGetPgId fails", __func__);

    return OK;
}

status_t GraphConfigServer::pipelineGetConnections(void* pData, size_t dataSize) {
    CheckAndLogError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);

    GraphBaseInfo info;
    std::vector<std::string> pgList;
    bool ret = mIpc.serverUnFlattenGetConnection(pData, dataSize, &info, &pgList);
    CheckAndLogError(ret == false, UNKNOWN_ERROR, "@%s, serverUnFlattenGetPgId fails", __func__);

    auto it = mGraphConfigMap.find(info);
    CheckAndLogError(it == mGraphConfigMap.end(), UNKNOWN_ERROR,
                     "<id%d> @%s, Failed to find the graph config", info.cameraId, __func__);

    std::vector<IGraphType::PipelineConnection> confVector;
    std::vector<IGraphType::ScalerInfo> scalerInfo;
    std::vector<IGraphType::PrivPortFormat> tnrPortFormat;
    status_t rt = it->second->pipelineGetConnections(pgList, &scalerInfo,
                                                     &confVector, &tnrPortFormat);
    CheckAndLogError(rt != OK, UNKNOWN_ERROR, "<id%d> @%s, Failed to getConnection",
                     info.cameraId, __func__);

    ret = mIpc.serverFlattenGetConnection(pData, dataSize, scalerInfo,
                                          confVector, tnrPortFormat);
    CheckAndLogError(ret == false, UNKNOWN_ERROR, "@%s, serverFlattenGetPgId fails", __func__);

    return OK;
}
}  // namespace icamera
