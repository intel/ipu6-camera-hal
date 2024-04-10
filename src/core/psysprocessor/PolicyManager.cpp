/*
 * Copyright (C) 2017-2024 Intel Corporation
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
#define LOG_TAG Camera_PolicyManager

#include "PolicyManager.h"

#include "AiqResultStorage.h"
#include "iutils/Errors.h"
#include "iutils/CameraLog.h"

namespace icamera {

PolicyManager::PolicyManager(int cameraId) : mCameraId(cameraId), mIsActive(false) {
    LOG1("@%s: camera id:%d", __func__, mCameraId);
}

PolicyManager::~PolicyManager() {
    LOG1("@%s: camera id:%d", __func__, mCameraId);

    releaseBundles();
}

void PolicyManager::releaseBundles() {
    LOG1("@%s: camera id:%d", __func__, mCameraId);

    for (const auto& bundle : mBundles) {
        delete bundle;
    }

    mBundles.clear();
}

void PolicyManager::setActive(bool isActive) {
    AutoMutex lock(mPolicyLock);

    LOG1("@%s: camera id:%d update active mode from %d to %d", __func__, mCameraId, mIsActive,
         isActive);

    if (mIsActive == isActive) return;  // No action is needed if the mode unchanged.

    for (auto& bundle : mBundles) {
        AutoMutex lock(bundle->mLock);

        bundle->mWaitingCount = 0;
        bundle->mIsActive = isActive;
        for (auto& executorData : bundle->mExecutorData) {
            executorData.second.mRunCount = 0;
        }

        // Wake up the executors who are waiting for other executors.
        if (!bundle->mIsActive) {
            bundle->mCondition.broadcast();
        }
    }

    mIsActive = isActive;
}

int PolicyManager::addExecutorBundle(const std::vector<std::string>& executors,
                                     const std::vector<int>& depths, int64_t startSequence) {
    LOG1("@%s: camera id:%d", __func__, mCameraId);

    AutoMutex lock(mPolicyLock);

    uint8_t size = executors.size();
    CheckAndLogError(size != depths.size(), BAD_VALUE,
                     "The size for executor and its depth not match");

    int maxDepth = 0;
    std::map<std::string, ExecutorData> executorData;

    for (uint8_t i = 0; i < size; i++) {
        executorData[executors[i]] = ExecutorData(depths[i]);
        if (depths[i] > maxDepth) {
            maxDepth = depths[i];
        }
        LOG1("%s, bundled executor name:%s, depth:%d)", __func__, executors[i].c_str(), depths[i]);
    }

    ExecutorBundle* bundle = new ExecutorBundle();
    bundle->mExecutorData = executorData;
    bundle->mExecutorNum = size;
    bundle->mMaxDepth = maxDepth;
    bundle->mWaitingCount = 0;
    bundle->mStartSequence = startSequence;
    bundle->mIsActive = true;

    mBundles.push_back(bundle);

    return OK;
}

int PolicyManager::wait(std::string executorName, int64_t sequence) {
    ExecutorBundle* bundle = nullptr;
    {
        AutoMutex lock(mPolicyLock);

        // No need to wait when it's already inactive.
        if (!mIsActive) return OK;

        for (const auto& item : mBundles) {
            if (item->mExecutorData.find(executorName) != item->mExecutorData.end()) {
                bundle = item;
                break;
            }
        }
        // If the executor not in mBundles, it means it doesn't need to wait for others.
        if (bundle == nullptr) return OK;
    }

    ConditionLock lock(bundle->mLock);

    // If it's already inactive, there is no need to align the executors anymore.
    if (!bundle->mIsActive) return OK;
    // start to sync when frame sequence exceed the setting sequence
    if (sequence <= bundle->mStartSequence) return OK;

    ExecutorData& executorData = bundle->mExecutorData[executorName];
    executorData.mRunCount++;

    /**
     * If an executor's run count plus its depth less than the max depth of all executors,
     * it means the executor can run without checking other executors' status, since other
     * may wait on this executor's output to reach the precondition of running together.
     */
    if (executorData.mRunCount + executorData.mDepth <= bundle->mMaxDepth) {
        return OK;
    }

    bundle->mWaitingCount++;

    int64_t waitDuration = 66000000;  // 66ms
    const AiqResult* aiqResult = AiqResultStorage::getInstance(mCameraId)->getAiqResult(sequence);
    if (aiqResult && aiqResult->mAiqParam.aeFpsRange.min >= 30.0) {
        waitDuration = 33000000;  // 33ms
    }

    /**
     * If waiting count less than total executor number in the bundle, it means
     * we need to wait for other executors to run with them together.
     */
    if (bundle->mWaitingCount < bundle->mExecutorNum) {
        LOG2("%s: need wait for other executors.", executorName.c_str());
        int ret = bundle->mCondition.waitRelative(lock, waitDuration * SLOWLY_MULTIPLIER);
        if (ret == TIMED_OUT) {
            LOG2("%s: wait executors timeout", executorName.c_str());
            return ret;
        }
    } else {
        bundle->mWaitingCount = 0;
        bundle->mCondition.broadcast();
    }

    return OK;
}

}  // end of namespace icamera
