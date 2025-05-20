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

#include <pthread.h>

#include <memory>
#include <unordered_map>
extern "C" {
#include "ia_pal_types_isp_parameters_autogen.h"
}

#include "CameraBuffer.h"
#include "PlatformData.h"
#include "TNRCommon.h"
#include "iutils/Thread.h"

#ifdef TNR7_CM
#define HANDLE cancel_fw_pre_define
/* the cm_rt.h has some build error with current clang build flags
 * use the ignored setting to ignore these errors, and use
 * push/pop to make the ignore only take effect on this file */
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wbitfield-constant-conversion"
#pragma clang diagnostic ignored "-Wunused-private-field"
// HANDLE is redefined in cm_rt.h, avoid the redefinition build error
#include "cm_rt.h"
#pragma clang diagnostic pop

extern int run_tnr7us_frame(int width, int height, int stride, CmSurface2DUP*& inputSurface,
                            CmSurface2DUP*& outputSurface, tnr_scale_1_0_t* dsPtr,
                            tnr7_ims_1_0_t* imsPtr, tnr7_bc_1_0_t* bcPtr,
                            tnr7_blend_1_0_t* blendPtr, bool updateParam = false, int type = 0);

extern int32_t createCmSurface2DUP(uint32_t width, uint32_t height, CM_SURFACE_FORMAT format,
                                   void* sysMem, CmSurface2DUP*& surface);

extern int32_t destroyCMSurface2DUP(CmSurface2DUP*& surface);
// update parameters when gain changes
extern int tnr7usParamUpdate(int gain, bool forceUpdate = false, int type = 0);
extern int32_t getSurface2DInfo(uint32_t width, uint32_t height, CM_SURFACE_FORMAT format,
                                uint32_t& pitch, uint32_t& physicalSize);
#endif
namespace icamera {

class IntelTNR7US {
 public:
    static IntelTNR7US* createIntelTNR(int cameraId);
    virtual ~IntelTNR7US() {}
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
                            uint32_t outBufSize, Tnr7Param* tnrParam, bool syncUpdate,
                            int fd = -1) = 0;
    Tnr7Param* allocTnr7ParamBuf();
    virtual void* allocCamBuf(uint32_t bufSize, int id) = 0;
    virtual void freeAllBufs() = 0;
    virtual int prepareSurface(void* bufAddr, int size) { return OK; }
    virtual int asyncParamUpdate(int gain, bool forceUpdate) { return OK; }
    virtual int getTnrBufferSize(int width, int height, uint32_t* size) { return BAD_VALUE; }

 protected:
    explicit IntelTNR7US(int cameraId)
            : mCameraId(cameraId),
              mWidth(0),
              mHeight(0),
              mTnrType(TNR_INSTANCE_MAX),
              mTnrParam(nullptr){};

 protected:
    int mCameraId;
    int mWidth;
    int mHeight;
    TnrType mTnrType;
    Tnr7Param* mTnrParam;

    DISALLOW_COPY_AND_ASSIGN(IntelTNR7US);
};

#ifdef TNR7_CM
class IntelC4mTNR : public IntelTNR7US, public icamera::Thread {
 public:
    explicit IntelC4mTNR(int cameraId) : IntelTNR7US(cameraId) {}
    virtual ~IntelC4mTNR();
    virtual int init(int width, int height, TnrType type = TNR_INSTANCE0) override;
    /**
     * call tnr api to calc tnr result
     *
     * \param inBufAddr: input image buffer
     * \param outBufAddr: tnr output
     * \param tnrParam: tnr parameters from ISP
     * \param fd: user output buffer file handle
     */
    virtual int runTnrFrame(const void* inBufAddr, void* outBufAddr, uint32_t inBufSize,
                            uint32_t outBufSize, Tnr7Param* tnrParam, bool syncUpdate,
                            int fd = -1) override;
    virtual void* allocCamBuf(uint32_t bufSize, int id) override;
    virtual void freeAllBufs() override;
    virtual int prepareSurface(void* bufAddr, int size) override;
    virtual int asyncParamUpdate(int gain, bool forceUpdate) override;
    virtual int getTnrBufferSize(int width, int height, uint32_t* size) override;

 private:
    /* tnr api use CmSurface2DUP object as data buffer, call this api to create
     * CmSurface2DUP object from user data buffer */
    CmSurface2DUP* createCMSurface(void* bufAddr);
    int32_t destroyCMSurface(CmSurface2DUP* surface);
    // get the CmSurface object of the bufAddr in mCMSurfaceMap
    CmSurface2DUP* getBufferCMSurface(void* bufAddr);
    DISALLOW_COPY_AND_ASSIGN(IntelC4mTNR);

 private:
    // Tnr will create CMSurface for input buffers and cache them in the map
    std::unordered_map<void*, CmSurface2DUP*> mCMSurfaceMap;
    virtual bool threadLoop() override;
    std::mutex mLock;
    std::condition_variable mRequestCondition;
    const uint64_t kMaxDuration = 5000000000;  // 5s
    typedef struct {
        int gain;
        bool forceUpdate;
    } ParamGain;
    std::queue<ParamGain> mParamGainRequest;
    bool mThreadRunning;
};
#endif
}  // namespace icamera
