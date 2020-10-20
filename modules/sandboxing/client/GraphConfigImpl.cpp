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

#define LOG_TAG "ClientGraphConfigImpl"

#include "modules/sandboxing/client/GraphConfigImpl.h"

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

    LOGIPC("@%s, done, cameraId: %d", __func__, mCameraId);
    mInitialized = true;
}

GraphConfigImpl::GraphConfigImpl(int32_t camId, ConfigMode mode, GraphSettingType type)
        : mCameraId(camId),
          mConfigMode(mode),
          mType(type),
          mInitialized(false) {
    uintptr_t personal = reinterpret_cast<uintptr_t>(this);
    std::string configStreamsName = "/graphConfigStreams" + std::to_string(personal) + "Shm";
    std::string getGraphDataName = "/graphGetData" + std::to_string(personal) + "Shm";
    std::string getPgIdName = "/graphGetPgId" + std::to_string(personal) + "Shm";
    std::string getConnection = "/graphGetConnection" + std::to_string(personal) + "Shm";

    mMems = {{configStreamsName.c_str(), sizeof(GraphConfigStreamParams), &mMemConfig, false},
             {getGraphDataName.c_str(), sizeof(GraphGetDataParams), &mMemGetData, false},
             {getPgIdName.c_str(), sizeof(GraphGetPgIdParams), &mMemGetPgId, false},
             {getConnection.c_str(), sizeof(GraphGetConnectionParams), &mMemGetConnection, false}};

    bool success = mCommon.allocateAllShmMems(&mMems);
    if (!success) {
        mCommon.releaseAllShmMems(mMems);
        return;
    }

    mInitialized = true;
    LOGIPC("@%s, done, cameraId: %d, configMode: %d, type", __func__, mCameraId, mConfigMode,
           mType);
}

GraphConfigImpl::~GraphConfigImpl() {
    mInitialized = false;
    mCommon.releaseAllShmMems(mMems);
    mMems.clear();
}

void GraphConfigImpl::addCustomKeyMap() {
    LOGIPC("@%s", __func__);
    CheckError(!mInitialized, VOID_VALUE, "@%s, mInitialized is false", __func__);

    bool ret = mCommon.requestSync(IPC_GRAPH_ADD_KEY);
    CheckError(!ret, VOID_VALUE, "@%s, requestSync fails", __func__);
}

status_t GraphConfigImpl::parse(int cameraId, const char* graphDescFile, const char* settingsFile) {
    LOGIPC("@%s", __func__);
    CheckError(!mInitialized, UNKNOWN_ERROR, "@%s, mInitialized is false", __func__);

    bool ret = mIpc.clientFlattenParse(mMemParse.mAddr, mMemParse.mSize, cameraId, graphDescFile,
                                       settingsFile);
    CheckError(!ret, UNKNOWN_ERROR, "@%s, clientFlattenParse fails", __func__);

    status_t rt = mCommon.requestSync(IPC_GRAPH_PARSE, mMemParse.mHandle);
    CheckError(!rt, UNKNOWN_ERROR, "@%s, requestSync fails", __func__);

    return OK;
}

void GraphConfigImpl::releaseGraphNodes() {
    LOGIPC("@%s", __func__);
    CheckError(!mInitialized, VOID_VALUE, "@%s, mInitialized is false", __func__);

    bool ret = mCommon.requestSync(IPC_GRAPH_RELEASE_NODES);
    CheckError(!ret, VOID_VALUE, "@%s, requestSync fails", __func__);
}

status_t GraphConfigImpl::configStreams(const vector<HalStream*>& activeStreams) {
    LOGIPC("@%s", __func__);
    CheckError(!mInitialized, UNKNOWN_ERROR, "@%s, mInitialized is false", __func__);

    GraphBaseInfo info = {mCameraId, mConfigMode};
    bool ret = mIpc.clientFlattenConfigStreams(mMemConfig.mAddr, mMemConfig.mSize, info, mType,
                                               activeStreams);
    CheckError(!ret, UNKNOWN_ERROR, "@%s, clientFlattenConfigStreams fails", __func__);

    ret = mCommon.requestSync(IPC_GRAPH_CONFIG_STREAMS, mMemConfig.mHandle);
    CheckError(!ret, UNKNOWN_ERROR, "@%s, requestSync fails", __func__);

    return OK;
}

status_t GraphConfigImpl::getGraphConfigData(IGraphType::GraphConfigData* data) {
    LOGIPC("@%s", __func__);
    CheckError(!mInitialized, UNKNOWN_ERROR, "@%s, mInitialized is false", __func__);

    GraphBaseInfo info = {mCameraId, mConfigMode};
    bool ret = mIpc.clientFlattenGetGraphData(mMemGetData.mAddr, mMemGetData.mSize, info);
    CheckError(!ret, UNKNOWN_ERROR, "@%s, clientFlattenGetGraphData fails", __func__);

    ret = mCommon.requestSync(IPC_GRAPH_GET_CONFIG_DATA, mMemGetData.mHandle);
    CheckError(!ret, UNKNOWN_ERROR, "@%s, requestSync fails", __func__);

    ret = mIpc.clientUnflattenGetGraphData(mMemGetData.mAddr, mMemGetData.mSize, data);
    CheckError(!ret, UNKNOWN_ERROR, "@%s, clientUnflattenGetGraphData fails", __func__);

    return OK;
}

status_t GraphConfigImpl::pipelineGetConnections(
    const std::vector<std::string>& pgList, std::vector<IGraphType::ScalerInfo>* scalerInfo,
    std::vector<IGraphType::PipelineConnection>* confVector) {
    LOGIPC("@%s", __func__);
    CheckError(!mInitialized, UNKNOWN_ERROR, "@%s, mInitialized is false", __func__);

    GraphBaseInfo info = {mCameraId, mConfigMode};
    bool ret = mIpc.clientFlattenGetConnection(mMemGetConnection.mAddr, mMemGetConnection.mSize,
                                               info, pgList);
    CheckError(!ret, UNKNOWN_ERROR, "@%s, clientFlattenGetConnection fails", __func__);

    ret = mCommon.requestSync(IPC_GRAPH_GET_CONNECTION, mMemGetConnection.mHandle);
    CheckError(!ret, UNKNOWN_ERROR, "@%s, requestSync fails", __func__);

    ret = mIpc.clientUnFlattenGetConnection(mMemGetConnection.mAddr, mMemGetConnection.mSize,
                                            scalerInfo, confVector);
    CheckError(!ret, UNKNOWN_ERROR, "@%s, clientUnFlattenGetConnection fails", __func__);

    return OK;
}

status_t GraphConfigImpl::getPgIdForKernel(const uint32_t streamId, const int32_t kernelId,
                                           int32_t* pgId) {
    LOGIPC("@%s", __func__);
    CheckError(!mInitialized, UNKNOWN_ERROR, "@%s, mInitialized is false", __func__);

    GraphBaseInfo info = {mCameraId, mConfigMode};
    bool ret =
        mIpc.clientFlattenGetPgId(mMemGetPgId.mAddr, mMemGetPgId.mSize, info, streamId, kernelId);
    CheckError(!ret, UNKNOWN_ERROR, "@%s, clientFlattenGetPgId fails", __func__);

    ret = mCommon.requestSync(IPC_GRAPH_GET_PG_ID, mMemGetPgId.mHandle);
    CheckError(!ret, UNKNOWN_ERROR, "@%s, requestSync fails", __func__);

    ret = mIpc.clientUnFlattenGetPgId(mMemGetPgId.mAddr, mMemGetPgId.mSize, pgId);
    CheckError(!ret, UNKNOWN_ERROR, "@%s, clientUnFlattenGetPgId fails", __func__);

    return OK;
}

status_t GraphConfigImpl::getProgramGroup(std::string pgName,
                                          ia_isp_bxt_program_group* programGroup) {
    // TODO: Add this API support in the future.
    LOGIPC("@%s", __func__);
    return OK;
}
}  // namespace icamera
