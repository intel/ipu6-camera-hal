/*
 * Copyright (C) 2015-2020 Corporation.
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

#define LOG_TAG "V4l2DeviceFactory"

#include <fcntl.h>

#include "iutils/CameraLog.h"

#include "V4l2DeviceFactory.h"

namespace icamera {

std::map<int, V4l2DeviceFactory*> V4l2DeviceFactory::sInstances;
Mutex V4l2DeviceFactory::sLock;

V4l2DeviceFactory::V4l2DeviceFactory(int cameraId) : mCameraId(cameraId)
{
    LOG1("V4l2DeviceFactory created for id:%d", mCameraId);
}

V4l2DeviceFactory::~V4l2DeviceFactory()
{
    LOG1("V4l2DeviceFactory released for id:%d", mCameraId);
}

/**
 * Create a static instance of V4l2DeviceFactory for cameraId.
 * It should be called before any device is used.
 */
void V4l2DeviceFactory::createDeviceFactory(int cameraId)
{
    AutoMutex lock(sLock);
    getInstance(cameraId);
}

/**
 * Release the static instance of V4l2DeviceFactory for cameraId.
 * All device related to the instance of of V4l2DeviceFactory will be release here as well
 * After calling this function, all device could not be used anymore.
 */
void V4l2DeviceFactory::releaseDeviceFactory(int cameraId)
{
    AutoMutex lock(sLock);
    V4l2DeviceFactory* factory = getInstance(cameraId);
    sInstances.erase(cameraId);
    factory->releaseSubDevices(cameraId);
    delete factory;
}

/**
 * Get an opened sub device
 *
 * The caller is supposed to get an opened sub device.
 * If openSubDev failed, it just return non-opened instance,
 * and using this instance to call its funtion will cause 'device not open' error,
 *
 * Return a not nullptr sub device pointer
 */
V4L2Subdevice* V4l2DeviceFactory::getSubDev(int cameraId, const std::string& devName)
{
    AutoMutex lock(sLock);
    V4l2DeviceFactory* factory = getInstance(cameraId);
    // If an existing sub device found, then just return it.

    if (factory->mDevices.find(devName) != factory->mDevices.end()) {
        return factory->mDevices[devName];
    }
    // Create a new sub device for devName, since it's not created before.
    V4L2Subdevice* subdev = new V4L2Subdevice(devName);

    // Make sure the caller always got an opened device.
    subdev->Open(O_RDWR);
    // Add the new allocated sub device into device map.
    factory->mDevices[devName] = subdev;
    return subdev;
}

/**
 * Release all sub devices in device map
 *
 * It's a private function with no lock in it, must be called with lock protection.
 *
 * It MUST be called after all sub devices are not used anymore
 */
void V4l2DeviceFactory::releaseSubDevices(int  /*cameraId*/)
{
    for (auto it = mDevices.begin(); it != mDevices.end(); it++) {
        V4L2Subdevice* subdev = it->second;
        if (subdev) {
            subdev->Close();
            delete subdev;
        }
    }
    mDevices.clear();
}

/**
 * Private function with no lock in it, must be called with lock protection
 */
V4l2DeviceFactory* V4l2DeviceFactory::getInstance(int cameraId)
{
    if (sInstances.find(cameraId) != sInstances.end()) {
        return sInstances[cameraId];
    }

    sInstances[cameraId] = new V4l2DeviceFactory(cameraId);
    return sInstances[cameraId];
}

} //namespace icamera
