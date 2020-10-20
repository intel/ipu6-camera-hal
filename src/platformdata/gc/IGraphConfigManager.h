/*
 * Copyright (C) 2018-2020 Intel Corporation
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

#include <memory>
#include <map>

#include "CameraTypes.h"
#include "IGraphConfig.h"
#include "Parameters.h"
#include "iutils/Thread.h"

namespace icamera {
class IGraphConfigManager {
public:
    virtual ~IGraphConfigManager() = default;

    virtual int configStreams(const stream_config_t* streams) = 0;
    virtual int getSelectedMcId() = 0;
    virtual std::shared_ptr<IGraphConfig> getGraphConfig(ConfigMode configMode) = 0;
    virtual bool isGcConfigured(void) = 0;
    static void releaseInstance(int cameraId);
    static IGraphConfigManager* getInstance(int cameraId);

private:
    // Guard for singleton instance creation.
    static Mutex sLock;
    static std::map<int, IGraphConfigManager*> sInstances;
};
}
