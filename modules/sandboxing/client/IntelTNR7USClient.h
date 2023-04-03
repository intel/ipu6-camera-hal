/*
 * Copyright (C) 2020-2023 Intel Corporation
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

#include <vector>

#include "BufferQueue.h"
#include "CameraBuffer.h"
#include "Parameters.h"
#include "PlatformData.h"
#include "TNRCommon.h"
// LEVEL0_ICBM_S
#include "src/icbm/ICBMTypes.h"
#include "src/icbm/ICBMThread.h"
// LEVEL0_ICBM_E

namespace icamera {
class IntelTNR7US {
 public:
    static IntelTNR7US* createIntelTNR(int cameraId);
    virtual ~IntelTNR7US(){};
    virtual int init(int width, int height, TnrType type = TNR_INSTANCE0) = 0;
    /**
     * call tnr api to calc tnr result
     *
     * \param inBufAddr: input image buffer
     * \param outBufAddr: tnr output
     * \param tnrParam: tnr parameters from ISP
     * \param fd: user output buffer file handle
     */
    virtual int runTnrFrame(const void* inBufAddr, void* outBufAddr, uint32_t inBufSize,
                            uint32_t outBufSize, Tnr7Param* tnrParam, bool syncUpdate = true,
                            int fd = -1) = 0;
    virtual Tnr7Param* allocTnr7ParamBuf();
    virtual void* allocCamBuf(uint32_t bufSize, int id);
    virtual void freeAllBufs();
    int prepareSurface(void* bufAddr, int size) { return OK; }
    virtual int asyncParamUpdate(int gain, bool forceUpdate) { return OK; }
    virtual int getTnrBufferSize(int width, int height, uint32_t* size) { return BAD_VALUE; }

 protected:
    int mCameraId;
    IntelAlgoCommon mCommon;
    std::vector<ShmMemInfo> mCamBufMems;
    ShmMemInfo mParamMems;
    explicit IntelTNR7US(int cameraId) : mCameraId(cameraId){};
};

class IntelC4mTNR : public IntelTNR7US {
 public:
    explicit IntelC4mTNR(int cameraId);
    virtual ~IntelC4mTNR();
    virtual int init(int width, int height, TnrType type = TNR_INSTANCE0);
    virtual int runTnrFrame(const void* inBufAddr, void* outBufAddr, uint32_t inBufSize,
                            uint32_t outBufSize, Tnr7Param* tnrParam, bool syncUpdate = true,
                            int fd = -1);
    virtual void* allocCamBuf(uint32_t bufSize, int id);
    virtual int asyncParamUpdate(int gain, bool forceUpdate);
    virtual int getTnrBufferSize(int width, int height, uint32_t* size);

 private:
    TnrType mTnrType;
    TnrRequestInfo* mTnrRequestInfo;
    ShmMemInfo mTnrRequestInfoMem;
    DISALLOW_COPY_AND_ASSIGN(IntelC4mTNR);
};

// LEVEL0_ICBM_S
class IntelLevel0TNR : public IntelTNR7US {
 public:
    explicit IntelLevel0TNR(int cameraId) : IntelTNR7US(cameraId){};
    virtual ~IntelLevel0TNR();
    virtual int init(int width, int height, TnrType type = TNR_INSTANCE0);
    virtual int runTnrFrame(const void* inBufAddr, void* outBufAddr, uint32_t inBufSize,
                            uint32_t outBufSize, Tnr7Param* tnrParam, bool syncUpdate = true,
                            int fd = -1);

 private:
    std::unique_ptr<ICBMThread> mIntelICBM;
    int mWidth;
    int mHeight;

 private:
    DISALLOW_COPY_AND_ASSIGN(IntelLevel0TNR);
};
// LEVEL0_ICBM_E
}  // namespace icamera
