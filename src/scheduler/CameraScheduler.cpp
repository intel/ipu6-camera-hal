/*
 * Copyright (C) 2022-2023 Intel Corporation
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

#define LOG_TAG Scheduler

#include "src/scheduler/CameraScheduler.h"

#include <map>
#include <sstream>
#include <utility>

#include "iutils/CameraLog.h"
#include "iutils/Errors.h"

namespace icamera {

CameraScheduler::CameraScheduler() : mTriggerCount(0) {
    mPolicy = CameraSchedulerPolicy::getInstance();
}

CameraScheduler::~CameraScheduler() {
    destoryExecutors();
}

int32_t CameraScheduler::configurate(const std::set<int32_t>& graphIds) {
    int ret = mPolicy->setConfig(graphIds);
    CheckAndLogError(ret != OK, ret, "configurate error");

    mTriggerCount = 0;
    destoryExecutors();
    return createExecutors();
}

int32_t CameraScheduler::createExecutors() {
    std::map<const char*, const char*> executors;
    int32_t exeNumber = mPolicy->getExecutors(&executors);
    CheckAndLogError(exeNumber <= 0, UNKNOWN_ERROR, "Can't get Executors' names");

    std::lock_guard<std::mutex> l(mLock);
    for (auto& exe : executors) {
        ExecutorGroup group;
        group.executor = std::shared_ptr<Executor>(new Executor(exe.first));
        group.triggerSource = exe.second;
        if (!group.triggerSource.empty()) {
            // Check if trigger source is one executor
            std::shared_ptr<Executor> source = findExecutor(group.triggerSource.c_str());
            if (source) source->addListener(group.executor);
        }
        mPolicy->getNodeList(exe.first, &group.nodeList);

        mExeGroups.push_back(group);
        group.executor->run(exe.first, PRIORITY_NORMAL);
    }
    return OK;
}

void CameraScheduler::destoryExecutors() {
    std::lock_guard<std::mutex> l(mLock);
    mRegisteredNodes.clear();
    mExeGroups.clear();
}

int32_t CameraScheduler::registerNode(ISchedulerNode* node) {
    std::lock_guard<std::mutex> l(mLock);

    ExecutorGroup* group = nullptr;
    for (size_t i = 0; i < mExeGroups.size(); i++) {
        for (auto& nodeName : mExeGroups[i].nodeList) {
            if (strcmp(nodeName.c_str(), node->getName()) == 0) {
                group = &mExeGroups[i];
                break;
            }
        }
    }
    CheckWarning(!group, BAD_VALUE, "register node %s fail", node->getName());

    group->executor->addNode(node);
    mRegisteredNodes[node] = group;
    return OK;
}

void CameraScheduler::unregisterNode(ISchedulerNode* node) {
    std::lock_guard<std::mutex> l(mLock);
    if (mRegisteredNodes.find(node) != mRegisteredNodes.end()) {
        mRegisteredNodes[node]->executor->removeNode(node);
        mRegisteredNodes.erase(node);
    }
}

int32_t CameraScheduler::executeNode(std::string triggerSource, int64_t triggerId) {
    mTriggerCount++;
    for (auto& group : mExeGroups) {
        if (group.triggerSource == triggerSource)
            group.executor->trigger(triggerId < 0 ? mTriggerCount : triggerId);
    }
    return OK;
}

std::shared_ptr<CameraScheduler::Executor> CameraScheduler::findExecutor(const char* exeName) {
    if (!exeName) return nullptr;

    for (auto& group : mExeGroups) {
        if (strcmp(group.executor->getName(), exeName) == 0) return group.executor;
    }

    return nullptr;
}

CameraScheduler::Executor::Executor(const char* name)
        : mName(name ? name : "unknown"),
          mActive(false),
          mTriggerTick(0) {}

CameraScheduler::Executor::~Executor() {
    LOG1("%s: destory", getName());
    requestExit();
}

void CameraScheduler::Executor::addNode(ISchedulerNode* node) {
    std::lock_guard<std::mutex> l(mNodeLock);
    mNodes.push_back(node);
    LOG1("%s: %s added to %s, pos %lu", __func__, node->getName(), getName(), mNodes.size());
}

void CameraScheduler::Executor::removeNode(ISchedulerNode* node) {
    std::lock_guard<std::mutex> l(mNodeLock);
    for (size_t i = 0; i < mNodes.size(); i++) {
        if (mNodes[i] == node) {
            LOG1("%s: %s moved from %s", __func__, node->getName(), getName());
            mNodes.erase(mNodes.begin() + i);
            break;
        }
    }
}

void CameraScheduler::Executor::trigger(int64_t tick) {
    PERF_CAMERA_ATRACE_PARAM1(getName(), tick);
    std::lock_guard<std::mutex> l(mNodeLock);
    mActive = true;
    mTriggerTick = tick;
    mTriggerSignal.signal();
}

void CameraScheduler::Executor::requestExit() {
    LOG2("%s: requestExit", getName());
    mActive = false;
    icamera::Thread::requestExit();
    std::lock_guard<std::mutex> l(mNodeLock);
    mTriggerSignal.signal();
}

bool CameraScheduler::Executor::threadLoop() {
    int64_t tick = -1;
    {
        ConditionLock lock(mNodeLock);
        int ret = mTriggerSignal.waitRelative(lock, kWaitDuration * SLOWLY_MULTIPLIER);
        CheckWarning(ret == TIMED_OUT && !mNodes.empty(), true, "%s: wait trigger time out",
                     getName());
        tick = mTriggerTick;
    }
    if (!mActive) return false;

    for (auto& node : mNodes) {
        LOG2("%s process %ld", getName(), tick);
        bool ret = node->process(tick);
        CheckAndLogError(!ret, true, "%s: node %s process error", getName(), node->getName());
    }

    for (auto& listener : mListeners) {
        LOG2("%s: trigger listener %s", getName(), listener->getName());
        listener->trigger(tick);
    }
    return true;
}

}  // namespace icamera
