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

#ifndef HAVE_ANDROID_OS
#include <semaphore.h>

#include "PlatformData.h"
#endif

namespace icamera {

/**
  * CameraSharedMemory : Maintain the camera shared memory for communication
  * between camera applications running in different processes.
  *
  * camera device status:
  * On camera device open, check the camera device status in shared memory.
  *       --IDLE, set the camera device status to "occupied" and open normally.
  *       --occupied, camera device has already been opened, and should not be
  *           opened again.
  * On camera device close, set the camera device status to "IDLE" in shared memory.
  *
  * More infos that need to be shared between camera applications can be stored
  * in the shared memory as well.
  *
  * This class is created along with cameraHal class.
  */
#ifdef HAVE_ANDROID_OS
// Shared memory is not necessary for Android since only single instance of HAL
// is supported.
class CameraSharedMemory {
public:
    CameraSharedMemory() { mCameraDeviceOpenNum = 0; }
    ~CameraSharedMemory() {}
    int CameraDeviceOpen(int cameraId) { mCameraDeviceOpenNum++; return OK; }
    void CameraDeviceClose(int cameraId) { mCameraDeviceOpenNum--; }
    int cameraDeviceOpenNum() { return mCameraDeviceOpenNum; }

private:
    int mCameraDeviceOpenNum;
};

#else
class CameraSharedMemory {

public:
    CameraSharedMemory();
    ~CameraSharedMemory();
    /**
     * \brief Check and update the camera device status in shared memory.
     *
     * On deviceOpen(), call this function to check the camera device status in
     * shared memory. If the camera device has already been opened in another
     * process, it should not be opened again. Otherwise, set the camera device
     * to "occupied" in shared memory. Then it will not be opened again in
     * another process.
     *
     * \return OK if succeed, other value indicates failed
     */
    int CameraDeviceOpen(int cameraId);

    /**
     * \brief Update the camera device status to "IDLE" in shared memory.
     *
     * On deviceClose(), call this function to set the camera device status to
     * "IDLE" in shared memory.
     *
     */
    void CameraDeviceClose(int cameraId);

    /**
     * \brief Get camera device open number.
     *
     * On cameraDeviceOpenNum()a call this function to get the camera device open times
     *
     */
    int cameraDeviceOpenNum();

private:
    CameraSharedMemory(const CameraSharedMemory& copyClass);
    CameraSharedMemory& operator = (const CameraSharedMemory& rightClass);

    void acquireSharedMemory();
    void releaseSharedMemory();

    int getNameByPid(pid_t pid, char *name);
    bool processExist(pid_t pid, const char *storedName);
    void openSemLock();
    void closeSemLock();
    int lock();
    void unlock();

private:

    static const int MAX_PROCESS_NAME_LENGTH = 64;
    struct cameraDevStatus {
        pid_t pid;
        char name[MAX_PROCESS_NAME_LENGTH];
    };
    struct camera_shared_info {
        cameraDevStatus camDevStatus[MAX_CAMERA_NUMBER];
        // Other static variables need to be stored should be added here
    };

    sem_t *mSemLock;
    int mSharedMemId;
    camera_shared_info *mCameraSharedInfo;
};
#endif

} //namespace icamera
