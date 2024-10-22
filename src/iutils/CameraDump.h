/*
 * Copyright (C) 2015-2023 Intel Corporation.
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

#include <linux/v4l2-subdev.h>
#include <string.h>

#include <string>
#include <vector>

#include "iutils/Thread.h"
#include "CameraBuffer.h"
#include "CameraTypes.h"

namespace icamera {

/**
 * global dump level
 * This global variable is set from system properties
 * It is used to control the type of frame dump
 */
extern int gDumpType;
extern char gDumpPath[50];

// Dump bit mask definition
enum {
    // IPU Buffer dump (bit[0-3]), export cameraDump=0xf
    DUMP_ISYS_BUFFER = 1 << 0,
    DUMP_PSYS_OUTPUT_BUFFER = 1 << 1,
    DUMP_PSYS_INTERM_BUFFER = 1 << 2,  // dump Psys intermediate buffers like PreGDC output
    DUMP_EXECUTOR_OUTPUT = 1 << 3,

    // Other buffer dump (bit[4-7]), export cameraDump=0xf0
    DUMP_JPEG_BUFFER = 1 << 4,
    DUMP_UT_BUFFER = 1 << 5,
    DUMP_SW_IMG_PROC_OUTPUT = 1 << 6,
    DUMP_GPU_TNR = 1 << 7,

    // PG/PAL/Stats dump (bit[8-11]), export cameraDump=0xf00
    DUMP_PSYS_PAL = 1 << 8,            // ISP param binary
    DUMP_PSYS_PG = 1 << 9,             // PSYS whole PG dump assisted by libiacss
    DUMP_PSYS_DECODED_STAT = 1 << 10,  // p2p decoded statistics

    // AAL dump (bit[12-15]), export cameraDump=0xf000
    DUMP_AAL_OUTPUT = 1 << 12,
    DUMP_AAL_INPUT = 1 << 13,

    // Other dump (bit[16-19]), export cameraDump=0xf0000
    DUMP_NVM_DATA = 1 << 16,
    DUMP_MAKER_NOTE = 1 << 17,
    DUMP_EMBEDDED_METADATA = 1 << 18,

    // Enable dump thread for dynamic control, export cameraDump=0x100000
    DUMP_THREAD = 1 << 20,
};

enum {
    DUMP_FORMAT_NORMAL = 1 << 0,    // Normal format
    DUMP_FORMAT_IQSTUDIO = 1 << 1,  // IQStudio format
};

const int MAX_NAME_LEN = 256;

typedef enum {
    M_NA,
    M_SENSOR,  // MIPI frame dump
    M_ISYS,    // ISYS param, payload, frame dump
    M_PSYS,    // PSYS param, payload, frame dump
    M_DEINTR,  // De-interlaced frame dump
    M_SWIPOP,  // Sw Image processor frame dump
    M_GPUTNR,  // GPU TNR frame dump
    M_NVM,     // NVM data dump
    M_MKN,     // Makernote dump
} ModuleType_t;

typedef enum {
    BIN_TYPE_GENERAL,
    BIN_TYPE_STATISTIC,
    BIN_TYPE_SENSOR_METADATA,
    BIN_TYPE_BUFFER,
} BinType_t;

typedef struct {
    const char* appendix;
} GeneralParam_t;

typedef struct {
    int gridWidth;
    int gridHeight;
    const char* appendix;
} StatParam_t;

typedef struct {
    int width;
    int height;
    int planeIdx;
    int metaFormat;
} SensorMetadataParam_t;

typedef struct {
    int width;
    int height;
    int format;
} BufferParam_t;

typedef struct {
    BinType_t bType;
    ModuleType_t mType;
    int64_t sequence;
    union {
        GeneralParam_t gParam;
        StatParam_t sParam;
        SensorMetadataParam_t mParam;
        BufferParam_t bParam;
    };
    int sUsage;
} BinParam_t;

/**
 * Dump files with formated file name, put under getDumpPath()
 * Supported dump type:
 *   Image(RAW/YUV/RGB)
 *   PAL bin
 *   Decoded statistics
 *   Sensor Metadata
 * File name format example:
 * Path/cameraId_sensorName_isys(psys)_
 *      frame(pal/stats/)_sequence_resolution_appendix.suffix
 */
namespace CameraDump {
/**
 * File dump control functions.
 */
void setDumpLevel(void);
void setDumpThread(void);
bool isDumpTypeEnable(int dumpType);
bool isDumpFormatEnable(int dumpFormat);
void writeData(const void* data, int size, const char* fileName);
const char* getDumpPath(void);
/**
 * Dump image according to CameraBuffer properties
 */
void dumpImage(int cameraId, const std::shared_ptr<CameraBuffer>& camBuffer,
               ModuleType_t mType = M_NA, Port port = INVALID_PORT, const char* desc = nullptr);
/**
 * Dump any buffer to binary file
 */
void dumpBinary(int cameraId, const void* data, int size, BinParam_t* binParam);

class DumpThread : public Thread {
public:
    DumpThread();
    ~DumpThread();

    bool threadLoop();
};

}  // namespace CameraDump

}  // namespace icamera
