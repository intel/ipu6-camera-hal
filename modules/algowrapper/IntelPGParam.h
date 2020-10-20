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

#pragma once

extern "C" {
#include <ia_css_program_group_data.h>
#include <ia_css_program_group_param.h>
#include <ia_css_psys_process_group.h>
#include <ia_css_psys_program_group_manifest.h>
#include <ia_css_psys_terminal.h>
#include <ia_css_psys_terminal_manifest.h>
#include <ia_css_terminal_manifest_types.h>
#include <ia_css_terminal_types.h>
#include <ia_isp_bxt.h>
#include <ia_isp_types.h>
#include <ia_p2p.h>
#include <ia_p2p_types.h>
#include <ia_pal_types_isp_ids_autogen.h>
#include <pg_control_init_framework.h>
}

#include <map>
#include <memory>
#include <vector>

#include "ia_tools/css_types.h"
#include "iutils/Errors.h"
#include "iutils/Utils.h"
#include "modules/ia_cipr/include/Utils.h"
#include "src/core/psysprocessor/PGUtils.h"

namespace icamera {

#define PSYS_MAX_KERNELS_PER_PG IA_CSS_KERNEL_BITMAP_BITS

/**
 * \class IntelPGParam
 *
 * \brief This is a version P2P implementation which is used to encode parameter terminal
 *        and decode statistic terminal for PSYS pipeline.
 *
 * The call sequence as follows:
 * 1. init();
 * 2. prepare();
 * 3. allocatePGBuffer() (optional);
 * 4. setPGAndPrepareProgram();
 * 5. getPayloadSizes(), and allocate payload buffers,
 *    allocatePayloads() is provided for that;
 * 6. getFragmentDescriptors();
 * 7. loop frame {
 *      updatePALAndEncode();
 *      decode();
 *    }
 * 8. deinit();
 */
class IntelPGParam {
 public:
    explicit IntelPGParam(int pgId);
    ~IntelPGParam();

    /**
     * Use to init and config P2P handle.
     */
    int init(ia_p2p_platform_t platform, const PgConfiguration& Pgconfiguration);

    /**
     * Query and save the requirement for each terminal, calculate the final kernel bitmap.
     */
    int prepare(const ia_binary_data* ipuParameters, const ia_css_rbm_t* rbm,
                ia_css_kernel_bitmap_t* bitmap, uint32_t* maxStatsSize = nullptr);

    /**
     * Allocate PG buffer for caller
     */
    void* allocatePGBuffer(int pgSize);

    /**
     * Accept pg outside and init program_control_init terminal.
     */
    int setPGAndPrepareProgram(ia_css_process_group_t* pg);

    /**
     * Allocate payload memory for terminals.
     */
    int allocatePayloads(int payloadCount, ia_binary_data* payloads);

    /**
     * Update PAL and encode payload data for all terminals. Will skip inactive terminals.
     */
    int updatePALAndEncode(const ia_binary_data* ipuParams, int payloadCount,
                           ia_binary_data* payloads);

    /**
     * Decode payload data for all related terminals.
     */
    int decode(int payloadCount, ia_binary_data* payload, ia_binary_data* statistics);

    /**
     * Use to deinit P2P handle.
     */
    void deinit();

    /**
     * Get fragment descriptors calculated according to PAL data
     * Called after prepare().
     */
    int getFragmentDescriptors(int descCount, ia_p2p_fragment_desc* descs);

    /**
     * Get payload size for all terminals, and return valid payloads number.
     */
    int getPayloadSizes(int payloadCount, ia_binary_data* payloads);

 private:
    enum FragmentDataTerminalType {
        FRAG_TERM_TYPE_INPUT = 0,
        FRAG_TERM_TYPE_OUTPUT_START,  // Mapping to data out terminal in order for all postgdc pgs.
        FRAG_TERM_TYPE_DISPALY_OUTPUT = FRAG_TERM_TYPE_OUTPUT_START,
        FRAG_TERM_TYPE_MAIN_OUTPUT,
        FRAG_TERM_TYPE_PP_OUTPUT,
        FRAG_TERM_TYPE_COUNT,
    };

    int mPgId;
    int mTerminalCount;
    PgFrameDesc mInputMainFrame;
    PgFrameDesc mOutputMainFrame;

    uint8_t mFragmentCount;
    ia_p2p_fragment_desc* mFragmentDesc;
    ia_p2p_fragment_configuration_t* mFragmentConfig;
    // for pg fragment with new api:
    // ia_p2p_calculate_fragments_rbm
    // Instead of mFragmentDesc
    ia_p2p_handle mP2pHandle;
    ia_binary_data mP2pCacheBuffer;

    ia_css_program_group_manifest_t* mPgManifest;
    std::vector<int> mDisableDataTermials;
    ia_css_process_group_t* mProcessGroup;
    int mProgramControlInitTerminalIndex;

    struct IpuPgTerminalKernelInfo {
        IpuPgTerminalKernelInfo() {}
        uint8_t id = 0;
        uint8_t sections = 0;
        uint32_t size = 0;
        bool initialize = false;
    };

    struct IpuPgTerminaRequirements {
        IpuPgTerminaRequirements() { kernelBitmap = ia_css_kernel_bitmap_clear(); }
        ia_css_terminal_type_t type = IA_CSS_N_TERMINAL_TYPES;
        uint32_t payloadSize = 0;
        ia_css_kernel_bitmap_t kernelBitmap;
        uint32_t sectionCount = 0;
        IpuPgTerminalKernelInfo* kernelOrder = nullptr;
        ia_p2p_fragment_desc* fragment_descs = nullptr;

        // Use for program_control_init
        uint32_t userParamSize;
        std::unique_ptr<uint8_t[]> userParamAddress;
    };

    struct IpuPgRequirements {
        IpuPgRequirements() {}
        uint32_t terminalCount = 0;
        IpuPgTerminaRequirements terminals[IPU_MAX_TERMINAL_COUNT];
    };

    struct KernelRequirement {
        KernelRequirement() { mKernelBitmap = ia_css_kernel_bitmap_clear(); }
        ia_p2p_terminal_requirements_t mSections[PSYS_MAX_KERNELS_PER_PG];
        ia_p2p_payload_desc mPayloads[PSYS_MAX_KERNELS_PER_PG];
        int mPayloadSize = 0;
        ia_css_kernel_bitmap_t mKernelBitmap;
    };

    KernelRequirement mKernel;
    IpuPgRequirements mPgReqs;

    // Allocate them here, for sandboxing case (shared memory)
    std::vector<ia_binary_data> mAllocatedPayloads;
    void* mProcessGroupMemory;

 private:
    int getKernelIdByBitmap(ia_css_kernel_bitmap_t bitmap);
    ia_css_kernel_bitmap_t getCachedTerminalKernelBitmap(
        ia_css_param_terminal_manifest_t* manifest);
    ia_css_kernel_bitmap_t getProgramTerminalKernelBitmap(
        ia_css_program_terminal_manifest_t* manifest);
    int disableZeroSizedTerminals(ia_css_kernel_bitmap_t* kernelBitmap);
    css_err_t getKernelOrderForProgramTerm(ia_css_program_terminal_manifest_t* terminalManifest,
                                           IpuPgTerminalKernelInfo* kernelOrder);
    css_err_t getKernelOrderForParamCachedInTerm(ia_css_param_terminal_manifest_t* terminalManifest,
                                                 IpuPgTerminalKernelInfo* kernelOrder);
    int8_t terminalEnumerateByType(IpuPgRequirements* reqs, ia_css_terminal_type_t terminalType,
                                   uint8_t num);
    int8_t terminalEnumerateByBitmap(IpuPgRequirements* reqs, ia_css_terminal_type_t terminal_type,
                                     ia_css_kernel_bitmap_t bitmap);
    bool isKernelIdInKernelOrder(IpuPgRequirements* reqs, int8_t termIndex, int kernelId,
                                 uint8_t* orderedIndex);
    uint32_t getKernelCountFromKernelOrder(IpuPgRequirements* reqs, int8_t termIndex, int kernelId);
    void processTerminalKernelRequirements(IpuPgRequirements* reqs, int8_t termIndex,
                                           ia_css_terminal_type_t terminalType, int kernelId);
    css_err_t payloadSectionSizeSanityTest(ia_p2p_payload_desc* current, uint16_t kernelId,
                                           uint8_t terminalIndex, uint32_t currentOffset,
                                           size_t payloadSize);

    int calcFragmentDescriptors(int fragmentCount, const PgFrameDesc& inputMainFrame,
                                const PgFrameDesc& outputMainFrame, const ia_css_rbm_t* rbm);

    void dumpFragmentDesc(int fragmentCount);
    int encodeTerminal(ia_css_terminal_t* terminal, ia_binary_data payload);
    int decodeTerminal(ia_css_terminal_t* terminal, ia_binary_data payload);
    int serializeDecodeCache(ia_binary_data* result);

    void destroyPayloads();
    void destroyPGBuffer();

    DISALLOW_COPY_AND_ASSIGN(IntelPGParam);
};

}  // namespace icamera
