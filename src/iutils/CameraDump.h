/*
 * Copyright (C) 2015-2020 Intel Corporation.
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

#include <string.h>
#include <vector>
#include <string>
#include <linux/v4l2-subdev.h>

#include "CameraTypes.h"
#include "CameraBuffer.h"

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
    // ISYS Buffer dump (bit[0-3])
    DUMP_ISYS_BUFFER =          1 << 0,
    // Reserve bit[1-3] for detailed buffer dump control

    DUMP_JPEG_BUFFER =          1 << 3, // JPEG buffer
    // ISYS PG/PAL/Stats dump (bit[4-7])
    DUMP_ISYS_PAL =             1 << 4, // ISP param binary
    DUMP_ISYS_PG =              1 << 5, // ISYS whole PG dump assisted by libiacss
    DUMP_ISYS_ENCODED_STAT =    1 << 6, // p2p encoded statistics
    DUMP_ISYS_AIQ_STAT =        1 << 7, // rgbs_grid format stats for AIQ use

    // PSYS dump (bit[8-11])
    DUMP_PSYS_OUTPUT_BUFFER =   1 << 8,
    DUMP_PSYS_INTERM_BUFFER =   1 << 9, // dump Psys intermediate buffers like PreGDC output
    // Reserve bit[10-11] for detailed buffer dump control

    DUMP_AIQ_DVS_RESULT =       1 << 10, // dump dvs result

    // PSYS PG/PAL/Stats dump (bit[12-15])
    DUMP_PSYS_PAL =             1 << 12, // ISP param binary
    DUMP_PSYS_PG =              1 << 13, // PSYS whole PG dump assisted by libiacss
    DUMP_PSYS_AIQ_STAT =        1 << 14, // rgbs_grid format stats for AIQ use
    DUMP_PSYS_DECODED_STAT =    1 << 15, // p2p decoded statistics

    // Other dump
    DUMP_MIPI_BUFFER =          1 << 16, // e.g. export cameraDump=0x10000
    DUMP_UT_BUFFER =            1 << 17, // e.g. export cameraDump=0x20000
    DUMP_EMBEDDED_METADATA =    1 << 18,
    DUMP_DEINTERLACED_BUFFER =  1 << 19, // 0x80000  Decimal val 524288
    DUMP_SW_IMG_PROC_OUTPUT =   1 << 20, // 0x100000 Decimal val 1048576

    // Pipe executors' dump
    DUMP_EXECUTOR_OUTPUT =   1 << 21, // 0x200000 Decimal val 2097152

    // LTM output's dump
    DUMP_LTM_OUTPUT = 1 << 22, // 0x400000 Decimal val 4194304

    DUMP_AAL_OUTPUT = 1 << 23, // 0x800000 Decimal val 8388608
    DUMP_AAL_INPUT = 1 << 24, // 0x1000000 Decimal val 16777216
};

enum {
    DUMP_FORMAT_NORMAL =          1 << 0,  // Normal format
    DUMP_FORMAT_IQSTUDIO =        1 << 1,  // IQStudio format
};

const int MAX_NAME_LEN = 256;

typedef enum {
    M_NA,
    M_SENSOR, // MIPI frame dump
    M_ISYS,   // ISYS param, payload, frame dump
    M_PSYS,   // PSYS param, payload, frame dump
    M_DEINTR, // De-interlaced frame dump
    M_SWIPOP, // Sw Image processor frame dump
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
    BinType_t       bType;
    ModuleType_t    mType;
    long sequence;
    union {
        GeneralParam_t        gParam;
        StatParam_t           sParam;
        SensorMetadataParam_t mParam;
        BufferParam_t         bParam;
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
    bool isDumpTypeEnable(int dumpType);
    bool isDumpFormatEnable(int dumpFormat);
    void writeData(const void* data, int size, const char* fileName);
    const char* getDumpPath(void);
    /**
     * Dump image according to CameraBuffer properties
     */
    void dumpImage(int cameraId, const std::shared_ptr<CameraBuffer> &camBuffer,
                   ModuleType_t mType = M_NA, Port port = INVALID_PORT);
    /**
     * Dump any buffer to binary file
     */
    void dumpBinary(int cameraId, const void *data, int size, BinParam_t *binParam);
}

} //namespace icamera
