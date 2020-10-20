/*
 * Copyright (C) 2017-2020 Intel Corporation.
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
#define LOG_TAG "PolicyParser"

#include <string.h>
#include <expat.h>

#include "iutils/CameraLog.h"

#include "PolicyParser.h"

namespace icamera {
#define PSYS_POLICY_FILE_NAME "psys_policy_profiles.xml"
PolicyParser::PolicyParser(PlatformData::StaticCfg *cfg) :
    mStaticCfg(cfg),
    mCurrentDataField(FIELD_INVALID),
    pCurrentConf(nullptr) {
    LOGXML("@%s", __func__);
    CheckError(!mStaticCfg, VOID_VALUE, "@%s, cfg parameter is wrong", __func__);
    mStaticCfg->mPolicyConfig.clear();

    int ret = getDataFromXmlFile(PSYS_POLICY_FILE_NAME);
    CheckError(ret != OK, VOID_VALUE,
               "Failed to get policy profiles data frome %s", PSYS_POLICY_FILE_NAME);
}

/**
 * This function will check which field that the parser parses to.
 *
 * The field is set to 2 types.
 * FIELD_INVALID FIELD_GRAPH
 *
 * \param profiles: the pointer of the PolicyParser.
 * \param name: the element's name.
 * \param atts: the element's attribute.
 */
void PolicyParser::checkField(PolicyParser *profiles, const char *name, const char **atts)
{
    LOGXML("@%s, name:%s", __func__, name);
    if (strcmp(name, "PsysPolicyConfig") == 0) {
        profiles->mCurrentDataField = FIELD_INVALID;
        return;
    } else if (strcmp(name, "graph") == 0) {
        profiles->pCurrentConf = new PolicyConfig;

        int idx = 0;
        while (atts[idx]) {
            const char* key = atts[idx];
            const char* val = atts[idx + 1];
            LOGXML("@%s, name:%s, atts[%d]:%s, atts[%d]:%s", __func__, name, idx, key, idx+1, val);
            if (strcmp(key, "id") == 0) {
                profiles->pCurrentConf->graphId = atoi(val);
            } else if (strcmp(key, "description") == 0) {
                profiles->pCurrentConf->policyDescription = val;
            }
            idx += 2;
        }
        profiles->mCurrentDataField = FIELD_GRAPH;
        return;
    }

    LOGE("@%s, name:%s, atts[0]:%s, xml format wrong", __func__, name, atts[0]);
    return;
}

void PolicyParser::handlePipeExecutor(PolicyParser *profiles, const char *name, const char **atts)
{
    int idx = 0;
    ExecutorPolicy policy;

    while (atts[idx]) {
        const char *key = atts[idx];
        LOGXML("%s: name: %s, value: %s", __func__, atts[idx], atts[idx + 1]);
        if (strcmp(key, "name") == 0) {
            policy.exeName = atts[idx + 1];
        } else if (strcmp(key, "pgs") == 0) {
            parseXmlConvertStrings(atts[idx + 1], policy.pgList, convertCharToString);
        } else if (strcmp(key, "op_modes") == 0) {
            parseXmlConvertStrings(atts[idx + 1], policy.opModeList, atoi);
        } else if (strcmp(key, "notify_policy") == 0) {
            int notifyPolicy = std::stoi(atts[idx + 1]);
            if (notifyPolicy >= 0 && notifyPolicy < POLICY_INVALID) {
                policy.notifyPolicy = (ExecutorNotifyPolicy)notifyPolicy;
            } else {
                LOGW("Invalid notify policy value: %d", notifyPolicy);
            }
        } else if (strcmp(key, "cyclic_feedback_routine") == 0) {
            parseXmlConvertStrings(atts[idx + 1], policy.cyclicFeedbackRoutineList, atoi);
        } else if (strcmp(key, "cyclic_feedback_delay") == 0) {
            parseXmlConvertStrings(atts[idx + 1], policy.cyclicFeedbackDelayList, atoi);
        } else {
            LOGW("Invalid policy attribute: %s", key);
        }
        idx += 2;
    }

    LOGXML("@%s, name:%s, atts[0]:%s", __func__, name, atts[0]);
    profiles->pCurrentConf->pipeExecutorVec.push_back(policy);
}

void PolicyParser::handleExclusivePGs(PolicyParser *profiles, const char *name, const char **atts)
{
    int idx = 0;
    LOGXML("%s: name: %s, value: %s", __func__, atts[idx], atts[idx + 1]);
    const char *key = atts[idx];
    if (strcmp(key, "pgs") == 0) {
        parseXmlConvertStrings(atts[idx + 1], profiles->pCurrentConf->exclusivePgs, convertCharToString);
    } else {
        LOGE("Invalid policy attribute %s in exclusive label.", key);
    }
}

void PolicyParser::handleBundles(PolicyParser *profiles, const char *name, const char **atts)
{
    int idx = 0;
    LOGXML("%s: name: %s, value: %s", __func__, atts[idx], atts[idx + 1]);
    const char *key = atts[idx];

    CheckError(strcmp(key, "executors") != 0, VOID_VALUE, "Invalid policy attribute %s in bundle label.", key);

    // The structure of a bundle looks like: "proc:0,post:1" which uses ',' to split
    // different executors' names, and uses ':' to specify the executor's depth.
    std::vector<std::string> bundledExecutors;
    std::vector<int> depths;
    std::vector<std::string> executors = CameraUtils::splitString(atts[idx + 1], ',');

    for (const auto & item : executors) {
        std::vector<std::string> executorDepth = CameraUtils::splitString(item.c_str(), ':');
        CheckError(executorDepth.size() != 2, VOID_VALUE, "Invalid executor-depth mapping.");

        bundledExecutors.push_back(executorDepth[0]);
        depths.push_back(std::stoi(executorDepth[1]));
    }

    ExecutorDepth executorDepth = {bundledExecutors, depths};
    profiles->pCurrentConf->bundledExecutorDepths.push_back(executorDepth);
}

void PolicyParser::handleShareReferPair(PolicyParser *profiles,
                                        const char *name, const char **atts) {
    // example: <shareReferPair pair="post_gdc_video_bayer:6,post_gdc_stills:6"/>
    if (strcmp(atts[0], "pair") != 0) return;
    int sz = strlen(atts[1]);
    char src[sz + 1];
    MEMCPY_S(src, sz, atts[1], sz);
    src[sz] = '\0';

    char* consumer = nullptr;
    char* producer = strtok_r(src, ",", &consumer);
    if (!consumer || !producer) return;

    char* pPort = nullptr;
    char* pName = strtok_r(producer, ":", &pPort);
    char* cPort = nullptr;
    char* cName = strtok_r(consumer, ":", &cPort);
    if (!pPort || !cPort) return;

    ShareReferIdDesc pPair;
    ShareReferIdDesc cPair;
    pPair.first = pName;
    pPair.second = atoi(pPort);
    cPair.first = cName;
    cPair.second = atoi(cPort);

    LOGXML("@%s, pair: %s:%d -> %s:%d", __func__,
           pPair.first.c_str(), pPair.second, cPair.first.c_str(), cPair.second);
    profiles->pCurrentConf->shareReferPairList.push_back(pPair);
    profiles->pCurrentConf->shareReferPairList.push_back(cPair);
}

/**
 * This function will handle all the sensor related elements.
 *
 * It will be called in the function startElement
 *
 * \param profiles: the pointer of the CameraParser.
 * \param name: the element's name.
 * \param atts: the element's attribute.
 */
void PolicyParser::handlePolicyConfig(PolicyParser *profiles, const char *name, const char **atts)
{
    LOGXML("@%s, name:%s, atts[0]:%s", __func__, name, atts[0]);
    if (strcmp(name, "pipe_executor") == 0) {
        handlePipeExecutor(profiles, name, atts);
    } else if (strcmp(name, "exclusive") == 0) {
        handleExclusivePGs(profiles, name, atts);
    } else if (strcmp(name, "bundle") == 0) {
        handleBundles(profiles, name, atts);
    } else if (strcmp(name, "enableBundleInSdv") == 0) {
        profiles->pCurrentConf->enableBundleInSdv = (strcmp(atts[1], "true") == 0) ? true : false;
        LOGXML("%s: enableBundleInSdv: %s", __func__, atts[1]);
    } else if (strcmp(name, "shareReferPair") == 0) {
        handleShareReferPair(profiles, name, atts);
    }
}

/**
 * the callback function of the libexpat for handling of one element start
 *
 * When it comes to the start of one element. This function will be called.
 *
 * \param userData: the pointer we set by the function XML_SetUserData.
 * \param name: the element's name.
 */
void PolicyParser::startParseElement(void *userData, const char *name, const char **atts)
{
    PolicyParser *profiles = reinterpret_cast<PolicyParser*>(userData);

    if (profiles->mCurrentDataField == FIELD_INVALID) {
        profiles->checkField(profiles, name, atts);
        return;
    }

    switch (profiles->mCurrentDataField) {
        case FIELD_GRAPH:
            profiles->handlePolicyConfig(profiles, name, atts);
            break;
        default:
            LOGE("@%s, line:%d, go to default handling", __func__, __LINE__);
            break;
    }
}

/**
 * the callback function of the libexpat for handling of one element end
 *
 * When it comes to the end of one element. This function will be called.
 *
 * \param userData: the pointer we set by the function XML_SetUserData.
 * \param name: the element's name.
 */
void PolicyParser::endParseElement(void *userData, const char *name)
{
    LOGXML("@%s %s", __func__, name);

    PolicyParser *profiles = reinterpret_cast<PolicyParser*>(userData);

    if (strcmp(name, "graph") == 0) {
        LOGXML("@%s, add policyConf, graphId: %d", __func__, profiles->pCurrentConf->graphId);
        profiles->mStaticCfg->mPolicyConfig.push_back(*(profiles->pCurrentConf));
        delete profiles->pCurrentConf;
        profiles->pCurrentConf = nullptr;
        profiles->mCurrentDataField = FIELD_INVALID;
    }
}

} //namespace icamera
