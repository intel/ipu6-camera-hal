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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vector>

#include "modules/ia_cipr/include/ipu-psys.h"

namespace icamera {
namespace CIPR {
enum class Result {
    OK,
    GeneralError,
    NoMemory,
    DataError,
    InternalError,
    InvaildArg,
    NoEntry,
    TimeOut,
    End,
    Full,
    Retry,
    NoImple
};

enum MemoryFlag : uint32_t {
    CpuPtr = (uint32_t)(1 << 0),
    MemoryHandle = (uint32_t)(1 << 1),
    AllocateCpuPtr = (uint32_t)(1 << 2),
    CopyFromUser = (uint32_t)(1 << 3),
    HardwareOnly = (uint32_t)(1 << 4),
    ReadOnly = (uint32_t)(1 << 5),
    WriteOnly = (uint32_t)(1 << 6),
    UnCached = (uint32_t)(1 << 7),
    Uninitialized = (uint32_t)(1 << 8),
    Secured = (uint32_t)(1 << 9),
    CpuOnly = (uint32_t)(1 << 10),
    FlushCpuCache = (uint32_t)(1 << 11),
    Allocated = (uint32_t)(1 << 13),
    MemoryFromUser = (uint32_t)(1 << 14),
    Migrated = (uint32_t)(1 << 15),
    NoFlush = (uint32_t)(1 << 17),
    PSysAPI = (uint32_t)(1 << 31),
    DeviceMask = (uint32_t)0xFFFF0000
};

inline MemoryFlag operator|(MemoryFlag a, MemoryFlag b) {
    return static_cast<MemoryFlag>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline MemoryFlag operator&(MemoryFlag a, MemoryFlag b) {
    return static_cast<MemoryFlag>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

inline MemoryFlag operator~(MemoryFlag a) {
    return static_cast<MemoryFlag>(~static_cast<uint32_t>(a));
}

// NOLINTNEXTLINE
inline MemoryFlag& operator|=(MemoryFlag& a, MemoryFlag b) {
    return a = static_cast<MemoryFlag>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

// NOLINTNEXTLINE
inline MemoryFlag& operator&=(MemoryFlag& a, MemoryFlag b) {
    return a = static_cast<MemoryFlag>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

class Command;
class Event;
class Buffer;
class Context;

typedef ipu_psys_buffer IPUPSysBuffer;
struct MemoryDesc {
    uint32_t size;
    void* cpuPtr;
    uint64_t handle;
    MemoryFlag flags;
    Buffer* anchor;
    IPUPSysBuffer* sysBuff;
};

struct PSysEvent {
    struct ipu_psys_event event;
    int32_t timeout;
};

struct PSysEventConfig {
    uint32_t type;
    uint64_t id;
    uint64_t commandToken;
    uint64_t commandIssueID;
    uint64_t bufferHandoverBitmap;
    uint32_t error;
    int32_t timeout;
};

const int PSYS_CAP_DRIVER_FIELD_LENGTH = 20;
const int PSYS_CAP_DEVMODEL_FIELD_LENGTH = 32;
struct PSYSCapability {
    uint32_t version;
    uint8_t driver[PSYS_CAP_DRIVER_FIELD_LENGTH];
    uint8_t devModel[PSYS_CAP_DEVMODEL_FIELD_LENGTH];
    uint32_t programGroupCount;
};

#define psys_command_ext_na 0
#define psys_command_ext_ppg_0 1
#define psys_command_ext_ppg_1 2
struct PSysCmdExtHeader {
    uint32_t offset;
    uint64_t size;
    uint32_t version;
};

struct ProcessGroupCommand {
    PSysCmdExtHeader header;
    uint8_t frameCounter;
    uint8_t queueID;
    uint16_t fragmentState;
    uint64_t eventReqBitmap;
    uint64_t irqReqBitmap;
    uint32_t dynamicKernelBitmap[4];
};

struct PSysCommandConfig {
    PSysCommandConfig()
            : id(0),
              token(0),
              issueID(0),
              priority(0),
              psysFrequency(0),
              extBuf(nullptr),
              pgParamsBuf(nullptr),
              pgManifestBuf(nullptr),
              pg(nullptr) {}
    uint64_t id;
    uint64_t token;
    uint64_t issueID;
    uint32_t priority;
    uint32_t psysFrequency;
    std::vector<Buffer*> buffers;
    Buffer* extBuf;
    Buffer* pgParamsBuf;
    Buffer* pgManifestBuf;
    Buffer* pg;
};

struct PSysCommand {
    PSysCommand()
            : pg(nullptr),
              pgParams(nullptr),
              pgParamsSize(0),
              extBuf(nullptr),
              pgManifestBuf(nullptr) {
        memset(&iocCmd, 0, sizeof(iocCmd));
    }

    struct ipu_psys_command iocCmd;
    std::vector<Buffer*> userBuffers;
    Buffer* pg;
    void* pgParams;
    uint32_t pgParamsSize;
    Buffer* extBuf;
    Buffer* pgManifestBuf;
};

class MemoryOper {
 public:
    virtual ~MemoryOper() = default;

    virtual Result allocate(MemoryDesc* mem) = 0;
    virtual Result migrate(MemoryDesc* mem) = 0;
    virtual Result getMemory(MemoryDesc* mem, MemoryDesc* out) = 0;
    virtual Result destroy(MemoryDesc* mem) = 0;
};
}  // namespace CIPR
}  // namespace icamera
