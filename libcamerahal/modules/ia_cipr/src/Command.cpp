/*
 * Copyright (C) 2020-2021 Intel Corporation.
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

#define LOG_TAG CIPR_COMMAND

#include "modules/ia_cipr/include/Command.h"

#include <algorithm>

#include "iutils/CameraLog.h"
#include "iutils/Utils.h"

using icamera::CAMERA_DEBUG_LOG_INFO;
using icamera::CAMERA_DEBUG_LOG_ERR;
using icamera::CAMERA_DEBUG_LOG_WARNING;

namespace icamera {
namespace CIPR {
Command::Command(const PSysCommandConfig& cfg) {
    mInitialized = false;

    mCmd = new PSysCommand;

    if (cfg.buffers.size() > 0) {
        mCmd->iocCmd.bufcount = cfg.buffers.size();
        mCmd->iocCmd.buffers = static_cast<struct ipu_psys_buffer*>(
            CIPR::callocMemory(cfg.buffers.size(), sizeof(struct ipu_psys_buffer)));

        if (!mCmd->iocCmd.buffers) {
            LOGE("Failed to allocate memory for psys command");
            delete mCmd;
            return;
        }

        mCmd->userBuffers.resize(cfg.buffers.size());
        std::fill(mCmd->userBuffers.begin(), mCmd->userBuffers.end(), nullptr);
    }
    mInitialized = true;
}

Command::~Command() {
    if (mCmd) {
        CIPR::freeMemory(mCmd->iocCmd.buffers);
        delete mCmd;
    }
}

Result Command::getConfig(PSysCommandConfig* cfg) {
    CheckAndLogError(!cfg, Result::InvaildArg, "cfg is nullptr");

    cfg->token = mCmd->iocCmd.user_token;
    cfg->issueID = mCmd->iocCmd.issue_id;
    cfg->priority = mCmd->iocCmd.priority;
    cfg->psysFrequency = mCmd->iocCmd.min_psys_freq;
    cfg->pg = mCmd->pg;
    cfg->extBuf = mCmd->extBuf;
    cfg->pgManifestBuf = mCmd->pgManifestBuf;
    cfg->id = 0;

    cfg->buffers = mCmd->userBuffers;

    return Result::OK;
}

Result Command::updateKernel(const PSysCommandConfig& cfg, const MemoryDesc& memory) {
    ProcessGroupCommand* ppg_command_ext = reinterpret_cast<ProcessGroupCommand*>(memory.cpuPtr);

    CheckAndLogError(ppg_command_ext->header.size != memory.size ||
                     ppg_command_ext->header.offset != sizeof(PSysCmdExtHeader) ||
                     (ppg_command_ext->header.version != psys_command_ext_ppg_0 &&
                     ppg_command_ext->header.version != psys_command_ext_ppg_1),
                     Result::InvaildArg, "Invalid command extension buffer received! (%p)",
                     cfg.extBuf);

    if (ppg_command_ext->header.version == psys_command_ext_ppg_1) {
        CheckAndLogError(sizeof(mCmd->iocCmd.kernel_enable_bitmap) !=
                         sizeof(ppg_command_ext->dynamicKernelBitmap), Result::DataError,
                         "Invalid bitmap buffer size");
        MEMCPY_S(
            &(mCmd->iocCmd.kernel_enable_bitmap), sizeof(mCmd->iocCmd.kernel_enable_bitmap),
            ppg_command_ext->dynamicKernelBitmap, sizeof(ppg_command_ext->dynamicKernelBitmap));
    }

    mCmd->iocCmd.frame_counter = static_cast<uint32_t>(ppg_command_ext->frameCounter);

    return Result::OK;
}

Result Command::getLegacyPGMem(const PSysCommandConfig& cfg, MemoryDesc* memory) {
    auto ret = cfg.extBuf->getMemory(memory);
    if (ret != Result::OK && cfg.pgParamsBuf != nullptr) {
        LOG2("pg_params_buf-field of CIPR command deprecated!");
        ret = cfg.pgParamsBuf->getMemory(memory);
        if (ret != Result::OK) {
            return ret;
        }
    }

    return Result::OK;
}

Result Command::updatePG(const PSysCommandConfig& cfg) {
    MemoryDesc memory = {};

    Result ret = cfg.pgManifestBuf->getMemory(&memory);
    if (ret != Result::OK) return ret;

    mCmd->iocCmd.pg_manifest = memory.cpuPtr;
    mCmd->iocCmd.pg_manifest_size = memory.size;
    mCmd->pgManifestBuf = cfg.pgManifestBuf;

    ret = getLegacyPGMem(cfg, &memory);
    CheckAndLogError(ret != Result::OK, ret, "Failed to get legacy PG memory");

    if (memory.flags & MemoryFlag::PSysAPI) {
        ret = updateKernel(cfg, memory);
    } else {
        mCmd->pgParams = memory.cpuPtr;
        mCmd->pgParamsSize = memory.size;
    }

    return ret;
}

Result Command::setConfig(const PSysCommandConfig& cfg) {
    CheckAndLogError(mCmd->userBuffers.size() < cfg.buffers.size(), Result::InvaildArg,
                     "Config bufcount cannot be higher than in the command!");
    CheckAndLogError(cfg.buffers.empty() && mCmd->iocCmd.buffers, Result::InvaildArg,
                     "To nullify buffers, create command with bufcount 0");

    Result ret = updatePG(cfg);
    CheckAndLogError(ret != Result::OK, ret, "Failed to update PG");

    mCmd->extBuf = cfg.extBuf;

    mCmd->iocCmd.user_token = cfg.token;
    mCmd->iocCmd.issue_id = cfg.issueID;
    mCmd->iocCmd.priority = cfg.priority;
    mCmd->iocCmd.min_psys_freq = cfg.psysFrequency;
    mCmd->iocCmd.bufcount = cfg.buffers.size();
    mCmd->pg = cfg.pg;
    if (cfg.pg && cfg.pg->mMemoryDesc.sysBuff) {
        CheckAndLogError(!((cfg.pg->mMemoryDesc.sysBuff)->flags & IPU_BUFFER_FLAG_DMA_HANDLE),
                         Result::GeneralError, "Wrong flag and not a DMA handle");

        mCmd->iocCmd.pg = cfg.pg->mMemoryDesc.sysBuff->base.fd;
    }

    if (!mCmd->iocCmd.buffers) return Result::OK;

    return grokBuffers(cfg);
}

Result Command::grokBuffers(const PSysCommandConfig& cfg) {
    for (size_t i = 0; i < cfg.buffers.size(); ++i) {
        auto current = cfg.buffers[i];
        if (!current) {
            memset(&mCmd->iocCmd.buffers[i], 0, sizeof(mCmd->iocCmd.buffers[i]));
            continue;
        } else if (!current->mMemoryDesc.sysBuff && current->isRegion()) {
            current = current->getParent();
            CheckAndLogError(!current, Result::InvaildArg, "Cannot find the parent of buffer:%p",
                             cfg.buffers[i]);
        }

        CheckAndLogError(
            !current->mMemoryDesc.sysBuff || !(current->mMemoryDesc.flags & MemoryFlag::Migrated),
            Result::InvaildArg, "Cannot queue singular buffer object %p", current);

        mCmd->iocCmd.buffers[i] = *current->mMemoryDesc.sysBuff;
        mCmd->iocCmd.buffers[i].data_offset = current->mOffset;
        mCmd->iocCmd.buffers[i].bytes_used = current->mMemoryDesc.size;
    }

    return Result::OK;
}

Result Command::enqueue(Context* ctx) {
    CheckAndLogError(!ctx, Result::InvaildArg, "Context is nullptr");

    return ctx->doIoctl(static_cast<int>(IPU_IOC_QCMD), mCmd);
}

}  // namespace CIPR
}  // namespace icamera
