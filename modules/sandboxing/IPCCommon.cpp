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

#include "modules/sandboxing/IPCCommon.h"

#include <iostream>
#include <string>

namespace icamera {
const char* IntelAlgoIpcCmdToString(IPC_CMD cmd) {
    static const char* gIpcCmdMapping[] = {
        "IPC_FD_INIT", "IPC_FD_RUN", "IPC_FD_DEINIT", "IPC_GRAPH_ADD_KEY", "IPC_GRAPH_PARSE",
        "IPC_GRAPH_RELEASE_NODES", "IPC_GRAPH_QUERY_GRAPH_SETTINGS", "IPC_GRAPH_CONFIG_STREAMS",
        "IPC_GRAPH_GET_CONFIG_DATA", "IPC_GRAPH_GET_CONNECTION", "IPC_GRAPH_GET_PG_ID",
        "IPC_CCA_CONSTRUCT", "IPC_CCA_DESTRUCT", "IPC_CCA_INIT", "IPC_CCA_SET_STATS",
        "IPC_CCA_RUN_AEC", "IPC_CCA_RUN_AIQ", "IPC_CCA_RUN_LTM", "IPC_CCA_UPDATE_ZOOM",
        "IPC_CCA_RUN_DVS", "IPC_CCA_GET_CMC", "IPC_CCA_GET_MKN", "IPC_CCA_GET_AIQD",
        "IPC_CCA_UPDATE_TUNING", "IPC_CCA_DEINIT", "IPC_CCA_RUN_AIC", "IPC_CCA_GET_PAL_SIZE",
        "IPC_CCA_DECODE_STATS", "IPC_PG_PARAM_INIT", "IPC_PG_PARAM_PREPARE",
        "IPC_PG_PARAM_ALLOCATE_PG", "IPC_PG_PARAM_GET_FRAG_DESCS", "IPC_PG_PARAM_PREPARE_PROGRAM",
        "IPC_PG_PARAM_REGISTER_PAYLOADS", "IPC_PG_PARAM_ENCODE", "IPC_PG_PARAM_DECODE",
        "IPC_PG_PARAM_DEINIT", "IPC_GPU_TNR_INIT", "IPC_GPU_TNR_GET_SURFACE_INFO",
        "IPC_GPU_TNR_PREPARE_SURFACE", "IPC_GPU_TNR_RUN_FRAME", "IPC_GPU_TNR_PARAM_UPDATE",
        "IPC_GPU_TNR_DEINIT",
        // ENABLE_EVCP_S
        "IPC_EVCP_INIT", "IPC_EVCP_UPDCONF", "IPC_EVCP_SETCONF", "IPC_EVCP_RUN_FRAME",
        "IPC_EVCP_DEINIT",
        // ENABLE_EVCP_E
        // LEVEL0_ICBM_S
        "IPC_ICBM_INIT", "IPC_ICBM_RUN_FRAME", "IPC_ICBM_DEINIT",
        // LEVEL0_ICBM_E
        "IPC_GPU_TNR_THREAD2_RUN_FRAME",
        "IPC_GPU_TNR_THREAD2_PARAM_UPDATE"};

    unsigned int num = sizeof(gIpcCmdMapping) / sizeof(gIpcCmdMapping[0]);
    return cmd < num ? gIpcCmdMapping[cmd] : gIpcCmdMapping[0];
}

IPC_GROUP IntelAlgoIpcCmdToGroup(IPC_CMD cmd) {
    IPC_GROUP group = IPC_GROUP_CPU_OTHER;
    if (cmd >= IPC_CCA_CONSTRUCT && cmd <= IPC_CCA_DEINIT) {
        group = IPC_GROUP_AIQ;
    } else if (cmd >= IPC_CCA_RUN_AIC && cmd <= IPC_CCA_GET_PAL_SIZE) {
        group = IPC_GROUP_PAL;
    } else if (cmd == IPC_CCA_DECODE_STATS) {
        group = IPC_GROUP_STATS;
    } else if (cmd >= IPC_PG_PARAM_INIT && cmd <= IPC_PG_PARAM_DEINIT) {
        group = IPC_GROUP_PSYS;
    } else if (cmd >= IPC_FD_INIT && cmd <= IPC_FD_DEINIT) {
        group = IPC_GROUP_FD;
    } else if (cmd >= IPC_GPU_TNR_INIT && cmd <= IPC_GPU_TNR_DEINIT) {
        group = IPC_GROUP_GPU;
    } else if (cmd >= IPC_GPU_TNR_THREAD2_RUN_FRAME && cmd <= IPC_GPU_TNR_THREAD2_PARAM_UPDATE) {
        group = IPC_GROUP_GPU_THREAD2;
        // ENABLE_EVCP_S
    } else if (cmd >= IPC_EVCP_INIT && cmd <= IPC_EVCP_DEINIT) {
        group = IPC_GROUP_GPU_EVCP;
        // ENABLE_EVCP_E
        // LEVEL0_ICBM_S
    } else if (cmd >= IPC_ICBM_INIT && cmd <= IPC_ICBM_DEINIT) {
        group = IPC_GROUP_GPU_ICBM;
        // LEVEL0_ICBM_E
    } else {
        group = IPC_GROUP_CPU_OTHER;
    }

    return group;
}

const char* IntelAlgoServerThreadName(int index) {
    int count = 0;
#ifndef GPU_ALGO_SERVER
    static const char* gIpcCmdMapping[IPC_CPU_GROUP_NUM] = {
        "AiqCPUAlgoServer",  "PalCPUAlgoServer", "StatsCPUAlgoServer",
        "PsysCPUAlgoServer", "FdCPUAlgoServer",  "OtherCPUAlgoServer"};
    count = IPC_CPU_GROUP_NUM;
#else
    static const char* gIpcCmdMapping[IPC_GPU_GROUP_NUM] = {"GPUAlgoServer",
                                                            // ENABLE_EVCP_S
                                                            "GPUEvcpServer",
                                                            // ENABLE_EVCP_E
                                                            // LEVEL0_ICBM_S
                                                            "GPUICBMServer",
                                                            // LEVEL0_ICBM_E
                                                            "GPUAlgoServer2"};
    count = IPC_GPU_GROUP_NUM;
#endif

    return index < count ? gIpcCmdMapping[index] : "unknown";
}
} /* namespace icamera */
