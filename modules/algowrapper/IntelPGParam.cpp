/*
 * Copyright (C) 2018-2020 Intel Corporation.
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

#define LOG_TAG "IntelPGParam"

#include "modules/algowrapper/IntelPGParam.h"

#include <algorithm>

#include "iutils/CameraLog.h"
#include "iutils/Utils.h"

namespace icamera {

IntelPGParam::IntelPGParam(int pgId)
        : mPgId(pgId),
          mTerminalCount(0),
          mFragmentCount(0),
          mFragmentDesc(nullptr),
          mFragmentConfig(nullptr),
          mP2pHandle(nullptr),
          mPgManifest(nullptr),
          mProcessGroup(nullptr),
          mProgramControlInitTerminalIndex(-1),
          mProcessGroupMemory(nullptr) {
    CLEAR(mP2pCacheBuffer);
}

IntelPGParam::~IntelPGParam() {
    if (mFragmentDesc) {
        delete[] mFragmentDesc;
    }
    if (mFragmentConfig) {
        delete mFragmentConfig;
    }

    for (int i = 0; i < mTerminalCount; i++) {
        if (mPgReqs.terminals[i].kernelOrder) {
            delete[] mPgReqs.terminals[i].kernelOrder;
        }
    }

    destroyPayloads();
    destroyPGBuffer();
}

int IntelPGParam::init(ia_p2p_platform_t platform, const PgConfiguration& pgConfig) {
    mP2pHandle = ia_p2p_init(platform);
    CheckError(!mP2pHandle, UNKNOWN_ERROR, "ia_p2p_init has failed");

    mP2pCacheBuffer.size = ia_p2p_get_cache_buffer_size(mP2pHandle);
    mP2pCacheBuffer.data = CIPR::callocMemory(1, mP2pCacheBuffer.size);
    LOG1("%s: mP2pCacheBuffer.size=%d", __func__, mP2pCacheBuffer.size);
    CheckError(!mP2pCacheBuffer.data, UNKNOWN_ERROR, "Failed to allocate P2P cache buffer.");

    mPgManifest = pgConfig.pgManifest;
    mDisableDataTermials = pgConfig.disableDataTermials;
    mTerminalCount = ia_css_program_group_manifest_get_terminal_count(mPgManifest);

    mFragmentCount = pgConfig.fragmentCount;
    mInputMainFrame = pgConfig.inputMainFrame;
    mOutputMainFrame = pgConfig.outputMainFrame;

    return OK;
}

int IntelPGParam::calcFragmentDescriptors(int fragmentCount, const PgFrameDesc& inputMainFrame,
                                          const PgFrameDesc& outputMainFrame,
                                          const ia_css_rbm_t* rbm) {
    if (mFragmentDesc) {
        delete[] mFragmentDesc;
        mFragmentDesc = nullptr;
    }

    delete mFragmentConfig;
    mFragmentConfig = nullptr;

    mFragmentDesc = new ia_p2p_fragment_desc[FRAG_TERM_TYPE_COUNT * fragmentCount];
    memset(mFragmentDesc, 0x0, sizeof(ia_p2p_fragment_desc) * FRAG_TERM_TYPE_COUNT * fragmentCount);
    if (fragmentCount <= 1) {
        ia_p2p_fragment_desc desc;
        desc.fragment_width = inputMainFrame.width;
        desc.fragment_height = inputMainFrame.height;
        desc.fragment_start_x = 0;
        desc.fragment_start_y = 0;
        for (int i = 0; i < FRAG_TERM_TYPE_COUNT; i++) {
            mFragmentDesc[i] = desc;
        }
    }

    mFragmentConfig = new ia_p2p_fragment_configuration_t;
    CLEAR(*mFragmentConfig);
    int ret = ia_p2p_calculate_fragments_rbm(mP2pHandle, mPgId, (unsigned int)fragmentCount, rbm,
                                             nullptr,  // only for IA_P2P_PLATFORM_IPU6 now
                                             mFragmentConfig);

    dumpFragmentDesc(fragmentCount);
    return ret;
}

static int kernel_id_ffs(ia_css_kernel_bitmap_t bitmap) {
    int n = 0;
    if (ia_css_is_kernel_bitmap_empty(bitmap)) return -1;
    while (!ia_css_is_kernel_bitmap_set(bitmap, (unsigned int)n)) n++;
    return n;
}

int IntelPGParam::getFragmentDescriptors(int descCount, ia_p2p_fragment_desc* descs) {
    CheckError(descCount < mTerminalCount * mFragmentCount, BAD_VALUE, "descCount is small",
               descCount);

    int descLen = sizeof(ia_p2p_fragment_desc) * mFragmentCount;
    int terminalCount = ia_css_process_group_get_terminal_count(mProcessGroup);
    for (int i = 0; i < terminalCount; i++) {
        ia_css_terminal_t* terminal = ia_css_process_group_get_terminal(mProcessGroup, i);
        CheckError(!terminal, BAD_VALUE, "terminal is nullptr");

        int termIdx = terminal->tm_index;
        if ((mPgReqs.terminals[termIdx].type != IA_CSS_TERMINAL_TYPE_DATA_OUT) &&
            (mPgReqs.terminals[termIdx].type != IA_CSS_TERMINAL_TYPE_DATA_IN)) {
            continue;
        }

        if (mFragmentConfig) {
            int kernelId = kernel_id_ffs(mPgReqs.terminals[termIdx].kernelBitmap);
            CheckError((kernelId < 0 || kernelId >= IA_CSS_KERNEL_BITMAP_BITS), -1,
                       "error terminal %d", termIdx);
            MEMCPY_S(&descs[termIdx * mFragmentCount], descLen,
                     mFragmentConfig->pixel_fragment_descs[kernelId], descLen);
            LOG2("PG %d: Terminal %d: selected fragment desc (<%d,%d> %dx%d) with kernel id %d",
                 mPgId, termIdx, descs[termIdx].fragment_start_x, descs[termIdx].fragment_start_y,
                 descs[termIdx].fragment_width, descs[termIdx].fragment_height, kernelId);
        } else {
            /* PG uses legacy fragment calculation logic */
            MEMCPY_S(&descs[termIdx * mFragmentCount], descLen,
                     mPgReqs.terminals[termIdx].fragment_descs, descLen);
            LOG2("PG %d: Terminal %d: selected legacy fragment descriptor (<%d,%d> %dx%d)", mPgId,
                 termIdx, descs[termIdx].fragment_start_x, descs[termIdx].fragment_start_y,
                 descs[termIdx].fragment_width, descs[termIdx].fragment_height);
        }
    }
    return mFragmentCount;
}

int IntelPGParam::prepare(const ia_binary_data* ipuParameters, const ia_css_rbm_t* rbm,
                          ia_css_kernel_bitmap_t* bitmap, uint32_t* maxStatsSize) {
    CheckError(ipuParameters == nullptr || bitmap == nullptr, BAD_VALUE,
               "The input paramter is nullptr.");

    ia_css_terminal_type_t terminalType;
    int8_t termIndex;
    int kernelId = 0;

    ia_err err = ia_p2p_parse(mP2pHandle, ipuParameters, mP2pCacheBuffer.data);
    CheckError(err != ia_err_none, UNKNOWN_ERROR, "Failed to parse PAL data.");

    int ret = calcFragmentDescriptors(mFragmentCount, mInputMainFrame, mOutputMainFrame, rbm);
    CheckError(ret != OK, ret, "Failed to calc fragment desc.");

    int outputDataTerminalCount = FRAG_TERM_TYPE_OUTPUT_START;
    for (termIndex = 0; termIndex < mTerminalCount; termIndex++) {
        ia_css_terminal_manifest_t* terminalManifest =
            ia_css_program_group_manifest_get_term_mnfst(mPgManifest, (unsigned int)termIndex);
        CheckError(!terminalManifest, css_err_internal, "No terminal manifest for terminal %d",
                   termIndex);

        terminalType = ia_css_terminal_manifest_get_type(terminalManifest);
        mPgReqs.terminals[termIndex].type = terminalType;
        mPgReqs.terminals[termIndex].kernelOrder = nullptr;
        size_t kernelInfoSize = PSYS_MAX_KERNELS_PER_PG * sizeof(IpuPgTerminalKernelInfo);

        switch (terminalType) {
            case IA_CSS_TERMINAL_TYPE_PARAM_CACHED_IN:
            case IA_CSS_TERMINAL_TYPE_PARAM_CACHED_OUT: {
                ia_css_param_terminal_manifest_t* paramMani =
                    reinterpret_cast<ia_css_param_terminal_manifest_t*>(terminalManifest);
                mPgReqs.terminals[termIndex].kernelOrder =
                    new IpuPgTerminalKernelInfo[PSYS_MAX_KERNELS_PER_PG];
                memset(reinterpret_cast<void*>(mPgReqs.terminals[termIndex].kernelOrder), UINT8_MAX,
                       kernelInfoSize);
                ret = getKernelOrderForParamCachedInTerm(paramMani,
                                                         mPgReqs.terminals[termIndex].kernelOrder);
                CheckError(ret != css_err_none, ret, "getKernelOrderForParamCachedInTerm failed");
                break;
            }
            case IA_CSS_TERMINAL_TYPE_PROGRAM: {
                ia_css_program_terminal_manifest_t* proMani =
                    reinterpret_cast<ia_css_program_terminal_manifest_t*>(terminalManifest);
                mPgReqs.terminals[termIndex].kernelOrder =
                    new IpuPgTerminalKernelInfo[PSYS_MAX_KERNELS_PER_PG];
                memset(reinterpret_cast<void*>(mPgReqs.terminals[termIndex].kernelOrder), UINT8_MAX,
                       kernelInfoSize);

                ret =
                    getKernelOrderForProgramTerm(proMani, mPgReqs.terminals[termIndex].kernelOrder);
                CheckError(ret != css_err_none, ret, "getKernelOrderForProgramTerm failed");
                break;
            }
            case IA_CSS_TERMINAL_TYPE_DATA_IN: {
                ia_css_data_terminal_manifest_t* dataMani =
                    reinterpret_cast<ia_css_data_terminal_manifest_t*>(terminalManifest);
                /**
                 *Save the kernel bitmaps so that it can later be determined
                 * whether the terminals are disabled or not.
                 */
                mPgReqs.terminals[termIndex].kernelBitmap =
                    ia_css_data_terminal_manifest_get_kernel_bitmap(dataMani);
                if (!mFragmentConfig) {
                    mPgReqs.terminals[termIndex].fragment_descs =
                        &mFragmentDesc[FRAG_TERM_TYPE_INPUT * mFragmentCount];
                }
                break;
            }
            case IA_CSS_TERMINAL_TYPE_DATA_OUT: {
                ia_css_data_terminal_manifest_t* dataMani =
                    reinterpret_cast<ia_css_data_terminal_manifest_t*>(terminalManifest);
                /**
                 * Save the kernel bitmaps so that it can later be determined
                 * whether the terminals are disabled or not.
                 */
                mPgReqs.terminals[termIndex].kernelBitmap =
                    ia_css_data_terminal_manifest_get_kernel_bitmap(dataMani);
                if (!mFragmentConfig) {
                    mPgReqs.terminals[termIndex].fragment_descs =
                        &mFragmentDesc[outputDataTerminalCount * mFragmentCount];
                    outputDataTerminalCount++;
                }
                break;
            }
            case IA_CSS_TERMINAL_TYPE_PARAM_SPATIAL_IN:
            case IA_CSS_TERMINAL_TYPE_PARAM_SPATIAL_OUT: {
                ia_css_spatial_param_terminal_manifest_t* paramMani =
                    reinterpret_cast<ia_css_spatial_param_terminal_manifest_t*>(terminalManifest);
                kernelId = (int32_t)(paramMani->kernel_id);
                mPgReqs.terminals[termIndex].kernelBitmap =
                    ia_css_kernel_bit_mask((uint32_t)kernelId);
                break;
            }
            case IA_CSS_TERMINAL_TYPE_PROGRAM_CONTROL_INIT:
                /* Calculate the payload later when the final bitmask is known,
                 * in imaging_pipe_ctrl_identify_property, where it is actually
                 * needed. Just save here the index to the terminal. */
                mProgramControlInitTerminalIndex = termIndex;
                break;
            default:
                break;
        }
    }

    mPgReqs.terminalCount = mTerminalCount;

    ia_css_kernel_bitmap_t kernelBitmap = ia_p2p_get_kernel_bitmap(mP2pHandle, mPgId);
    kernelBitmap = ia_css_kernel_bitmap_intersection(
        kernelBitmap, ia_css_program_group_manifest_get_kernel_bitmap(mPgManifest));

    while (!ia_css_is_kernel_bitmap_empty(kernelBitmap)) {
        kernelId = getKernelIdByBitmap(kernelBitmap);
        CheckError((kernelId < 0 || kernelId >= PSYS_MAX_KERNELS_PER_PG), ia_err_internal,
                   "kernelId is out of range! %d", kernelId);

        /* Get terminal requirements */
        ret = ia_p2p_get_kernel_terminal_requirements(mP2pHandle, mPgId, (uint32_t)kernelId,
                                                      &mKernel.mSections[kernelId]);
        CheckError(ret != ia_err_none, ret, "%s: failed to get requirements for pg %d kernel %d",
                   __func__, mPgId, kernelId);

        /* Get payload descriptor */
        ret = ia_p2p_get_kernel_payload_desc(
            mP2pHandle, mPgId, (uint32_t)kernelId,
#if defined(IPU_SYSVER_IPU6) && defined(UNIFIED_PROG_TERM_FRAG_DESC)
            1,
#else
            mFragmentCount,
#endif
            (mFragmentConfig ? mFragmentConfig->pixel_fragment_descs[kernelId]
                             : &mFragmentDesc[FRAG_TERM_TYPE_INPUT * mFragmentCount]),
            &mKernel.mPayloads[kernelId]);
        CheckError(ret != ia_err_none, ret, "%s: failed to get payload for pg %d kernel %d, ret %d",
                   __func__, mPgId, kernelId, ret);

        uint8_t kernelOrder = 0;
        termIndex = -1;
        if (mKernel.mSections[kernelId].param_in_section_count) {
            terminalType = IA_CSS_TERMINAL_TYPE_PARAM_CACHED_IN;

            /* P2P assumes single CACHED IN, cumulate to first */
            termIndex = terminalEnumerateByType(&mPgReqs, terminalType, 0);
            CheckError(termIndex < 0, ia_err_internal, "No PARAM_CACHED_IN according to manifest!");
            if (isKernelIdInKernelOrder(&mPgReqs, termIndex, kernelId, &kernelOrder)) {
                if (mPgReqs.terminals[termIndex].kernelOrder[kernelOrder].sections !=
                    mKernel.mSections[kernelId].param_in_section_count) {
                    LOGW("%s: p2p cached in section count differs (kernel_id:%i p2p:%d vs pg:%d)",
                         __func__, kernelId, mKernel.mSections[kernelId].param_in_section_count,
                         mPgReqs.terminals[termIndex].kernelOrder[kernelOrder].sections);
                    /* Overwrite P2P requirements with manifest */
                    mKernel.mSections[kernelId].param_in_section_count =
                        mPgReqs.terminals[termIndex].kernelOrder[kernelOrder].sections;
                    mKernel.mPayloads[kernelId].param_in_payload_size =
                        std::max(mKernel.mPayloads[kernelId].param_in_payload_size,
                                 mPgReqs.terminals[termIndex].kernelOrder[kernelOrder].size);
                    mPgReqs.terminals[termIndex].kernelOrder[kernelOrder].initialize = true;
                }
                processTerminalKernelRequirements(&mPgReqs, termIndex, terminalType, kernelId);
            }
        }

        if (mKernel.mSections[kernelId].param_out_section_count_per_fragment) {
            terminalType = IA_CSS_TERMINAL_TYPE_PARAM_CACHED_OUT;
            for (termIndex = 0; termIndex < mTerminalCount; termIndex++) {
                if (mPgReqs.terminals[termIndex].type != terminalType) {
                    continue;
                }

                if (isKernelIdInKernelOrder(&mPgReqs, termIndex, kernelId, nullptr)) {
                    processTerminalKernelRequirements(&mPgReqs, termIndex, terminalType, kernelId);
                }
            }
        }

        if (mKernel.mSections[kernelId].program_section_count_per_fragment) {
            terminalType = IA_CSS_TERMINAL_TYPE_PROGRAM;
            for (termIndex = 0; termIndex < mTerminalCount; termIndex++) {
                if (mPgReqs.terminals[termIndex].type != terminalType) {
                    continue;
                }
                if (isKernelIdInKernelOrder(&mPgReqs, termIndex, kernelId, &kernelOrder)) {
                    if (mPgReqs.terminals[termIndex].kernelOrder[kernelOrder].sections !=
                        mKernel.mSections[kernelId].program_section_count_per_fragment) {
                        LOGW("%s: p2p program section count differs (kernel_id:%i p2p:%d vs pg:%d)",
                             __func__, kernelId,
                             mKernel.mSections[kernelId].program_section_count_per_fragment,
                             mPgReqs.terminals[termIndex].kernelOrder[kernelOrder].sections);
                        /* Overwrite P2P requirements with manifest */
                        mKernel.mSections[kernelId].program_section_count_per_fragment =
                            mPgReqs.terminals[termIndex].kernelOrder[kernelOrder].sections;
                        mKernel.mPayloads[kernelId].program_payload_size =
                            std::max(mKernel.mPayloads[kernelId].program_payload_size,
                                     mPgReqs.terminals[termIndex].kernelOrder[kernelOrder].size);
                        mPgReqs.terminals[termIndex].kernelOrder[kernelOrder].initialize = true;
                    }
                    processTerminalKernelRequirements(&mPgReqs, termIndex, terminalType, kernelId);
                }
            }
        }

        /* P2P assumes each spatial kernel parameter has its own terminal */
        if (mKernel.mSections[kernelId].spatial_param_in_section_count) {
            terminalType = IA_CSS_TERMINAL_TYPE_PARAM_SPATIAL_IN;
            termIndex = terminalEnumerateByBitmap(&mPgReqs, terminalType,
                                                  ia_css_kernel_bit_mask((uint32_t)kernelId));
            if (termIndex < 0) {
                LOG1("%s: No PARAM_SPATIAL_IN for kernel id %d according to manifest!", __func__,
                     kernelId);
            } else if (isKernelIdInKernelOrder(&mPgReqs, termIndex, kernelId, nullptr)) {
                mPgReqs.terminals[termIndex].sectionCount +=
                    mKernel.mSections[kernelId].spatial_param_in_section_count;
                mPgReqs.terminals[termIndex].payloadSize +=
                    mKernel.mPayloads[kernelId].spatial_param_in_payload_size;
                mPgReqs.terminals[termIndex].kernelBitmap =
                    ia_css_kernel_bit_mask((uint32_t)kernelId);
            }
        }

        if (mKernel.mSections[kernelId].spatial_param_out_section_count) {
            terminalType = IA_CSS_TERMINAL_TYPE_PARAM_SPATIAL_OUT;
            termIndex = terminalEnumerateByBitmap(&mPgReqs, terminalType,
                                                  ia_css_kernel_bit_mask((uint32_t)kernelId));
            if (termIndex < 0) {
                LOG1("%s: No PARAM_SPATIAL_OUT for kernel id %d according to manifest!", __func__,
                     kernelId);
            } else if (isKernelIdInKernelOrder(&mPgReqs, termIndex, kernelId, nullptr)) {
                mPgReqs.terminals[termIndex].sectionCount +=
                    mKernel.mSections[kernelId].spatial_param_out_section_count;
                mPgReqs.terminals[termIndex].payloadSize +=
                    mKernel.mPayloads[kernelId].spatial_param_out_payload_size;
                mPgReqs.terminals[termIndex].kernelBitmap =
                    ia_css_kernel_bit_mask((uint32_t)kernelId);
            }
        }

        kernelBitmap = ia_css_kernel_bitmap_unset(kernelBitmap, (uint32_t)kernelId);
    }

    /* get all kernel bits back */
    kernelBitmap = ia_css_program_group_manifest_get_kernel_bitmap(mPgManifest);

    /* get disabled kernels from p2p and remove them */
    kernelBitmap = ia_css_kernel_bitmap_intersection(
        kernelBitmap,
        ia_css_kernel_bitmap_complement(ia_p2p_get_kernel_disable_bitmap(mP2pHandle, mPgId)));

    /* get disabled data terminal kernels and remove them */
    for (auto& item : mDisableDataTermials) {
        ia_css_terminal_manifest_t* terminalManifest =
            ia_css_program_group_manifest_get_term_mnfst(mPgManifest, (unsigned int)item);
        ia_css_kernel_bitmap_t dataTerminalKernelBitmap =
            ia_css_data_terminal_manifest_get_kernel_bitmap(
                reinterpret_cast<ia_css_data_terminal_manifest_t*>(terminalManifest));
        kernelBitmap = ia_css_kernel_bitmap_intersection(
            kernelBitmap, ia_css_kernel_bitmap_complement(dataTerminalKernelBitmap));
    }

    /* disable params terminals which payload size are zero */
    ret = disableZeroSizedTerminals(&kernelBitmap);
    CheckError(ret != OK, ret, "%s: failed to disable zero size terminals", __func__);

    *bitmap = kernelBitmap;

    if (maxStatsSize) *maxStatsSize = ia_p2p_get_statistics_buffer_size(mP2pHandle);
    return ret;
}

void* IntelPGParam::allocatePGBuffer(int pgSize) {
    destroyPGBuffer();
    void* memory = CIPR::mallocAlignedMemory(PAGE_ALIGN(pgSize), CIPR::getPageSize());
    mProcessGroupMemory = reinterpret_cast<ia_css_process_group_t*>(memory);
    return mProcessGroupMemory;
}

void IntelPGParam::destroyPGBuffer() {
    if (mProcessGroupMemory) {
        CIPR::freeMemory(mProcessGroupMemory);
        mProcessGroupMemory = nullptr;
    }
}

int IntelPGParam::setPGAndPrepareProgram(ia_css_process_group_t* pg) {
    CheckError(!pg, UNKNOWN_ERROR, "input pg nullptr!");
    mProcessGroup = pg;

    int ret = OK;
    int terminalCount = ia_css_process_group_get_terminal_count(mProcessGroup);
    for (int i = 0; i < terminalCount; i++) {
        ia_css_terminal_t* terminal = ia_css_process_group_get_terminal(mProcessGroup, i);
        CheckError(!terminal, UNKNOWN_ERROR, "failed to get terminal");
        int termIdx = terminal->tm_index;

        if (mPgReqs.terminals[termIdx].type == IA_CSS_TERMINAL_TYPE_PROGRAM_CONTROL_INIT) {
            unsigned int payloadSize = 0;
            ret = pg_control_init_get_payload_size(pg, &payloadSize);
            CheckError(ret != OK, UNKNOWN_ERROR, "call pg_control_init_get_payload_size fail");
            mPgReqs.terminals[termIdx].payloadSize = payloadSize;

            ret = pg_control_init_terminal_init(
                mProcessGroup, reinterpret_cast<ia_css_program_control_init_terminal_t*>(terminal));
            CheckError(ret != ia_err_none, ret, "Failed to call pg_control_init_terminal_init.");
        }

        if (mPgReqs.terminals[termIdx].type == IA_CSS_TERMINAL_TYPE_PROGRAM) {
            if (mFragmentConfig) {
                ret = ia_p2p_program_terminal_init_v2(
                    mP2pHandle, mPgId, mFragmentConfig,
                    reinterpret_cast<ia_css_program_terminal_t*>(terminal));
            } else {
                ret = ia_p2p_program_terminal_init(
                    mP2pHandle, mPgId, mFragmentCount, mFragmentDesc,
                    reinterpret_cast<ia_css_program_terminal_t*>(terminal));
            }
            CheckError(ret != ia_err_none, ret, "Failed to init program terminal.");
        }
    }
    return OK;
}

int IntelPGParam::getPayloadSizes(int payloadCount, ia_binary_data* payloads) {
    CheckError(payloadCount < mTerminalCount || !payloads, BAD_VALUE, "Can't get payload sizes!");
    for (int termIdx = 0; termIdx < mTerminalCount; termIdx++) {
        payloads[termIdx].size = mPgReqs.terminals[termIdx].payloadSize;
    }
    return mTerminalCount;
}

int IntelPGParam::allocatePayloads(int payloadCount, ia_binary_data* payloads) {
    CheckError(!payloads, BAD_VALUE, "nullptr payloads!");
    for (int idx = 0; idx < payloadCount; idx++) {
        ia_binary_data payload = {nullptr, payloads[idx].size};
        if (payload.size) {
            payload.data = CIPR::mallocAlignedMemory(PAGE_ALIGN(payload.size),
                CIPR::getPageSize());
            CheckError(!payload.data, BAD_VALUE, "no memory for payload size %d!", payload.size);
            mAllocatedPayloads.push_back(payload);
        }
        payloads[idx].data = payload.data;
    }
    return OK;
}

void IntelPGParam::destroyPayloads() {
    while (!mAllocatedPayloads.empty()) {
        if (mAllocatedPayloads.back().data)
            CIPR::freeMemory(mAllocatedPayloads.back().data);
        mAllocatedPayloads.pop_back();
    }
}

int IntelPGParam::updatePALAndEncode(const ia_binary_data* ipuParams, int payloadCount,
                                     ia_binary_data* payloads) {
    ia_err err = ia_p2p_parse(mP2pHandle, ipuParams, mP2pCacheBuffer.data);
    CheckError(err != ia_err_none, UNKNOWN_ERROR, "Failed to parse PAL data.");

    CheckError(!payloads, UNKNOWN_ERROR, "no payloads for encode.");
    CheckError(payloadCount < mTerminalCount, UNKNOWN_ERROR, "small payload count %d, should be %d",
               payloadCount, mTerminalCount);
    CheckError(!mProcessGroup, INVALID_OPERATION, "Can't encode due to null pg.");

    int ret = OK;
    int terminalCount = ia_css_process_group_get_terminal_count(mProcessGroup);
    ia_css_terminal_t* programControlInitTerminal = nullptr;
    for (int i = 0; i < terminalCount; i++) {
        ia_css_terminal_t* terminal = ia_css_process_group_get_terminal(mProcessGroup, i);
        CheckError(!terminal, UNKNOWN_ERROR, "failed to get terminal");
        if (!payloads[terminal->tm_index].data) {
            continue;
        }

        // Encode that terminal at last
        if (terminal->tm_index == mProgramControlInitTerminalIndex) {
            programControlInitTerminal = terminal;
            continue;
        }

        ret = encodeTerminal(terminal, payloads[terminal->tm_index]);
        CheckError(ret != OK, ret, "Failed to encode for terminal %d.", terminal->tm_index);
    }
    if (programControlInitTerminal) {
        ret = encodeTerminal(programControlInitTerminal,
                             payloads[programControlInitTerminal->tm_index]);
        CheckError(ret != OK, ret, "Failed to encode for program control init terminal %d.",
                   programControlInitTerminal->tm_index);
    }

    return ret;
}

int IntelPGParam::encodeTerminal(ia_css_terminal_t* terminal, ia_binary_data payload) {
    int ret = OK;

    int terminalIndex = terminal->tm_index;
    if (mPgReqs.terminals[terminalIndex].type == IA_CSS_TERMINAL_TYPE_PROGRAM_CONTROL_INIT) {
        unsigned int kupSize = 0;
        ret = ia_p2p_get_kernel_user_parameter_size(mP2pHandle, mPgId, mFragmentCount, &kupSize);
        CheckError(ret != ia_err_none, ret,
                   "Failed to call ia_p2p_get_kernel_user_parameter_size.");
        if (kupSize != mPgReqs.terminals[terminalIndex].userParamSize) {
            mPgReqs.terminals[terminalIndex].userParamSize = kupSize;
            mPgReqs.terminals[terminalIndex].userParamAddress =
                std::unique_ptr<uint8_t[]>(new uint8_t[kupSize]);
        }

        if (mFragmentConfig) {
            ret = ia_p2p_get_kernel_user_parameters_v2(
                mP2pHandle, mPgId, mFragmentCount, mFragmentConfig,
                mPgReqs.terminals[terminalIndex].userParamAddress.get());
            CheckError(ret != ia_err_none, ret,
                       "Failed to call ia_p2p_get_kernel_user_parameters_v2.");
        } else {
            ret = ia_p2p_get_kernel_user_parameters(
                mP2pHandle, mPgId, mFragmentCount, mFragmentDesc,
                mPgReqs.terminals[terminalIndex].userParamAddress.get());
            CheckError(ret != ia_err_none, ret,
                       "Failed to call ia_p2p_get_kernel_user_parameters.");
        }

        ia_css_kernel_user_param_t* userParam = reinterpret_cast<ia_css_kernel_user_param_t*>(
            mPgReqs.terminals[terminalIndex].userParamAddress.get());
        ret = pg_control_init_fill_payload(mProcessGroup, userParam, payload.data);
        CheckError(ret != ia_err_none, ret, "Failed to call pg_control_init_fill_payload.");
        return ret;
    }

    ia_css_kernel_bitmap_t kernelBitmap = mPgReqs.terminals[terminalIndex].kernelBitmap;
    uint16_t kernelId = 0;
    uint8_t kernelIndex = 0;
    unsigned int curSection = 0;
    unsigned int curOffset = 0;
    ia_p2p_payload_desc tmpPayloadDesc;
    while (!ia_css_is_kernel_bitmap_empty(kernelBitmap)) {
        /* Use specific ordering of kernels when available */
        if (mPgReqs.terminals[terminalIndex].kernelOrder) {
            kernelId = mPgReqs.terminals[terminalIndex].kernelOrder[kernelIndex++].id;
            if (kernelId >= PSYS_MAX_KERNELS_PER_PG) {
                /* All the kernels have now been encoded. */
                break;
            }
            /* Initialize parameter payload for current kernel with zeros in
             * case P2P has reported less sections for the kernel */
            if (mPgReqs.terminals[terminalIndex].kernelOrder[kernelIndex - 1].initialize) {
                LOG2("%s: initializing kernel %d payload in terminal %d (offset:%d, size:%d)",
                     __func__, kernelId, terminalIndex, curOffset,
                     mPgReqs.terminals[terminalIndex].kernelOrder[kernelIndex - 1].size);
                unsigned char* start = reinterpret_cast<unsigned char*>(payload.data);
                memset(start + curOffset, 0,
                       mPgReqs.terminals[terminalIndex].kernelOrder[kernelIndex - 1].size);
            }
        } else {
            kernelId = getKernelIdByBitmap(kernelBitmap);
        }

        /* Sanity check sections sizes and return the size to be used */
        css_err_t result = payloadSectionSizeSanityTest(&tmpPayloadDesc, kernelId, terminalIndex,
                                                        curOffset, payload.size);
        CheckError((result != css_err_none), UNKNOWN_ERROR,
                   "Failed sanity check of terminal payload sizes");

        switch (mPgReqs.terminals[terminalIndex].type) {
            case IA_CSS_TERMINAL_TYPE_PARAM_CACHED_IN:
                ret = ia_p2p_param_in_terminal_encode(
                    mP2pHandle, mPgId, kernelId,
                    reinterpret_cast<ia_css_param_terminal_t*>(terminal), curSection,
                    reinterpret_cast<uint8_t*>(payload.data), payload.size, curOffset);
                CheckError(ret != ia_err_none, ret, "Failed to encode param in terminal.");

                curSection += mKernel.mSections[kernelId].param_in_section_count;
                curOffset += tmpPayloadDesc.param_in_payload_size;
                break;

            case IA_CSS_TERMINAL_TYPE_PARAM_CACHED_OUT:
                ret = ia_p2p_param_out_terminal_prepare(
                    mP2pHandle, mPgId, kernelId, mFragmentCount,
                    (mFragmentConfig ? mFragmentConfig->pixel_fragment_descs[kernelId]
                                     : mFragmentDesc),
                    reinterpret_cast<ia_css_param_terminal_t*>(terminal), curSection,
                    mPgReqs.terminals[terminalIndex].sectionCount, payload.size, curOffset);
                CheckError(ret != ia_err_none, ret, "Failed to prepare param out terminal.");

                curSection += mKernel.mSections[kernelId].param_out_section_count_per_fragment;
                curOffset += tmpPayloadDesc.param_out_payload_size;
                break;

            case IA_CSS_TERMINAL_TYPE_PROGRAM:
#if defined(IPU_SYSVER_IPU6) && defined(UNIFIED_PROG_TERM_FRAG_DESC)
                reinterpret_cast<ia_css_program_terminal_t*>(terminal)->payload_fragment_stride =
                    mPgReqs.terminals[terminalIndex].payloadSize / mFragmentCount;
#endif
                ret = ia_p2p_program_terminal_encode(
                    mP2pHandle, mPgId, kernelId, mFragmentCount,
                    (mFragmentConfig ? mFragmentConfig->pixel_fragment_descs[kernelId]
                                     : mFragmentDesc),
                    reinterpret_cast<ia_css_program_terminal_t*>(terminal), curSection,
                    mPgReqs.terminals[terminalIndex].sectionCount,
                    reinterpret_cast<uint8_t*>(payload.data), payload.size, curOffset);
                CheckError(ret != ia_err_none, ret, "Failed to encode program terminal.");

                curSection += mKernel.mSections[kernelId].program_section_count_per_fragment;
                curOffset += tmpPayloadDesc.program_payload_size;
                break;

            case IA_CSS_TERMINAL_TYPE_PARAM_SPATIAL_IN:
                /* TODO: ensure program terminal gets encoded first */
                ret = ia_p2p_spatial_param_in_terminal_encode(
                    mP2pHandle, mPgId, kernelId, mFragmentCount,
                    (mFragmentConfig ? mFragmentConfig->pixel_fragment_descs[kernelId]
                                     : mFragmentDesc),
                    reinterpret_cast<ia_css_spatial_param_terminal_t*>(terminal), curSection,
                    reinterpret_cast<uint8_t*>(payload.data), payload.size, curOffset);
                CheckError(ret != ia_err_none, ret, "Failed to encode spatial in terminal.");

                curOffset += tmpPayloadDesc.spatial_param_in_payload_size;
                curSection += mKernel.mSections[kernelId].spatial_param_in_section_count;
                break;

            case IA_CSS_TERMINAL_TYPE_PARAM_SPATIAL_OUT:
                ret = ia_p2p_spatial_param_out_terminal_prepare(
                    mP2pHandle, mPgId, kernelId, mFragmentCount,
                    (mFragmentConfig ? mFragmentConfig->pixel_fragment_descs[kernelId]
                                     : mFragmentDesc),
                    reinterpret_cast<ia_css_spatial_param_terminal_t*>(terminal), curSection,
                    payload.size, curOffset);
                CheckError(ret != ia_err_none, ret, "Failed to prepare spatial out terminal.");

                curOffset += tmpPayloadDesc.spatial_param_out_payload_size;
                curSection += mKernel.mSections[kernelId].spatial_param_out_section_count;
                break;

            case IA_CSS_TERMINAL_TYPE_DATA_IN:
            case IA_CSS_TERMINAL_TYPE_DATA_OUT:
                /* No encode done for frame terminals */
                break;

            default:
                LOGE("%s: terminal type %d encode not implemented", __func__,
                     mPgReqs.terminals[terminalIndex].type);
                return UNKNOWN_ERROR;
        }

        if (!mPgReqs.terminals[terminalIndex].kernelOrder) {
            kernelBitmap = ia_css_kernel_bitmap_unset(kernelBitmap, kernelId);
        }
    }

    return ret;
}

int IntelPGParam::decode(int payloadCount, ia_binary_data* payload, ia_binary_data* statistics) {
    CheckError(!mProcessGroup, INVALID_OPERATION, "Can't decode due to null pg.");
    CheckError(!payload, INVALID_OPERATION, "nullptr payload.");

    if (statistics && statistics->data) {
        ia_p2p_set_statistics_buffer(mP2pHandle, statistics->data);
    }
    int ret = OK;
    int terminalCount = ia_css_process_group_get_terminal_count(mProcessGroup);
    for (int i = 0; i < terminalCount; i++) {
        ia_css_terminal_t* terminal = ia_css_process_group_get_terminal(mProcessGroup, i);
        CheckError(!terminal, UNKNOWN_ERROR, "failed to get terminal");
        if ((terminal->terminal_type != IA_CSS_TERMINAL_TYPE_PARAM_CACHED_OUT) &&
            (terminal->terminal_type != IA_CSS_TERMINAL_TYPE_PARAM_SPATIAL_OUT)) {
            continue;
        }
        CheckError(terminal->tm_index >= payloadCount, UNKNOWN_ERROR,
                   "no payload for term %d decoding", terminal->tm_index);
        ret = decodeTerminal(terminal, payload[terminal->tm_index]);
        CheckError(ret != OK, ret, "%s, call p2p decode fail", __func__);
    }

    return serializeDecodeCache(statistics);
}

int IntelPGParam::decodeTerminal(ia_css_terminal_t* terminal, ia_binary_data payload) {
    int ret = OK;
    int terminalIndex = terminal->tm_index;
    unsigned int currentSection = 0;
    int kernelIndex = 0;
    uint16_t kernelId;
    ia_css_kernel_bitmap_t kernelBitmap = mPgReqs.terminals[terminal->tm_index].kernelBitmap;
    while (!ia_css_is_kernel_bitmap_empty(kernelBitmap)) {
        /* Use specific ordering of kernels when available */
        if (mPgReqs.terminals[terminalIndex].kernelOrder) {
            kernelId = mPgReqs.terminals[terminalIndex].kernelOrder[kernelIndex++].id;
            CheckError(kernelId >= PSYS_MAX_KERNELS_PER_PG, css_err_internal,
                       "%s: Kernel bitmap for terminal %d covers more kernels than in manifest",
                       __func__, terminalIndex);
        } else {
            kernelId = getKernelIdByBitmap(kernelBitmap);
        }

        switch (mPgReqs.terminals[terminalIndex].type) {
            case IA_CSS_TERMINAL_TYPE_PARAM_CACHED_OUT:
                ret = ia_p2p_param_out_terminal_decode(
                    mP2pHandle, mPgId, kernelId, mFragmentCount,
                    reinterpret_cast<ia_css_param_terminal_t*>(terminal), currentSection,
                    mPgReqs.terminals[terminalIndex].sectionCount,
                    reinterpret_cast<unsigned char*>(payload.data), payload.size);
                currentSection += mKernel.mSections[kernelId].param_out_section_count_per_fragment;
                break;
            case IA_CSS_TERMINAL_TYPE_PARAM_SPATIAL_OUT:
                ret = ia_p2p_spatial_param_out_terminal_decode_v2(
                    mP2pHandle, mPgId, kernelId, mFragmentCount,
                    (mFragmentConfig ? mFragmentConfig->pixel_fragment_descs[kernelId]
                                     : mFragmentDesc),
                    reinterpret_cast<ia_css_spatial_param_terminal_t*>(terminal), currentSection,
                    (unsigned char*)payload.data, payload.size, mP2pCacheBuffer.data);
                currentSection += mKernel.mSections[kernelId].spatial_param_out_section_count;
                break;
            default:
                LOGE("%s: terminal type %d decode not implemented", __func__,
                     mPgReqs.terminals[terminalIndex].type);
                return UNKNOWN_ERROR;
        }

        CheckError(ret != ia_err_none, ret, "%s: failed to decode terminal %d", __func__,
                   terminalIndex);
        kernelBitmap = ia_css_kernel_bitmap_unset(kernelBitmap, kernelId);
    }

    return ret;
}

int IntelPGParam::serializeDecodeCache(ia_binary_data* result) {
    CheckError(!result, UNKNOWN_ERROR, "The statistics buffer is nullptr");

    ia_err ia_ret = ia_p2p_serialize_statistics(mP2pHandle, result, nullptr);
    CheckError(ia_ret != ia_err_none, UNKNOWN_ERROR, "Serializ statistics fail");

    return OK;
}

void IntelPGParam::deinit() {
    ia_p2p_deinit(mP2pHandle);
    if (mP2pCacheBuffer.data) {
        CIPR::freeMemory(mP2pCacheBuffer.data);
    }
}

int IntelPGParam::getKernelIdByBitmap(ia_css_kernel_bitmap_t bitmap) {
    int kernelId = 0;
    CheckError(ia_css_is_kernel_bitmap_empty(bitmap), BAD_VALUE, "The bitmap is empty");
    while (!ia_css_is_kernel_bitmap_set(bitmap, (unsigned int)kernelId)) {
        kernelId++;
    }

    return kernelId;
}

ia_css_kernel_bitmap_t IntelPGParam::getCachedTerminalKernelBitmap(
    ia_css_param_terminal_manifest_t* manifest) {
    ia_css_kernel_bitmap_t kernelBitmap = ia_css_kernel_bitmap_clear();
    unsigned int section, sectionCount;

    /* Loop through all the sections in manifest and put the kernel ids into the kernel bitmap. */
    sectionCount = manifest->param_manifest_section_desc_count;
    for (section = 0; section < sectionCount; section++) {
        ia_css_param_manifest_section_desc_t* desc =
            ia_css_param_terminal_manifest_get_prm_sct_desc(manifest, section);
        CheckError(!desc, kernelBitmap, "failed to get desc");
        int index = ia_css_param_manifest_section_desc_get_kernel_id(desc);
        kernelBitmap = ia_css_kernel_bitmap_set(kernelBitmap, index);
    }

    return kernelBitmap;
}

ia_css_kernel_bitmap_t IntelPGParam::getProgramTerminalKernelBitmap(
    ia_css_program_terminal_manifest_t* manifest) {
    ia_css_kernel_bitmap_t kernelBitmap = ia_css_kernel_bitmap_clear();
    unsigned int section, sectionCount;

    /* Loop through all the sections in manifest and put the kernel ids into the kernel bitmap. */
    sectionCount = manifest->fragment_param_manifest_section_desc_count;
    for (section = 0; section < sectionCount; section++) {
        ia_css_fragment_param_manifest_section_desc_t* desc =
            ia_css_program_terminal_manifest_get_frgmnt_prm_sct_desc(manifest, section);
        CheckError(!desc, kernelBitmap, "failed to get desc");
        int index = ia_css_fragment_param_manifest_section_desc_get_kernel_id(desc);
        kernelBitmap = ia_css_kernel_bitmap_set(kernelBitmap, index);
    }

    return kernelBitmap;
}

int IntelPGParam::disableZeroSizedTerminals(ia_css_kernel_bitmap_t* kernelBitmap) {
    int ret = OK;
    ia_css_terminal_type_t terminalType;
    ia_css_kernel_bitmap_t terminalKernelsBitmap = ia_css_kernel_bitmap_clear();
    ia_css_kernel_bitmap_t disabledTerminalKernelsBitmap = ia_css_kernel_bitmap_clear();
    for (int i = 0; i < mTerminalCount; i++) {
        terminalKernelsBitmap = ia_css_kernel_bitmap_clear();
        unsigned int payloadSize = mPgReqs.terminals[i].payloadSize;
        ia_css_terminal_manifest_t* manifest =
            ia_css_program_group_manifest_get_term_mnfst(mPgManifest, i);
        terminalType = ia_css_terminal_manifest_get_type(manifest);

        if (payloadSize == 0) {
            switch (terminalType) {
                case IA_CSS_TERMINAL_TYPE_PARAM_SPATIAL_IN:
                case IA_CSS_TERMINAL_TYPE_PARAM_SPATIAL_OUT: {
                    /* Spatial terminals are only associated to a single kernel. */
                    ia_css_spatial_param_terminal_manifest_t* paramMani =
                        reinterpret_cast<ia_css_spatial_param_terminal_manifest_t*>(manifest);
                    terminalKernelsBitmap =
                        ia_css_kernel_bitmap_set(terminalKernelsBitmap, paramMani->kernel_id);
                    break;
                }
                case IA_CSS_TERMINAL_TYPE_PARAM_CACHED_IN:
                case IA_CSS_TERMINAL_TYPE_PARAM_CACHED_OUT: {
                    ia_css_param_terminal_manifest_t* paramMani =
                        reinterpret_cast<ia_css_param_terminal_manifest_t*>(manifest);
                    terminalKernelsBitmap = getCachedTerminalKernelBitmap(paramMani);
                    break;
                }
                case IA_CSS_TERMINAL_TYPE_PROGRAM: {
                    ia_css_program_terminal_manifest_t* proMani =
                        reinterpret_cast<ia_css_program_terminal_manifest_t*>(manifest);
                    terminalKernelsBitmap = getProgramTerminalKernelBitmap(proMani);
                    break;
                }
                case IA_CSS_TERMINAL_TYPE_PROGRAM_CONTROL_INIT:
                    LOG1("%s: program control init terminal is always enabled.", __func__);
                    break;
                default:
                    break;
            }
            disabledTerminalKernelsBitmap =
                ia_css_kernel_bitmap_union(disabledTerminalKernelsBitmap, terminalKernelsBitmap);
        }
    }

    *kernelBitmap = ia_css_kernel_bitmap_intersection(
        *kernelBitmap, ia_css_kernel_bitmap_complement(disabledTerminalKernelsBitmap));
    return ret;
}

css_err_t IntelPGParam::getKernelOrderForParamCachedInTerm(
    ia_css_param_terminal_manifest_t* terminalManifest, IpuPgTerminalKernelInfo* kernelOrder) {
    CheckError((!terminalManifest || !kernelOrder), ia_err_argument, "No manifest or order info");

    uint16_t sectionCount = terminalManifest->param_manifest_section_desc_count;
    CheckError(sectionCount == 0, css_err_argument, "No static sections in manifest");
    uint8_t kernelCount = 0;

    for (uint16_t section = 0; section < sectionCount; section++) {
        ia_css_param_manifest_section_desc_t* param =
            ia_css_param_terminal_manifest_get_prm_sct_desc(terminalManifest, section);
        CheckError(!param, css_err_internal, "Failed to get param from terminal manifest!");

        /* there is agreement that sections of the same kernel are
         * encoded in a row. Here, skipping sections of the same kernel
         * based on this assumption.
         */
        /* info: Indication of the kernel this parameter belongs to,
         * may stand for mem_type, region and kernel_id for ipu6
         */
        int index = ia_css_param_manifest_section_desc_get_kernel_id(param);
        if (kernelCount > 0 && kernelOrder[kernelCount - 1].id == index) {
            ++kernelOrder[kernelCount - 1].sections;
            kernelOrder[kernelCount - 1].size += param->max_mem_size;
            continue;
        }
        kernelOrder[kernelCount].id = (uint8_t)index;
        kernelOrder[kernelCount].sections = 1;
        kernelOrder[kernelCount].size = param->max_mem_size;
        kernelOrder[kernelCount].initialize = false;
        kernelCount++;
    }

    return css_err_none;
}

css_err_t IntelPGParam::getKernelOrderForProgramTerm(
    ia_css_program_terminal_manifest_t* terminalManifest, IpuPgTerminalKernelInfo* kernelOrder) {
    CheckError((!terminalManifest || !kernelOrder), css_err_argument, "No manifest or order info");
    uint16_t sectionCount = terminalManifest->fragment_param_manifest_section_desc_count;
    CheckError(sectionCount == 0, ia_err_internal, "No static sections in manifest");
    uint8_t kernelCount = 0;

    for (uint16_t section = 0; section < sectionCount; section++) {
        ia_css_fragment_param_manifest_section_desc_t* param =
            ia_css_program_terminal_manifest_get_frgmnt_prm_sct_desc(terminalManifest, section);
        CheckError(!param, css_err_internal, "Failed to get param from terminal manifest!");

        /* there is agreement that sections of the same kernel are
         * encoded in a row. Here, skipping sections of the same kernel
         * based on this assumption.
         */
        /* info: Indication of the kernel this parameter belongs to,
         * may stand for mem_type, region and kernel_id for ipu6
         */
        int index = ia_css_fragment_param_manifest_section_desc_get_kernel_id(param);
        if (kernelCount > 0 && kernelOrder[kernelCount - 1].id == index) {
            ++kernelOrder[kernelCount - 1].sections;
            kernelOrder[kernelCount - 1].size += param->max_mem_size;
            continue;
        }
        kernelOrder[kernelCount].id = (uint8_t)index;
        kernelOrder[kernelCount].sections = 1;
        kernelOrder[kernelCount].size = param->max_mem_size;
        kernelOrder[kernelCount].initialize = false;
        kernelCount++;
    }

    return css_err_none;
}

int8_t IntelPGParam::terminalEnumerateByType(IpuPgRequirements* reqs,
                                             ia_css_terminal_type_t terminalType, uint8_t num) {
    CheckError(reqs->terminalCount == 0, -1, "%s: no terminals!", __func__);

    for (uint8_t terminal = 0; terminal < reqs->terminalCount; terminal++) {
        if (reqs->terminals[terminal].type == terminalType) {
            if (num)
                num--;
            else
                return (int8_t)terminal;
        }
    }

    return -1;
}

int8_t IntelPGParam::terminalEnumerateByBitmap(IpuPgRequirements* reqs,
                                               ia_css_terminal_type_t terminal_type,
                                               ia_css_kernel_bitmap_t bitmap) {
    CheckError(reqs->terminalCount == 0, -1, "%s: no terminals!", __func__);

    for (uint8_t terminal = 0; terminal < reqs->terminalCount; terminal++) {
        if (reqs->terminals[terminal].type == terminal_type &&
            ia_css_is_kernel_bitmap_equal(reqs->terminals[terminal].kernelBitmap, bitmap)) {
            return (int8_t)terminal;
        }
    }

    return -1;
}

bool IntelPGParam::isKernelIdInKernelOrder(IpuPgRequirements* reqs, int8_t termIndex, int kernelId,
                                           uint8_t* orderedIndex) {
    /* No kernel order, return true always */
    if (!reqs->terminals[termIndex].kernelOrder) return true;

    /* Check if the kernel_id can be found from the kernelOrder */
    for (uint8_t i = 0; i < PSYS_MAX_KERNELS_PER_PG; i++) {
        if (reqs->terminals[termIndex].kernelOrder[i].id == kernelId) {
            if (orderedIndex) *orderedIndex = i;
            return true;
        }
    }

    return false;
}

uint32_t IntelPGParam::getKernelCountFromKernelOrder(IpuPgRequirements* reqs, int8_t termIndex,
                                                     int kernelId) {
    if (!reqs->terminals[termIndex].kernelOrder) {
        /* If no kernel order is present, assuming kernel appears once. */
        return 1;
    }

    uint32_t count = 0;
    for (int i = 0; i < PSYS_MAX_KERNELS_PER_PG; i++) {
        if (reqs->terminals[termIndex].kernelOrder[i].id == kernelId) {
            ++count;
        }
    }

    return count;
}

void IntelPGParam::processTerminalKernelRequirements(IpuPgRequirements* reqs, int8_t termIndex,
                                                     ia_css_terminal_type_t terminalType,
                                                     int kernelId) {
    uint32_t kernelCount = getKernelCountFromKernelOrder(reqs, termIndex, kernelId);
    uint32_t sectionCount = 0, payloadSize = 0;
#if defined(IPU_SYSVER_IPU6) && defined(UNIFIED_PROG_TERM_FRAG_DESC)
    uint32_t multiplier = 1;
#endif
    for (unsigned int i = 0; i < kernelCount; ++i) {
        switch (terminalType) {
            case IA_CSS_TERMINAL_TYPE_PARAM_CACHED_IN:
                sectionCount = mKernel.mSections[kernelId].param_in_section_count;
                payloadSize = mKernel.mPayloads[kernelId].param_in_payload_size;
                break;
            case IA_CSS_TERMINAL_TYPE_PARAM_CACHED_OUT:
                sectionCount = mKernel.mSections[kernelId].param_out_section_count_per_fragment;
#if defined(IPU_SYSVER_IPU6) && defined(UNIFIED_PROG_TERM_FRAG_DESC)
                payloadSize = mKernel.mPayloads[kernelId].param_out_payload_size * mFragmentCount;
#else
                payloadSize = mKernel.mPayloads[kernelId].param_out_payload_size;
#endif
                break;
            case IA_CSS_TERMINAL_TYPE_PROGRAM:
                sectionCount = mKernel.mSections[kernelId].program_section_count_per_fragment;
                payloadSize = mKernel.mPayloads[kernelId].program_payload_size;
#if defined(IPU_SYSVER_IPU6) && defined(UNIFIED_PROG_TERM_FRAG_DESC)
                multiplier = mFragmentCount;
#endif
                break;
            default:
                LOG1("%s: terminal type %d encode not implemented", __func__, terminalType);
                break;
        }
        reqs->terminals[termIndex].sectionCount += sectionCount;

#if defined(IPU_SYSVER_IPU6) && defined(UNIFIED_PROG_TERM_FRAG_DESC)
        reqs->terminals[termIndex].payloadSize += multiplier * payloadSize;
#else
        reqs->terminals[termIndex].payloadSize += payloadSize;
#endif

        mKernel.mPayloadSize = reqs->terminals[termIndex].payloadSize;
    }

    reqs->terminals[termIndex].kernelBitmap =
        ia_css_kernel_bitmap_set(reqs->terminals[termIndex].kernelBitmap, (unsigned int)kernelId);
}

css_err_t IntelPGParam::payloadSectionSizeSanityTest(ia_p2p_payload_desc* current,
                                                     uint16_t kernelId, uint8_t terminalIndex,
                                                     uint32_t currentOffset, size_t payloadSize) {
    size_t nextPayloadSize = 0;
    ia_p2p_payload_desc init = mKernel.mPayloads[kernelId];
    /* calculate again the memory requirements for each kernel
     * and compare it with what we stored at init time. */
    ia_err ia_ret = ia_p2p_get_kernel_payload_desc(
        mP2pHandle, mPgId, kernelId,
#if defined(IPU_SYSVER_IPU6) && defined(UNIFIED_PROG_TERM_FRAG_DESC)
        1,
#else
        mFragmentCount,
#endif
        (mFragmentConfig ? mFragmentConfig->pixel_fragment_descs[kernelId] : mFragmentDesc),
        current);
    CheckError(ia_ret != ia_err_none, css_err_internal,
               "Failed to get payload description during sanity check (kernel %d)", kernelId);

    switch (mPgReqs.terminals[terminalIndex].type) {
        case IA_CSS_TERMINAL_TYPE_PARAM_CACHED_IN:
            if (current->param_in_payload_size > init.param_in_payload_size) {
                LOGW(
                    "%s: param-in section size mismatch in pg[%d] kernel[%d]"
                    " p2p size %d pg_die size %d",
                    __func__, mPgId, kernelId, current->param_in_payload_size,
                    init.param_in_payload_size);
            } else {
                current->param_in_payload_size = init.param_in_payload_size;
            }
            nextPayloadSize = current->param_in_payload_size;
            break;
        case IA_CSS_TERMINAL_TYPE_PARAM_CACHED_OUT:
            if (current->param_out_payload_size > init.param_out_payload_size) {
                LOGW(
                    "%s: param-out section size mismatch in pg[%d] kernel[%d]"
                    " p2p size %d pg_die size %d",
                    __func__, mPgId, kernelId, current->param_out_payload_size,
                    init.param_out_payload_size);
            } else {
                current->param_out_payload_size = init.param_out_payload_size;
            }
            nextPayloadSize = current->param_out_payload_size;
            break;
        case IA_CSS_TERMINAL_TYPE_PROGRAM:
            if (current->program_payload_size > init.program_payload_size) {
                LOG1(
                    "%s: program section size mismatch in pg[%d] kernel[%d]"
                    " p2p size %d pg_die size %d",
                    __func__, mPgId, kernelId, current->program_payload_size,
                    init.program_payload_size);
            } else {
                current->program_payload_size = init.program_payload_size;
            }
            nextPayloadSize = current->program_payload_size;
            break;
        case IA_CSS_TERMINAL_TYPE_PARAM_SPATIAL_IN:
            if (current->spatial_param_in_payload_size > init.spatial_param_in_payload_size) {
                LOGW(
                    "%s: spatial-in section size mismatch in pg[%d] kernel[%d]"
                    " p2p size %d pg_die size %d",
                    __func__, mPgId, kernelId, current->spatial_param_in_payload_size,
                    init.spatial_param_in_payload_size);
            } else {
                current->spatial_param_in_payload_size = init.spatial_param_in_payload_size;
            }
            nextPayloadSize = current->spatial_param_in_payload_size;
            break;
        case IA_CSS_TERMINAL_TYPE_PARAM_SPATIAL_OUT:
            if (current->spatial_param_out_payload_size > init.spatial_param_out_payload_size) {
                LOGW(
                    "%s: spatial-out section size mismatch in pg[%d] kernel[%d]"
                    " p2p size %d pg_die size %d",
                    __func__, mPgId, kernelId, current->spatial_param_out_payload_size,
                    init.spatial_param_out_payload_size);
            } else {
                current->spatial_param_out_payload_size = init.spatial_param_out_payload_size;
            }
            nextPayloadSize = current->spatial_param_out_payload_size;
            break;
        case IA_CSS_TERMINAL_TYPE_DATA_IN:
        case IA_CSS_TERMINAL_TYPE_DATA_OUT:
        case IA_CSS_TERMINAL_TYPE_PROGRAM_CONTROL_INIT:
            /* No check done for frame terminals */
            break;
        default:
            LOGE("%s: terminal type %d payload check not implemented", __func__,
                 mPgReqs.terminals[terminalIndex].type);
            return css_err_argument;
    }

    CheckError(
        (currentOffset + nextPayloadSize > payloadSize), css_err_nomemory,
        "pg %d terminal %d payload size small, encoding for kernel %d will exceed size by %u bytes",
        mPgId, terminalIndex, kernelId, currentOffset + nextPayloadSize - payloadSize);
    return css_err_none;
}

void IntelPGParam::dumpFragmentDesc(int fragmentCount) {
    if (!Log::isDebugLevelEnable(CAMERA_DEBUG_LOG_LEVEL2)) return;

    LOG2("%s: pg %d get frag count %d (new api)", __func__, mPgId, fragmentCount);
    for (int kernel = 0; kernel < IA_P2P_MAX_KERNELS_PER_PG; kernel++) {
        for (int frag = 0; frag < fragmentCount; frag++) {
            LOG2("   kernel %d, frag %d: [%d %d %d %d]", kernel, frag,
                 mFragmentConfig->pixel_fragment_descs[kernel][frag].fragment_width,
                 mFragmentConfig->pixel_fragment_descs[kernel][frag].fragment_height,
                 mFragmentConfig->pixel_fragment_descs[kernel][frag].fragment_start_x,
                 mFragmentConfig->pixel_fragment_descs[kernel][frag].fragment_start_y);
        }
    }
}

}  // namespace icamera
