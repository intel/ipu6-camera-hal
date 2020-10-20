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

#define LOG_TAG "SyncManager"

#include <sys/sysinfo.h>
#include <math.h>
#include "iutils/CameraLog.h"
#include "SyncManager.h"

namespace icamera {
SyncManager *SyncManager::sInstance = nullptr;
Mutex  SyncManager::sLock;

#define SEC_TO_MS(sec) ((sec) * (1000))
#define USEC_TO_MS(usec) ((usec) / (1000))

const int max_vc_sync_count = 128;

SyncManager* SyncManager::getInstance()
{
    AutoMutex lock(sLock);
    if (sInstance == nullptr) {
        sInstance = new SyncManager();
    }

    return sInstance;
}

void SyncManager::releaseInstance()
{
    AutoMutex lock(sLock);
    LOG1("@%s", __func__);

    if (sInstance) {
        delete sInstance;
        sInstance = nullptr;
    }
}

SyncManager::SyncManager()
{
    LOG1("@%s", __func__);
    AutoMutex lock(mLock);
    for (int i = 0; i < MAX_CAMERA_NUMBER; i++) {
       for (int j = 0; j < MAX_BUFFER_COUNT; j++) {
           mCameraBufInfo[i][j].sequence = -1;
           CLEAR(mCameraBufInfo[i][j].sof_ts);
       }
    }

    mTotalSyncCamNum = 0;
    for (int i = 0; i < MAX_CAMERA_NUMBER; i++)
        mVcSyncCount[i] = 0;
}

SyncManager::~SyncManager()
{
    LOG1("@%s", __func__);
}

bool SyncManager::isSynced(int cameraId, long sequence)
{
    LOG1("@%s", __func__);
    const int TIME_DIFF_MS = 2;
    bool sync = true;
    int index = sequence % MAX_BUFFER_COUNT;

    AutoMutex lock(mLock);
    camera_buf_info bufInfo = mCameraBufInfo[cameraId][index];

    int  syncNum = 0;
    bool isSync[MAX_CAMERA_NUMBER];
    long frameSyncedMs[MAX_CAMERA_NUMBER];
    long curFrameMs = USEC_TO_MS(bufInfo.sof_ts.tv_usec) + SEC_TO_MS(bufInfo.sof_ts.tv_sec);

    //first step: To check whether the current frame is synced with others camera channel
    //            if timestamp difference <= 2ms, then think the frame is synced
    for (int i = 0; i < MAX_CAMERA_NUMBER; i++) {
        isSync[i] = false;
        frameSyncedMs[i] = 0;
        if (mCameraBufInfo[i][0].sequence == -1 || i == cameraId) {
            continue;
        }
        for (int j = 0; j < MAX_BUFFER_COUNT; j++) {
            if (mCameraBufInfo[i][j].sequence >= 0) {
                camera_buf_info &temp = mCameraBufInfo[i][j];
                long tempFrameMs = USEC_TO_MS(temp.sof_ts.tv_usec) + SEC_TO_MS(temp.sof_ts.tv_sec);
                if (abs(tempFrameMs - curFrameMs ) <= TIME_DIFF_MS) {
                    isSync[syncNum] = true;
                    frameSyncedMs[syncNum] = tempFrameMs;
                    syncNum++;
                    break;
                }
            }
        }
    }
    //second step: if current frame is synced with frames from other cameraID,
    //             to check whether other 3 channel frames synced or not
    if (syncNum >= mTotalSyncCamNum - 1) {
        for (int i = 0; i < mTotalSyncCamNum - 1; i++) {
            if (isSync[i]) {
                if ((i + 1 < mTotalSyncCamNum - 1) &&
                        abs(frameSyncedMs[i]-frameSyncedMs[i+1]) <= TIME_DIFF_MS) {
                    sync &= true;
                } else if ((i + 1 == mTotalSyncCamNum - 1) &&
                        abs(frameSyncedMs[i]-frameSyncedMs[0]) <= TIME_DIFF_MS) {
                    sync &= true;
                } else {
                    sync &= false;
                }
            }
        }
    } else {
        sync = false;
    }
    LOG1("Id:%d, sof_ts:%ldms, sequence:%ld sync %d", cameraId, curFrameMs, sequence, sync);
    return sync;
}

void SyncManager::updateCameraBufInfo(int cameraId, camera_buf_info* info)
{
    LOG1("@%s", __func__);
    int index = info->sequence % MAX_BUFFER_COUNT;
    AutoMutex lock(mLock);
    mCameraBufInfo[cameraId][index] = *info;
}

void SyncManager::updateSyncCamNum()
{
    AutoMutex l(mLock);
    CheckError(mTotalSyncCamNum >= MAX_CAMERA_NUMBER, VOID_VALUE,
               "%s: sync cameras enough!", __func__);
    mTotalSyncCamNum++;
}

bool SyncManager::vcSynced(int vc)
{
    CheckError(vc >= MAX_CAMERA_NUMBER, false, "%s: vc %d error!", __func__, vc);

    AutoMutex l(mVcSyncLock);
    int count = mVcSyncCount[vc];
    int minCount = INT_MAX;
    int maxCount = 0;

    for (int i = 0; i < mTotalSyncCamNum; i++) {
        minCount = std::min(minCount, mVcSyncCount[i]);
        maxCount = std::max(maxCount, mVcSyncCount[i]);
    }

    // Check again if status is circling back to 0.
    // Most of time handling code won't be executed because condition is false.
    if (maxCount - minCount > max_vc_sync_count / 2) {
        minCount = max_vc_sync_count;
        maxCount = 0;
        for (int i = 0; i < mTotalSyncCamNum; i++) {
            count = (mVcSyncCount[i] + max_vc_sync_count) % (max_vc_sync_count + max_vc_sync_count / 4);
            minCount = std::min(minCount, count);
            maxCount = std::max(maxCount, count);
        }
        count = (mVcSyncCount[vc] + max_vc_sync_count) % (max_vc_sync_count + max_vc_sync_count / 4);
    }

    if (count > minCount) {
        LOGVCSYNC("vc %d ready: false", vc);
        return false;
    } else
        return true;
};

void SyncManager::updateVcSyncCount(int vc)
{
    CheckError(vc >= MAX_CAMERA_NUMBER, VOID_VALUE, "%s: vc %d error!", __func__, vc);
    AutoMutex l(mVcSyncLock);
    mVcSyncCount[vc] = (mVcSyncCount[vc] + 1) % (max_vc_sync_count + 1);
};

void SyncManager::printVcSyncCount(void)
{
    AutoMutex l(mVcSyncLock);
    for (int i = 0; i < mTotalSyncCamNum; i++)
        LOGVCSYNC("[%d]", mVcSyncCount[i]);
}

} // namespace icamera
