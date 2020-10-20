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

#define LOG_TAG "CameraDump"

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>

#include "PlatformData.h"
#include "iutils/Errors.h"
#include "iutils/Utils.h"
#include "iutils/CameraLog.h"
#include "iutils/CameraDump.h"

#include "3a/AiqResult.h"
#include "3a/AiqResultStorage.h"

using std::string;
using std::shared_ptr;

namespace icamera {

int  gDumpType = 0;
int  gDumpFormat = 0;
uint32_t gDumpSkipNum = 0;
uint32_t gDumpRangeMin = 0;
uint32_t gDumpRangeMax = 0;
int  gDumpFrequency = 1;
char gDumpPath[50];
bool gDumpRangeEnabled = false;
static const char *ModuleName[] = {
    "na",     // not available
    "sensor",
    "isys",
    "psys",
    "de-inter",
    "swip-op"
}; // map to the ModuleType

static const char *StreamUsage[] = {
    "preview",
    "video",
    "still",
    "app",
}; // map to the StreamUsage

void CameraDump::setDumpLevel(void)
{
    const char* PROP_CAMERA_HAL_DUMP      = "cameraDump";
    const char* PROP_CAMERA_HAL_DUMP_FORMAT = "cameraDumpFormat";
    const char* PROP_CAMERA_HAL_DUMP_PATH = "cameraDumpPath";
    const char* PROP_CAMERA_HAL_DUMP_SKIP_NUM = "cameraDumpSkipNum";
    const char* PROP_CAMERA_HAL_DUMP_RANGE = "cameraDumpRange";
    const char* PROP_CAMERA_HAL_DUMP_FREQUENCY = "cameraDumpFrequency";

    // dump, it's used to dump images or some parameters to a file.
    char *dumpType = getenv(PROP_CAMERA_HAL_DUMP);
    if (dumpType) {
        gDumpType = strtoul(dumpType, nullptr, 0);
        LOGD("Dump type is 0x%x", gDumpType);
    }

    char *dumpFormat = getenv(PROP_CAMERA_HAL_DUMP_FORMAT);
    if (dumpFormat) {
        gDumpFormat = strtoul(dumpFormat, nullptr, 0);
        LOGD("Dump format is 0x%x", gDumpFormat);
    }

    char* cameraDumpPath = getenv(PROP_CAMERA_HAL_DUMP_PATH);
    snprintf(gDumpPath, sizeof(gDumpPath), "%s", "./");
    if (cameraDumpPath) {
        snprintf(gDumpPath, sizeof(gDumpPath), "%s", cameraDumpPath);
    }

    char* cameraDumpSkipNum = getenv(PROP_CAMERA_HAL_DUMP_SKIP_NUM);
    if (cameraDumpSkipNum) {
        gDumpSkipNum = strtoul(cameraDumpSkipNum, nullptr, 0);
        LOGD("Dump skip num is %d", gDumpSkipNum);
    }

    char* cameraDumpRange = getenv(PROP_CAMERA_HAL_DUMP_RANGE);
    if (cameraDumpRange) {
        int sz = strlen(cameraDumpRange);
        char dumpRange[sz + 1];
        char *savePtr = nullptr, *tablePtr = nullptr;
        MEMCPY_S(dumpRange, sz, cameraDumpRange, sz);
        dumpRange[sz] = '\0';

        tablePtr = strtok_r(dumpRange, ",~-", &savePtr);
        if (tablePtr)
            gDumpRangeMin = strtoul(tablePtr, nullptr, 0);

        tablePtr = strtok_r(nullptr, ",~-", &savePtr);
        if (tablePtr)
            gDumpRangeMax = strtoul(tablePtr, nullptr, 0);

        gDumpRangeEnabled = true;
        LOGD("Dump range is %d-%d", gDumpRangeMin, gDumpRangeMax);
    }

    char* cameraDumpFrequency = getenv(PROP_CAMERA_HAL_DUMP_FREQUENCY);
    if (cameraDumpFrequency) {
        gDumpFrequency = strtoul(cameraDumpFrequency, nullptr, 0);
        if (gDumpFrequency == 0)
            gDumpFrequency = 1;
        LOGD("Dump frequency is %d", gDumpFrequency);
    }

    // the PG dump is implemented in libiacss
    if (gDumpType & DUMP_PSYS_PG) {
        const char* PROP_CAMERA_CSS_DEBUG     = "camera_css_debug";
        const char* PROP_CAMERA_CSS_DUMP_PATH = "camera_css_debug_dump_path";

        char newCssDebugEnv[16];
        char *cssDebugEnv = getenv(PROP_CAMERA_CSS_DEBUG);
        int  cssDebugType = cssDebugEnv ? strtoul(cssDebugEnv, nullptr, 0) : 0;
        // defined in ia_log.h IA_CSS_LOG_LEVEL_DUMP = 64
        const int IA_CSS_LOG_LEVEL_DUMP = 64;
        snprintf(newCssDebugEnv, sizeof(newCssDebugEnv), "%d",
                (cssDebugType | IA_CSS_LOG_LEVEL_DUMP));
        // enable dump env in libiacss
        if (setenv(PROP_CAMERA_CSS_DEBUG, newCssDebugEnv, 1)) {
            LOGE("setenv error for %s, current value:%d\n", PROP_CAMERA_CSS_DEBUG,
                    cssDebugType);
        }

        const char* cssDumpPath = getenv(PROP_CAMERA_CSS_DUMP_PATH);
        // set dump path to hal dump path
        if (setenv(PROP_CAMERA_CSS_DUMP_PATH, gDumpPath, 1)) {
            LOGE("setenv error for %s, current path:%s\n", PROP_CAMERA_CSS_DUMP_PATH,
                    cssDumpPath ? cssDumpPath : "null");
        }
    }
}

bool CameraDump::isDumpTypeEnable(int dumpType)
{
    return gDumpType & dumpType;
}

bool CameraDump::isDumpFormatEnable(int dumpFormat)
{
    return gDumpFormat & dumpFormat;
}

const char* CameraDump::getDumpPath(void)
{
    return gDumpPath;
}

void CameraDump::writeData(const void* data, int size, const char* fileName) {
    CheckError((data == nullptr || size == 0 || fileName == nullptr), VOID_VALUE, "Nothing needs to be dumped");

    FILE *fp = fopen (fileName, "w+");
    CheckError(fp == nullptr, VOID_VALUE, "open dump file %s failed", fileName);

    LOG1("Write data to file:%s", fileName);
    if ((fwrite(data, size, 1, fp)) != 1)
        LOGW("Error or short count writing %d bytes to %s", size, fileName);
    fclose (fp);
}

static string getNamePrefix(int cameraId, ModuleType_t type, Port port, int sUsage = 0)
{
    const char* dumpPath   = CameraDump::getDumpPath();
    const char* sensorName = PlatformData::getSensorName(cameraId);
    char prefix[MAX_NAME_LEN] = {'\0'};

    if ((sUsage >= static_cast<int>(ARRAY_SIZE(StreamUsage))) || (sUsage < 0)) {
        sUsage = 0;
    }

    if (icamera::CameraDump::isDumpFormatEnable(DUMP_FORMAT_IQSTUDIO)) {
        snprintf(prefix, (MAX_NAME_LEN - 1), "%s/name#%s_%s", dumpPath, sensorName, StreamUsage[sUsage]);
    } else {
        if (port == INVALID_PORT) {
            snprintf(prefix, (MAX_NAME_LEN - 1), "%s/cam%d_%s_%s_%s", dumpPath, cameraId,
                 sensorName, ModuleName[type], StreamUsage[sUsage]);
        } else {
            snprintf(prefix, (MAX_NAME_LEN - 1), "%s/cam%d_%s_%s_port%d_%s", dumpPath, cameraId,
                 sensorName, ModuleName[type], port, StreamUsage[sUsage]);
        }
    }

    return string(prefix);
}

static string getAiqSettingAppendix(int cameraId, long sequence)
{
    char settingAppendix[MAX_NAME_LEN] = {'\0'};

    AiqResult* aiqResults = const_cast<AiqResult*>(AiqResultStorage::getInstance(cameraId)->getAiqResult(sequence));
    if (aiqResults == nullptr) {
        LOGW("%s: no result for sequence %ld! use the latest instead", __func__, sequence);
        aiqResults = const_cast<AiqResult*>(AiqResultStorage::getInstance(cameraId)->getAiqResult());
        CheckError((aiqResults == nullptr), string(settingAppendix), "Cannot find available aiq result.");
    }

    ia_aiq_exposure_sensor_parameters *sensorExposure =
                                       aiqResults->mAeResults.exposures[0].sensor_exposure;
    ia_aiq_exposure_parameters *exposure =
                                       aiqResults->mAeResults.exposures[0].exposure;

    CheckError((sensorExposure == nullptr || exposure == nullptr), string(settingAppendix), "Cannot find aiq exposures");

    double ag = sensorExposure->analog_gain_code_global;
    double dg = sensorExposure->digital_gain_global;
    float ispDg = 1.0f;

    LOG2("%s: original sensorExposure AG: %f, DG: %f, exposure: AG: %f, DG: %f",
           __func__, ag, dg, exposure->analog_gain, exposure->digital_gain);

    if (icamera::CameraDump::isDumpFormatEnable(DUMP_FORMAT_IQSTUDIO)) {

        // Convert AG and DG per sensor for IQ Studio input.
        const int nSteps = 256;
        const char* sensorName = PlatformData::getSensorName(cameraId);
        ispDg = sensorExposure->digital_gain_global;

        if (strstr(sensorName, "imx185") != nullptr) {
            LOG2("%s: AG and DG conversion made for %s.", __func__, sensorName);
            if ((double)sensorExposure->analog_gain_code_global * 0.3 > 24) {
                ag = 16.0 * nSteps;
                // real gain should be  pwd(10, (db value / 20))
                dg = nSteps * pow(10, ((double)sensorExposure->analog_gain_code_global * 0.3 - 24) / 20);
            } else {
                ag = nSteps * pow(10, ((double)sensorExposure->analog_gain_code_global * 0.3) / 20);
                dg = 1.0 * nSteps;
            }
            LOG2("%s: converted AG: %f, DG: %f ispDG: %f for %s", __func__, ag, dg, ispDg, sensorName);
        } else if (strstr(sensorName, "imx274") != nullptr) {
            ag = nSteps * exposure->analog_gain;
            dg = nSteps * PlatformData::getSensorDigitalGain(cameraId, exposure->digital_gain);
            ispDg = nSteps * PlatformData::getIspDigitalGain(cameraId, exposure->digital_gain);
            LOG2("%s: converted AG: %f, DG: %f ispDG: %f for %s", __func__, ag, dg, ispDg, sensorName);
        }

        if (aiqResults->mAeResults.num_exposures == 2) {
            snprintf(settingAppendix, (MAX_NAME_LEN - 1), "~ag#%.0f~dg#%.0f~cmnt#ispdg_%.0f~exp#%d,%d",
                ag, dg, ispDg, exposure->exposure_time_us,
                aiqResults->mAeResults.exposures[1].exposure->exposure_time_us);
        } else {
            snprintf(settingAppendix, (MAX_NAME_LEN - 1), "~ag#%.0f~dg#%.0f~cmnt#ispdg_%.0f~exp#%d",
                ag, dg, ispDg, exposure->exposure_time_us);
        }
    } else {

        if (PlatformData::isUsingIspDigitalGain(cameraId)) {
            dg = PlatformData::getSensorDigitalGain(cameraId, exposure->digital_gain);
            ispDg = PlatformData::getIspDigitalGain(cameraId, exposure->digital_gain);
        }

        if (aiqResults->mAeResults.num_exposures == 2) {
            snprintf(settingAppendix, (MAX_NAME_LEN - 1), "_ag#%.0f_dg#%.0f_ispdg#%.3f_exp#%d,%d",
                ag, dg, ispDg, exposure->exposure_time_us,
                aiqResults->mAeResults.exposures[1].exposure->exposure_time_us);
        } else {
            snprintf(settingAppendix, (MAX_NAME_LEN - 1), "_ag#%.0f_dg#%.0f_ispdg#%.3f_exp#%d",
                ag, dg, ispDg, exposure->exposure_time_us);
        }
    }

    return string(settingAppendix);
}

static string formatFrameFileName(const char *prefix,
                                  const char *appendix,
                                  const char *suffix,
                                  long sequence,
                                  int width, int height)
{
    char fileName[MAX_NAME_LEN] = {'\0'};

    if (icamera::CameraDump::isDumpFormatEnable(DUMP_FORMAT_IQSTUDIO)) {

        if (strstr(suffix, "GRBG") || strstr(suffix, "RGGB")
            || strstr(suffix, "GBRG") || strstr(suffix, "BGGR")) {
            snprintf(fileName, (MAX_NAME_LEN - 1), "%s~rev#v1~type#studio%s~msid#4442075~rep#%ld.raw",
                prefix, appendix, sequence);
        } else {
            snprintf(fileName, (MAX_NAME_LEN - 1), "%s~rev#v1~type#studio%s~msid#4442075~rep#%ld.%s",
                prefix, appendix, sequence, suffix);
        }
    } else {

        snprintf(fileName, (MAX_NAME_LEN - 1), "%s_frame_%04ld_%dx%d%s.%s",
             prefix, sequence, width, height, appendix, suffix);
    }
    return string(fileName);
}

static string formatBinFileName(int cameraId, const char *prefix, BinParam_t *binParam)
{
    char fileName[MAX_NAME_LEN] = {'\0'};
    string appendix;

    switch(binParam->bType) {
    case BIN_TYPE_GENERAL:
        snprintf(fileName, (MAX_NAME_LEN - 1), "%s_bin_%04ld_%s.bin",
                 prefix, binParam->sequence, binParam->gParam.appendix);
        break;
    case BIN_TYPE_STATISTIC:
        snprintf(fileName, (MAX_NAME_LEN - 1),
                 "%s_stat_%04ld_grid%dx%d_%s.bin",
                 prefix, binParam->sequence,
                 binParam->sParam.gridWidth, binParam->sParam.gridHeight,
                 binParam->sParam.appendix);
        break;
    case BIN_TYPE_SENSOR_METADATA:
        snprintf(fileName, (MAX_NAME_LEN - 1),
                 "%s_metadata_%04ld_%dx%d_plane%d.%s",
                 prefix, binParam->sequence,
                 binParam->mParam.width, binParam->mParam.height,
                 binParam->mParam.planeIdx,
                 CameraUtils::format2string(binParam->mParam.metaFormat).c_str());
        break;
    case BIN_TYPE_BUFFER:
        appendix = getAiqSettingAppendix(cameraId, binParam->sequence);
        return formatFrameFileName(prefix, appendix.c_str(),
                                   CameraUtils::format2string(binParam->bParam.format).c_str(),
                                   binParam->sequence,
                                   binParam->bParam.width, binParam->bParam.height);

    default:
        LOGW("Unknow binary type:%d", binParam->bType);
        break;
    }

    return string(fileName);
}

void CameraDump::dumpImage(int cameraId, const shared_ptr<CameraBuffer> &camBuffer,
                           ModuleType_t type, Port port)
{
    CheckError(camBuffer == nullptr, VOID_VALUE, "invalid param");

    if (camBuffer->getSequence() < gDumpSkipNum) return;

    if (gDumpRangeEnabled &&
        (camBuffer->getSequence() < gDumpRangeMin
         || camBuffer->getSequence() > gDumpRangeMax)) {
        return;
    }

    if (camBuffer->getSequence() % gDumpFrequency != 0) return;

    string prefix   = getNamePrefix(cameraId, type, port, camBuffer->getUserBuffer()->s.usage);
    string appendix = getAiqSettingAppendix(cameraId, camBuffer->getSequence());

    string fileName = formatFrameFileName(prefix.c_str(), appendix.c_str(),
                                          CameraUtils::format2string(camBuffer->getFormat()).c_str(),
                                          camBuffer->getSequence(),
                                          camBuffer->getWidth(), camBuffer->getHeight());

    int fd = camBuffer->getFd();
    int bufferSize = camBuffer->getBufferSize();
    int memoryType = camBuffer->getMemory();
    void* pBuf = (memoryType == V4L2_MEMORY_DMABUF)
                    ? CameraBuffer::mapDmaBufferAddr(fd, bufferSize)
                    : camBuffer->getBufferAddr();
    LOGD("@%s, fd:%d, buffersize:%d, buf:%p, memoryType:%d, fileName:%s",
            __func__, fd, bufferSize, pBuf, memoryType, fileName.c_str());
    writeData(pBuf, bufferSize, fileName.c_str());
    if (memoryType == V4L2_MEMORY_DMABUF) {
        CameraBuffer::unmapDmaBufferAddr(pBuf, bufferSize);
    }
}

void CameraDump::dumpBinary(int cameraId, const void *data, int size, BinParam_t *binParam)
{
    CheckError(binParam == nullptr, VOID_VALUE, "invalid param");

    if (binParam->sequence < gDumpSkipNum) return;

    if (gDumpRangeEnabled &&
        (binParam->sequence < gDumpRangeMin
         || binParam->sequence > gDumpRangeMax)) {
        return;
    }

    if (binParam->sequence % gDumpFrequency != 0) return;

    string prefix   = getNamePrefix(cameraId, binParam->mType, INVALID_PORT, binParam->sUsage);
    string fileName = formatBinFileName(cameraId, prefix.c_str(), binParam);
    LOG2("@%s, fileName:%s", __func__, fileName.c_str());
    writeData(data, size, fileName.c_str());
}

} //namespace icamera
