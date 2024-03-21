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

#define LOG_TAG CameraShm

#include "CameraShm.h"

#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "iutils/CameraLog.h"

namespace icamera {

static const int CAMERA_DEVICE_IDLE = 0;
static const int CAMERA_IPCKEY = 0x43414D;
static const int CAMERA_SHM_LOCK_TIME = 2;

#define SEM_NAME "/camlock"
#define SEM_FD_NAME "/dev/shm/sem.camlock"

CameraSharedMemory::CameraSharedMemory()
        : mSemLock(nullptr),
          mSharedMemId(-1),
          mCameraSharedInfo(nullptr) {
    PERF_CAMERA_ATRACE();

    acquireSharedMemory();
}

CameraSharedMemory::~CameraSharedMemory() {
    PERF_CAMERA_ATRACE();

    releaseSharedMemory();
}

int CameraSharedMemory::CameraDeviceOpen(int cameraId) {
    int ret = OK;

    CheckAndLogError(mCameraSharedInfo == nullptr, ret, "No attached camera shared memory!");

    CheckAndLogError(lock() != OK, ret, "Fail to lock shared memory!");

    // Check camera device status from the shared memory
    pid_t pid = mCameraSharedInfo->camDevStatus[cameraId].pid;
    char* name = mCameraSharedInfo->camDevStatus[cameraId].name;
    if (pid != CAMERA_DEVICE_IDLE && processExist(pid, name)) {
        LOG1("@%s(pid %d): device has been opened in another process(pid %d/%s)", __func__,
             getpid(), pid, name);
        ret = INVALID_OPERATION;
    } else {
        mCameraSharedInfo->camDevStatus[cameraId].pid = getpid();
        getNameByPid(getpid(), mCameraSharedInfo->camDevStatus[cameraId].name);
    }
    unlock();

    return ret;
}

void CameraSharedMemory::CameraDeviceClose(int cameraId) {
    CheckAndLogError(mCameraSharedInfo == nullptr, VOID_VALUE, "No attached camera shared memory!");

    CheckAndLogError(lock() != OK, VOID_VALUE, "Fail to lock shared memory!");
    if (mCameraSharedInfo->camDevStatus[cameraId].pid == getpid()) {
        mCameraSharedInfo->camDevStatus[cameraId].pid = CAMERA_DEVICE_IDLE;
        CLEAR(mCameraSharedInfo->camDevStatus[cameraId].name);
    } else {
        LOGW("@%s: The stored pid is not the pid of current process!", __func__);
    }
    unlock();
}

void CameraSharedMemory::acquireSharedMemory() {
    openSemLock();

    bool newCreated = false;
    const size_t CAMERA_SM_SIZE = (sizeof(camera_shared_info) / getpagesize() + 1) * getpagesize();

    CheckAndLogError(lock() != OK, VOID_VALUE, "Fail to lock shared memory!");
    // get the shared memory ID, create shared memory if not exist
    mSharedMemId = shmget(CAMERA_IPCKEY, CAMERA_SM_SIZE, 0640);
    if (mSharedMemId == -1) {
        mSharedMemId = shmget(CAMERA_IPCKEY, CAMERA_SM_SIZE, IPC_CREAT | 0640);
        if (mSharedMemId < 0) {
            LOGE("Fail to allocate shared memory by shmget.");
            unlock();
            return;
        }
        newCreated = true;
    }

    // attach shared memory
    mCameraSharedInfo = reinterpret_cast<camera_shared_info*>(shmat(mSharedMemId, nullptr, 0));
    if (mCameraSharedInfo == (void*)-1) {
        LOGE("Fail to attach shared memory");
        mCameraSharedInfo = nullptr;
    } else {
        struct shmid_ds shmState;
        int ret = shmctl(mSharedMemId, IPC_STAT, &shmState);

        // attach number is 1, current process is the only camera process
        if (ret == 0 && shmState.shm_nattch == 1) {
            if (newCreated)
                LOG1("The shared memory is new created, init the values.");
            else
                LOG1("Some camera process exited abnormally. Reinit the values.");

            for (int i = 0; i < MAX_CAMERA_NUMBER; i++) {
                mCameraSharedInfo->camDevStatus[i].pid = CAMERA_DEVICE_IDLE;
                CLEAR(mCameraSharedInfo->camDevStatus[i].name);
            }
        } else {
            // Check if the process stored in share memory is still running.
            // Set the device status to IDLE if the stored process is not running.
            for (int i = 0; i < MAX_CAMERA_NUMBER; i++) {
                pid_t pid = mCameraSharedInfo->camDevStatus[i].pid;
                char* name = mCameraSharedInfo->camDevStatus[i].name;
                if (pid != CAMERA_DEVICE_IDLE && !processExist(pid, name)) {
                    LOG1("process %d(%s) opened the device but it's not running now.", pid, name);
                    mCameraSharedInfo->camDevStatus[i].pid = CAMERA_DEVICE_IDLE;
                }
            }
        }
    }
    unlock();
}

void CameraSharedMemory::releaseSharedMemory() {
    CheckAndLogError(mCameraSharedInfo == nullptr, VOID_VALUE, "No attached camera shared memory!");

    // Make sure the camera device occupied info by current process is cleared
    pid_t pid = getpid();
    CheckAndLogError(lock() != OK, VOID_VALUE, "Fail to lock shared memory!");
    for (int i = 0; i < MAX_CAMERA_NUMBER; i++) {
        if (mCameraSharedInfo->camDevStatus[i].pid == pid) {
            mCameraSharedInfo->camDevStatus[i].pid = CAMERA_DEVICE_IDLE;
            LOGW("Seems camera device %d is not closed properly (pid %d).", i, pid);
        }
    }

    // detach shared memory
    int ret = shmdt(mCameraSharedInfo);
    if (ret != 0) {
        LOGE("Fail to detach shared memory");
    }

    // delete shared memory if no one is attaching
    struct shmid_ds shmState;
    ret = shmctl(mSharedMemId, IPC_STAT, &shmState);
    if (ret == 0 && shmState.shm_nattch == 0) {
        LOG1("No attaches to the camera shared memory. Release it.");
        shmctl(mSharedMemId, IPC_RMID, 0);
    }
    unlock();

    closeSemLock();
}

int CameraSharedMemory::cameraDeviceOpenNum() {
    CheckAndLogError(mCameraSharedInfo == nullptr, 0, "No attached camera shared memory!");

    pid_t pid = getpid();
    int camOpenNum = 0;

    CheckAndLogError(lock() != OK, 0, "Fail to lock shared memory!");
    for (int i = 0; i < MAX_CAMERA_NUMBER; i++) {
        if (mCameraSharedInfo->camDevStatus[i].pid != CAMERA_DEVICE_IDLE) {
            LOG1("The camera device: %d is opened by pid: %d", i, pid);
            camOpenNum++;
        }
    }
    unlock();
    LOG1("Camera device is opened number: %d", camOpenNum);

    return camOpenNum;
}

int CameraSharedMemory::getNameByPid(pid_t pid, char* name) {
    const int BUF_SIZE = 1024;
    char procPidPath[BUF_SIZE] = {'\0'};
    char buf[BUF_SIZE] = {'\0'};

    snprintf(procPidPath, BUF_SIZE, "/proc/%d/status", static_cast<int>(pid));
    FILE* fp = fopen(procPidPath, "r");
    CheckAndLogError(fp == nullptr, UNKNOWN_ERROR, "Fail to get the pid status!");

    if (fgets(buf, BUF_SIZE - 1, fp) != nullptr) {
        sscanf(buf, "%*s %64s", name);
    }
    fclose(fp);

    return OK;
}

bool CameraSharedMemory::processExist(pid_t pid, const char* storedName) {
    char name[MAX_PROCESS_NAME_LENGTH];
    return kill(pid, 0) == 0 && getNameByPid(pid, name) == OK && strcmp(storedName, name) == 0;
}

void CameraSharedMemory::openSemLock() {
    mSemLock = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0644, 1);
    if (mSemLock == SEM_FAILED) {
        mSemLock = sem_open(SEM_NAME, O_RDWR);
        if (mSemLock == SEM_FAILED) {
            LOGE("failed to open sem lock, errno: %s\n", strerror(errno));
            return;
        } else {
            LOG1("Open the sem lock");
        }
    } else {
        chmod(SEM_FD_NAME, 0666);
        LOG1("Create the sem lock");
        return;
    }

    // Check if the semaphore is still available
    int ret = OK;
    struct timespec ts;
    CLEAR(ts);

    // Wait the semaphore lock for 2 seconds
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += CAMERA_SHM_LOCK_TIME;
    while ((ret = sem_timedwait(mSemLock, &ts)) == -1 && errno == EINTR) {
    }
    if (ret == 0) {
        sem_post(mSemLock);
        return;
    }

    if (ret != 0 && errno == ETIMEDOUT) {
        LOG1("Lock timed out, process holding it may have crashed. Re-create the semaphore.");
        sem_close(mSemLock);
        sem_unlink(SEM_NAME);
        mSemLock = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0644, 1);
        if (mSemLock == SEM_FAILED) {
            LOGE("failed to re-create sem lock, errno: %s\n", strerror(errno));
        } else {
            chmod(SEM_FD_NAME, 0666);
        }
    }
}

void CameraSharedMemory::closeSemLock() {
    sem_close(mSemLock);
}

int CameraSharedMemory::lock() {
    int ret = OK;
    struct timespec ts;
    CheckAndLogError(mSemLock == SEM_FAILED, BAD_VALUE, "invalid sem lock");

    // Wait the semaphore lock for 2 seconds
    CLEAR(ts);
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += CAMERA_SHM_LOCK_TIME;
    while (((ret = sem_timedwait(mSemLock, &ts)) == -1) && errno == EINTR) {
    }
    CheckAndLogError(ret != 0, UNKNOWN_ERROR, "Lock failed or timed out");

    return OK;
}

void CameraSharedMemory::unlock() {
    sem_post(mSemLock);
}

}  // namespace icamera
