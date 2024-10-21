/*
 * Copyright (C) 2019-2022 Intel Corporation
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

#define LOG_TAG IPC_GRAPH_CONFIG

#include "modules/sandboxing/IPCGraphConfig.h"

#include <sys/stat.h>

#include <memory>

#include "iutils/CameraLog.h"
#include "iutils/Errors.h"
#include "iutils/Utils.h"

namespace icamera {

status_t IPCGraphConfig::readDataFromXml(const char* fileName, char* dataPtr, size_t* dataSize,
                                         int maxSize) {
    CheckAndLogError(!dataSize || !fileName || !dataPtr, UNKNOWN_ERROR,
                     "%s, Wrong parameters, dataSize: %p, fileName: %p, dataPtr: %p", __func__,
                     dataSize, fileName, dataPtr);

    struct stat statBuf;
    int ret = stat(fileName, &statBuf);
    CheckAndLogError((ret != 0), UNKNOWN_ERROR, "Failed to query the size of file: %s!", fileName);
    CheckAndLogError(statBuf.st_size > maxSize, BAD_VALUE,
                     "The memory size: %d less than file size: %d", maxSize, statBuf.st_size);

    *dataSize = static_cast<size_t>(statBuf.st_size);
    LOG1("%s, fileName: %s, size: %zu", __func__, fileName, *dataSize);

    FILE* file = fopen(fileName, "rb");
    CheckAndLogError(!file, NAME_NOT_FOUND, "%s, Failed to open file: %s", __func__, fileName);

    size_t len = fread(dataPtr, 1, *dataSize, file);
    fclose(file);

    CheckAndLogError((len != *dataSize), UNKNOWN_ERROR, "%s, Failed to read data from file: %s",
                     __func__, fileName);

    return OK;
}

bool IPCGraphConfig::clientFlattenParse(void* pData, uint32_t size, int cameraId,
                                        const char* graphDescFile, const char* settingsFile) {
    CheckAndLogError(!pData || !graphDescFile || !settingsFile || size < sizeof(GraphParseParams),
                     false, "@%s, wrong parameters, pData: %p, GD: %p, settings: %p, size: %u",
                     __func__, pData, graphDescFile, settingsFile, size);

    GraphParseParams* params = static_cast<GraphParseParams*>(pData);
    CLEAR(*params);

    params->cameraId = cameraId;
    int ret =
        readDataFromXml(graphDescFile, params->GD, &(params->gdSize), MAX_GRAPH_DESCRIPTOR_SIZE);
    CheckAndLogError(ret != OK, false, "Failed to read the graph descriptor file: %s",
                     graphDescFile);

    ret = readDataFromXml(settingsFile, params->GS, &(params->gsSize), MAX_GRAPH_SETTINGS_SIZE);
    CheckAndLogError(ret != OK, false, "Failed to read the graph settings file: %s", settingsFile);

    return true;
}

bool IPCGraphConfig::serverUnflattenParse(void* pData, uint32_t size,
                                          GraphParseParams** parseParam) {
    CheckAndLogError(!pData || !parseParam || size < sizeof(GraphParseParams), false,
                     "@%s, Wrong parameters, pData: %p, parseParam: %p, size: %u", __func__, pData,
                     parseParam, size);

    GraphParseParams* params = static_cast<GraphParseParams*>(pData);
    *parseParam = params;

    return true;
}

bool IPCGraphConfig::clientFlattenConfigStreams(void* pData, uint32_t size, GraphBaseInfo info,
                                                GraphSettingType type, bool dummyStillSink,
                                                const std::vector<HalStream*>& streams) {
    LOG1("<id%d>@%s, pData: %p, configMode: %d, dummyStillSink: %d", info.cameraId, __func__, pData,
         info.configMode, dummyStillSink);

    CheckAndLogError(!pData || size < sizeof(GraphConfigStreamParams) || streams.empty(), false,
                     "@%s, Wrong parameters, pData: %p, size: %u, streams count: %d", __func__,
                     pData, size, streams.size());

    GraphConfigStreamParams* params = static_cast<GraphConfigStreamParams*>(pData);
    CLEAR(*params);

    params->baseInfo = info;
    params->type = type;
    for (size_t i = 0; i < streams.size(); ++i) {
        params->streamCfg[i] = *(streams[i]);
        params->streamPriv[i] = *(static_cast<stream_t*>(streams[i]->mPrivate));
        params->streamNum++;
    }
    params->dummyStillSink = dummyStillSink;
    return true;
}

bool IPCGraphConfig::serverUnflattenConfigStreams(void* pData, uint32_t size, GraphBaseInfo* info,
                                                  GraphSettingType* type, bool* dummyStillSink,
                                                  std::vector<HalStream*>* streams) {
    CheckAndLogError(!pData || size < sizeof(GraphConfigStreamParams) || !info || !type ||
                         !streams || !dummyStillSink,
                     false,
                     "@%s, Wrong parameters, pData: %p, size: %u, info: %p, type: %p, streams: %p, "
                     "dummyStillSink: %p",
                     __func__, pData, size, info, type, streams, dummyStillSink);

    GraphConfigStreamParams* params = static_cast<GraphConfigStreamParams*>(pData);

    *info = params->baseInfo;
    *type = params->type;
    *dummyStillSink = params->dummyStillSink;
    for (uint32_t i = 0; i < params->streamNum; ++i) {
        params->streamCfg[i].mPrivate = static_cast<void*>(&(params->streamPriv[i]));
        streams->push_back(&(params->streamCfg[i]));
    }

    return true;
}

bool IPCGraphConfig::clientFlattenGetGraphData(void* pData, uint32_t size, GraphBaseInfo info) {
    CheckAndLogError(!pData || size < sizeof(GraphGetDataParams), false,
                     "@%s, Wrong parameters, pData: %p, size: %u", __func__, pData, size);

    GraphGetDataParams* params = static_cast<GraphGetDataParams*>(pData);
    CLEAR(*params);

    params->baseInfo = info;

    return true;
}

bool IPCGraphConfig::serverUnflattenGetGraphData(void* pData, uint32_t size, GraphBaseInfo* info) {
    CheckAndLogError(!pData || !info || size < sizeof(GraphGetDataParams), false,
                     "@%s, Wrong parameters, pData: %p, info: %p, size: %u", __func__, pData, info,
                     size);

    GraphGetDataParams* params = static_cast<GraphGetDataParams*>(pData);
    *info = params->baseInfo;

    return true;
}

bool IPCGraphConfig::serverFlattenGetGraphData(void* pData, uint32_t size,
                                               IGraphType::GraphConfigData graphData) {
    CheckAndLogError(!pData || size < sizeof(GraphGetDataParams), false,
                     "@%s, Wrong parameters, pData: %p, size: %u", __func__, pData, size);

    GraphGetDataParams* params = static_cast<GraphGetDataParams*>(pData);

    params->mcId = graphData.mcId;
    params->graphId = graphData.graphId;

    params->csiReso = graphData.csiReso;

    LOG1("@%s, mcId: %d, graphId: %d", __func__, params->mcId, params->graphId);

    params->gdcInfoNum = graphData.gdcInfos.size();
    for (size_t i = 0; i < graphData.gdcInfos.size(); ++i) {
        params->mGdcInfo[i] = graphData.gdcInfos[i];
    }

    params->streamIdNum = graphData.streamIds.size();
    for (size_t i = 0; i < graphData.streamIds.size(); ++i) {
        params->streamIdData[i] = graphData.streamIds[i];
    }

    params->tuningModeNum = graphData.tuningModes.size();
    for (size_t i = 0; i < graphData.tuningModes.size(); ++i) {
        params->tuningModes[i] = graphData.tuningModes[i];
    }

    params->pgInfoNum = graphData.pgInfo.size();
    for (size_t i = 0; i < graphData.pgInfo.size(); ++i) {
        size_t len = graphData.pgInfo[i].pgName.copy(params->pgInfoData[i].pgName, MAX_NAME_LENGTH);
        params->pgInfoData[i].pgName[len] = '\0';
        params->pgInfoData[i].pgId = graphData.pgInfo[i].pgId;
        params->pgInfoData[i].streamId = graphData.pgInfo[i].streamId;
        params->pgInfoData[i].rbmByte = graphData.pgInfo[i].rbmValue.rbm_bytes;

        if (params->pgInfoData[i].rbmByte > 0) {
            MEMCPY_S(params->pgInfoData[i].rbmData, MAX_RBM_STR_SIZE,
                     graphData.pgInfo[i].rbmValue.rbm, graphData.pgInfo[i].rbmValue.rbm_bytes);
        }
    }

    params->mBrInfoNum = graphData.mbrInfo.size();
    for (size_t i = 0; i < graphData.mbrInfo.size(); ++i) {
        params->mBrInfoData[i].streamId = graphData.mbrInfo[i].streamId;
        params->mBrInfoData[i].mBrData = graphData.mbrInfo[i].data;
    }

    params->pgNamesNum = graphData.pgNames.size();
    for (size_t i = 0; i < params->pgNamesNum; ++i) {
        size_t len = graphData.pgNames[i].copy(params->pgNames[i], MAX_NAME_LENGTH);
        params->pgNames[i][len] = '\0';
    }

    params->kernelArrayNum = graphData.programGroup.size();
    for (size_t i = 0; i < params->kernelArrayNum; ++i) {
        params->kernelArray[i].streamId = graphData.programGroup[i].streamId;
        ia_isp_bxt_program_group* pgPtr = graphData.programGroup[i].pgPtr;
        params->kernelArray[i].group = *pgPtr;

        for (unsigned int j = 0; j < params->kernelArray[i].group.kernel_count; ++j) {
            params->kernelArray[i].runKernels[j] = pgPtr->run_kernels[j];
            if (pgPtr->run_kernels[j].resolution_info) {
                params->kernelArray[i].resoInfo[j] = *(pgPtr->run_kernels[j].resolution_info);
                params->kernelArray[i].runKernels[j].resolution_info =
                    &(params->kernelArray[i].resoInfo[j]);
            } else {
                params->kernelArray[i].runKernels[j].resolution_info = nullptr;
            }

            if (pgPtr->run_kernels[j].resolution_history) {
                params->kernelArray[i].resoHistory[j] = *(pgPtr->run_kernels[j].resolution_history);
                params->kernelArray[i].runKernels[j].resolution_history =
                    &(params->kernelArray[i].resoHistory[j]);
            } else {
                params->kernelArray[i].runKernels[j].resolution_history = nullptr;
            }
        }
        params->kernelArray[i].group.run_kernels = params->kernelArray[i].runKernels;

        if (pgPtr->pipe) {
            params->kernelArray[i].pipeInfo = *(pgPtr->pipe);
            params->kernelArray[i].group.pipe = &(params->kernelArray[i].pipeInfo);
        } else {
            params->kernelArray[i].group.pipe = nullptr;
        }
    }

    return true;
}

bool IPCGraphConfig::clientUnflattenGetGraphData(void* pData, uint32_t size,
                                                 IGraphType::GraphConfigData* graphData) {
    CheckAndLogError(!pData || !graphData || size < sizeof(GraphGetDataParams), false,
                     "@%s, Wrong parameters, pData: %p, graphData: %p, size: %u", __func__, pData,
                     graphData, size);

    GraphGetDataParams* params = static_cast<GraphGetDataParams*>(pData);

    graphData->mcId = params->mcId;
    graphData->graphId = params->graphId;

    graphData->csiReso = params->csiReso;

    LOG1("@%s, mcId: %d, graphId: %d", __func__, params->mcId, params->graphId);

    for (size_t i = 0; i < params->gdcInfoNum; i++) {
        graphData->gdcInfos.push_back(params->mGdcInfo[i]);
    }

    for (size_t i = 0; i < params->streamIdNum; ++i) {
        graphData->streamIds.push_back(params->streamIdData[i]);
    }

    for (size_t i = 0; i < params->tuningModeNum; ++i) {
        graphData->tuningModes.push_back(params->tuningModes[i]);
    }

    for (size_t i = 0; i < params->pgInfoNum; ++i) {
        IGraphType::PgInfo info;
        info.pgName = params->pgInfoData[i].pgName;
        info.pgId = params->pgInfoData[i].pgId;
        info.streamId = params->pgInfoData[i].streamId;
        info.rbmValue.rbm_bytes = params->pgInfoData[i].rbmByte;
        if (params->pgInfoData[i].rbmByte > 0) {
            MEMCPY_S(info.rbmValue.rbm, MAX_RBM_STR_SIZE, params->pgInfoData[i].rbmData,
                     params->pgInfoData[i].rbmByte);
        }
        graphData->pgInfo.push_back(info);
    }

    for (size_t i = 0; i < params->mBrInfoNum; ++i) {
        IGraphType::MbrInfo info;
        info.streamId = params->mBrInfoData[i].streamId;
        info.data = params->mBrInfoData[i].mBrData;
        graphData->mbrInfo.push_back(info);
    }

    for (size_t i = 0; i < params->pgNamesNum; ++i) {
        graphData->pgNames.push_back(params->pgNames[i]);
    }

    for (size_t i = 0; i < params->kernelArrayNum; ++i) {
        IGraphType::ProgramGroupInfo info;

        info.streamId = params->kernelArray[i].streamId;
        info.pgPtr = &(params->kernelArray[i].group);

        info.pgPtr->run_kernels = params->kernelArray[i].runKernels;
        for (unsigned j = 0; j < info.pgPtr->kernel_count; ++j) {
            if (params->kernelArray[i].runKernels[j].resolution_info) {
                info.pgPtr->run_kernels[j].resolution_info = &(params->kernelArray[i].resoInfo[j]);
            } else {
                info.pgPtr->run_kernels[j].resolution_info = nullptr;
            }

            if (params->kernelArray[i].runKernels[j].resolution_history) {
                info.pgPtr->run_kernels[j].resolution_history =
                    &(params->kernelArray[i].resoHistory[j]);
            } else {
                info.pgPtr->run_kernels[j].resolution_history = nullptr;
            }
        }

        if (params->kernelArray[i].group.pipe) {
            info.pgPtr->pipe = &(params->kernelArray[i].pipeInfo);
        } else {
            info.pgPtr->pipe = nullptr;
        }

        graphData->programGroup.push_back(info);
    }

    return true;
}

bool IPCGraphConfig::clientFlattenGetPgId(void* pData, uint32_t size, GraphBaseInfo info,
                                          const int streamId, const int kernelId) {
    CheckAndLogError(!pData || size < sizeof(GraphGetPgIdParams), false,
                     "@%s, Wrong parameters, pData: %p, size: %u", __func__, pData, size);

    GraphGetPgIdParams* params = static_cast<GraphGetPgIdParams*>(pData);
    CLEAR(*params);

    params->baseInfo = info;
    params->streamId = streamId;
    params->kernelId = kernelId;

    return true;
}

bool IPCGraphConfig::serverUnFlattenGetPgId(void* pData, uint32_t size, GraphBaseInfo* info,
                                            uint32_t* streamId, int32_t* kernelId) {
    CheckAndLogError(!pData || size < sizeof(GraphGetPgIdParams) || !info || !streamId || !kernelId,
                     false,
                     "@%s, Wrong parameters, pData: %p, size: %u, info: %p, streamId: %p, "
                     "kernelId: %p",
                     __func__, pData, size, info, streamId, kernelId);

    GraphGetPgIdParams* params = static_cast<GraphGetPgIdParams*>(pData);

    *info = params->baseInfo;
    *streamId = params->streamId;
    *kernelId = params->kernelId;

    return true;
}

bool IPCGraphConfig::serverFlattenGetPgId(void* pData, uint32_t size, int32_t pgId) {
    CheckAndLogError(!pData || size < sizeof(GraphGetPgIdParams), false,
                     "@%s, Wrong parameters, pData: %p, size: %u", __func__, pData, size);

    GraphGetPgIdParams* params = static_cast<GraphGetPgIdParams*>(pData);
    params->pgId = pgId;

    return true;
}

bool IPCGraphConfig::clientUnFlattenGetPgId(void* pData, uint32_t size, int32_t* pgId) {
    CheckAndLogError(!pData || !pgId || size < sizeof(GraphGetPgIdParams), false,
                     "@%s, Wrong parameters, pData: %p, pgId: %p, size: %u", __func__, pData, pgId,
                     size);

    GraphGetPgIdParams* params = static_cast<GraphGetPgIdParams*>(pData);
    *pgId = params->pgId;

    return true;
}

bool IPCGraphConfig::clientFlattenGetConnection(void* pData, uint32_t size, GraphBaseInfo info,
                                                const std::vector<std::string>& pgList) {
    CheckAndLogError(!pData || size < sizeof(GraphGetConnectionParams), false,
                     "@%s, Wrong parameters, pData: %p, size: %u", __func__, pData, size);
    GraphGetConnectionParams* params = static_cast<GraphGetConnectionParams*>(pData);
    CLEAR(*params);

    params->baseInfo = info;
    params->pgListNum = pgList.size();
    for (size_t i = 0; i < pgList.size(); ++i) {
        size_t len = pgList[i].copy(params->pgList[i], MAX_NAME_LENGTH);
        params->pgList[i][len] = '\0';
    }

    return true;
}

bool IPCGraphConfig::serverUnFlattenGetConnection(void* pData, uint32_t size, GraphBaseInfo* info,
                                                  std::vector<std::string>* pgList) {
    CheckAndLogError(!pData || size < sizeof(GraphGetConnectionParams) || !info || !pgList, false,
                     "@%s, Wrong parameters, pData: %p, size: %u, info: %p, pgList: %p", __func__,
                     pData, size, info, pgList);

    GraphGetConnectionParams* params = static_cast<GraphGetConnectionParams*>(pData);

    *info = params->baseInfo;
    for (size_t i = 0; i < params->pgListNum; ++i) {
        pgList->push_back(params->pgList[i]);
    }

    return true;
}

bool IPCGraphConfig::serverFlattenGetConnection(
    void* pData, uint32_t size, const std::vector<IGraphType::ScalerInfo>& scalerInfo,
    const std::vector<IGraphType::PipelineConnection>& confVector,
    const std::vector<IGraphType::PrivPortFormat>& tnrPortFormat) {
    CheckAndLogError(!pData || size < sizeof(GraphGetConnectionParams), false,
                     "@%s, Wrong parameters, pData: %p, size: %u", __func__, pData, size);

    GraphGetConnectionParams* params = static_cast<GraphGetConnectionParams*>(pData);

    params->connectionArraySize = confVector.size();
    for (size_t i = 0; i < confVector.size(); ++i) {
        params->connectionArray[i].connection = confVector[i];
        if (params->connectionArray[i].connection.stream) {
            params->connectionArray[i].stream = *(confVector[i].stream);
        }
    }

    params->scalerInfoNum = scalerInfo.size();
    for (size_t i = 0; i < scalerInfo.size(); ++i) {
        params->scalerInfoArray[i] = scalerInfo[i];
    }

    params->portFormatNum = tnrPortFormat.size();
    for (size_t i = 0; i < tnrPortFormat.size(); ++i) {
        params->portFormatArray[i] = tnrPortFormat[i];
    }

    return true;
}

bool IPCGraphConfig::clientUnFlattenGetConnection(
    void* pData, uint32_t size, std::vector<IGraphType::ScalerInfo>* scalerInfo,
    std::vector<IGraphType::PipelineConnection>* confVector,
    std::vector<IGraphType::PrivPortFormat>* tnrPortFormat) {
    CheckAndLogError(
        !pData || !scalerInfo || !confVector || size < sizeof(GraphGetConnectionParams), false,
        "@%s, Wrong parameters, pData: %p, scalerInfo: %p, confVector: %p, size: %u", __func__,
        pData, scalerInfo, confVector, size);

    GraphGetConnectionParams* params = static_cast<GraphGetConnectionParams*>(pData);

    for (size_t i = 0; i < params->connectionArraySize; ++i) {
        if (params->connectionArray[i].connection.stream) {
            params->connectionArray[i].connection.stream = &(params->connectionArray[i].stream);
        }
        confVector->push_back(params->connectionArray[i].connection);
    }

    for (size_t i = 0; i < params->scalerInfoNum; ++i) {
        scalerInfo->push_back(params->scalerInfoArray[i]);
    }

    if (tnrPortFormat) {
        for (size_t i = 0; i < params->portFormatNum; ++i) {
            tnrPortFormat->push_back(params->portFormatArray[i]);
        }
    }

    return true;
}
}  // namespace icamera
