/*
 * Copyright (C) 2020-2021 Intel Corporation.
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

#include "Types.h"
#include "iutils/Utils.h"

namespace icamera {
namespace CIPR {
class Event {
 public:
    explicit Event(const PSysEventConfig& eventConfig);
    ~Event();
    bool isInitialized() { return mInitialized; }

    Result getConfig(PSysEventConfig* eventConfig);
    Result setConfig(const PSysEventConfig& eventConfig);
    Result wait(Context* ctx);

 private:
    PSysEvent* mEvent = nullptr;
    bool mInitialized = false;

    DISALLOW_COPY_AND_ASSIGN(Event);
};

}  // namespace CIPR
}  // namespace icamera
