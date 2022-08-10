/*
 * Copyright (C) 2015-2021 Intel Corporation.
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

#define LOG_TAG I3AControlFactory

#include "iutils/CameraLog.h"
#include "PlatformData.h"

#include "I3AControlFactory.h"

namespace icamera {

AiqUnitBase *I3AControlFactory::createI3AControl(int cameraId, SensorHwCtrl *sensorHw,
                                                 LensHw *lensHw)
{
    LOG1("<id%d>@%s", cameraId, __func__);
    if (PlatformData::isEnableAIQ(cameraId)) {
        return new AiqUnit(cameraId, sensorHw, lensHw);
    }
    return new AiqUnitBase();
}

} /* namespace icamera */
