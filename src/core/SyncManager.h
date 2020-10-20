/*
 * Copyright (C) 2018 Intel Corporation.
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

#include "PlatformData.h"

namespace icamera {

struct camera_buf_info {
    long sequence;
    struct timeval sof_ts;
};
class SyncManager {
private:
    //Prevent to create multiple instances
    SyncManager();
    ~SyncManager();
public:
     /**
      * releaseInstance
      * This function must be called when the hal is destroyed.
      */
    static void releaseInstance();
    static SyncManager* getInstance();

    bool isSynced(int cameraId, long sequence);
    void updateCameraBufInfo(int cameraId, camera_buf_info* info);

    void updateSyncCamNum();

    bool vcSynced(int vc);
    void updateVcSyncCount(int vc);
    void printVcSyncCount();
private:
    static SyncManager* sInstance;
    static Mutex sLock;
    Mutex mLock;
    struct camera_buf_info mCameraBufInfo[MAX_CAMERA_NUMBER][MAX_BUFFER_COUNT];

    int mVcSyncCount[MAX_CAMERA_NUMBER];
    Mutex mVcSyncLock;
    int mTotalSyncCamNum;
};
} /* namespace icamera */
