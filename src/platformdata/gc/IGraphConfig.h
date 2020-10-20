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
#pragma once

#include <string>
#include "HalStream.h"
#include "Parameters.h"
#include "iutils/CameraLog.h"
#include "iutils/Errors.h"
#include "iutils/Utils.h"

#include <gcss.h>
#include <gcss_aic_utils.h>
#include "ia_isp_bxt_types.h"
#include "ia_view_types.h"

typedef uint32_t ia_uid;

namespace GCSS {
    class GraphConfigNode;
    class GraphQueryManager;
    class ItemUID;
}

typedef GCSS::GraphConfigNode Node;
typedef std::vector<Node*> NodesPtrVector;

namespace icamera {

/**
 * Stream id associated with still capture.
 */
static const int32_t STILL_STREAM_ID = 60000;
/**
 * Stream id associated with video stream.
 */
#ifdef TNR7_CM
static const int32_t VIDEO_STREAM_ID = 60006;
#else
static const int32_t VIDEO_STREAM_ID = 60001;
#endif

namespace IGraphType {
class ConnectionConfig {
 public:
    ConnectionConfig(): mSourceStage(0),
                        mSourceTerminal(0),
                        mSourceIteration(0),
                        mSinkStage(0),
                        mSinkTerminal(0),
                        mSinkIteration(0),
                        mConnectionType(0) {}

    ConnectionConfig(ia_uid sourceStage,
                     ia_uid sourceTerminal,
                     ia_uid sourceIteration,
                     ia_uid sinkStage,
                     ia_uid sinkTerminal,
                     ia_uid sinkIteration,
                     int connectionType):
                         mSourceStage(sourceStage),
                         mSourceTerminal(sourceTerminal),
                         mSourceIteration(sourceIteration),
                         mSinkStage(sinkStage),
                         mSinkTerminal(sinkTerminal),
                         mSinkIteration(sinkIteration),
                         mConnectionType(connectionType) {}
    void dump() {
        LOG1("connection src 0x%x (0x%x) sink 0x%x(0x%x)",
             mSourceStage, mSourceTerminal, mSinkStage, mSinkTerminal);
    }

    ia_uid mSourceStage;
    ia_uid mSourceTerminal;
    ia_uid mSourceIteration;
    ia_uid mSinkStage;
    ia_uid mSinkTerminal;
    ia_uid mSinkIteration;
    int mConnectionType;
};

/**
* \struct PortFormatSettings
* Format settings for a port in the graph
*/
struct PortFormatSettings {
    int32_t      enabled;
    uint32_t     terminalId; /**< Unique terminal id (is a fourcc code) */
    int32_t      width;    /**< Width of the frame in pixels */
    int32_t      height;   /**< Height of the frame in lines */
    int32_t      fourcc;   /**< Frame format */
    int32_t      bpl;      /**< Bytes per line*/
    int32_t      bpp;      /**< Bits per pixel */
};

/**
 * \struct PipelineConnection
 * Group port format, connection, stream, edge port for
 * pipeline configuration
 */
struct PipelineConnection {
    PipelineConnection() : stream(nullptr), hasEdgePort(false) { CLEAR(portFormatSettings); }
    PortFormatSettings portFormatSettings;
    ConnectionConfig connectionConfig;
    HalStream *stream;
    bool hasEdgePort;
};

struct StageAttr{
    void *rbm;
    uint32_t rbm_bytes;
    StageAttr() : rbm(nullptr), rbm_bytes(0) {}
};

enum terminal_connection_type {
    connection_type_push, /* data is pushed by source stage execute */
    connection_type_pull  /* data is pulled by sink stage execute */
};

struct PgInfo {
    PgInfo() : pgId(-1), streamId(-1) {}
    std::string pgName;
    int pgId;
    int streamId;
    StageAttr rbmValue;
};

struct MbrInfo {
    MbrInfo() { streamId = -1; CLEAR(data); }
    int streamId;
    ia_isp_bxt_gdc_limits data;
};

struct ProgramGroupInfo {
    ProgramGroupInfo() { streamId = -1; pgPtr = nullptr; }
    int streamId;
    ia_isp_bxt_program_group *pgPtr;
};

struct GraphConfigData {
    int mcId;
    int graphId;
    uint32_t gdcKernelId;
    camera_resolution_t csiReso;
    ia_isp_bxt_resolution_info_t gdcReso;
    std::vector<int32_t> streamIds;
    std::vector<PgInfo> pgInfo;
    std::vector<MbrInfo> mbrInfo;
    std::vector<std::string> pgNames;
    std::vector<ProgramGroupInfo> programGroup;
    GraphConfigData() : mcId(-1),
                        graphId(-1),
                        gdcKernelId(-1) {
        CLEAR(csiReso);
        CLEAR(gdcReso);
    }
};

struct ScalerInfo {
    int32_t streamId;
    float scalerWidth;
    float scalerHeight;
};
}  // namespace IGraphType

class IGraphConfig {
public:
    virtual ~IGraphConfig() = default;

    virtual void getCSIOutputResolution(camera_resolution_t &reso) = 0;
    virtual status_t getGdcKernelSetting(uint32_t *kernelId,
                                         ia_isp_bxt_resolution_info_t *resolution) = 0;
    virtual status_t graphGetStreamIds(std::vector<int32_t> &streamIds) = 0;
    virtual int getGraphId(void) = 0;
    virtual int getStreamIdByPgName(std::string pgName) = 0;
    virtual int getPgIdByPgName(std::string pgName) = 0;
    virtual ia_isp_bxt_program_group *getProgramGroup(int32_t streamId) = 0;
    virtual int getProgramGroup(std::string pgName,
                                ia_isp_bxt_program_group* programGroupForPG) {return OK;}
    virtual status_t getMBRData(int32_t streamId, ia_isp_bxt_gdc_limits *data) = 0;
    virtual status_t getPgRbmValue(std::string pgName,
                                   IGraphType::StageAttr *stageAttr) {return OK;}
    virtual status_t getPgIdForKernel(const uint32_t streamIds,
                                      const int32_t kernelId, int32_t *pgId) {return OK;}
    virtual status_t getPgNames(std::vector<std::string>* pgNames) = 0;
    virtual status_t pipelineGetConnections(
                         const std::vector<std::string> &pgList,
                         std::vector<IGraphType::PipelineConnection> *confVector) = 0;
};
}
