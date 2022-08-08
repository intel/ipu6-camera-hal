/*
 * Copyright (C) 2017-2018 Intel Corporation.
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

#include <stdint.h>

namespace icamera {

namespace IspControlUtils {
    const char* getNameById(uint32_t ctrlId);

    uint32_t getIdByName(const char* name);

    uint32_t getSizeById(uint32_t ctrlId);

    uint32_t getTagById(uint32_t ctrlId);

    /**
     * Find the data pointer which belongs to ctrlId in fullData
     *
     * Return NULL if cannot find ctrlId in fullData, otherwise will return the data pointer.
     */
    void* findDataById(uint32_t ctrlId, void* fullData, uint32_t size);
} // end of IspControlUtils

} // end of icamera
