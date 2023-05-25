/*
 * Copyright (C) 2016-2023 Intel Corporation.
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
#define LOG_TAG TunningParser

#include "TunningParser.h"

#include <expat.h>
#include <string.h>

#include "iutils/CameraLog.h"
#include "iutils/Utils.h"

namespace icamera {
#define TUNNING_PROFILE_NAME "tunning_profiles.xml"
TunningParser::TunningParser(PlatformData::StaticCfg* cfg)
        : mStaticCfg(cfg),
          mCurrentDataField(FIELD_INVALID),
          mCurrentCam(nullptr) {
    LOG2("@%s", __func__);
    CheckAndLogError(!mStaticCfg, VOID_VALUE, "@%s, cfg parameter is wrong", __func__);

    int ret = getDataFromXmlFile(TUNNING_PROFILE_NAME);
    if (ret != OK) LOGI("No tunning data from %s", TUNNING_PROFILE_NAME);
}

// According sensorName to find the CameraInfo in mStaticCfg
void TunningParser::getCameraInfoByName(TunningParser* profiles, const char* name) {
    size_t i = 0;

    for (i = 0; i < profiles->mStaticCfg->mCameras.size(); i++) {
        if (strcmp(profiles->mStaticCfg->mCameras[i].sensorName.c_str(), name) == 0) {
            profiles->mCurrentCam = &profiles->mStaticCfg->mCameras[i];
            break;
        }
    }
    if (i == profiles->mStaticCfg->mCameras.size()) {
        LOGE("Couldn't find the CameraInfo, please check the sensor name in xml");
        return;
    }
    LOG2("find the CameraInfo for sensor: %s", profiles->mCurrentCam->sensorName.c_str());
}

/**
 * This function will check which field that the parser parses to.
 *
 * The field is set to 3 types.
 * FIELD_INVALID FIELD_SENSOR and FIELD_COMMON
 *
 * \param profiles: the pointer of the TunningParser.
 * \param name: the element's name.
 * \param atts: the element's attribute.
 */
void TunningParser::checkField(TunningParser* profiles, const char* name, const char** atts) {
    if (strcmp(name, "TunningSettings") == 0) {
        profiles->mCurrentDataField = FIELD_INVALID;
        return;
    } else if (strcmp(name, "Sensor") == 0) {
        int idx = 0;
        while (atts[idx]) {
            const char* key = atts[idx];
            const char* val = atts[idx + 1];
            LOG2("@%s, name:%s, atts[%d]:%s, atts[%d]:%s", __func__, name, idx, key, idx + 1, val);
            if (strcmp(key, "name") == 0) {
                profiles->getCameraInfoByName(profiles, val);
            }
            idx += 2;
        }
        profiles->mCurrentDataField = FIELD_SENSOR;
        return;
    } else if (strcmp(name, "Modules") == 0) {
        profiles->mCurrentDataField = FIELD_MODULE;
        return;
    } else if (strcmp(name, "Common") == 0) {
        profiles->mCurrentDataField = FIELD_COMMON;
        return;
    }

    LOGE("@%s, name:%s, atts[0]:%s, xml format wrong", __func__, name, atts[0]);
    return;
}

/**
 * This function will handle all the common related elements.
 *
 * It will be called in the function startElement
 *
 * \param profiles: the pointer of the CameraParser.
 * \param name: the element's name.
 * \param atts: the element's attribute.
 */
void TunningParser::handleCommon(TunningParser* /*profiles*/, const char* name, const char** atts) {
    LOG2("@%s, name:%s, atts[0]:%s", __func__, name, atts[0]);

    if (strcmp(atts[0], "value") != 0) {
        LOGE("@%s, name:%s, atts[0]:%s, xml format wrong", __func__, name, atts[0]);
        return;
    }
}

// CUSTOM_WEIGHT_GRID_S
void TunningParser::handleWeightGrid(TunningParser* profiles, const char* name, const char** atts) {
    WeightGridTable wg;
    int idx = 0;

    CLEAR(wg);
    while (atts[idx]) {
        const char* key = atts[idx];
        LOG2("%s: name: %s, value: %s", __func__, atts[idx], atts[idx + 1]);
        if (strcmp(key, "width") == 0) {
            wg.width = (unsigned short)strtoul(atts[idx + 1], nullptr, 10);
        } else if (strcmp(key, "height") == 0) {
            wg.height = (unsigned short)strtoul(atts[idx + 1], nullptr, 10);
        } else if (strcmp(key, "table") == 0) {
            if (wg.table) {
                delete[] wg.table;
                wg.table = nullptr;
            }
            if (0 < wg.width && wg.width < MAX_WEIGHT_GRID_SIDE_LEN && 0 < wg.height &&
                wg.height < MAX_WEIGHT_GRID_SIDE_LEN) {
                wg.table = new unsigned char[wg.width * wg.height];
            }

            if (wg.table) parseXmlParameterToChar(atts[idx + 1], wg.table);
        }
        idx += 2;
    }

    LOG2("@%s, name:%s, atts[0]:%s", __func__, name, atts[0]);
    profiles->mCurrentCam->mWGTable.push_back(wg);
}
// CUSTOM_WEIGHT_GRID_E

#define CAMERA_MODULE_STR "CameraModuleInfo_"
void TunningParser::handleModule(TunningParser* profiles, const char* name, const char** atts) {
    if (strncmp(name, CAMERA_MODULE_STR, strlen(CAMERA_MODULE_STR)) == 0) {
        // tag name like this: CameraModuleInfo_xxx
        std::string tagName(name);
        std::string moduleName = tagName.substr(strlen(CAMERA_MODULE_STR));
        for (size_t i = 0; i < profiles->mStaticCfg->mCameras.size(); i++) {
            if (!profiles->mStaticCfg->mCameras[i].mCamModuleName.empty() &&
                (strcmp(profiles->mStaticCfg->mCameras[i].mCamModuleName.c_str(),
                        moduleName.c_str()) == 0)) {
                profiles->mCurrentCam = &profiles->mStaticCfg->mCameras[i];
                break;
            }
        }
    }
    if (!profiles->mCurrentCam) return;

    LOG2("@%s, name:%s, atts[0]:%s, sensor:%s", __func__, name, atts[0],
         profiles->mCurrentCam->sensorName.c_str());

    if (strcmp(name, "edgeNrTable") == 0) {
        EdgeNrSetting setting;

        int size = strlen(atts[1]);
        char src[size+1];
        MEMCPY_S(src, size, atts[1], size);
        src[size] = '\0';
        char* savePtr;
        char* totalGain = strtok_r(src, ",", &savePtr);

        while (totalGain) {
            float tg = atof(totalGain);
            char* hdrRatio = strtok_r(nullptr, ",", &savePtr);
            char* edge = strtok_r(nullptr, ",", &savePtr);
            char* nr = strtok_r(nullptr, ",", &savePtr);
            CheckWarning(!hdrRatio || !edge || !nr, VOID_VALUE, "wrong str %s", atts[1]);

            float hr = atof(hdrRatio);
            setting.edgeStrength = atoi(edge);
            setting.nrStrength = atoi(nr);
            LOG3("Edge and Nr table: totalGain %f, hdrRatio %f, edge %d, nr %d", tg, hr,
                 setting.edgeStrength, setting.nrStrength);
            auto& tmpMap = profiles->mCurrentCam->mTotalGainHdrRatioToEdgeNrMap[tg];
            tmpMap[hr] = setting;

            totalGain = strtok_r(nullptr, ",", &savePtr);
        }
    }
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
void TunningParser::handleSensor(TunningParser* profiles, const char* name, const char** atts) {
    if (profiles->mCurrentCam == nullptr) {
        LOGW("@%s, can't get the sensor name, will not process sensor weightgrid", __func__);
        return;
    }

    LOG2("@%s, name:%s, atts[0]:%s, sensor:%s", __func__, name, atts[0],
         profiles->mCurrentCam->sensorName.c_str());
// CUSTOM_WEIGHT_GRID_S
    if (strcmp(name, "WeightGrid") == 0) {
        handleWeightGrid(profiles, name, atts);
    }
// CUSTOM_WEIGHT_GRID_E
}

/**
 * the callback function of the libexpat for handling of one element start
 *
 * When it comes to the start of one element. This function will be called.
 *
 * \param userData: the pointer we set by the function XML_SetUserData.
 * \param name: the element's name.
 */
void TunningParser::startParseElement(void* userData, const char* name, const char** atts) {
    TunningParser* profiles = reinterpret_cast<TunningParser*>(userData);

    if (profiles->mCurrentDataField == FIELD_INVALID) {
        profiles->checkField(profiles, name, atts);
        return;
    }

    switch (profiles->mCurrentDataField) {
        case FIELD_SENSOR:
            profiles->handleSensor(profiles, name, atts);
            break;
        case FIELD_MODULE:
            profiles->handleModule(profiles, name, atts);
            break;
        case FIELD_COMMON:
            profiles->handleCommon(profiles, name, atts);
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
void TunningParser::endParseElement(void* userData, const char* name) {
    LOG2("@%s %s", __func__, name);

    TunningParser* profiles = reinterpret_cast<TunningParser*>(userData);

    if (strcmp(name, "Sensor") == 0 || strcmp(name, "Common") == 0 ||
        strcmp(name, "Modules") == 0) {
        profiles->mCurrentDataField = FIELD_INVALID;
    } else if (strncmp(name, CAMERA_MODULE_STR, strlen(CAMERA_MODULE_STR)) == 0) {
        profiles->mCurrentCam = nullptr;
    }
}

}  // namespace icamera
