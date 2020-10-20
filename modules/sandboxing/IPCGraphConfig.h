/*
 * Copyright (C) 2019-2020 Intel Corporation.
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

#pragma once

#include <string>
#include <vector>

#include "modules/sandboxing/IPCCommon.h"
#include "src/platformdata/gc/IGraphConfig.h"

namespace icamera {

#define MAX_STREAM (4)                             // Max stream number
#define MAX_GRAPH_SETTINGS_SIZE (2 * 1024 * 1024)  // Max graph settings file size
#define MAX_GRAPH_DESCRIPTOR_SIZE (512 * 1024)     // Max graph descriptor file size
#define MAX_PG_NUMBER (10)                         // Max pg number
#define MAX_NAME_LENGTH (128)                      // Max length for name
#define MAX_CONNECTION_COUNT (64)                  // Max connection count
#define MAX_STREAM_KERNEL_COUNT (500)              // Max kernels info per one stream

struct GraphBaseInfo {
    int cameraId;
    ConfigMode configMode;

    bool operator<(const GraphBaseInfo& b) const {
        return (cameraId < b.cameraId) ? true : (configMode < b.configMode ? true : false);
    }
};

struct GraphPgInfo {
    char pgName[MAX_NAME_LENGTH];
    uint32_t pgId;
    int streamId;
    uint32_t rbmByte;
    char rbmData[MAX_NAME_LENGTH];
};

struct GraphMbrInfo {
    int32_t streamId;
    ia_isp_bxt_gdc_limits mBrData;
};

struct GraphKernelArray {
    int32_t streamId;
    ia_isp_bxt_program_group group;
    ia_isp_bxt_run_kernels_t runKernels[MAX_STREAM_KERNEL_COUNT];
    ia_isp_bxt_resolution_info_t resoInfo[MAX_STREAM_KERNEL_COUNT];
    ia_isp_bxt_resolution_info_t resoHistory[MAX_STREAM_KERNEL_COUNT];
    ia_isp_bxt_pipe_t pipeInfo;
};

struct GraphParseParams {
    int cameraId;
    size_t gdSize;
    char GD[MAX_GRAPH_DESCRIPTOR_SIZE];
    size_t gsSize;
    char GS[MAX_GRAPH_SETTINGS_SIZE];
};

struct GraphConfigStreamParams {
    GraphBaseInfo baseInfo;
    GraphSettingType type;
    uint32_t streamNum;
    HalStream streamCfg[MAX_STREAM];
    stream_t streamPriv[MAX_STREAM];
};

struct GraphGetDataParams {
    GraphBaseInfo baseInfo;

    int mcId;
    int graphId;
    uint32_t gdcKernelId;

    camera_resolution_t csiReso;
    ia_isp_bxt_resolution_info_t gdcReso;

    uint32_t streamIdNum;
    int32_t streamIdData[MAX_STREAM];

    uint32_t pgInfoNum;
    GraphPgInfo pgInfoData[MAX_PG_NUMBER];

    uint32_t mBrInfoNum;
    GraphMbrInfo mBrInfoData[MAX_STREAM];

    uint32_t pgNamesNum;
    char pgNames[MAX_PG_NUMBER][MAX_NAME_LENGTH];

    uint32_t kernelArrayNum;
    GraphKernelArray kernelArray[MAX_STREAM];
};

struct GraphGetPgIdParams {
    GraphBaseInfo baseInfo;
    uint32_t streamId;
    int32_t kernelId;
    int32_t pgId;
};

struct GraphConnection {
    IGraphType::PipelineConnection connection;
    HalStream stream;
};

struct GraphGetConnectionParams {
    GraphBaseInfo baseInfo;
    uint32_t pgListNum;
    char pgList[MAX_PG_NUMBER][MAX_NAME_LENGTH];
    uint32_t connectionArraySize;
    GraphConnection connectionArray[MAX_CONNECTION_COUNT];
    uint32_t scalerInfoNum;
    IGraphType::ScalerInfo scalerInfoArray[MAX_STREAM];
};

class IPCGraphConfig {
 public:
    IPCGraphConfig();
    virtual ~IPCGraphConfig();

    bool clientFlattenParse(void* pData, uint32_t size, int cameraId, const char* graphDescFile,
                            const char* settingsFile);
    bool serverUnflattenParse(void* pData, uint32_t size, GraphParseParams** parseParam);
    bool clientFlattenConfigStreams(void* pData, uint32_t size, GraphBaseInfo info,
                                    GraphSettingType type, const std::vector<HalStream*>& streams);
    bool serverUnflattenConfigStreams(void* pData, uint32_t size, GraphBaseInfo* info,
                                      GraphSettingType* type, std::vector<HalStream*>* streams);
    bool clientFlattenGetGraphData(void* pData, uint32_t size, GraphBaseInfo info);
    bool serverUnflattenGetGraphData(void* pData, uint32_t size, GraphBaseInfo* info);
    bool serverFlattenGetGraphData(void* pData, uint32_t size,
                                   IGraphType::GraphConfigData graphData);
    bool clientUnflattenGetGraphData(void* pData, uint32_t size,
                                     IGraphType::GraphConfigData* graphData);
    bool clientFlattenGetPgId(void* pData, uint32_t size, GraphBaseInfo info, const int streamId,
                              const int kernelId);
    bool serverUnFlattenGetPgId(void* pData, uint32_t size, GraphBaseInfo* info, uint32_t* streamId,
                                int32_t* kernelId);
    bool serverFlattenGetPgId(void* pData, uint32_t size, int32_t pgId);
    bool clientUnFlattenGetPgId(void* pData, uint32_t size, int32_t* pgId);
    bool clientFlattenGetConnection(void* pData, uint32_t size, GraphBaseInfo info,
                                    const std::vector<std::string>& pgList);
    bool serverUnFlattenGetConnection(void* pData, uint32_t size, GraphBaseInfo* info,
                                      std::vector<std::string>* pgList);
    bool serverFlattenGetConnection(void* pData, uint32_t size,
                                    const std::vector<IGraphType::ScalerInfo>& scalerInfo,
                                    const std::vector<IGraphType::PipelineConnection>& confVector);
    bool clientUnFlattenGetConnection(void* pData, uint32_t size,
                                      std::vector<IGraphType::ScalerInfo>* scalerInfo,
                                      std::vector<IGraphType::PipelineConnection>* confVector);

 private:
    status_t readDataFromXml(const char* fileName, char* dataPtr, size_t* dataSize, int maxSize);

    // Disable copy constructor and assignment operator
    DISALLOW_COPY_AND_ASSIGN(IPCGraphConfig);
};
}  // namespace icamera
