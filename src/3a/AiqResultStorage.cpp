/*
 * Copyright (C) 2016-2020 Intel Corporation.
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

#define LOG_TAG "AiqResultStorage"

#include "AiqResultStorage.h"

#include "iutils/CameraLog.h"

namespace icamera {

std::map<int, AiqResultStorage*> AiqResultStorage::sInstances;
Mutex AiqResultStorage::sLock;

AiqResultStorage* AiqResultStorage::getInstance(int cameraId)
{
    AutoMutex lock(sLock);
    return getInstanceLocked(cameraId);
}

void AiqResultStorage::releaseAiqResultStorage(int cameraId)
{
    AutoMutex lock(sLock);
    AiqResultStorage* storage = getInstanceLocked(cameraId);
    sInstances.erase(cameraId);
    delete storage;
}

AiqResultStorage::AiqResultStorage(int cameraId) :
    mCameraId(cameraId)
{
    LOG1("AiqResultStorage created for id:%d", mCameraId);

    for (int i = 0; i < kStorageSize; i++) {
        mAiqResults[i] = new AiqResult(mCameraId);
        mAiqResults[i]->init();
    }
    // INTEL_DVS_S
    for (int i = 0; i < kDvsStorageSize; i++) {
        mDvsResults[i] = new DvsResult();
    }
    // INTEL_DVS_E
    // LOCAL_TONEMAP_S
    for (int i = 0; i < kLtmStorageSize; i++) {
        mLtmResults[i] = new ltm_result_t;
    }
    // LOCAL_TONEMAP_E
}

AiqResultStorage::~AiqResultStorage()
{
    LOG1("AiqResultStorage released for id:%d", mCameraId);

    for (int i = 0; i < kStorageSize; i++) {
        delete mAiqResults[i];
    }
    // INTEL_DVS_S
    for (int i = 0; i < kDvsStorageSize; i++) {
        delete mDvsResults[i];
    }
    // INTEL_DVS_E
    // LOCAL_TONEMAP_S
    for (int i = 0; i < kLtmStorageSize; i++) {
        delete mLtmResults[i];
    }
    // LOCAL_TONEMAP_E
}

// LOCAL_TONEMAP_S
ltm_result_t* AiqResultStorage::acquireLtmResult()
{
    AutoWMutex rlock(mDataLock);

    int index = mCurrentLtmIndex + 1;
    index %= kLtmStorageSize;

    mLtmResults[index]->sequence = -1;

    return mLtmResults[index];
}

void AiqResultStorage::updateLtmResult(long sequence)
{
    AutoWMutex wlock(mDataLock);

    mCurrentLtmIndex++;
    mCurrentLtmIndex %= kLtmStorageSize;

    mLtmResults[mCurrentLtmIndex]->sequence = sequence;
}

const ltm_result_t* AiqResultStorage::getLtmResult(long sequence)
{
    AutoRMutex rlock(mDataLock);

    if (mCurrentLtmIndex == -1)
        return nullptr;

    // Sequence is -1 means to get the latest result
    if (sequence == -1) {
        return mLtmResults[mCurrentLtmIndex];
    }

    // Try to find the matched result
    for (int i = 0; i < kLtmStorageSize; i++) {
        int tmpIdx = (mCurrentLtmIndex + kLtmStorageSize - i) % kLtmStorageSize;
        if (mLtmResults[tmpIdx]->sequence >= 0 && sequence >= mLtmResults[tmpIdx]->sequence) {
            LOG2("%s, find the ltm result (expect: %ld actual: %ld)",
                    __func__, sequence, mLtmResults[tmpIdx]->sequence);
            return mLtmResults[tmpIdx];
        }
    }

    return nullptr;
}
// LOCAL_TONEMAP_E

// INTEL_DVS_S
DvsResult* AiqResultStorage::acquireDvsResult()
{
    AutoWMutex rlock(mDataLock);

    int index = mCurrentDvsIndex + 1;
    index %= kDvsStorageSize;

    mDvsResults[index]->mSequence = -1;

    return mDvsResults[index];
}

void AiqResultStorage::updateDvsResult(long sequence)
{
    AutoWMutex wlock(mDataLock);

    mCurrentDvsIndex++;
    mCurrentDvsIndex %= kDvsStorageSize;

    mDvsResults[mCurrentDvsIndex]->mSequence = sequence;
}

const DvsResult* AiqResultStorage::getDvsResult(long sequence)
{
    AutoRMutex rlock(mDataLock);

    if (mCurrentDvsIndex == -1)
        return nullptr;

    CheckError(mDvsResults[mCurrentDvsIndex]->mSequence == -1, nullptr, "invalid sequence id -1");

    if (sequence == -1)
        return mDvsResults[mCurrentDvsIndex];

    // Try to find the matched result
    for (int i = 0; i < kDvsStorageSize; i++) {
        int tmpIdx = (mCurrentDvsIndex + kDvsStorageSize - i) % kDvsStorageSize;
        if (mDvsResults[tmpIdx]->mSequence >= 0 && sequence >= mDvsResults[tmpIdx]->mSequence) {
            LOG2("%s, find the DVS result (expect: %ld actual: %ld)",
                    __func__, sequence, mDvsResults[tmpIdx]->mSequence);
            return mDvsResults[tmpIdx];
        }
    }

    return nullptr;
}
// INTEL_DVS_E

AiqStatistics* AiqResultStorage::acquireAiqStatistics()
{
    AutoWMutex rlock(mDataLock);

    int index = (mCurrentAiqStatsIndex + 1) % kAiqStatsStorageSize;
    if (mAiqStatistics[index].mInUse) {
        // The "next" storage is still in use, skip it.
        mCurrentAiqStatsIndex = index;
        index = (mCurrentAiqStatsIndex + 1) % kAiqStatsStorageSize;
    }

    mAiqStatistics[index].mSequence = -1;

    return &mAiqStatistics[index];
}

void AiqResultStorage::updateAiqStatistics(long sequence)
{
    AutoWMutex wlock(mDataLock);

    mCurrentAiqStatsIndex++;
    mCurrentAiqStatsIndex %= kAiqStatsStorageSize;

    mAiqStatistics[mCurrentAiqStatsIndex].mSequence = sequence;
}

const AiqStatistics* AiqResultStorage::getAndLockAiqStatistics()
{
    AutoRMutex rlock(mDataLock);

    if (mCurrentAiqStatsIndex == -1)
        return nullptr;

    CheckError(mAiqStatistics[mCurrentAiqStatsIndex].mSequence == -1,
          nullptr, "Invalid sequence id -1 of stored aiq statistics");

    mAiqStatistics[mCurrentAiqStatsIndex].mInUse = true;
    return &mAiqStatistics[mCurrentAiqStatsIndex];
}

void AiqResultStorage::unLockAiqStatistics()
{
    AutoRMutex rlock(mDataLock);
    for (int i = 0; i < kAiqStatsStorageSize; i++) {
        mAiqStatistics[i].mInUse = false;
    }
}

AiqResult* AiqResultStorage::acquireAiqResult()
{
    AutoWMutex rlock(mDataLock);

    int index = mCurrentIndex + 1;
    index %= kStorageSize;

    mAiqResults[index]->mSequence = -1;

    return mAiqResults[index];
}

void AiqResultStorage::updateAiqResult(long sequence)
{
    AutoWMutex wlock(mDataLock);

    mCurrentIndex++;
    mCurrentIndex %= kStorageSize;

    mAiqResults[mCurrentIndex]->mSequence = sequence;
}

const AiqResult* AiqResultStorage::getAiqResult(long sequence)
{
    AutoRMutex rlock(mDataLock);

    // Sequence id is -1 means user wants get the latest result.
    if (sequence == -1) {
        // If mCurrentIndex is -1, that means no result is saved to the storage yet,
        // just return the first one in this case.
        return mAiqResults[(mCurrentIndex == -1) ? 0 : mCurrentIndex];
    }

    for (int i = 0; i < kStorageSize; i++) {
        // Search from the newest result
        int tmpIdx = (mCurrentIndex + kStorageSize - i) % kStorageSize;
        if (mAiqResults[tmpIdx]->mSequence >= 0 && sequence >= mAiqResults[tmpIdx]->mSequence) {
            return mAiqResults[tmpIdx];
        }
    }

    return nullptr;
}

/**
 * Private function with no lock in it, must be called with lock protection
 */
AiqResultStorage* AiqResultStorage::getInstanceLocked(int cameraId)
{
    if (sInstances.find(cameraId) != sInstances.end()) {
        return sInstances[cameraId];
    }

    sInstances[cameraId] = new AiqResultStorage(cameraId);
    return sInstances[cameraId];
}

} //namespace icamera

