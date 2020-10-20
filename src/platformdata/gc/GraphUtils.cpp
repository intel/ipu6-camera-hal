/*
 * Copyright (C) 2018-2019 Intel Corporation
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
#define LOG_TAG "GraphUtils"

#include "GraphUtils.h"
#include "iutils/CameraLog.h"
#include "iutils/Utils.h"

#define psys_2600_pg_uid(id) ia_fourcc(((id & 0xFF00) >> 8),id,'G','0')
#define psys_2600_pg_id_from_uid(uid) ((uid & 0xFFFF0000) >> 16)
#define psys_2600_term_idx_from_uid(uid) ((uid & 0x0000FFFF) - 1)

namespace icamera {

void GraphUtils::dumpConnections(const std::vector<IGraphType::PipelineConnection>& connections)
{
    if (!Log::isDebugLevelEnable(CAMERA_DEBUG_LOG_GRAPH)) {
        return;
    }

    LOGG("Graph connections:");
    for (auto& conn : connections) {

        LOGG("Format settings: enabled === %d ===, terminalIdx %d, width %d, height %d, fourcc %s, bpl %d, bpp %d",
                    conn.portFormatSettings.enabled,
                    conn.portFormatSettings.terminalId,
                    conn.portFormatSettings.width, conn.portFormatSettings.height,
                    CameraUtils::fourcc2String(conn.portFormatSettings.fourcc).c_str(),
                    conn.portFormatSettings.bpl, conn.portFormatSettings.bpp);

        LOGG("Connection config: sourceStage %d(%d), sourceTerminal %d(%d), sourceIteration %d, " \
                    "sinkStage %d(%d), sinkTerminal %d(%d), sinkIteration %d, connectionType %d",
                    psys_2600_pg_id_from_uid(conn.connectionConfig.mSourceStage),
                    conn.connectionConfig.mSourceStage,
                    conn.connectionConfig.mSourceTerminal - conn.connectionConfig.mSourceStage -1,
                    conn.connectionConfig.mSourceTerminal,
                    conn.connectionConfig.mSourceIteration,
                    psys_2600_pg_id_from_uid(conn.connectionConfig.mSinkStage),
                    conn.connectionConfig.mSinkStage,
                    conn.connectionConfig.mSinkTerminal - conn.connectionConfig.mSinkStage -1,
                    conn.connectionConfig.mSinkTerminal,
                    conn.connectionConfig.mSinkIteration,
                    conn.connectionConfig.mConnectionType);

        LOGG("Edge port: %d", conn.hasEdgePort);
    }

    return;
}

void GraphUtils::dumpKernelInfo(const ia_isp_bxt_program_group& programGroup)
{
    if (!Log::isDebugLevelEnable(CAMERA_DEBUG_LOG_GRAPH)) {
        return;
    }

    LOGG("Kernel info: count %d, opMode %d", programGroup.kernel_count, programGroup.operation_mode);

    for(unsigned int i = 0; i< programGroup.kernel_count; i++) {

        const ia_isp_bxt_run_kernels_t& curRunKernel = programGroup.run_kernels[i];

        LOGG("uid %d, streamId: %d, enabled %d", curRunKernel.kernel_uuid, curRunKernel.stream_id,
                    curRunKernel.enable);

        if (programGroup.run_kernels[i].resolution_info) {
            LOGG("Resolution: inputWidth %d, inputHeight %d, inputCrop %d %d %d %d," \
                       "outputWidth %d, outputHeight %d, outputCrop %d %d %d %d,",
                       curRunKernel.resolution_info->input_width, curRunKernel.resolution_info->input_height,
                       curRunKernel.resolution_info->input_crop.left, curRunKernel.resolution_info->input_crop.top,
                       curRunKernel.resolution_info->input_crop.right, curRunKernel.resolution_info->input_crop.bottom,
                       curRunKernel.resolution_info->output_width, curRunKernel.resolution_info->output_height,
                       curRunKernel.resolution_info->output_crop.left, curRunKernel.resolution_info->output_crop.top,
                       curRunKernel.resolution_info->output_crop.right, curRunKernel.resolution_info->output_crop.bottom);
        }

        if (programGroup.run_kernels[i].resolution_history) {
            LOGG("Resolution history: inputWidth %d, inputHeight %d, inputCrop %d %d %d %d," \
                       "outputWidth %d, outputHeight %d, outputCrop %d %d %d %d,",
                       curRunKernel.resolution_history->input_width, curRunKernel.resolution_history->input_height,
                       curRunKernel.resolution_history->input_crop.left, curRunKernel.resolution_history->input_crop.top,
                       curRunKernel.resolution_history->input_crop.right, curRunKernel.resolution_history->input_crop.bottom,
                       curRunKernel.resolution_history->output_width, curRunKernel.resolution_history->output_height,
                       curRunKernel.resolution_history->output_crop.left, curRunKernel.resolution_history->output_crop.top,
                       curRunKernel.resolution_history->output_crop.right, curRunKernel.resolution_history->output_crop.bottom);

        }

        LOGG("metadata %d %d %d %d, bppInfo: %d %d, outputCount %d",
                   curRunKernel.metadata[0], curRunKernel.metadata[1], curRunKernel.metadata[2], curRunKernel.metadata[3],
                   curRunKernel.bpp_info.input_bpp, curRunKernel.bpp_info.output_bpp,
                   curRunKernel.output_count);
    }

    return;
}
}
