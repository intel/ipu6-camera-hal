/*
 * Copyright (C) 2017-2021 Intel Corporation.
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

#define LOG_TAG IspControlUtils

#include "CameraMetadata.h"

#include "IspControl.h"
#include "IspControlUtils.h"

#include "iutils/CameraLog.h"
#include "iutils/Utils.h"

namespace icamera {

#include "IspControlInfoMap.cpp"

const char* IspControlUtils::getNameById(uint32_t ctrlId) {
    int size = ARRAY_SIZE(gIspControlInfoMap);
    for (int i = 0; i < size; i++) {
        if (gIspControlInfoMap[i].ctrlId == ctrlId) {
            return gIspControlInfoMap[i].name;
        }
    }

    return nullptr;
}

uint32_t IspControlUtils::getIdByName(const char* name) {
    int size = ARRAY_SIZE(gIspControlInfoMap);
    for (int i = 0; i < size; i++) {
        if (strcmp(gIspControlInfoMap[i].name, name) == 0) {
            return gIspControlInfoMap[i].ctrlId;
        }
    }

    return 0;
}

uint32_t IspControlUtils::getSizeById(uint32_t ctrlId) {
    int size = ARRAY_SIZE(gIspControlInfoMap);
    for (int i = 0; i < size; i++) {
        if (gIspControlInfoMap[i].ctrlId == ctrlId) {
            return gIspControlInfoMap[i].size;
        }
    }

    return 0;
}

uint32_t IspControlUtils::getTagById(uint32_t ctrlId) {
    int size = ARRAY_SIZE(gIspControlInfoMap);
    for (int i = 0; i < size; i++) {
        if (gIspControlInfoMap[i].ctrlId == ctrlId) {
            return gIspControlInfoMap[i].tag;
        }
    }

    return 0;
}

#include "ia_types.h"

void* IspControlUtils::findDataById(uint32_t ctrlId, void* fullData, uint32_t size) {
    CheckAndLogError(fullData == nullptr || size == 0, nullptr, "Invalid input parameters");

    char* pData = (char*)fullData;
    uint32_t offset = 0;

    // Find the corresponding data from ISP output data.
    while (offset < size) {
        ia_record_header* headerPtr = (ia_record_header*)(pData + offset);
        LOG2("ISP output UUID:%d, size:%d", headerPtr->uuid, headerPtr->size);
        if (headerPtr->uuid == 0 || headerPtr->size == 0) break;
        if (ctrlId == headerPtr->uuid) {
            char* offsetPtr = pData + (offset + sizeof(ia_record_header));
            return offsetPtr;
        }
        offset += headerPtr->size;
    }

    return nullptr;
}

}  // namespace icamera
