/*
 * Copyright (C) 2020-2022 Intel Corporation.
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

#include "src/iutils/CameraTrace.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#include <string>
#include <unordered_map>

#include "iutils/Utils.h"

#define MAX_PARAM_NUMBER 2

namespace icamera {

int CameraTrace::mTraceFd = -1;
Mutex CameraTrace::sLock;
std::unordered_map<std::string, TraceEvent> CameraTrace::mRegisteredEvent;

CameraTrace::CameraTrace(TraceEventType type, const char* eventName, uint32_t data1, uint32_t data2)
        : mEventType(type),
          mTraceEvent(0) {
    if ((mTraceEvent = registerTraceEvent(eventName)) == 0) return;

    cameraTraceLog(data1, data2);
}

CameraTrace::CameraTrace(TraceEventType type, const char* eventName, const char* paramStr,
                         uint32_t data1, uint32_t data2)
        : mEventType(type),
          mTraceEvent(0) {
    if ((mTraceEvent = registerTraceEvent(eventName)) == 0) return;

    cameraTraceLogString(paramStr, data1, data2);
}

CameraTrace::CameraTrace(TraceEventType type, const char* eventName, const char* structName,
                         void* pStruct, size_t structSize, uint32_t data1, uint32_t data2)
        : mEventType(type),
          mTraceEvent(0) {
    if ((mTraceEvent = registerTraceEvent(eventName)) == 0) return;

    cameraTraceLogStructure(structName, structSize, pStruct, data1, data2);
}

CameraTrace::~CameraTrace() {
    if (mTraceEvent == 0) return;

    // Don't support parameters in trace event end
    if (mEventType == TraceEventStart) {
        mEventType = TraceEventEnd;
        cameraTraceLog();
    }
}

void CameraTrace::closeDevice() {
    AutoMutex lock(sLock);

    if (mTraceFd >= 0) {
        close(mTraceFd);
        mTraceFd = -1;
    }
}

void CameraTrace::enableEvent(TraceEvent event) {
    unsigned int cmd[3];
    cmd[0] = event;
    cmd[1] = 1;  // 1: enable, 0: disable
    cmd[2] = 0;  // 0: current event, 1: recursive events

    ioctl(mTraceFd, TRACE_IOC_ENABLEEVENT, cmd);
}

TraceEvent CameraTrace::registerTraceEvent(const char* eventName) {
    if (!isTraceEventEnabled()) return 0;

    if (mRegisteredEvent.find(eventName) != mRegisteredEvent.end())
        return mRegisteredEvent[eventName];

    TraceEventInfo eventInfo = {};
    snprintf(eventInfo.name, CAMERA_TRACE_NAME_LEN - 1, "%s", eventName);
    eventInfo.parentId = CAMERA_TRACE_ROOT_EVENT;

    ioctl(mTraceFd, TRACE_IOC_REGEVENT, &eventInfo);
    enableEvent(eventInfo.parentId);

    mRegisteredEvent[eventName] = eventInfo.parentId;

    return eventInfo.parentId;
}

unsigned int CameraTrace::isTraceEventEnabled(void) {
    if (!initTraceFd()) return 0;

    unsigned int enable = 0;
    ioctl(mTraceFd, TRACE_IOC_ISENABLE, &enable);
    return enable;
}

bool CameraTrace::initTraceFd() {
    AutoMutex lock(sLock);

    if (mTraceFd < 0) {
        const char* fileName = "/dev/mmp";
        struct stat statBuf;

        if (stat(fileName, &statBuf) != 0) return false;
        if ((mTraceFd = open(fileName, O_RDONLY)) < 0) return false;

        mRegisteredEvent.clear();
    }

    return true;
}

int CameraTrace::cameraTraceLog(unsigned int data1, unsigned int data2) {
    unsigned int cmd[4];
    cmd[0] = mTraceEvent;
    cmd[1] = mEventType;
    cmd[2] = data1;
    cmd[3] = data2;

    return ioctl(mTraceFd, TRACE_IOC_LOG, cmd);
}

int CameraTrace::cameraTraceLogString(const char* paramStr, unsigned int data1,
                                      unsigned int data2) {
    if (ioctl(mTraceFd, TRACE_IOC_TRYLOG, mTraceEvent) != 0) return -1;

    TraceMetadataLog MetaLog = {};
    MetaLog.metaData.data1 = data1;
    MetaLog.metaData.data2 = data2;
    MetaLog.metaData.metaDataType = TraceMetaStringMBS;
    MetaLog.metaData.size = strlen(paramStr) + 1;
    MetaLog.metaData.segmentData[0] = new char[MetaLog.metaData.size];
    MetaLog.metaData.segmentSize[0] = MetaLog.metaData.size;
    snprintf(static_cast<char*>(MetaLog.metaData.segmentData[0]), strlen(paramStr) + 1, "%s",
             paramStr);

    MetaLog.event = mTraceEvent;
    MetaLog.type = mEventType;

    int ret = ioctl(mTraceFd, TRACE_IOC_METADATALOG, &MetaLog);
    delete[] static_cast<char*>(MetaLog.metaData.segmentData[0]);

    return ret;
}

int CameraTrace::cameraTraceLogStructure(const char* structName, size_t structSize, void* pStruct,
                                         uint32_t data1, uint32_t data2) {
    if (ioctl(mTraceFd, TRACE_IOC_TRYLOG, mTraceEvent) != 0) return -1;

    TraceMetadataLog MetaLog = {};
    MetaLog.metaData.data1 = data1;
    MetaLog.metaData.data2 = data2;
    MetaLog.metaData.metaDataType = TraceMetaStructure;
    MetaLog.metaData.size = strlen(structName) + 1 + structSize;
    MetaLog.metaData.segmentData[0] = new char[strlen(structName) + 1];
    MetaLog.metaData.segmentData[1] = new char[structSize];
    MetaLog.metaData.segmentSize[0] = strlen(structName) + 1;
    MetaLog.metaData.segmentSize[1] = structSize;
    snprintf(static_cast<char*>(MetaLog.metaData.segmentData[0]), strlen(structName) + 1, "%s",
             structName);
    MEMCPY_S(MetaLog.metaData.segmentData[1], MetaLog.metaData.segmentSize[1], pStruct, structSize);

    MetaLog.event = mTraceEvent;
    MetaLog.type = mEventType;

    int ret = ioctl(mTraceFd, TRACE_IOC_METADATALOG, &MetaLog);
    delete[] static_cast<char*>(MetaLog.metaData.segmentData[0]);
    delete[] static_cast<char*>(MetaLog.metaData.segmentData[1]);

    return ret;
}

unsigned int CameraTrace::setColor(uint32_t data) {
#define COLOR_(r, g, b) (r | (g << 8) | (b << 16))
    static unsigned int sColors[] = {
        COLOR_(255, 64, 0),  COLOR_(0, 255, 64),  COLOR_(64, 0, 255),  COLOR_(255, 255, 0),
        COLOR_(255, 0, 255), COLOR_(0, 255, 255), COLOR_(255, 128, 0), COLOR_(128, 0, 255),
        COLOR_(0, 255, 128), COLOR_(128, 255, 0), COLOR_(255, 0, 128), COLOR_(0, 128, 255),
    };
    static unsigned int sColorNum = sizeof(sColors) / sizeof(unsigned int);
    return sColors[data % sColorNum];
}

}  // namespace icamera
