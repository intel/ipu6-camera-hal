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

#define LOG_TAG SchedPolicy

#include "src/scheduler/CameraSchedulerPolicy.h"

#include <map>
#include <utility>
#include <vector>

#include "iutils/CameraLog.h"
#include "iutils/Errors.h"

namespace icamera {

#define SCHEDULER_POLICY_FILE_NAME "pipe_scheduler_profiles.xml"

CameraSchedulerPolicy* CameraSchedulerPolicy::sInstance = nullptr;
Mutex CameraSchedulerPolicy::sLock;

CameraSchedulerPolicy* CameraSchedulerPolicy::getInstance() {
    AutoMutex lock(sLock);
    if (!sInstance) {
        sInstance = new CameraSchedulerPolicy();
    }
    return sInstance;
}

void CameraSchedulerPolicy::releaseInstance() {
    AutoMutex lock(sLock);
    if (sInstance) {
        delete sInstance;
        sInstance = nullptr;
    }
}

CameraSchedulerPolicy::CameraSchedulerPolicy()
        : mCurrentDataField(FIELD_INVALID),
          mCurrentConfig(0),
          mActiveConfig(nullptr) {
    LOG1("%s", __func__);
    getDataFromXmlFile(SCHEDULER_POLICY_FILE_NAME);
    if (!mPolicyConfigs.empty()) mActiveConfig = &mPolicyConfigs.front();
}

CameraSchedulerPolicy::~CameraSchedulerPolicy() {
    LOG1("%s", __func__);
}

int32_t CameraSchedulerPolicy::setConfig(const std::set<int32_t>& graphIds) {
    size_t graphCount = graphIds.size();
    for (auto& item : mPolicyConfigs) {
        if (graphCount != item.graphIds.size()) continue;

        bool match = true;
        for (auto it = graphIds.cbegin(); it != graphIds.cend(); ++it) {
            if (item.graphIds.find(*it) == item.graphIds.end()) {
                match = false;
                break;
            }
        }
        if (match) {
            mActiveConfig = &item;
            return OK;
        }
    }

    LOGE("%s: no config for the following graphs %lu: )", __func__, graphCount);
    for (auto it = graphIds.cbegin(); it != graphIds.cend(); ++it) {
        LOGE("    graph id %d", *it);
    }
    return BAD_VALUE;
}

int32_t CameraSchedulerPolicy::getExecutors(std::map<const char*, const char*>* executors) const {
    CheckAndLogError(!executors, 0, "%s: nullptr", __func__);
    CheckAndLogError(!mActiveConfig, 0, "%s: No config", __func__);

    for (auto& iter : mActiveConfig->exeList) {
        (*executors)[iter.exeName.c_str()] = iter.triggerName.c_str();
    }
    return mActiveConfig->exeList.size();
}

int32_t CameraSchedulerPolicy::getNodeList(const char* exeName,
                                           std::vector<std::string>* nodeList) const {
    CheckAndLogError(!nodeList, BAD_VALUE, "nullptr input");
    CheckAndLogError(!mActiveConfig, BAD_VALUE, "No config");

    for (auto& exe : mActiveConfig->exeList) {
        if (strcmp(exe.exeName.c_str(), exeName) == 0) {
            *nodeList = exe.nodeList;
            return OK;
        }
    }
    return BAD_VALUE;
}

void CameraSchedulerPolicy::checkField(CameraSchedulerPolicy* profiles, const char* name,
                                       const char** atts) {
    LOG1("@%s, name:%s", __func__, name);
    if (strcmp(name, "PipeSchedulerPolicy") == 0) {
        profiles->mCurrentDataField = FIELD_INVALID;
        return;
    } else if (strcmp(name, "scheduler") == 0 &&
               profiles->mCurrentConfig == profiles->mPolicyConfigs.size()) {
        // Start the new one only when the last one is done
        PolicyConfigDesc desc;
        profiles->mPolicyConfigs.push_back(desc);

        int idx = 0;
        while (atts[idx]) {
            const char* key = atts[idx];
            const char* val = atts[idx + 1];
            LOG2("@%s, name:%s, atts[%d]:%s, atts[%d]:%s", __func__, name, idx, key, idx + 1, val);
            if (strcmp(key, "id") == 0) {
                profiles->mPolicyConfigs[profiles->mCurrentConfig].configId = atoi(val);
            } else if (strcmp(key, "graphId") == 0 ||
                       strcmp(key, "video") == 0 || strcmp(key, "still") == 0) {
                profiles->mPolicyConfigs[profiles->mCurrentConfig].graphIds.insert(atoi(val));
            }
            idx += 2;
        }
        profiles->mCurrentDataField = FIELD_SCHED;
        return;
    }

    LOGE("@%s, name:%s, atts[0]:%s, xml format wrong", __func__, name, atts[0]);
    return;
}

void CameraSchedulerPolicy::handleExecutor(CameraSchedulerPolicy* profiles, const char* name,
                                           const char** atts) {
    int idx = 0;
    ExecutorDesc desc;

    while (atts[idx]) {
        const char* key = atts[idx];
        LOG2("%s: name: %s, value: %s", __func__, atts[idx], atts[idx + 1]);
        if (strcmp(key, "name") == 0) {
            desc.exeName = atts[idx + 1];
        } else if (strcmp(key, "nodes") == 0) {
            parseXmlConvertStrings(atts[idx + 1], desc.nodeList, convertCharToString);
        } else if (strcmp(key, "trigger") == 0) {
            desc.triggerName = atts[idx + 1];
        } else {
            LOGW("Invalid policy attribute: %s", key);
        }
        idx += 2;
    }

    LOG2("@%s, name:%s, atts[0]:%s", __func__, name, atts[0]);
    profiles->mPolicyConfigs[profiles->mCurrentConfig].exeList.push_back(desc);
}

void CameraSchedulerPolicy::handlePolicyConfig(CameraSchedulerPolicy* profiles, const char* name,
                                               const char** atts) {
    LOG2("@%s, name:%s, atts[0]:%s", __func__, name, atts[0]);
    if (strcmp(name, "pipe_executor") == 0) {
        handleExecutor(profiles, name, atts);
    }
}

void CameraSchedulerPolicy::startParseElement(void* userData, const char* name, const char** atts) {
    CameraSchedulerPolicy* profiles = reinterpret_cast<CameraSchedulerPolicy*>(userData);

    if (profiles->mCurrentDataField == FIELD_INVALID) {
        profiles->checkField(profiles, name, atts);
        return;
    }

    switch (profiles->mCurrentDataField) {
        case FIELD_SCHED:
            profiles->handlePolicyConfig(profiles, name, atts);
            break;
        default:
            LOGE("@%s, line:%d, go to default handling", __func__, __LINE__);
            break;
    }
}

void CameraSchedulerPolicy::endParseElement(void* userData, const char* name) {
    CameraSchedulerPolicy* profiles = reinterpret_cast<CameraSchedulerPolicy*>(userData);
    if (strcmp(name, "scheduler") == 0) {
        profiles->mCurrentDataField = FIELD_INVALID;
        profiles->mCurrentConfig++;
    }
}

}  // namespace icamera
