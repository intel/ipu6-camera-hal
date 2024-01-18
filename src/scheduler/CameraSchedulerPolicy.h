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

#pragma once

#include <mutex>
#include <string>
#include <utility>
#include <vector>
#include <set>

#include "ParserBase.h"

namespace icamera {

class CameraSchedulerPolicy : public ParserBase {
 public:
    static CameraSchedulerPolicy* getInstance();
    static void releaseInstance();

 private:
    // Prevent to create multiple instances
    CameraSchedulerPolicy();
    ~CameraSchedulerPolicy();

 public:
    int32_t setConfig(const std::set<int32_t>& graphIds);
    // Return <exeName, trigger source name>
    int32_t getExecutors(std::map<const char*, const char*>* executors) const;
    int32_t getNodeList(const char* exeName, std::vector<std::string>* nodeList) const;

    void startParseElement(void* userData, const char* name, const char** atts);
    void endParseElement(void* userData, const char* name);

 private:
    struct ExecutorDesc {
        std::string exeName;
        std::string triggerName;
        std::vector<std::string> nodeList;
    };

    struct PolicyConfigDesc {
        // static data
        uint32_t configId;
        std::set<int32_t> graphIds;
        std::vector<ExecutorDesc> exeList;

        PolicyConfigDesc() {
            configId = 0;
        }
    };

 private:
    void checkField(CameraSchedulerPolicy* profiles, const char* name, const char** atts);
    void handlePolicyConfig(CameraSchedulerPolicy* profiles, const char* name, const char** atts);
    void handleExecutor(CameraSchedulerPolicy* profiles, const char* name, const char** atts);

 private:
    enum DataField {
        FIELD_INVALID = 0,
        FIELD_SCHED,
    } mCurrentDataField;
    uint32_t mCurrentConfig;

 private:
    static CameraSchedulerPolicy* sInstance;
    static Mutex sLock;

    std::vector<PolicyConfigDesc> mPolicyConfigs;
    PolicyConfigDesc* mActiveConfig;

 private:
    DISALLOW_COPY_AND_ASSIGN(CameraSchedulerPolicy);
};

}  // namespace icamera
