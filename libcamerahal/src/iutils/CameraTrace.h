/*
 * Copyright (C) 2020 Intel Corporation.
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

#include <string>
#include <unordered_map>

#include "iutils/Thread.h"

namespace icamera {

#define CAMERA_TRACE_NAME_LEN 32
#define METADATA_SEGMENT_NUM 4
#define CAMERA_TRACE_ROOT_EVENT 1

#define TRACE_IOC_MAGIC 'M'
#define TRACE_IOC_ENABLE _IOW(TRACE_IOC_MAGIC, 1, int)
#define TRACE_IOC_START _IOW(TRACE_IOC_MAGIC, 2, int)
#define TRACE_IOC_TIME _IOW(TRACE_IOC_MAGIC, 3, int)
#define TRACE_IOC_REGEVENT _IOWR(TRACE_IOC_MAGIC, 4, int)
#define TRACE_IOC_FINDEVENT _IOWR(TRACE_IOC_MAGIC, 5, int)
#define TRACE_IOC_ENABLEEVENT _IOW(TRACE_IOC_MAGIC, 6, int)
#define TRACE_IOC_LOG _IOW(TRACE_IOC_MAGIC, 7, int)
#define TRACE_IOC_DUMPEVENTINFO _IOR(TRACE_IOC_MAGIC, 8, int)
#define TRACE_IOC_METADATALOG _IOW(TRACE_IOC_MAGIC, 9, int)
#define TRACE_IOC_DUMPMETADATA _IOR(TRACE_IOC_MAGIC, 10, int)
#define TRACE_IOC_SELECTBUFFER _IOW(TRACE_IOC_MAGIC, 11, int)
#define TRACE_IOC_TRYLOG _IOWR(TRACE_IOC_MAGIC, 12, int)
#define TRACE_IOC_ISENABLE _IOR(TRACE_IOC_MAGIC, 13, int)
#define TRACE_IOC_TEST _IOWR(TRACE_IOC_MAGIC, 100, int)

typedef unsigned int TraceEvent;

typedef enum {
    TraceEventStart = 1 << 0,
    TraceEventEnd = 1 << 1,
    TraceEventPoint = 1 << 2,
} TraceEventType;

struct TraceEventInfo {
    unsigned int parentId;
    char name[CAMERA_TRACE_NAME_LEN];
};

typedef enum { TraceLog = 0, TraceMetaStringMBS = 1, TraceMetaStructure = 3 } TraceLogType;

struct TraceMetadata {
    unsigned int data1;                              // data1 (user defined)
    unsigned int data2;                              // data2 (user defined)
    TraceLogType metaDataType;                       // meta data type
    unsigned int size;                               // meta data size
    void* segmentData[METADATA_SEGMENT_NUM];         // meta data segment pointers
    unsigned int segmentSize[METADATA_SEGMENT_NUM];  // meta data segment sizes
};

struct TraceMetadataLog {
    TraceEvent event;
    TraceEventType type;
    TraceMetadata metaData;
};

class CameraTrace {
 public:
    // Basic trace: support to print two uint32_t parameters
    CameraTrace(TraceEventType type, const char* eventName, uint32_t data1 = 0, uint32_t data2 = 0);

    // String trace: support to print a string and two uint32_t parameters
    CameraTrace(TraceEventType type, const char* eventName, const char* paramStr,
                uint32_t data1 = 0, uint32_t data2 = 0);

    // Structure trace: support to print the struct data and two uint32_t parameters
    CameraTrace(TraceEventType type, const char* eventName, const char* structName, void* pStruct,
                size_t structSize, uint32_t data1 = 0, uint32_t data2 = 0);

    static void closeDevice();
    static unsigned int setColor(uint32_t data);

    ~CameraTrace();

 private:
    bool initTraceFd();
    unsigned int isTraceEventEnabled(void);
    TraceEvent registerTraceEvent(const char* eventName);
    void enableEvent(TraceEvent event);

    int cameraTraceLog(uint32_t data1 = 0, uint32_t data2 = 0);
    int cameraTraceLogString(const char* paramStr = nullptr, uint32_t data1 = 0,
                             uint32_t data2 = 0);
    int cameraTraceLogStructure(const char* structName = nullptr, size_t structSize = 0,
                                void* pStruct = 0, uint32_t data1 = 0, uint32_t data2 = 0);

 private:
    TraceEventType mEventType;
    TraceEvent mTraceEvent;

    static std::unordered_map<std::string, TraceEvent> mRegisteredEvent;
    static int mTraceFd;
    static Mutex sLock;  // Guard for opening the trace file
};

}  // namespace icamera
