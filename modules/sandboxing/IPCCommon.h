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
    IPC_LARD_INIT = 1,
    IPC_LARD_GET_TAG_LIST,
    IPC_LARD_RUN,
    IPC_LARD_DEINIT,
    IPC_FD_INIT,
    IPC_FD_RUN,
    IPC_FD_DEINIT,
    IPC_GRAPH_ADD_KEY,
    IPC_GRAPH_PARSE,
    IPC_GRAPH_RELEASE_NODES,
    IPC_GRAPH_CONFIG_STREAMS,
    IPC_GRAPH_GET_CONFIG_DATA,
    IPC_GRAPH_GET_CONNECTION,
    IPC_GRAPH_GET_PG_ID,
    IPC_CMC_INIT,
    IPC_CMC_DEINIT,
    IPC_MKN_INIT,
    IPC_MKN_ENABLE,
    IPC_MKN_PREPARE,
    IPC_MKN_DEINIT,
    IPC_LTM_INIT,
    IPC_LTM_RUN,
    IPC_LTM_DEINIT,
    IPC_AIQ_INIT,
    IPC_AIQ_AE_RUN,
    IPC_AIQ_AF_RUN,
    IPC_AIQ_AWB_RUN,
    IPC_AIQ_GBCE_RUN,
    IPC_AIQ_PA_RUN_V1,
    IPC_AIQ_SA_RUN_V2,
    IPC_AIQ_STATISTICS_SET_V4,
    IPC_AIQ_GET_AIQD_DATA,
    IPC_AIQ_DEINIT,
    IPC_AIQ_GET_VERSION,
    IPC_DVS_INIT,
    IPC_DVS_CONFIG,
    IPC_DVS_SET_NONE_BLANK_RATION,
    IPC_DVS_SET_DIGITAL_ZOOM_MODE,
    IPC_DVS_SET_DIGITAL_ZOOM_REGION,
    IPC_DVS_SET_DIGITAL_ZOOM_COORDINATE,
    IPC_DVS_SET_DIGITAL_ZOOM_MAGNITUDE,
    IPC_DVS_FREE_MORPH_TABLE,
    IPC_DVS_ALLOCATE_MORPH_TABLE,
    IPC_DVS_GET_MORPH_TABLE,
    IPC_DVS_SET_STATISTICS,
    IPC_DVS_EXECUTE,
    IPC_DVS_GET_IMAGE_TRANSFORMATION,
    IPC_DVS_DEINIT,
    IPC_ISP_ADAPTOR_INIT,
    IPC_ISP_ADAPTOR_DEINIT,
    IPC_ISP_GET_PAL_SIZE,
    IPC_ISP_CONVERT_STATS,
    IPC_ISP_RUN_PAL,
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
    IPC_GPU_TNR_DEINIT
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
};
#define IPC_GROUP_NUM (IPC_GROUP_GPU + 1)
#define IPC_CPU_GROUP_NUM (IPC_GROUP_CPU_OTHER + 1)
#define IPC_GPU_GROUP_NUM (IPC_GROUP_NUM - IPC_GROUP_GPU)

IPC_GROUP IntelAlgoIpcCmdToGroup(IPC_CMD cmd);
const char* IntelAlgoServerThreadName(int index);
} /* namespace icamera */
