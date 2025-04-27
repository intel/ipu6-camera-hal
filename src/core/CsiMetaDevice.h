/*
 * Copyright (C) 2016-2021 Intel Corporation.
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

#ifdef HAVE_CHROME_OS
#include <cros-camera/v4l2_device.h>
#else
#include <v4l2_device.h>
#endif

#include <atomic>
#include <vector>

#include "CameraBuffer.h"
#include "CameraEvent.h"
#include "iutils/Errors.h"
#include "iutils/Thread.h"

namespace icamera {

/**
 * \struct EmbeddedMetaData: sensor and csi meta device info
 *
 * EmbeddedMetaData is used to carray embedded meta data info.
 */
struct EmbeddedMetaData {
    int csiMetaFormat;
    int width;
    int height;
    int bpl;
    int planesNum;
    int size[VIDEO_MAX_PLANES];
};

/**
 * \class CsiMetaDevice
 *
 * \brief
 *  Manage CSI meta device, and provide sensor metadata for other class.
 *
 * This class is used to control CSI meta device, and grabs embedded sensor
 * metadata and supplies sensor metadata for other class.
 *
 */
class CsiMetaDevice : public EventSource {
 public:
    explicit CsiMetaDevice(int cameraId);
    ~CsiMetaDevice();

    int init();
    void deinit();
    int configure();
    int start();
    int stop();
    bool isEnabled() { return mIsCsiMetaEnabled; }

    void deinitLocked();
    int resetState();
    int initDev();
    void deinitDev();
    int initEmdMetaData();
    int poll();
    int hasBufferIndevice();
    void handleCsiMetaBuffer();
    int setFormat();
    int allocCsiMetaBuffers();
    int queueCsiMetaBuffer(const std::shared_ptr<CameraBuffer>& camBuffer);
    // DOL_FEATURE_S
    int decodeMetaData(EventDataMeta& metaData, const std::shared_ptr<CameraBuffer>& camBuffer);
    // DOL_FEATURE_E

 private:
    static const int CSI_META_BUFFER_NUM = 10;

    class PollThread : public Thread {
        CsiMetaDevice* mCsiMetaDevice;

     public:
        explicit PollThread(CsiMetaDevice* csiMetaDevice) : mCsiMetaDevice(csiMetaDevice) {}
        virtual ~PollThread() {}
        virtual bool threadLoop() {
            int ret = mCsiMetaDevice->poll();
            return ret == OK;
        }
    };

    PollThread* mPollThread;
    int mCameraId;
    V4L2VideoNode* mCsiMetaDevice;
    std::vector<V4L2VideoNode*> mConfiguredDevices;
    EmbeddedMetaData mEmbeddedMetaData;

    // Guard for CsiMetaDevice public API
    Mutex mCsiMetaDeviceLock;

    bool mIsCsiMetaEnabled;

    Mutex mCsiMetaBuffersLock;
    int mCsiMetaBufferDQIndex;
    CameraBufVector mCsiMetaCameraBuffers;
    std::atomic<int> mBuffersInCsiMetaDevice;

    enum {
        CSI_META_DEVICE_UNINIT,
        CSI_META_DEVICE_INIT,
        CSI_META_DEVICE_CONFIGURED,
        CSI_META_DEVICE_START,
        CSI_META_DEVICE_STOP,
    } mState;

    bool mExitPending;

 private:
    DISALLOW_COPY_AND_ASSIGN(CsiMetaDevice);
};

} /* namespace icamera */
