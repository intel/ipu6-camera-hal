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

#define LOG_TAG GraphConfigImplClient

#include "modules/sandboxing/client/GraphConfigImplClient.h"

#include "iutils/CameraLog.h"

using std::map;
using std::string;
using std::vector;

namespace icamera {

GraphConfigImpl::GraphConfigImpl() : mCameraId(-1), mInitialized(false) {
    uintptr_t personal = reinterpret_cast<uintptr_t>(this);
    std::string parseName = "/graphParse" + std::to_string(personal) + "Shm";

    mMems = {{parseName.c_str(), sizeof(GraphParseParams), &mMemParse, false}};

    bool success = mCommon.allocateAllShmMems(&mMems);
    if (!success) {
        mCommon.releaseAllShmMems(mMems);
        return;
    }

    mInitialized = true;
    LOG1("<id%d> %s Construct done", mCameraId, __func__);
}

GraphConfigImpl::GraphConfigImpl(int32_t camId, ConfigMode mode, GraphSettingType type)
        : mCameraId(camId),
          mConfigMode(mode),
          mType(type),
          mInitialized(false) {
    uintptr_t personal = reinterpret_cast<uintptr_t>(this);
    std::string queryGraphSettings = "/graphQueryGraph" + std::to_string(personal) + "Shm";
    std::string configStreamsName = "/graphConfigStreams" + std::to_string(personal) + "Shm";
    std::string getGraphDataName = "/graphGetData" + std::to_string(personal) + "Shm";
    std::string getPgIdName = "/graphGetPgId" + std::to_string(personal) + "Shm";
    std::string getConnection = "/graphGetConnection" + std::to_string(personal) + "Shm";

    mMems = {
        {queryGraphSettings.c_str(), sizeof(GraphQueryGraphParams), &mMemQueryGraphSettings, false},
        {configStreamsName.c_str(), sizeof(GraphConfigStreamParams), &mMemConfig, false},
        {getGraphDataName.c_str(), sizeof(GraphGetDataParams), &mMemGetData, false},
        {getPgIdName.c_str(), sizeof(GraphGetPgIdParams), &mMemGetPgId, false},
        {getConnection.c_str(), sizeof(GraphGetConnectionParams), &mMemGetConnection, false}};

    bool success = mCommon.allocateAllShmMems(&mMems);
    if (!success) {
        mCommon.releaseAllShmMems(mMems);
        return;
    }

    mInitialized = true;
    LOG1("<id%d> %s Construct done, configMode: %d, type %d", mCameraId, __func__, mConfigMode,
         mType);
}

GraphConfigImpl::~GraphConfigImpl() {
    LOG1("<id%d> %s Destroy, configMode: %d, type %d", mCameraId, __func__, mConfigMode, mType);

    mInitialized = false;
    mCommon.releaseAllShmMems(mMems);
    mMems.clear();
}

void GraphConfigImpl::addCustomKeyMap() {
    LOG1("<id%d> Add Custom KeyMap", mCameraId);
    CheckAndLogError(!mInitialized, VOID_VALUE, "addCustomKeyMap mInitialized is false");

    bool ret = mCommon.requestSync(IPC_GRAPH_ADD_KEY);
    CheckAndLogError(!ret, VOID_VALUE, "addCustomKeyMap requestSync fails");
}

status_t GraphConfigImpl::parse(int cameraId, const char* graphDescFile, const char* settingsFile) {
    LOG1("<id%d> Parse GD file %s and setting file %s", cameraId, graphDescFile, settingsFile);
    CheckAndLogError(!mInitialized, UNKNOWN_ERROR, "parse mInitialized is false");

    bool ret = mIpc.clientFlattenParse(mMemParse.mAddr, mMemParse.mSize, cameraId, graphDescFile,
                                       settingsFile);
    CheckAndLogError(!ret, UNKNOWN_ERROR, "clientFlattenParse fails");

    status_t rt = mCommon.requestSync(IPC_GRAPH_PARSE, mMemParse.mHandle);
    CheckAndLogError(!rt, UNKNOWN_ERROR, "parse requestSync fails");

    return OK;
}

void GraphConfigImpl::releaseGraphNodes() {
    CheckAndLogError(!mInitialized, VOID_VALUE, "releaseGraphNodes mInitialized is false");

    bool ret = mCommon.requestSync(IPC_GRAPH_RELEASE_NODES);
    CheckAndLogError(!ret, VOID_VALUE, "releaseGraphNodes requestSync fails");
}

bool GraphConfigImpl::queryGraphSettings(const vector<HalStream*>& activeStreams) {
    CheckAndLogError(!mInitialized, false, "queryGraphSettings mInitialized is false");

    GraphBaseInfo info = {mCameraId, mConfigMode};
    memset(mMemQueryGraphSettings.mAddr, 0, sizeof(GraphQueryGraphParams));
    bool ret = mIpc.clientFlattenConfigStreams(mMemQueryGraphSettings.mAddr, mMemConfig.mSize, info,
                                               mType, false, activeStreams);
    CheckAndLogError(!ret, false, "clientFlattenConfigStreams fails");

    ret = mCommon.requestSync(IPC_GRAPH_QUERY_GRAPH_SETTINGS, mMemQueryGraphSettings.mHandle);
    CheckAndLogError(!ret, false, "queryGraphSettings requestSync fails");

    GraphQueryGraphParams* params =
        static_cast<GraphQueryGraphParams*>(mMemQueryGraphSettings.mAddr);
    return params->isHasGraphSettings;
}

status_t GraphConfigImpl::configStreams(const vector<HalStream*>& activeStreams,
                                        bool dummyStillSink) {
    LOG1("<id%d> %s, dummyStillSink: %d", mCameraId, __func__, dummyStillSink);
    CheckAndLogError(!mInitialized, UNKNOWN_ERROR, "configStreams mInitialized is false");

    GraphBaseInfo info = {mCameraId, mConfigMode};
    bool ret = mIpc.clientFlattenConfigStreams(mMemConfig.mAddr, mMemConfig.mSize, info, mType,
                                               dummyStillSink, activeStreams);
    CheckAndLogError(!ret, UNKNOWN_ERROR, "clientFlattenConfigStreams fails");

    ret = mCommon.requestSync(IPC_GRAPH_CONFIG_STREAMS, mMemConfig.mHandle);
    CheckAndLogError(!ret, UNKNOWN_ERROR, "configStreams requestSync fails");

    return OK;
}

status_t GraphConfigImpl::getGraphConfigData(IGraphType::GraphConfigData* data) {
    CheckAndLogError(!mInitialized, UNKNOWN_ERROR, "getGraphConfigData mInitialized is false");

    GraphBaseInfo info = {mCameraId, mConfigMode};
    bool ret = mIpc.clientFlattenGetGraphData(mMemGetData.mAddr, mMemGetData.mSize, info);
    CheckAndLogError(!ret, UNKNOWN_ERROR, "clientFlattenGetGraphData fails");

    ret = mCommon.requestSync(IPC_GRAPH_GET_CONFIG_DATA, mMemGetData.mHandle);
    CheckAndLogError(!ret, UNKNOWN_ERROR, "getGraphConfigData requestSync fails");

    ret = mIpc.clientUnflattenGetGraphData(mMemGetData.mAddr, mMemGetData.mSize, data);
    CheckAndLogError(!ret, UNKNOWN_ERROR, "clientUnflattenGetGraphData fails");

    return OK;
}

status_t GraphConfigImpl::pipelineGetConnections(
    const std::vector<std::string>& pgList, std::vector<IGraphType::ScalerInfo>* scalerInfo,
    std::vector<IGraphType::PipelineConnection>* confVector,
    std::vector<IGraphType::PrivPortFormat>* tnrPortFormat) {
    CheckAndLogError(!mInitialized, UNKNOWN_ERROR, "GetConnections mInitialized is false");

    GraphBaseInfo info = {mCameraId, mConfigMode};
    bool ret = mIpc.clientFlattenGetConnection(mMemGetConnection.mAddr, mMemGetConnection.mSize,
                                               info, pgList);
    CheckAndLogError(!ret, UNKNOWN_ERROR, "clientFlattenGetConnection fails");

    ret = mCommon.requestSync(IPC_GRAPH_GET_CONNECTION, mMemGetConnection.mHandle);
    CheckAndLogError(!ret, UNKNOWN_ERROR, "GetConnections requestSync fails");

    ret = mIpc.clientUnFlattenGetConnection(mMemGetConnection.mAddr, mMemGetConnection.mSize,
                                            scalerInfo, confVector, tnrPortFormat);
    CheckAndLogError(!ret, UNKNOWN_ERROR, "clientUnFlattenGetConnection fails");

    return OK;
}

status_t GraphConfigImpl::getPgIdForKernel(const uint32_t streamId, const int32_t kernelId,
                                           int32_t* pgId) {
    LOG1("<id%d> getPgIdForKernel stream %d, kernel %d", mCameraId, streamId, kernelId);
    CheckAndLogError(!mInitialized, UNKNOWN_ERROR, "@%s, mInitialized is false", __func__);

    GraphBaseInfo info = {mCameraId, mConfigMode};
    bool ret =
        mIpc.clientFlattenGetPgId(mMemGetPgId.mAddr, mMemGetPgId.mSize, info, streamId, kernelId);
    CheckAndLogError(!ret, UNKNOWN_ERROR, "clientFlattenGetPgId fails");

    ret = mCommon.requestSync(IPC_GRAPH_GET_PG_ID, mMemGetPgId.mHandle);
    CheckAndLogError(!ret, UNKNOWN_ERROR, "getPgId requestSync fails");

    ret = mIpc.clientUnFlattenGetPgId(mMemGetPgId.mAddr, mMemGetPgId.mSize, pgId);
    CheckAndLogError(!ret, UNKNOWN_ERROR, "clientUnFlattenGetPgId fails");

    return OK;
}

status_t GraphConfigImpl::getProgramGroup(std::string pgName,
                                          ia_isp_bxt_program_group* programGroup) {
    return OK;
}
}  // namespace icamera
