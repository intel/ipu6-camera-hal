/*
 * Copyright (C) 2015-2018 Intel Corporation.
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

#include "AiqUnit.h"
#include "SensorHwCtrl.h"
#include "LensHw.h"

namespace icamera {

/*
 * \factory class I3AControlFactory
 * This class is used to create the right instance of 3A unit
 * automatically based on 3a enabled status
 */
class I3AControlFactory {
public:
    /**
     * \brief Select the AIQ unit according to config file and compiling option
     *
     * \param cameraId: the camera id
     * \param sensorHw: the hw sensor
     * \param lensHw: the hw lens
     *
     * \return the AIQ unit base class
     */
    static AiqUnitBase *createI3AControl(int cameraId, SensorHwCtrl *sensorHw, LensHw *lensHw);
};

} /* namespace icamera */
