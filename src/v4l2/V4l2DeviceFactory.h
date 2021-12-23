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

#pragma once

#ifdef CAL_BUILD
#include <cros-camera/v4l2_device.h>
#else
#include <v4l2_device.h>
#endif

#include <map>
#include <string>

#include "iutils/Thread.h"
#include "iutils/Utils.h"

namespace icamera {

/**
 * Create all v4l2 devices, and provide users an opened device pointer,
 * the users should not release any instance got from this factory,
 * all devices will be released together by releaseDeviceFactory.
 *
 * Currently only sub device is supported.
 */
class V4l2DeviceFactory {
 public:
    static void createDeviceFactory(int cameraId);
    static void releaseDeviceFactory(int cameraId);

    static V4L2Subdevice* getSubDev(int cameraId, const std::string& devName);

 private:
    V4l2DeviceFactory(int cameraId);
    ~V4l2DeviceFactory();

    static V4l2DeviceFactory* getInstance(int cameraId);
    void releaseSubDevices(int cameraId);

    static std::map<int, V4l2DeviceFactory*> sInstances;
    // Guard for V4l2DeviceFactory public API access
    static Mutex sLock;

    int mCameraId;
    std::map<std::string, V4L2Subdevice*> mDevices;
};

}  // namespace icamera
