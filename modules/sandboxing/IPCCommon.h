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

#pragma once

#include <ia_aiq_types.h>
#include <ia_cmc_types.h>
#include <ia_types.h>

#include "iutils/Errors.h"

namespace icamera {
#define IPC_MATCHING_KEY 0x56  // the value is randomly chosen
#define IPC_REQUEST_HEADER_USED_NUM 1
#define SHM_NAME "shm"

enum IPC_CMD {
    // CPU IPC COMMANDS
    IPC_FD_INIT,
    IPC_FD_RUN,
    IPC_FD_DEINIT,
    IPC_GRAPH_ADD_KEY,
    IPC_GRAPH_PARSE,
    IPC_GRAPH_RELEASE_NODES,
    IPC_GRAPH_QUERY_GRAPH_SETTINGS,
    IPC_GRAPH_CONFIG_STREAMS,
    IPC_GRAPH_GET_CONFIG_DATA,
    IPC_GRAPH_GET_CONNECTION,
    IPC_GRAPH_GET_PG_ID,
    IPC_CCA_CONSTRUCT,
    IPC_CCA_DESTRUCT,
    IPC_CCA_INIT,
    IPC_CCA_SET_STATS,
    IPC_CCA_RUN_AEC,
    IPC_CCA_RUN_AIQ,
    IPC_CCA_RUN_LTM,
    IPC_CCA_UPDATE_ZOOM,
    IPC_CCA_RUN_DVS,
    IPC_CCA_GET_CMC,
    IPC_CCA_GET_MKN,
    IPC_CCA_GET_AIQD,
    IPC_CCA_UPDATE_TUNING,
    IPC_CCA_DEINIT,
    IPC_CCA_RUN_AIC,
    IPC_CCA_DECODE_STATS,
    IPC_CCA_GET_PAL_SIZE,
    IPC_PG_PARAM_INIT,
    IPC_PG_PARAM_PREPARE,
    IPC_PG_PARAM_ALLOCATE_PG,
    IPC_PG_PARAM_GET_FRAG_DESCS,
    IPC_PG_PARAM_PREPARE_PROGRAM,
    IPC_PG_PARAM_REGISTER_PAYLOADS,
    IPC_PG_PARAM_ENCODE,
    IPC_PG_PARAM_DECODE,
    IPC_PG_PARAM_DEINIT,
    // GPU IPC COMMANDS
    IPC_GPU_TNR_INIT,
    IPC_GPU_TNR_PREPARE_SURFACE,
    IPC_GPU_TNR_RUN_FRAME,
    IPC_GPU_TNR_PARAM_UPDATE,
    IPC_GPU_TNR_DEINIT,
    IPC_GPU_TNR_THREAD2_RUN_FRAME,
    IPC_GPU_TNR_THREAD2_PARAM_UPDATE,
};

#define MAX_IA_BINARY_DATA_SIZE 800000
struct ia_binary_data_mod {
    unsigned int size;
    char data[MAX_IA_BINARY_DATA_SIZE];
};

const char* IntelAlgoIpcCmdToString(IPC_CMD cmd);

enum IPC_GROUP {
    // IPC command group for cpu
    IPC_GROUP_AIQ,
    IPC_GROUP_PAL,
    IPC_GROUP_PSYS,
    IPC_GROUP_FD,
    IPC_GROUP_CPU_OTHER,
    // IPU command group for gpu
    IPC_GROUP_GPU,
    IPC_GROUP_GPU_THREAD2,
};
#define IPC_GROUP_NUM (IPC_GROUP_GPU_THREAD2 + 1)
#define IPC_CPU_GROUP_NUM (IPC_GROUP_CPU_OTHER + 1)
#define IPC_GPU_GROUP_NUM (IPC_GROUP_GPU_THREAD2 - IPC_GROUP_GPU + 1)

IPC_GROUP IntelAlgoIpcCmdToGroup(IPC_CMD cmd);
const char* IntelAlgoServerThreadName(int index);
} /* namespace icamera */
