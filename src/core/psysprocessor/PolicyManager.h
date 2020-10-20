/*
 * Copyright (C) 2017-2018 Intel Corporation
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

#include <map>
#include <vector>
#include <string>

#include "iutils/Utils.h"
#include "iutils/Thread.h"

namespace icamera {

class PolicyManager {
public:
    PolicyManager(int cameraId);
    ~PolicyManager();

    /**
     * Create a bundle for the given set of executors, and add the bundle into mBundles.
     * These executors are guaranteed running at the same time.
     */
    int addExecutorBundle(const std::vector<std::string>& executors, const std::vector<int>& depths);

    void setActive(bool isActive);

    /**
     * Check whether the given executor can run or not.
     * If the executor cannot run then it'll wait for other executors in the same bundle.
     * Once all executors are ready to run, then a broadcast will be sent out to wake all
     * executors up and then run together.
     */
    int wait(std::string executorName);

private:
    DISALLOW_COPY_AND_ASSIGN(PolicyManager);

    void releaseBundles();

private:
    struct ExecutorData {
        ExecutorData(int depth = 0) : mRunCount(0), mDepth(depth) {}
        long mRunCount; // How many times the executor has run.
        int mDepth;     // Indicates how many direct dependencies the executor has.
    };

    struct ExecutorBundle {
        std::map<std::string, ExecutorData> mExecutorData; // The index of the map is executor name.
        int mMaxDepth;     // The max depth among all executors.
        int mExecutorNum;  // Indicates how many executors the bundle has.
        int mWaitingCount; // How many executors have already waited.
        bool mIsActive;
        //Guard for the Bundle data
        Mutex mLock;
        Condition mCondition;
    };

    int mCameraId;
    //Guard for the PolicyManager public API
    Mutex mPolicyLock;
    std::vector<ExecutorBundle*> mBundles;
    bool mIsActive;
};

}
