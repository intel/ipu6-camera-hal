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

extern "C" {
#include <ia_css_psys_program_group_manifest.h>
#include <ia_css_psys_terminal_manifest.h>
#include <ia_css_program_group_data.h>
#include <ia_css_program_group_param.h>
#include <ia_css_psys_process_group.h>
#include <ia_css_psys_terminal.h>
#include <ia_css_terminal_types.h>
#include <ia_css_terminal_manifest_types.h>
#include <ia_css_psysapi_fw_version.h>
}

#include <vector>
#include <memory>

#ifdef ENABLE_SANDBOXING
#include "modules/sandboxing/client/IntelPGParam.h"
#else
#include "modules/algowrapper/IntelPGParam.h"
#endif
#include "IspParamAdaptor.h"
#include "ShareReferBufferPool.h"
#include "BufferQueue.h"
#include "PGUtils.h"

#include "modules/ia_cipr/include/Buffer.h"
#include "modules/ia_cipr/include/Context.h"
#include "modules/ia_cipr/include/Event.h"
#include "modules/ia_cipr/include/Command.h"

namespace icamera {

typedef std::map<ia_uid, FrameInfo> TerminalFrameInfoMap;
typedef std::map<ia_uid, std::shared_ptr<CameraBuffer>> CameraBufferMap;

#define FRAGMENT_OVERLAP 64

/**
 * \class PGCommon
 *
 * \brief This is a version PG implementation which is used to config and run PG.
 *
 * The call sequence as follows:
 * 1. setShareReferPool() && init();
 * 2. setInputInfo();setOutputInfo();
 * 3. setDisabledTerminals();
 * 4. prepare():
 *          configTerminalFormat();
 *          calcFragmentCount();
 *          handlePGParams();
 *          setKernelBitMap();
 *          setTerminalParams();
 *          allocatePGBuffer();
 *          setPGAndPrepareProgram();
 *          configureFragmentDesc();
 * 5. loop frame: iterate():
 *          encodeTerminals();
 *          handleCmd();
 *          handleEvent();
 *          decode();
 * 6. deInit();
 */
class PGCommon {
public:
    static int getFrameSize(int format, int width, int height,
                            bool needAlignedHeight = false, bool needExtraSize = true, bool needCompression = false);

    PGCommon(int pgId, const std::string& pgName, ia_uid terminalBaseUid = 0);
    virtual ~PGCommon();
    void setShareReferPool(std::shared_ptr<ShareReferBufferPool> referPool) { mShareReferPool = referPool; }

    /**
     * allocate memory for some variables.
     */
    int init();

    /**
     * recycle memory.
     */
    void deInit();

    /**
     * set the input buffers info for terminals.
     * use ia_fourcc
     */
    virtual void setInputInfo(const TerminalFrameInfoMap& inputInfos);

    /**
     * set the output buffers info for terminals.
     * use ia_fourcc
     */
    virtual void setOutputInfo(const TerminalFrameInfoMap& outputInfos);

    /**
     * set the disabled terminals. Called before prepare()
     */
    virtual void setDisabledTerminals(const std::vector<ia_uid>& disabledTerminals);

    /**
     * set routing bitmap. Called before prepare()
     */
    virtual void setRoutingBitmap(const void* rbm, uint32_t bytes);

    /**
     * config the data terminals, init, config and prepare p2p, create process group.
     */
    virtual int prepare(IspParamAdaptor* adaptor, int streamId = -1);

    /**
     * run p2p to encode the params terminals, execute the PG and run p2p to decode the statistic terminals.
     */
    virtual int iterate(CameraBufferMap &inBufs, CameraBufferMap &outBufs,
                        ia_binary_data *statistics, const ia_binary_data *ipuParameters);

    const char* getName() { return mName.c_str(); }
private:
    DISALLOW_COPY_AND_ASSIGN(PGCommon);

protected:
    int getCapability();
    int getManifest(int pgId);

    // PG parameters intialization, for prepare()
    virtual int configTerminalFormat();
    int initParamAdapt();
    virtual int calcFragmentCount(int overlap = FRAGMENT_OVERLAP);
    int handlePGParams(const ia_css_frame_format_type* frameFormatTypes);
    int setKernelBitMap();
    virtual int setTerminalParams(const ia_css_frame_format_type* frameFormatTypes);
    virtual int configureFragmentDesc();
    int configureTerminalFragmentDesc(int termIdx, const ia_p2p_fragment_desc* srcDesc);
    ia_css_process_group_t* createPG(CIPR::Buffer** pgBuffer);
    int createCommands();
    int createCommand(CIPR::Buffer* pg, CIPR::Command** cmd, CIPR::Buffer** extBuffer, int bufCount);
    void destoryCommands();
    int preparePayloadBuffers();
    int allocateTnrDataBuffers();
    int allocateTnrSimBuffers();

    // For iteration
    virtual int prepareTerminalBuffers(const ia_binary_data *ipuParameters,
                                       const CameraBufferMap& inBufs,
                                       const CameraBufferMap& outBufs,
                                       long sequence);
    int executePG();
    int startPPG();
    int stopPPG();
    int handleCmd(CIPR::Command** cmd, CIPR::PSysCommandConfig* cmdCfg);

    void postTerminalBuffersDone(long sequence);

    // Memory helper
    CIPR::Buffer* createDMACiprBuffer(int size, int fd, bool flush = false);
    CIPR::Buffer* createUserPtrCiprBuffer(int size, void* ptr = nullptr, bool flush = false);
    void* getCiprBufferPtr(CIPR::Buffer* buffer);
    CIPR::Buffer* registerUserBuffer(int size, void* ptr, bool flush = false);
    CIPR::Buffer* registerUserBuffer(int size, int fd, bool flush = false);
    int getCiprBufferSize(CIPR::Buffer* buffer);

    void dumpTerminalPyldAndDesc(int pgId, long sequence, ia_css_process_group_t* pgGroup);

protected:
    enum PPGCommandType {
        PPG_CMD_TYPE_START = 0,
        PPG_CMD_TYPE_STOP,
        PPG_CMD_TYPE_COUNT
    };

    struct CiprBufferMapping {
        CiprBufferMapping() {}
        void* userPtr = nullptr;
        int userFd = -1;
        CIPR::Buffer* baseCiprBuf = nullptr;
        CIPR::Buffer* ciprBuf = nullptr;
    };

    static const int kEventTimeout = 8000;

    CIPR::Context* mCtx = nullptr;
    CIPR::Buffer* mManifestBuffer = nullptr;
    CIPR::Buffer* mPGParamsBuffer = nullptr;
    std::unique_ptr<IntelPGParam> mPGParamAdapt;

    int mPGId;
    std::string mName;  // For debug
    ia_uid mTerminalBaseUid;
    int32_t mStreamId;
    int mPGCount;
    ia_p2p_platform_t mPlatform;
    int mProgramCount;
    int mTerminalCount;
    int mManifestSize;
    ia_css_kernel_bitmap_t mKernelBitmap;
    std::unique_ptr<ia_css_rbm_t> mRoutingBitmap;
    int mFragmentCount;
    std::unique_ptr<uint8_t[]> mPgTerminals; // save terminal num in PG for each terminal
    std::unique_ptr<ia_css_frame_format_type[]> mFrameFormatType;
    std::vector<int> mDisableDataTermials;

    ia_binary_data __attribute__ ((aligned (PG_PAGE_SIZE))) mParamPayload[IPU_MAX_TERMINAL_COUNT];

    CIPR::Buffer* mPGBuffer = nullptr;
    ia_css_process_group_t* mProcessGroup;
    CIPR::Command* mCmd = nullptr;
    CIPR::Buffer* mCmdExtBuffer = nullptr;

    bool mPPG;
    bool mPPGStarted;
    CIPR::Buffer* mPPGBuffer = nullptr;
    ia_css_process_group_t* mPPGProcessGroup;
    CIPR::Command* mPPGCmd[PPG_CMD_TYPE_COUNT] = {nullptr, };
    CIPR::Buffer* mPPGCmdExtBuffer[PPG_CMD_TYPE_COUNT] = {nullptr, };
    uint64_t mToken;

    CIPR::PSysCommandConfig mCmdCfg;
    CIPR::Event* mEvent = nullptr;

    CIPR::Buffer** mTerminalBuffers;;
    std::map<int, FrameInfo> mTerminalFrameInfos; // valid for data terminals only
    int mInputMainTerminal;
    int mOutputMainTerminal;

    std::vector<CiprBufferMapping> mBuffers;

    TerminalPair mTnrTerminalPair;
    std::vector<uint8_t*> mTnrDataBuffers;

    std::shared_ptr<ShareReferBufferPool> mShareReferPool;
    int64_t mShareReferIds[IPU_MAX_TERMINAL_COUNT];  // 0 is invalid id

    std::vector<TerminalPair> mDvsTerminalPairs;
    std::vector<TerminalPair> mTnrSimTerminalPairs;
};

} //namespace icamera
