/*
 * Copyright (C) 2021 Intel Corporation.
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
#define LOG_TAG PnpDebugControl

#include "src/platformdata/PnpDebugControl.h"

#include <expat.h>
#include <string.h>

#include "iutils/CameraLog.h"

namespace icamera {
PnpDebugControl* PnpDebugControl::sInstance = nullptr;
Mutex PnpDebugControl::sLock;

PnpDebugControl* PnpDebugControl::getInstance() {
    AutoMutex lock(sLock);
    if (sInstance == nullptr) {
        sInstance = new PnpDebugControl();
    }

    return sInstance;
}

void PnpDebugControl::releaseInstance() {
    AutoMutex lock(sLock);

    if (sInstance) {
        delete sInstance;
        sInstance = nullptr;
    }
}

void PnpDebugControl::updateConfig() {
    PnpDebugParser PnpDebugParser(&(getInstance()->mStaticCfg));
}

PnpDebugControl::PnpDebugControl() {
    PnpDebugParser PnpDebugParser(&mStaticCfg);
}

bool PnpDebugControl::useMockAAL() {
    return getInstance()->mStaticCfg.useMockAAL;
}

bool PnpDebugControl::isBypass3A() {
    return getInstance()->mStaticCfg.isBypass3A;
}

bool PnpDebugControl::isBypassPAL() {
    return getInstance()->mStaticCfg.isBypassPAL;
}

bool PnpDebugControl::isBypassPG() {
    return getInstance()->mStaticCfg.isBypassPG;
}

bool PnpDebugControl::isBypassFDAlgo() {
    return getInstance()->mStaticCfg.isBypassFDAlgo;
}

bool PnpDebugControl::isBypassISys() {
    return getInstance()->mStaticCfg.isBypassISys;
}

bool PnpDebugControl::useMockHal() {
    return getInstance()->mStaticCfg.useMockHal;
}

bool PnpDebugControl::isBypassP2p() {
    return getInstance()->mStaticCfg.isBypassP2p;
}

#define PNP_DEBUG_FILE_NAME "pnp_profiles.xml"
PnpDebugParser::PnpDebugParser(PnpDebugControl::StaticCfg* cfg)
        : mStaticCfg(cfg),
          mCurrentDataField(FIELD_INVALID) {
    int ret = getDataFromXmlFile(PNP_DEBUG_FILE_NAME);
    CheckAndLogError(ret != OK, VOID_VALUE, "Failed to get policy profiles data frome %s",
                     PNP_DEBUG_FILE_NAME);
}

/**
 * This function will check which field that the parser parses to.
 *
 * The field is set to 3 types.
 * FIELD_INVALID FIELD_POWER FIELD_PERFORMANCE
 *
 * \param profiles: the pointer of the PnpDebugParser.
 * \param name: the element's name.
 * \param atts: the element's attribute.
 */
void PnpDebugParser::checkField(PnpDebugParser* profiles, const char* name, const char** atts) {
    if (strcmp(name, "PnpDebugConfig") == 0) {
        profiles->mCurrentDataField = FIELD_INVALID;
        return;
    } else if (strcmp(name, "Power") == 0) {
        profiles->mCurrentDataField = FIELD_POWER;
        return;
    } else if (strcmp(name, "Performance") == 0) {
        profiles->mCurrentDataField = FIELD_PERFORMANCE;
        return;
    }

    LOGE("@%s, name:%s, atts[0]:%s, xml format wrong", __func__, name, atts[0]);
    return;
}

/**
 * This function will handle all the Power related elements.
 *
 * It will be called in the function startElement
 *
 * \param profiles: the pointer of the CameraParser.
 * \param name: the element's name.
 * \param atts: the element's attribute.
 */
void PnpDebugParser::handlePowerConfig(PnpDebugParser* profiles, const char* name,
                                       const char** atts) {
    LOG2("@%s, name:%s, atts[0]:%s, atts[1]: %s", __func__, name, atts[0], atts[1]);
    if (strcmp(name, "useMockAAL") == 0) {
        mStaticCfg->useMockAAL = strcmp(atts[1], "true") == 0;
    } else if (strcmp(name, "bypass3A") == 0) {
        mStaticCfg->isBypass3A = strcmp(atts[1], "true") == 0;
    } else if (strcmp(name, "bypassPAL") == 0) {
        mStaticCfg->isBypassPAL = strcmp(atts[1], "true") == 0;
    } else if (strcmp(name, "bypassPG") == 0) {
        mStaticCfg->isBypassPG = strcmp(atts[1], "true") == 0;
    } else if (strcmp(name, "bypassFDAlgo") == 0) {
        mStaticCfg->isBypassFDAlgo = strcmp(atts[1], "true") == 0;
    } else if (strcmp(name, "bypassISys") == 0) {
        mStaticCfg->isBypassISys = strcmp(atts[1], "true") == 0;
    } else if (strcmp(name, "useMockHal") == 0) {
        mStaticCfg->useMockHal = strcmp(atts[1], "true") == 0;
    } else if (strcmp(name, "bypassP2p") == 0) {
        mStaticCfg->isBypassP2p = strcmp(atts[1], "true") == 0;
    }
}

void PnpDebugParser::handlePerformanceConfig(PnpDebugParser* profiles, const char* name,
                                             const char** atts) {
    return;
}
/**
 * the callback function of the libexpat for handling of one element start
 *
 * When it comes to the start of one element. This function will be called.
 *
 * \param userData: the pointer we set by the function XML_SetUserData.
 * \param name: the element's name.
 */
void PnpDebugParser::startParseElement(void* userData, const char* name, const char** atts) {
    PnpDebugParser* profiles = reinterpret_cast<PnpDebugParser*>(userData);

    if (profiles->mCurrentDataField == FIELD_INVALID) {
        profiles->checkField(profiles, name, atts);
        return;
    }

    switch (profiles->mCurrentDataField) {
        case FIELD_POWER:
            profiles->handlePowerConfig(profiles, name, atts);
            break;
        case FIELD_PERFORMANCE:
            profiles->handlePerformanceConfig(profiles, name, atts);
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
void PnpDebugParser::endParseElement(void* userData, const char* name) {
    PnpDebugParser* profiles = reinterpret_cast<PnpDebugParser*>(userData);

    if (strcmp(name, "Power") == 0 || strcmp(name, "Performance") == 0) {
        profiles->mCurrentDataField = FIELD_INVALID;
    }
}

}  // namespace icamera
