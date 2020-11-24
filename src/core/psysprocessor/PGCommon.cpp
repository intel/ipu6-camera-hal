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

#define LOG_TAG "PGCommon"

#include "PGCommon.h"

#include <stdint.h>
#include <math.h>
#include <utility>
#include "iutils/Utils.h"
#include "iutils/CameraLog.h"
#include "iutils/CameraDump.h"

namespace icamera {

#define IS_VALID_TERMINAL(terminal) (terminal >=0 && terminal < mTerminalCount)

int PGCommon::getFrameSize(int format, int width, int height,
                           bool needAlignedHeight, bool needExtraSize, bool needCompression)
{
    int size = 0;
    int cssFormat = PGUtils::getCssFmt(format);
    int stride = PGUtils::getCssStride(format, width);
    switch (cssFormat) {
    case IA_CSS_DATA_FORMAT_BAYER_LINE_INTERLEAVED:  // CSL6
        if (needAlignedHeight) {
            height = ALIGN_64(height);
        }
        size = stride * height * 3 / 2;
        break;
    default:
        break;
    }

    if (!size) {
        size = CameraUtils::getFrameSize(format, width, height,
                                         needAlignedHeight, needExtraSize, needCompression);
    }
    return size;
}

PGCommon::PGCommon(int pgId, const std::string& pgName, ia_uid terminalBaseUid):
    mCtx(nullptr),
    mManifestBuffer(nullptr),
    mPGParamsBuffer(nullptr),
    mPGParamAdapt(nullptr),
    mPGId(pgId),
    mName(pgName),
    mTerminalBaseUid(terminalBaseUid),
    mStreamId(-1),
    mPGCount(0),
    mPlatform(IA_P2P_PLATFORM_BXT_B0),
    mProgramCount(0),
    mTerminalCount(0),
    mManifestSize(0),
    mKernelBitmap(ia_css_kernel_bitmap_clear()),
    mRoutingBitmap(nullptr),
    mFragmentCount(1),
    mPGBuffer(nullptr),
    mProcessGroup(nullptr),
    mCmdExtBuffer(nullptr),
    mPPG(false),
    mPPGStarted(false),
    mPPGBuffer(nullptr),
    mPPGProcessGroup(nullptr),
    mToken(0),
    mEvent(nullptr),
    mTerminalBuffers(nullptr),
    mInputMainTerminal(-1),
    mOutputMainTerminal(-1),
    mShareReferPool(nullptr)
{
    mTnrTerminalPair.inId = -1;
    mTnrTerminalPair.outId = -1;
    CLEAR(mParamPayload);
    CLEAR(mShareReferIds);
}

PGCommon::~PGCommon()
{
}

int PGCommon::init()
{
    mDisableDataTermials.clear();
    mPGParamAdapt = std::unique_ptr<IntelPGParam>(new IntelPGParam(mPGId));

    mCtx = new CIPR::Context();
    int ret = getCapability();
    if (ret != OK) return ret;

    // create mManifestBuffer
    ret = getManifest(mPGId);
    if (ret != OK) return ret;

    mTerminalBuffers = (CIPR::Buffer**)CIPR::callocMemory(mTerminalCount, sizeof(CIPR::Buffer*));
    CheckError(!mTerminalBuffers, NO_MEMORY, "Allocate terminal buffers fail");
    memset(mTerminalBuffers, 0, (mTerminalCount * sizeof(CIPR::Buffer*)));

    mFrameFormatType = std::unique_ptr<ia_css_frame_format_type[]>(new ia_css_frame_format_type[mTerminalCount]);
    for (int i = 0; i < mTerminalCount; i++) {
        mFrameFormatType[i] = IA_CSS_N_FRAME_FORMAT_TYPES;
    }

    mPgTerminals = std::unique_ptr<uint8_t[]>(new uint8_t[mTerminalCount]);
    for (int i = 0; i < mTerminalCount; i++) {
        mPgTerminals[i] = IPU_MAX_TERMINAL_COUNT;
    }

    std::vector<TerminalPair> tnrTerminalPairs;
    if (PGUtils::getTerminalPairs(mPGId, PGUtils::TERMINAL_PAIR_TNR, &tnrTerminalPairs)) {
        mTnrTerminalPair = tnrTerminalPairs[0];
    }
    PGUtils::getTerminalPairs(mPGId, PGUtils::TERMINAL_PAIR_DVS, &mDvsTerminalPairs);
    PGUtils::getTerminalPairs(mPGId, PGUtils::TERMINAL_PAIR_TNR_SIM, &mTnrSimTerminalPairs);

    return ret;
}

void PGCommon::deInit()
{
    if (mPPGStarted) {
        stopPPG();
        mPPGStarted = false;
    }

    destoryCommands();

    while (!mTnrDataBuffers.empty()) {
        uint8_t* ptr = mTnrDataBuffers.back();
        mTnrDataBuffers.pop_back();
        free(ptr);
    }

    mDvsTerminalPairs.clear();
    mTnrSimTerminalPairs.clear();

    mDisableDataTermials.clear();
    if (mTerminalBuffers) {
        CIPR::freeMemory(mTerminalBuffers);
    }

    delete mManifestBuffer;
    delete mPGParamsBuffer;
    delete mPGBuffer;
    if (mPPGBuffer) {
        delete mPPGBuffer;
    }
    for (auto& item : mBuffers) {
        delete item.ciprBuf;
    }

    delete mCtx;

    mPGParamAdapt->deinit();
    mRoutingBitmap.reset();
}

void PGCommon::setInputInfo(const TerminalFrameInfoMap& inputInfos)
{
    mInputMainTerminal = -1;
    int maxFrameSize = 0;
    for (const auto& item : inputInfos) {
        int terminal = item.first - mTerminalBaseUid;
        CheckError(!IS_VALID_TERMINAL(terminal), VOID_VALUE, "error input terminal %d", item.first);

        FrameInfo frameInfo;
        frameInfo.mWidth = item.second.mWidth;
        frameInfo.mHeight = item.second.mHeight;
        frameInfo.mFormat = item.second.mFormat;
        frameInfo.mBpp = CameraUtils::getBpp(frameInfo.mFormat);
        frameInfo.mStride = CameraUtils::getStride(frameInfo.mFormat, frameInfo.mWidth);
        mTerminalFrameInfos[terminal] = frameInfo;
        int size = frameInfo.mWidth * frameInfo.mHeight;
        if (maxFrameSize < size) {
            maxFrameSize = size;
            mInputMainTerminal = terminal;
        }
    }

    // Create frame info for tnr terminals (i.e. data terminals)
    FrameInfo config = mTerminalFrameInfos[mInputMainTerminal];
    if (config.mHeight % 32) {
        LOG1("%s: height %d not multiple of 32, rounding up!", __func__, config.mHeight);
        config.mHeight = ((config.mHeight / 32) + 1) * 32;
    }

    for (int i = PAIR_BUFFER_IN_INDEX; i <= PAIR_BUFFER_OUT_INDEX; i++) {
        int tnrId = (i == PAIR_BUFFER_IN_INDEX) ? mTnrTerminalPair.inId : mTnrTerminalPair.outId;
        if (tnrId < 0) continue;

        mFrameFormatType[tnrId] = IA_CSS_DATA_FORMAT_NV12; // for IPU6
        mTerminalFrameInfos[tnrId] = config;
    }

    LOG1("%s:%d use input terminal %d as main", __func__, mPGId, mInputMainTerminal);
}

void PGCommon::setOutputInfo(const TerminalFrameInfoMap& outputInfos)
{
    mOutputMainTerminal = -1;
    int maxFrameSize = 0;
    for (const auto& item : outputInfos) {
        int terminal = item.first - mTerminalBaseUid;
        CheckError(!IS_VALID_TERMINAL(terminal), VOID_VALUE, "error output terminal %d", item.first);

        FrameInfo frameInfo;
        frameInfo.mWidth = item.second.mWidth;
        frameInfo.mHeight = item.second.mHeight;
        frameInfo.mFormat = item.second.mFormat;
        frameInfo.mBpp = CameraUtils::getBpp(frameInfo.mFormat);
        frameInfo.mStride = CameraUtils::getStride(frameInfo.mFormat, frameInfo.mWidth);
        mTerminalFrameInfos[terminal] = frameInfo;
        int size = frameInfo.mWidth * frameInfo.mHeight;
        if (maxFrameSize < size) {
            maxFrameSize = size;
            mOutputMainTerminal = terminal;
        }
    }
}

void PGCommon::setDisabledTerminals(const std::vector<ia_uid>& disabledTerminals)
{
    for (auto const terminalUid : disabledTerminals) {
        int terminal = terminalUid - mTerminalBaseUid;
        CheckError(!IS_VALID_TERMINAL(terminal), VOID_VALUE, "error disabled terminal %d", terminalUid);
        mDisableDataTermials.push_back(terminal);
    }
}

void PGCommon::setRoutingBitmap(const void* rbm, uint32_t bytes)
{
    if (!rbm || !bytes) {
        return;
    }
    const unsigned char* rbmData = (const unsigned char*)rbm;

    if (mRoutingBitmap.get() == nullptr) {
        mRoutingBitmap = std::unique_ptr<ia_css_rbm_t>(new ia_css_rbm_t);
    }

    ia_css_rbm_t* rbmPtr = mRoutingBitmap.get();
    *rbmPtr = ia_css_rbm_clear();
    for (uint32_t bit = 0; bit < bytes * 8; bit++) {
        if (rbmData[bit / 8] & (1 << (bit %8))) {
            *rbmPtr = ia_css_rbm_set(*rbmPtr, bit);
        }
    }
}

int PGCommon::prepare(IspParamAdaptor* adaptor, int streamId)
{
    mStreamId = streamId;
    // Set the data terminal frame format
    int ret = configTerminalFormat();
    CheckError((ret != OK), ret, "%s, call configTerminal fail", __func__);

    // Init and config p2p handle
    ret = initParamAdapt();
    CheckError((ret != OK), ret, "%s, init p2p fail", __func__);

    // Query and save the requirement for each terminal, get the final kernel bitmap
    ret = mPGParamAdapt->prepare(adaptor->getIpuParameter(-1, streamId), mRoutingBitmap.get(), &mKernelBitmap);
    CheckError((ret != OK), ret, "%s, prepare p2p fail", __func__);

    // Init PG parameters
    ret = handlePGParams(mFrameFormatType.get());
    CheckError((ret != OK), ret, "%s, call handlePGParams fail", __func__);

    ret = setKernelBitMap();
    CheckError((ret != OK), ret, "%s, call setKernelBitMap fail", __func__);

    ret = setTerminalParams(mFrameFormatType.get());
    CheckError((ret != OK), ret, "%s, call setTerminalParams fail", __func__);

   // Create process group
    mProcessGroup = createPG(&mPGBuffer);
    CheckError(!mProcessGroup, UNKNOWN_ERROR, "%s, create pg fail", __func__);
    uint8_t pgTerminalCount = ia_css_process_group_get_terminal_count(mProcessGroup);
    for (uint8_t termNum = 0 ; termNum < pgTerminalCount; termNum++) {
        ia_css_terminal_t* terminal = ia_css_process_group_get_terminal(mProcessGroup, termNum);
        CheckError(!terminal, UNKNOWN_ERROR, "failed to get terminal");
        uint16_t termIdx = ia_css_terminal_get_terminal_manifest_index(terminal);
        CheckError((termIdx >= IPU_MAX_TERMINAL_COUNT), UNKNOWN_ERROR, "wrong term index for terminal num %d", termNum);
        mPgTerminals[termIdx] = termNum;
    }

    mPGParamAdapt->setPGAndPrepareProgram(mProcessGroup);
    ret = configureFragmentDesc();
    CheckError((ret != OK), ret, "%s, call configureFragmentDesc fail", __func__);

    ret = allocateTnrDataBuffers();
    CheckError((ret != OK), ret, "%s, call allocateTnrDataBuffers fail", __func__);
    ret = preparePayloadBuffers();
    CheckError((ret != OK), NO_MEMORY, "%s, preparePayloadBuffers fails", __func__);

    return OK;
}

ia_css_process_group_t* PGCommon::createPG(CIPR::Buffer** pgBuffer)
{
    CheckError(*pgBuffer, nullptr, "pg has already created");

   // Create process group
    ia_css_program_group_param_t* pgParamsBuf =
        (ia_css_program_group_param_t*)getCiprBufferPtr(mPGParamsBuffer);
    ia_css_program_group_manifest_t* manifestBuf =
        (ia_css_program_group_manifest_t*)getCiprBufferPtr(mManifestBuffer);

    size_t pgSize = ia_css_sizeof_process_group(manifestBuf, pgParamsBuf);
    LOG1("%s process group size is %zu", __func__, pgSize);

    void* pgMemory = mPGParamAdapt->allocatePGBuffer(pgSize);
    CheckError(!pgMemory, nullptr, "allocate PG error");
    *pgBuffer = createUserPtrCiprBuffer(pgSize, pgMemory);
    CheckError(!*pgBuffer, nullptr, "%s, call createUserPtrCiprBuffer fail", __func__);

    ia_css_process_group_t* pg = ia_css_process_group_create(getCiprBufferPtr(*pgBuffer),
                   (ia_css_program_group_manifest_t*)getCiprBufferPtr(mManifestBuffer),
                   (ia_css_program_group_param_t*)getCiprBufferPtr(mPGParamsBuffer));
    CheckError(!pg, nullptr, "Create process group failed.");

    if (mPPG) {
        ia_css_process_group_set_num_queues(pg, 1);
    }

    if (mRoutingBitmap.get()) {
        ia_css_process_group_set_routing_bitmap(pg, *mRoutingBitmap.get());
    }
    return pg;
}

int PGCommon::createCommands()
{
    int bufCount = ia_css_process_group_get_terminal_count(mProcessGroup);
    int ret = createCommand(mPGBuffer, &mCmd, &mCmdExtBuffer, bufCount);
    CheckError(ret, NO_MEMORY, "create cmd fail!");
    if (mPPG) {
        ret = createCommand(mPPGBuffer, &mPPGCmd[PPG_CMD_TYPE_START], &mPPGCmdExtBuffer[PPG_CMD_TYPE_START], bufCount);
        CheckError(ret, NO_MEMORY, "create ppg start buffer %d fail");
        ret = createCommand(mPPGBuffer, &mPPGCmd[PPG_CMD_TYPE_STOP], &mPPGCmdExtBuffer[PPG_CMD_TYPE_STOP], 0);
        CheckError(ret, NO_MEMORY, "create ppg stop %d fail");
    }

    CIPR::PSysEventConfig eventCfg = {};
    eventCfg.timeout = kEventTimeout * SLOWLY_MULTIPLIER;
    mEvent = new CIPR::Event(eventCfg);

    return OK;
}

int PGCommon::createCommand(CIPR::Buffer* pg, CIPR::Command** cmd, CIPR::Buffer** extBuffer, int bufCount)
{
    CIPR::PSysCommandConfig cmdCfg;
    CIPR::ProcessGroupCommand *pgCommand;
    CIPR::Result ret;

    // Create command with basic setting
    cmdCfg.buffers.resize(bufCount);
    std::fill(cmdCfg.buffers.begin(), cmdCfg.buffers.end(), nullptr);

    *cmd = new CIPR::Command(cmdCfg);
    ret = (*cmd)->getConfig(&cmdCfg);
    CheckError(ret != CIPR::Result::OK, UNKNOWN_ERROR, "%s, call get_command_config fail", __func__);

    // Create ext buffer
    *extBuffer = new CIPR::Buffer(sizeof(CIPR::ProcessGroupCommand),
                                  CIPR::MemoryFlag::AllocateCpuPtr
                                  | CIPR::MemoryFlag::PSysAPI,
                                  nullptr);

    ret = (*extBuffer)->attatchDevice(mCtx);
    CheckError(ret != CIPR::Result::OK, NO_MEMORY, "unable to access extBuffer");

    void* p = nullptr;
    ret = (*extBuffer)->getMemoryCpuPtr(&p);
    CheckError(ret != CIPR::Result::OK, NO_MEMORY, "unable to access extBuffer memory");
    pgCommand = reinterpret_cast<CIPR::ProcessGroupCommand*>(p);
    CheckError(!pgCommand, NO_MEMORY, "unable to access memory.cpu_ptr");

    pgCommand->header.size = sizeof(CIPR::ProcessGroupCommand);
    pgCommand->header.offset = sizeof(pgCommand->header);
    pgCommand->header.version = psys_command_ext_ppg_1; // for ipu6
    if (pgCommand->header.version == psys_command_ext_ppg_1) {
        CIPR::memoryCopy(pgCommand->dynamicKernelBitmap, sizeof(ia_css_kernel_bitmap_t),
                        &mKernelBitmap, sizeof(ia_css_kernel_bitmap_t));
    }

    // Update setting and set back to command
    cmdCfg.id = mPGId;
    cmdCfg.priority = 1;
    cmdCfg.pgParamsBuf = mPPG ? nullptr : mPGParamsBuffer;
    cmdCfg.pgManifestBuf = mManifestBuffer;
    cmdCfg.pg = pg;
    cmdCfg.extBuf = *extBuffer;
    ret = (*cmd)->setConfig(cmdCfg);
    CheckError(ret != CIPR::Result::OK, UNKNOWN_ERROR, "%s, call set_command_config fail", __func__);

    return OK;
}

void PGCommon::destoryCommands()
{
    delete mCmd;
    delete mCmdExtBuffer;

    for (int i = 0; i < PPG_CMD_TYPE_COUNT; i++) {
        delete mPPGCmd[i];
        delete mPPGCmdExtBuffer[i];
    }

    if (mEvent) {
        delete mEvent;
    }
}

int PGCommon::configTerminalFormat()
{
    for (int i = 0; i < mTerminalCount; i++) {
        if (mTerminalFrameInfos.find(i) != mTerminalFrameInfos.end()) {
            mFrameFormatType[i] = PGUtils::getCssFmt(mTerminalFrameInfos[i].mFormat);
        }
    }
    return OK;
}

int PGCommon::initParamAdapt()
{
    mFragmentCount = calcFragmentCount();

    ia_css_program_group_manifest_t* manifestBuf =
        (ia_css_program_group_manifest_t*)getCiprBufferPtr(mManifestBuffer);

    PgConfiguration config;
    config.pgManifest = manifestBuf;
    config.pgManifestSize = getCiprBufferSize(mManifestBuffer);
    config.disableDataTermials = mDisableDataTermials;
    config.fragmentCount = mFragmentCount;

    FrameInfo* pgInFrame = nullptr;
    FrameInfo* pgOutFrame = nullptr;
    if (mInputMainTerminal >= 0) {
        pgInFrame = &mTerminalFrameInfos[mInputMainTerminal];
    }
    if (mOutputMainTerminal >= 0) {
        pgOutFrame = &mTerminalFrameInfos[mOutputMainTerminal];
    }
    if (pgInFrame) {
        config.inputMainFrame.width = pgInFrame->mWidth;
        config.inputMainFrame.height = pgInFrame->mHeight;
        config.inputMainFrame.bpe = pgInFrame->mBpp; //TODO: use bpe
    }

    if (pgOutFrame) {
        config.outputMainFrame.width = pgOutFrame->mWidth;
        config.outputMainFrame.height = pgOutFrame->mHeight;
        config.outputMainFrame.bpe = pgOutFrame->mBpp; //TODO: use bpe
    }

    // init and config p2p handle
    int ret = mPGParamAdapt->init(mPlatform, config);
    return ret;
}

// Support horizontal fragment only now
int PGCommon::calcFragmentCount(int overlap)
{
    int finalFragmentCount = 0;
    ia_css_data_terminal_manifest_t * data_terminal_manifest = nullptr;

    const ia_css_program_group_manifest_t *manifest =
            (const ia_css_program_group_manifest_t*)getCiprBufferPtr(mManifestBuffer);
    CheckError(!manifest, 1, "%s, can't get manifest ptr", __func__);

    for (int termIdx = 0; termIdx < mTerminalCount; termIdx++) {
        // Get max fragement size from manifest (align with 64)
        ia_css_terminal_manifest_t *terminal_manifest = ia_css_program_group_manifest_get_term_mnfst(manifest, termIdx);
        ia_css_terminal_type_t  terminal_type = ia_css_terminal_manifest_get_type(terminal_manifest);

        if (!((terminal_type == IA_CSS_TERMINAL_TYPE_DATA_OUT) || (terminal_type == IA_CSS_TERMINAL_TYPE_DATA_IN))) {
            continue;
        }

        data_terminal_manifest = ia_css_program_group_manifest_get_data_terminal_manifest(manifest, termIdx);
        CheckError(!data_terminal_manifest, -1, "%s, can't get data terminal manifest for term %d", __func__, termIdx);

        uint16_t size[IA_CSS_N_DATA_DIMENSION] = {0};
        int ret = ia_css_data_terminal_manifest_get_max_size(data_terminal_manifest, size);
        CheckError(ret < 0, 1, "%s: get max fragment size error for term %d", __func__, termIdx);

        size[IA_CSS_COL_DIMENSION] = ALIGN_64(size[IA_CSS_COL_DIMENSION]);
        // Overwrite the max value if need

        // Calc fragment count for terminal (only for horizontal direction)
        int maxFragmentWidth = size[IA_CSS_COL_DIMENSION];
        FrameInfo config;
        if (mTerminalFrameInfos.find(termIdx) != mTerminalFrameInfos.end()) {
            config = mTerminalFrameInfos[termIdx];
        }
        int fragmentCovered = maxFragmentWidth;
        int fragmentCount = 1;
        /*
         * Calculate how many fragment frames can cover the whole frame.
         * Consider overlap between two fragment frames.
         * Example: frame width = 300, max fragment width = 100, overlap = 10
         *       0|------------------------------|300
         *  f1   0|----------|100
         *  f2           90|----------|190
         *  f3                   180|----------|280
         *  f4                            270|---|300
         */
        while (fragmentCovered < config.mWidth) {
            fragmentCovered += (maxFragmentWidth - overlap);
            fragmentCount++;
        }

        if (finalFragmentCount < fragmentCount) {
            finalFragmentCount = fragmentCount;
        }
    }

    LOG2("%s: final fragment count %d for pg %d", __func__, finalFragmentCount, mPGId);
    return finalFragmentCount;
}

int PGCommon::handlePGParams(const ia_css_frame_format_type* frameFormatTypes)
{
    int pgParamsSize = ia_css_sizeof_program_group_param(mProgramCount, mTerminalCount, mFragmentCount);

    mPGParamsBuffer = createUserPtrCiprBuffer(pgParamsSize);
    CheckError(!mPGParamsBuffer, NO_MEMORY, "%s, call createUserPtrCiprBuffer fail", __func__);

    ia_css_program_group_param_t* pgParamsBuf = (ia_css_program_group_param_t*)getCiprBufferPtr(mPGParamsBuffer);
    int ret = ia_css_program_group_param_init(pgParamsBuf, mProgramCount, mTerminalCount, mFragmentCount, frameFormatTypes);
    CheckError((ret != OK), ret, "%s, call ia_css_program_group_param_init fail", __func__);

    if (mPPG) {
        ret = ia_css_program_group_param_set_protocol_version(
                pgParamsBuf,
                IA_CSS_PROCESS_GROUP_PROTOCOL_PPG);
        CheckError((ret != OK), ret, "%s, call ia_css_program_group_param_set_protocol_version fail", __func__);
    }
    return ret;
}

int PGCommon::setKernelBitMap()
{
    ia_css_program_group_param_t* pgParamsBuf = (ia_css_program_group_param_t*)getCiprBufferPtr(mPGParamsBuffer);
    LOG1("%s: mKernelBitmap: %#018lx", __func__, mKernelBitmap);
    int ret = ia_css_program_group_param_set_kernel_enable_bitmap(pgParamsBuf, mKernelBitmap);
    CheckError((ret != OK), ret, "%s, call ia_css_program_group_param_set_kernel_enable_bitmap fail", __func__);

    return ret;
}

int PGCommon::setTerminalParams(const ia_css_frame_format_type* frameFormatTypes)
{
    ia_css_program_group_param_t* pgParamsBuf =
        (ia_css_program_group_param_t*)getCiprBufferPtr(mPGParamsBuffer);
    ia_css_program_group_manifest_t* pg_manifest =
        (ia_css_program_group_manifest_t*)getCiprBufferPtr(mManifestBuffer);

    for (int i = 0; i < mTerminalCount; i++) {
        ia_css_terminal_param_t *terminalParam =
            ia_css_program_group_param_get_terminal_param(pgParamsBuf, i);
        CheckError(!terminalParam, UNKNOWN_ERROR, "%s, call ia_css_program_group_param_get_terminal_param fail", __func__);
        ia_css_terminal_manifest_t *terminal_manifest = ia_css_program_group_manifest_get_term_mnfst(pg_manifest, i);
        ia_css_terminal_type_t  terminal_type = ia_css_terminal_manifest_get_type(terminal_manifest);
        if (!((terminal_type == IA_CSS_TERMINAL_TYPE_DATA_OUT) || (terminal_type == IA_CSS_TERMINAL_TYPE_DATA_IN))) {
            continue;
        }

        FrameInfo config = mTerminalFrameInfos[i];
        terminalParam->frame_format_type = frameFormatTypes[i];
        terminalParam->dimensions[IA_CSS_COL_DIMENSION] = config.mWidth;
        terminalParam->dimensions[IA_CSS_ROW_DIMENSION] = config.mHeight;
        terminalParam->fragment_dimensions[IA_CSS_COL_DIMENSION] = config.mWidth;
        terminalParam->fragment_dimensions[IA_CSS_ROW_DIMENSION] = config.mHeight;

        terminalParam->bpp = PGUtils::getCssBpp(config.mFormat);
        terminalParam->bpe = terminalParam->bpp;
        terminalParam->stride = PGUtils::getCssStride(config.mFormat, config.mWidth);

        terminalParam->offset = 0;
        terminalParam->index[IA_CSS_COL_DIMENSION] = 0;
        terminalParam->index[IA_CSS_ROW_DIMENSION] = 0;

        LOG2("%s: setTerminalParams: index=%d, format=%d, w=%d, h=%d, fw=%d, fh=%d, bpp=%d, bpe=%d, stride=%d, offset=%d, col=%d, row=%d",
             getName(), i,
             terminalParam->frame_format_type,
             terminalParam->dimensions[IA_CSS_COL_DIMENSION],
             terminalParam->dimensions[IA_CSS_ROW_DIMENSION],
             terminalParam->fragment_dimensions[IA_CSS_COL_DIMENSION],
             terminalParam->fragment_dimensions[IA_CSS_ROW_DIMENSION],
             terminalParam->bpp,
             terminalParam->bpe,
             terminalParam->stride,
             terminalParam->offset,
             terminalParam->index[IA_CSS_COL_DIMENSION],
             terminalParam->index[IA_CSS_ROW_DIMENSION]);
    }

    return OK;
}

int PGCommon::configureFragmentDesc()
{
    int num = mTerminalCount * mFragmentCount;
    std::unique_ptr<ia_p2p_fragment_desc[]> srcFragDesc =
                    std::unique_ptr<ia_p2p_fragment_desc[]>(new ia_p2p_fragment_desc[num]);
    int count = mPGParamAdapt->getFragmentDescriptors(num, srcFragDesc.get());
    CheckError(!count, UNKNOWN_ERROR, "getFragmentDescriptors fails");

    for (int termIdx = 0; termIdx < mTerminalCount; termIdx++) {
        if (mPgTerminals[termIdx] >= IPU_MAX_TERMINAL_COUNT) {
            continue;
        }

        ia_css_terminal_t* terminal = ia_css_process_group_get_terminal(mProcessGroup, mPgTerminals[termIdx]);
        ia_css_terminal_type_t terminalType = ia_css_terminal_get_type(terminal);
        if (!((terminalType == IA_CSS_TERMINAL_TYPE_DATA_OUT) || (terminalType == IA_CSS_TERMINAL_TYPE_DATA_IN))) {
            continue;
        }
        configureTerminalFragmentDesc(termIdx, &srcFragDesc[termIdx]);
    }
    return OK;
}

int PGCommon::configureTerminalFragmentDesc(int termIdx, const ia_p2p_fragment_desc* srcDesc)
{
#define DDR_WORD_BYTES 64
    ia_css_terminal_t* terminal = ia_css_process_group_get_terminal(mProcessGroup, mPgTerminals[termIdx]);
    ia_css_terminal_type_t terminalType = ia_css_terminal_get_type(terminal);
    if (!((terminalType == IA_CSS_TERMINAL_TYPE_DATA_OUT) || (terminalType == IA_CSS_TERMINAL_TYPE_DATA_IN))) {
        return OK;
    }

    bool vectorized = false;
    int packed_multiplier = 1;
    int packed_divider = 1;
    int dimension_bpp = PGUtils::getCssBpp(mTerminalFrameInfos[termIdx].mFormat);

    switch(mFrameFormatType[termIdx]) {
    case IA_CSS_DATA_FORMAT_BAYER_VECTORIZED:
    case IA_CSS_DATA_FORMAT_BAYER_LINE_INTERLEAVED:
        vectorized = true;
        dimension_bpp = (uint8_t) ALIGN_8(PGUtils::getCssBpp(mTerminalFrameInfos[termIdx].mFormat));
        break;
    case IA_CSS_DATA_FORMAT_RAW:
        dimension_bpp = (uint8_t) ALIGN_8(PGUtils::getCssBpp(mTerminalFrameInfos[termIdx].mFormat));
        break;
    case IA_CSS_DATA_FORMAT_BAYER_GRBG:
    case IA_CSS_DATA_FORMAT_BAYER_RGGB:
    case IA_CSS_DATA_FORMAT_BAYER_BGGR:
    case IA_CSS_DATA_FORMAT_BAYER_GBRG:
        dimension_bpp = (uint8_t) ALIGN_8(PGUtils::getCssBpp(mTerminalFrameInfos[termIdx].mFormat));
        break;
    case IA_CSS_DATA_FORMAT_YYUVYY_VECTORIZED:
        dimension_bpp = (uint8_t) (PGUtils::getCssBpp(mTerminalFrameInfos[termIdx].mFormat) * 3 / 2);
        packed_multiplier = 3;
        packed_divider = 2;
        vectorized = true;
        break;
    default:
        break;
    }

    for (int fragIdx = 0; fragIdx < mFragmentCount; fragIdx++) {
        ia_css_fragment_descriptor_t* dstFragDesc =
                ia_css_data_terminal_get_fragment_descriptor((ia_css_data_terminal_t*)terminal, fragIdx);
        CheckError(!dstFragDesc, -1, "%s: Can't get frag desc from terminal", __func__);

        dstFragDesc->dimension[IA_CSS_COL_DIMENSION] = srcDesc[fragIdx].fragment_width;
        dstFragDesc->dimension[IA_CSS_ROW_DIMENSION] = srcDesc[fragIdx].fragment_height;
        dstFragDesc->index[IA_CSS_COL_DIMENSION] = (uint16_t)
                (((srcDesc[fragIdx].fragment_start_x * packed_multiplier)
                 / packed_divider) * (vectorized ? 2 : 1));
        dstFragDesc->index[IA_CSS_ROW_DIMENSION] = (uint16_t)
                (srcDesc[fragIdx].fragment_start_y / (vectorized ? 2 : 1));

        int colOffset = 0;
        int pixels_per_word = 0;
        switch (mFrameFormatType[termIdx]) {
        case IA_CSS_DATA_FORMAT_YUV420:
        case IA_CSS_DATA_FORMAT_YYUVYY_VECTORIZED:
        case IA_CSS_DATA_FORMAT_BAYER_VECTORIZED:
            /** \todo Fragmentation with DMA packed formats is still open, need to
             * check this again when it is more clear (see #H1804344344).
             */
            pixels_per_word = (uint16_t) floor(DDR_WORD_BYTES * 8 / dimension_bpp);
            colOffset = (uint16_t) (floor(dstFragDesc->index[IA_CSS_COL_DIMENSION] / pixels_per_word) * DDR_WORD_BYTES);
            colOffset = (uint16_t) (colOffset + (((dstFragDesc->index[IA_CSS_COL_DIMENSION] % pixels_per_word) * dimension_bpp) / 8));
            break;
        default:
            colOffset = (uint16_t) (dstFragDesc->index[IA_CSS_COL_DIMENSION] * dimension_bpp / 8);
            break;
        }

        dstFragDesc->offset[IA_CSS_COL_DIMENSION] = (uint16_t)colOffset;
        dstFragDesc->offset[IA_CSS_ROW_DIMENSION] = dstFragDesc->index[IA_CSS_ROW_DIMENSION];

        LOG2("%s: %d:%d: get frag desc %d (%d, %d, %d, %d)", __func__, mPGId, termIdx, fragIdx,
             srcDesc[fragIdx].fragment_width, srcDesc[fragIdx].fragment_height,
             srcDesc[fragIdx].fragment_start_x, srcDesc[fragIdx].fragment_start_y);
        LOG2("%s: %d:%d:       frag %d: size(%d, %d) index(%d, %d), offset(%d, %d)", __func__, mPGId, termIdx,fragIdx,
             dstFragDesc->dimension[IA_CSS_COL_DIMENSION],
             dstFragDesc->dimension[IA_CSS_ROW_DIMENSION],
             dstFragDesc->index[IA_CSS_COL_DIMENSION],
             dstFragDesc->index[IA_CSS_ROW_DIMENSION],
             dstFragDesc->offset[IA_CSS_COL_DIMENSION],
             dstFragDesc->offset[IA_CSS_ROW_DIMENSION]);
    }
    return OK;
}

int PGCommon::iterate(CameraBufferMap &inBufs, CameraBufferMap &outBufs,
                      ia_binary_data *statistics, const ia_binary_data *ipuParameters)
{
    LOG2("%s:%s ++", getName(), __func__);

    long sequence = 0;
    if (!inBufs.empty()) {
        sequence = inBufs.begin()->second->getSequence();
    }

    int ret = prepareTerminalBuffers(ipuParameters, inBufs, outBufs, sequence);
    CheckError((ret != OK), ret, "%s, prepareTerminalBuffers fail with %d", getName(), ret);

    // Create PPG & PPG start/stop commands at the beginning
    if (mPPG && !mPPGBuffer) {
        ia_css_program_group_param_t* pgParamsBuf =
            (ia_css_program_group_param_t*)getCiprBufferPtr(mPGParamsBuffer);
        ia_css_program_group_manifest_t* manifestBuf =
            (ia_css_program_group_manifest_t*)getCiprBufferPtr(mManifestBuffer);

        size_t pgSize = ia_css_sizeof_process_group(manifestBuf, pgParamsBuf);

        mPPGBuffer = createUserPtrCiprBuffer(pgSize);
        CheckError(!mPPGBuffer, NO_MEMORY, "%s, call createUserPtrCiprBuffer fail", __func__);
        mPPGProcessGroup = (ia_css_process_group_t*)getCiprBufferPtr(mPPGBuffer);
        MEMCPY_S(mPPGProcessGroup, pgSize, mProcessGroup, ia_css_process_group_get_size(mProcessGroup));

    }
    if (!mCmd) {
        ret = createCommands();
       CheckError((ret != OK), ret, "%s, call createCommands fail", __func__);
    }

    if (mPPG && !mPPGStarted) {
        ret = startPPG();
        CheckError((ret != OK), ret, "%s, startPPG fail", getName());
        mPPGStarted = true;
    }

    ret = executePG();
    CheckError((ret != OK), ret, "%s, executePG fail", getName());

    if (statistics) {
        ret = mPGParamAdapt->decode(mTerminalCount, mParamPayload, statistics);
        CheckError((ret != OK), ret, "%s, decode fail", getName());
    }

    postTerminalBuffersDone(sequence);
    LOG2("%s:%s -- ", getName(), __func__);
    return ret;
}

int PGCommon::preparePayloadBuffers() {
    int count = mPGParamAdapt->getPayloadSizes(mTerminalCount, mParamPayload);
    CheckError((count != mTerminalCount), NO_MEMORY, "%s, getPayloadSize fails", __func__);

    // For TNR parameter refer terminals
    int ret = allocateTnrSimBuffers();
    CheckError((ret != OK), NO_MEMORY, "%s, allocateTnrSimBuffers fails", __func__);

    // For other normal terminals
    std::vector<ia_binary_data> payloads;
    ia_binary_data payload = {nullptr, 0};
    for (int termIdx = 0; termIdx < mTerminalCount; termIdx++) {
        if (mParamPayload[termIdx].data) {
            payload.size = 0;  // allocated
        } else {
            payload.size = mParamPayload[termIdx].size;
        }
        payloads.push_back(payload);
    }
    ret = mPGParamAdapt->allocatePayloads(payloads.size(), payloads.data());
    CheckError(ret != OK, NO_MEMORY, "%s, allocate payloads fail", __func__);
    for (int i = 0; i < mTerminalCount; i++) {
        if (payloads[i].data) {
            CIPR::Buffer* ciprBuf = registerUserBuffer(payloads[i].size, payloads[i].data);
            CheckError(!ciprBuf, NO_MEMORY, "%s, register payload buffer %p for term %d fail",
                       __func__, payloads[i].data, i);
            memset(payloads[i].data, 0, PAGE_ALIGN(payloads[i].size));
            mParamPayload[i].data = payloads[i].data;
            mTerminalBuffers[i] = ciprBuf;
        }
    }

    return OK;
}

int PGCommon::allocateTnrDataBuffers() {
    if (mTnrTerminalPair.inId < 0 || !mTnrDataBuffers.empty()) return OK;

    // port = term index + 1
    int32_t termIndex = mTnrTerminalPair.inId;
    int64_t referId = ShareReferBufferPool::constructReferId(mStreamId, mPGId, termIndex + 1);
    int32_t bufferCount = PAIR_BUFFER_COUNT;
    if (mShareReferPool.get()) {
        bufferCount = mShareReferPool->getMinBufferNum(referId);
        if (bufferCount) {
            mShareReferIds[termIndex] = referId;  // mShareReferPool supports for the refer buffers
        }
    }
    if (bufferCount < PAIR_BUFFER_COUNT) {
        bufferCount = PAIR_BUFFER_COUNT;
    }

    int size = CameraUtils::getFrameSize(mTerminalFrameInfos[mInputMainTerminal].mFormat,
                                         mTerminalFrameInfos[mInputMainTerminal].mWidth,
                                         mTerminalFrameInfos[mInputMainTerminal].mHeight);
    for (int32_t i = 0; i < bufferCount; i++) {
        uint8_t* buffer = nullptr;
        int ret = posix_memalign((void**)&buffer, PAGE_SIZE_U, PAGE_ALIGN(size));
        CheckError(ret, NO_MEMORY, "%s, alloc %d tnr data buf fails", __func__, i);
        mTnrDataBuffers.push_back(buffer);

        CIPR::Buffer* ciprBuf = registerUserBuffer(size, buffer);
        CheckError(!ciprBuf, NO_MEMORY, "%s, register %d tnr buf %p fails", __func__, i, buffer);

        if (i == PAIR_BUFFER_IN_INDEX) mTerminalBuffers[mTnrTerminalPair.inId] = ciprBuf;
        else if (i == PAIR_BUFFER_OUT_INDEX) mTerminalBuffers[mTnrTerminalPair.outId] = ciprBuf;
        if (mShareReferIds[termIndex])
            mShareReferPool->registerReferBuffers(referId, ciprBuf);
    }

    return OK;
}

int PGCommon::allocateTnrSimBuffers() {
    for (auto pair : mTnrSimTerminalPairs) {
        int32_t inId = pair.inId;
        // port = term index + 1
        int64_t referId = ShareReferBufferPool::constructReferId(mStreamId, mPGId, inId + 1);
        int32_t bufferCount = PAIR_BUFFER_COUNT;
        if (mShareReferPool.get()) {
            bufferCount = mShareReferPool->getMinBufferNum(referId);
            if (bufferCount) {
                mShareReferIds[inId] = referId;  // flag it: get payload via mShareReferPool
            }
        }
        if (bufferCount < PAIR_BUFFER_COUNT) {
            bufferCount = PAIR_BUFFER_COUNT;
        }

        std::vector<ia_binary_data> payloads;
        ia_binary_data payload = {nullptr, mParamPayload[inId].size};
        for (int32_t i = 0; i < bufferCount; i++) {
            payloads.push_back(payload);
        }
        int ret = mPGParamAdapt->allocatePayloads(payloads.size(), payloads.data());
        CheckError(ret != OK, NO_MEMORY, "%s, allocate for term pair %d fail", __func__, inId);

        // Register all buffers and clear
        for (int32_t i = 0; i < bufferCount; i++) {
            CIPR::Buffer* ciprBuf = registerUserBuffer(payloads[i].size, payloads[i].data);
            CheckError(!ciprBuf, NO_MEMORY, "%s, register %d:%p for term pair %d fails",
                       __func__, i, payloads[i].data, inId);
            memset(payloads[i].data, 0, PAGE_ALIGN(payloads[i].size));

            if (mShareReferIds[inId])
                mShareReferPool->registerReferBuffers(referId, ciprBuf);

            // Set default payload for terminal pair to mark they are allocated.
            if (i == PAIR_BUFFER_IN_INDEX) {
                mParamPayload[pair.inId].data = payloads[PAIR_BUFFER_IN_INDEX].data;
                mTerminalBuffers[pair.inId] = ciprBuf;
            } else if (i == PAIR_BUFFER_OUT_INDEX) {
                mParamPayload[pair.outId].data = payloads[PAIR_BUFFER_OUT_INDEX].data;
                mTerminalBuffers[pair.outId] = ciprBuf;
            }
        }

    }
    return OK;
}

int PGCommon::prepareTerminalBuffers(const ia_binary_data *ipuParameters,
                                     const CameraBufferMap& inBufs, const CameraBufferMap& outBufs,
                                     long sequence) {
    CIPR::Buffer* ciprBuf = nullptr;
    // Prepare payload
    for (int termIdx = 0; termIdx < mTerminalCount; termIdx++) {
        // Payload for data terminals
        std::shared_ptr<CameraBuffer> buffer;
        ia_uid terminalUid = mTerminalBaseUid + termIdx;
        if (inBufs.find(terminalUid) != inBufs.end()) {
             buffer = inBufs.at(terminalUid);
        } else if (outBufs.find(terminalUid) != outBufs.end()) {
             buffer = outBufs.at(terminalUid);
        }

        if (buffer) {
            bool flush = buffer->getUsage() == BUFFER_USAGE_GENERAL ? true : false;
            ciprBuf = (buffer->getMemory() == V4L2_MEMORY_DMABUF) \
                     ? registerUserBuffer(buffer->getBufferSize(), buffer->getFd(), flush) \
                     : registerUserBuffer(buffer->getBufferSize(), buffer->getBufferAddr(), flush);
            CheckError(!ciprBuf, NO_MEMORY, "%s, register buffer size %d for terminal %d fail",
                       __func__, buffer->getBufferSize(), termIdx);
            mTerminalBuffers[termIdx] = ciprBuf;
        }
    }

    if (!mTnrDataBuffers.empty()) {
        if (mShareReferIds[mTnrTerminalPair.inId]) {
            mShareReferPool->acquireBuffer(mShareReferIds[mTnrTerminalPair.inId],
                                           &mTerminalBuffers[mTnrTerminalPair.inId],
                                           &mTerminalBuffers[mTnrTerminalPair.outId],
                                           sequence);
        } else {
            std::swap(mTerminalBuffers[mTnrTerminalPair.inId],
                      mTerminalBuffers[mTnrTerminalPair.outId]);
        }
    }

    for (auto& pair : mDvsTerminalPairs) {
        std::swap(mTerminalBuffers[pair.inId], mTerminalBuffers[pair.outId]);
    }

    for (auto& pair : mTnrSimTerminalPairs) {
        if (mShareReferIds[pair.inId]) {
            mShareReferPool->acquireBuffer(mShareReferIds[pair.inId],
                                           &mTerminalBuffers[pair.inId],
                                           &mTerminalBuffers[pair.outId],
                                           sequence);
        } else {
            std::swap(mTerminalBuffers[pair.inId], mTerminalBuffers[pair.outId]);
        }

        // Update palyload buffers
        mTerminalBuffers[pair.inId]->getMemoryCpuPtr(&mParamPayload[pair.inId].data);
        mTerminalBuffers[pair.outId]->getMemoryCpuPtr(&mParamPayload[pair.outId].data);
    }

    return mPGParamAdapt->updatePALAndEncode(ipuParameters, mTerminalCount, mParamPayload);
}

void PGCommon::postTerminalBuffersDone(long sequence) {
    if (!mTnrDataBuffers.empty() && mShareReferIds[mTnrTerminalPair.inId]) {
        mShareReferPool->releaseBuffer(mShareReferIds[mTnrTerminalPair.inId],
                                       mTerminalBuffers[mTnrTerminalPair.inId],
                                       mTerminalBuffers[mTnrTerminalPair.outId],
                                       sequence);
    }
    for (auto pair : mTnrSimTerminalPairs) {
        if (mShareReferIds[pair.inId]) {
            mShareReferPool->releaseBuffer(mShareReferIds[pair.inId],
                                           mTerminalBuffers[pair.inId],
                                           mTerminalBuffers[pair.outId],
                                           sequence);
        }
    }
}

int PGCommon::executePG()
{
    TRACE_LOG_PROCESS(mName.c_str(), __func__);
    CheckError((!mCmd), INVALID_OPERATION, "%s, Command is invalid.", __func__);
    CheckError((!mProcessGroup), INVALID_OPERATION, "%s, process group is invalid.", __func__);

    mCmd->getConfig(&mCmdCfg);
    int bufferCount = ia_css_process_group_get_terminal_count(mProcessGroup);
    mCmdCfg.id = mPGId;
    mCmdCfg.priority = 1;
    mCmdCfg.pgParamsBuf = mPPG ? nullptr : mPGParamsBuffer;
    mCmdCfg.pgManifestBuf = mManifestBuffer;
    mCmdCfg.pg = mPGBuffer;
    mCmdCfg.extBuf = mCmdExtBuffer;
    mCmdCfg.buffers.resize(bufferCount);

    for (int i = 0; i < bufferCount; i++) {
        ia_css_terminal_t *terminal = ia_css_process_group_get_terminal(mProcessGroup, i);
        CheckError(!terminal, UNKNOWN_ERROR, "failed to get terminal");
        mCmdCfg.buffers[i] = mTerminalBuffers[terminal->tm_index];
    }
    if (mPPG) {
         ia_css_process_group_set_token(mProcessGroup, mToken);
    }

    for (int fragIdx = 0; fragIdx < mFragmentCount; fragIdx++) {
        int ret = ia_css_process_group_set_fragment_state(mProcessGroup, (uint16_t)fragIdx);
        CheckError((ret != OK), ret, "%s, set fragment count %d fail %p", getName(), fragIdx, mProcessGroup);
        ret = ia_css_process_group_set_fragment_limit(mProcessGroup, (uint16_t)(fragIdx + 1));
        CheckError((ret != OK), ret, "%s, set fragment limit %d fail", getName(), fragIdx);

        ret = handleCmd(&mCmd, &mCmdCfg);
        CheckError((ret != OK), ret, "%s, call handleCmd fail", getName());
    }

    return OK;
}

int PGCommon::startPPG()
{
    // Get basic command config
    CIPR::PSysCommandConfig cmdCfg;
    mPPGCmd[PPG_CMD_TYPE_START]->getConfig(&cmdCfg);

    // Update config
    cmdCfg.id = mPGId;
    cmdCfg.priority = 1;
    cmdCfg.pgParamsBuf = mPPG ? nullptr : mPGParamsBuffer;
    cmdCfg.pgManifestBuf = mManifestBuffer;
    cmdCfg.pg = mPPGBuffer;
    cmdCfg.extBuf = mPPGCmdExtBuffer[PPG_CMD_TYPE_START];
    const int terminalCount = ia_css_process_group_get_terminal_count(mProcessGroup);
    cmdCfg.buffers.resize(terminalCount);
    std::fill(cmdCfg.buffers.begin(), cmdCfg.buffers.end(), nullptr);

    ia_css_process_group_set_fragment_state(mPPGProcessGroup, 0);
    ia_css_process_group_set_fragment_limit(mPPGProcessGroup, 1);

    int ret = handleCmd(&mPPGCmd[PPG_CMD_TYPE_START], &cmdCfg);
    mToken = ia_css_process_group_get_token(mPPGProcessGroup);
    return ret;
}

int PGCommon::stopPPG()
{
    CIPR::PSysCommandConfig cmdCfg;

    mPPGCmd[PPG_CMD_TYPE_STOP]->getConfig(&cmdCfg);

    cmdCfg.id = mCmdCfg.id;
    cmdCfg.priority = mCmdCfg.priority;
    cmdCfg.pgParamsBuf = mCmdCfg.pgParamsBuf;
    cmdCfg.pgManifestBuf = mCmdCfg.pgManifestBuf;
    cmdCfg.pg = mPPGBuffer;
    cmdCfg.extBuf = mPPGCmdExtBuffer[PPG_CMD_TYPE_STOP];
    cmdCfg.buffers.resize(0);

    int ret =  handleCmd(&mPPGCmd[PPG_CMD_TYPE_STOP], &cmdCfg);
    return ret;
}

int PGCommon::handleCmd(CIPR::Command** cmd, CIPR::PSysCommandConfig* cmdCfg)
{
    CIPR::PSysEventConfig eventCfg = {};
    mEvent->getConfig(&eventCfg);
    cmdCfg->issueID = reinterpret_cast<uint64_t>(cmd);
    eventCfg.commandIssueID = cmdCfg->issueID;

    CIPR::Result ret = (*cmd)->setConfig(*cmdCfg);
    CheckError((ret != CIPR::Result::OK), UNKNOWN_ERROR, "%s, call CIPR::Command::setConfig fail", __func__);

    ret = (*cmd)->getConfig(cmdCfg);
    CheckError((ret != CIPR::Result::OK), UNKNOWN_ERROR, "%s, call CIPR::Command::getConfig fail", __func__);

    ret = (*cmd)->enqueue(mCtx);
    CheckError((ret != CIPR::Result::OK), UNKNOWN_ERROR, "%s, call Context::enqueueCommand() fail %d", __func__, ret);

    // Wait event
    ret = mEvent->wait(mCtx);
    CheckError((ret != CIPR::Result::OK), UNKNOWN_ERROR, "%s, call Context::waitForEvent fail, ret: %d", __func__, ret);

    ret = mEvent->getConfig(&eventCfg);
    CheckError((ret != CIPR::Result::OK), UNKNOWN_ERROR, "%s, call Event::getConfig() fail, ret: %d", __func__, ret);
    // Ignore the error in event config since it's not a fatal error.
    if (eventCfg.error) {
        LOGW("%s, event config error: %d", __func__, eventCfg.error);
    }

    return (eventCfg.error == 0) ? OK : UNKNOWN_ERROR;
}

int PGCommon::getCapability()
{
    CIPR::PSYSCapability cap;
    int ret = OK;
    CIPR::Result err = mCtx->getCapabilities(&cap);
    CheckError((err != CIPR::Result::OK), UNKNOWN_ERROR, "Call Context::getCapabilities() fail, ret:%d", ret);

    LOG1("%s: capability.version:%d", __func__, cap.version);
    LOG1("%s: capability.driver:%s", __func__, cap.driver);
    LOG1("%s: capability.devModel:%s", __func__, cap.devModel);
    LOG1("%s: capability.programGroupCount:%d", __func__, cap.programGroupCount);
    mPGCount = cap.programGroupCount;

    if (strncmp((char *)cap.devModel, "ipu4p", 5) == 0) {
        mPlatform = IA_P2P_PLATFORM_CNL_B0;
        LOG1("%s: cnl/icl/ksl shared the same p2p platform id", __func__);
    } else if (strncmp((char *)cap.devModel, "ipu4", 4) == 0) {
        switch (cap.devModel[13]) {
            case 'B':
                 mPlatform = IA_P2P_PLATFORM_BXT_B0;
                 break;
            default:
                 LOGE("%s: unsupported psys device model :%s", __func__, cap.devModel);
                 ret = BAD_VALUE;
                 break;
        }
    } else if (strncmp((char *)cap.devModel, "ipu6", 4) == 0) {
        mPlatform = IA_P2P_PLATFORM_IPU6;
        mPPG = true;
    } else {
        LOGE("%s: unsupported psys device model : %s", __func__, cap.devModel);
        ret = BAD_VALUE;
    }

    return ret;
}

int PGCommon::getManifest(int pgId)
{
    int i = 0;

    for (; i < mPGCount; i++) {
        CIPR::Buffer* manifestBuffer = nullptr;
        int programCount = 0;
        int terminalCount = 0;
        int programGroupId = 0;
        int manifestSize = 0;
        ia_css_kernel_bitmap_t kernelBitmap = ia_css_kernel_bitmap_clear();
        uint32_t size = 0;

        CIPR::Result ret = mCtx->getManifest(i, &size, nullptr);
        if (ret != CIPR::Result::OK) continue;
        CheckError((size == 0), UNKNOWN_ERROR, "%s, the manifest size is 0", __func__);

        manifestBuffer = createUserPtrCiprBuffer(size);
        CheckError(!manifestBuffer, NO_MEMORY, "%s, call createUserPtrCiprBuffer fail", __func__);

        void* manifest = getCiprBufferPtr(manifestBuffer);

        ret = mCtx->getManifest(i, &size, manifest);
        if (ret != CIPR::Result::OK) {
            LOGE("%s, call Context::getManifest() fail", __func__);
            delete manifestBuffer;
            return UNKNOWN_ERROR;
        }

        LOG1("%s: pg index: %d, manifest size: %u", __func__, i, size);
        const ia_css_program_group_manifest_t *mf = (const ia_css_program_group_manifest_t*)manifest;
        programCount = ia_css_program_group_manifest_get_program_count(mf);
        terminalCount = ia_css_program_group_manifest_get_terminal_count(mf);
        programGroupId = ia_css_program_group_manifest_get_program_group_ID(mf);
        manifestSize = ia_css_program_group_manifest_get_size(mf);
        kernelBitmap = ia_css_program_group_manifest_get_kernel_bitmap(mf);

        LOG1("%s: pgIndex: %d, programGroupId: %d, manifestSize: %d, programCount: %d, terminalCount: %d",
             __func__, i, programGroupId, manifestSize, programCount, terminalCount);

        if (pgId == programGroupId) {
            mProgramCount = programCount;
            mTerminalCount = terminalCount;
            mManifestSize = manifestSize;
            mKernelBitmap = kernelBitmap;
            mManifestBuffer = manifestBuffer;
            break;
        }

        delete manifestBuffer;
    }

    CheckError((i == mPGCount), BAD_VALUE, "%s, Can't found available pg: %d", __func__, pgId);

    return OK;
}

CIPR::Buffer* PGCommon::createDMACiprBuffer(int size, int fd, bool flush)
{
    CIPR::MemoryFlag deviceFlags = CIPR::MemoryFlag::MemoryHandle;
    if (!flush) deviceFlags |= CIPR::MemoryFlag::NoFlush;

    CIPR::MemoryDesc mem;
    mem.size = size;
    mem.flags = CIPR::MemoryFlag::MemoryHandle | CIPR::MemoryFlag::HardwareOnly;
    mem.handle = fd;
    mem.cpuPtr = nullptr;
    mem.anchor = nullptr;

    CIPR::Buffer* buf = new CIPR::Buffer(size, mem.flags | deviceFlags, &mem);

    CIPR::Result ret = buf->attatchDevice(mCtx);
    if (ret != CIPR::Result::OK) {
        LOGE("%s, call Buffer::attatchDevice() fail", __func__);
        delete buf;
        return nullptr;
    }

    return buf;
}

CIPR::Buffer* PGCommon::createUserPtrCiprBuffer(int size, void* ptr, bool flush)
{
    CIPR::Buffer* buf = nullptr;
    if (ptr == nullptr) {
        buf = new CIPR::Buffer(size, CIPR::MemoryFlag::AllocateCpuPtr | CIPR::MemoryFlag::NoFlush,
                               nullptr);
    } else {
        CIPR::MemoryDesc mem;
        mem.size = size;
        mem.flags = CIPR::MemoryFlag::CpuPtr;
        if (!flush) mem.flags |= CIPR::MemoryFlag::NoFlush;
        mem.handle = 0;
        mem.cpuPtr = ptr;
        mem.anchor = nullptr;
        buf = new CIPR::Buffer(size, CIPR::MemoryFlag::CpuPtr, &mem);
    }

    CIPR::Result ret = buf->attatchDevice(mCtx);
    if (ret != CIPR::Result::OK) {
        LOGE("%s, call Buffer::attatchDevice() fail", __func__);
        delete buf;
        return nullptr;
    }

    return buf;
}

void* PGCommon::getCiprBufferPtr(CIPR::Buffer* buffer)
{
    CheckError(!buffer, nullptr, "%s, invalid cipr buffer", __func__);

    void* ptr = nullptr;
    CIPR::Result ret = buffer->getMemoryCpuPtr(&ptr);
    CheckError((ret != CIPR::Result::OK), nullptr, "%s, call Buffer::getMemoryCpuPtr() fail", __func__);

    return ptr;
}

int PGCommon::getCiprBufferSize(CIPR::Buffer* buffer)
{
    CheckError(!buffer, BAD_VALUE, "%s, invalid cipr buffer", __func__);

    int size = 0;
    CIPR::Result ret = buffer->getMemorySize(&size);
    CheckError((ret != CIPR::Result::OK), NO_MEMORY, "%s, call Buffer::getMemorySize() fail", __func__);

    return size;
}

CIPR::Buffer* PGCommon::registerUserBuffer(int size, void* ptr, bool flush)
{
    CheckError((size <= 0 || ptr == nullptr), nullptr, "Invalid parameter: size=%d, ptr=%p", size, ptr);

    for (auto it = mBuffers.begin(); it != mBuffers.end(); ++it) {
        if (ptr == it->userPtr) {
            if (size == getCiprBufferSize(it->ciprBuf)) {
                return it->ciprBuf;
            }

            LOG2("%s, the buffer size is changed: old(%d), new(%d) addr(%p)",
                 __func__, getCiprBufferSize(it->ciprBuf), size, it->userPtr);
            delete it->ciprBuf;
            it->ciprBuf = nullptr;
            it->userPtr = nullptr;
            mBuffers.erase(it);
            break;
        }
    }

    CIPR::Buffer* ciprBuf = createUserPtrCiprBuffer(size, ptr, flush);
    CheckError(!ciprBuf, nullptr, "Create cipr buffer for %p failed", ptr);

    CiprBufferMapping bufMap;
    bufMap.userPtr = ptr;
    bufMap.ciprBuf = ciprBuf;
    mBuffers.push_back(bufMap);

    return ciprBuf;
}

CIPR::Buffer* PGCommon::registerUserBuffer(int size, int fd, bool flush)
{
    CheckError((size <= 0 || fd < 0), nullptr, "Invalid parameter: size: %d, fd: %d", size, fd);

    for (auto it = mBuffers.begin(); it != mBuffers.end(); ++it) {
        if (fd == it->userFd) {
            if (size == getCiprBufferSize(it->ciprBuf)) {
                return it->ciprBuf;
            }

            LOG2("%s, the buffer size is changed: old(%d), new(%d) fd(%d)",
                 __func__, getCiprBufferSize(it->ciprBuf), size, it->userFd);
            delete it->ciprBuf;
            it->ciprBuf = nullptr;
            it->userFd = -1;
            mBuffers.erase(it);
            break;
        }
    }

    CIPR::Buffer* ciprBuf = createDMACiprBuffer(size, fd, flush);
    CheckError(!ciprBuf, nullptr, "Create cipr buffer for fd %d failed", fd);

    CiprBufferMapping bufMap;
    bufMap.userFd = fd;
    bufMap.ciprBuf = ciprBuf;
    mBuffers.push_back(bufMap);

    return ciprBuf;
}

void PGCommon::dumpTerminalPyldAndDesc(int pgId, long sequence, ia_css_process_group_t* pgGroup)
{
    if (!CameraDump::isDumpTypeEnable(DUMP_PSYS_PG)) return;

    char fileName[MAX_NAME_LEN] = {'\0'};
    uint32_t pgSize = ia_css_process_group_get_size(pgGroup);
    snprintf(fileName, (MAX_NAME_LEN - 1), "hal_pg_%d_%ld.bin", pgId, sequence);

    FILE *fp = fopen (fileName, "w+");
    CheckError(fp == nullptr, VOID_VALUE, "open dump file %s failed", fileName);
    const unsigned int* printPtr = (const unsigned int*)pgGroup;
    fprintf(fp, "::pg dump size %d(0x%x)\n", pgSize, pgSize);
    for (unsigned int i = 0; i < pgSize / sizeof(*printPtr); i++) {
        fprintf(fp, "%08x\n", printPtr[i]);
    }

    int terminalCount = ia_css_process_group_get_terminal_count(pgGroup);
    for (int i = 0; i < terminalCount; i++) {
        ia_css_terminal_t *terminal = ia_css_process_group_get_terminal(pgGroup, i);
        if (!terminal) {
            LOGE("failed to get terminal");
            fclose(fp);
            return;
        }
        if (terminal->terminal_type == IA_CSS_TERMINAL_TYPE_DATA_IN
            || terminal->terminal_type == IA_CSS_TERMINAL_TYPE_DATA_OUT) {
            continue;
        }

        void* ptr = getCiprBufferPtr(mTerminalBuffers[terminal->tm_index]);
        int size = getCiprBufferSize(mTerminalBuffers[terminal->tm_index]);
        const char* typeStr = (terminal->terminal_type == IA_CSS_TERMINAL_TYPE_DATA_IN) ? "DATA_IN"
                            : (terminal->terminal_type == IA_CSS_TERMINAL_TYPE_DATA_OUT) ? "DATA_OUT"
                            : (terminal->terminal_type == IA_CSS_TERMINAL_TYPE_PARAM_STREAM) ? "PARAM_STREAM"
                            : (terminal->terminal_type == IA_CSS_TERMINAL_TYPE_PARAM_CACHED_IN) ? "CACHED_IN"
                            : (terminal->terminal_type == IA_CSS_TERMINAL_TYPE_PARAM_CACHED_OUT) ? "CACHED_OUT"
                            : (terminal->terminal_type == IA_CSS_TERMINAL_TYPE_PARAM_SPATIAL_IN) ? "SPATIAL_IN"
                            : (terminal->terminal_type == IA_CSS_TERMINAL_TYPE_PARAM_SPATIAL_OUT) ? "SPATIAL_OUT"
                            : (terminal->terminal_type == IA_CSS_TERMINAL_TYPE_PARAM_SLICED_IN) ? "SLICED_IN"
                            : (terminal->terminal_type == IA_CSS_TERMINAL_TYPE_PARAM_SLICED_OUT) ? "SLICED_OU"
                            : (terminal->terminal_type == IA_CSS_TERMINAL_TYPE_STATE_IN) ? "STATE_IN"
                            : (terminal->terminal_type == IA_CSS_TERMINAL_TYPE_STATE_OUT) ? "STATE_OUT"
                            : (terminal->terminal_type == IA_CSS_TERMINAL_TYPE_PROGRAM) ? "PROGRAM"
                            : (terminal->terminal_type == IA_CSS_TERMINAL_TYPE_PROGRAM_CONTROL_INIT) ? "PROGRAM_CONTROL_INIT"
                            :                                                             "UNKNOWN";
        printPtr = (const unsigned int*)ptr;
        fprintf(fp, "::terminal %d dump size %d(0x%x), line %d, type %s\n", terminal->tm_index, size, size, PAGE_ALIGN(size)/4, typeStr);
        for (unsigned int i = 0; i < PAGE_ALIGN(size) / sizeof(*printPtr); i++) {
            fprintf(fp, "%08x\n", printPtr[i]);
        }
    }

    fclose (fp);
}

} // namespace icamera
