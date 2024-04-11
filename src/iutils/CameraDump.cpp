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

#define LOG_TAG CameraDump

#include "iutils/CameraDump.h"

#include <dirent.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <fstream>
#include <iostream>
#include <sstream>

#include "PlatformData.h"
#include "iutils/CameraLog.h"
#include "iutils/Errors.h"
#include "iutils/Utils.h"

#include "3a/AiqResult.h"
#include "3a/AiqResultStorage.h"

using std::shared_ptr;
using std::string;

namespace icamera {

Thread* gDumpThread = nullptr;

int gDumpType = 0;
int gDumpFormat = 0;
uint32_t gDumpSkipNum = 0;
uint32_t gDumpRangeMin = 0;
uint32_t gDumpRangeMax = 0;
int gDumpFrequency = 1;
char gDumpPath[50];
bool gDumpRangeEnabled = false;
int gDumpPatternEnabled = 0;
uint32_t gDumpPattern = 0xffffffff;
uint32_t gDumpPatternMask = 0xffffffff;
uint32_t gDumpPatternLineMin = 0;
uint32_t gDumpPatternLineMax = 0;
bool gDumpPatternLineEnabled = false;
static const char* ModuleName[] = {"na",  // not available
                                   "sensor",  "isys",    "psys", "de-inter",
                                   "swip-op", "gpu-tnr", "nvm",  "mkn"};  // map to the ModuleType

static const char* StreamUsage[] = {
    "preview",
    "video",
    "still",
    "app",
};  // map to the StreamUsage

void CameraDump::parseRange(char* rangeStr, uint32_t* rangeMin, uint32_t* rangeMax) {
    if (!rangeStr) return;

    std::string rangeArray(rangeStr);
    size_t i = rangeArray.find_first_of(",~-");
    if (i != std::string::npos) {
        *rangeMin = std::stoul(rangeArray.substr(0, i - 1));
        *rangeMax = std::stoul(rangeArray.substr(i + 1));
    }
}

void CameraDump::setDumpLevel(void) {
    const char* PROP_CAMERA_HAL_DUMP = "cameraDump";
    const char* PROP_CAMERA_HAL_DUMP_FORMAT = "cameraDumpFormat";
    const char* PROP_CAMERA_HAL_DUMP_PATH = "cameraDumpPath";
    const char* PROP_CAMERA_HAL_DUMP_SKIP_NUM = "cameraDumpSkipNum";
    const char* PROP_CAMERA_HAL_DUMP_RANGE = "cameraDumpRange";
    const char* PROP_CAMERA_HAL_DUMP_FREQUENCY = "cameraDumpFrequency";
    const char* PROP_CAMERA_HAL_DUMP_PATTERN_ENABLED = "cameraDumpPatternEnabled";
    const char* PROP_CAMERA_HAL_DUMP_PATTERN = "cameraDumpPattern";
    const char* PROP_CAMERA_HAL_DUMP_PATTERN_MASK = "cameraDumpPatternMask";
    const char* PROP_CAMERA_HAL_DUMP_PATTERN_RANGE = "cameraDumpPatternRange";

    // dump, it's used to dump images or some parameters to a file.
    char* dumpType = getenv(PROP_CAMERA_HAL_DUMP);
    if (dumpType) {
        gDumpType = strtoul(dumpType, nullptr, 0);
        LOGI("Dump type is 0x%x", gDumpType);
    }

    char* dumpFormat = getenv(PROP_CAMERA_HAL_DUMP_FORMAT);
    if (dumpFormat) {
        gDumpFormat = strtoul(dumpFormat, nullptr, 0);
        LOG1("Dump format is 0x%x", gDumpFormat);
    }

    char* cameraDumpPath = getenv(PROP_CAMERA_HAL_DUMP_PATH);

    if (cameraDumpPath) {
        snprintf(gDumpPath, sizeof(gDumpPath), "%s", cameraDumpPath);
        LOGI("User defined dump path %s", gDumpPath);
    } else {
        snprintf(gDumpPath, sizeof(gDumpPath), "%s", "./");
        LOG1("Default dump path %s", gDumpPath);
    }

    char* cameraDumpSkipNum = getenv(PROP_CAMERA_HAL_DUMP_SKIP_NUM);
    if (cameraDumpSkipNum) {
        gDumpSkipNum = strtoul(cameraDumpSkipNum, nullptr, 0);
        LOG1("Dump skip num is %d", gDumpSkipNum);
    }

    char* cameraDumpRange = getenv(PROP_CAMERA_HAL_DUMP_RANGE);
    if (cameraDumpRange) {
        parseRange(cameraDumpRange, &gDumpRangeMin, &gDumpRangeMax);
        gDumpRangeEnabled = true;
        LOG1("Dump range is %d-%d", gDumpRangeMin, gDumpRangeMax);
    }

    char* cameraDumpFrequency = getenv(PROP_CAMERA_HAL_DUMP_FREQUENCY);
    if (cameraDumpFrequency) {
        gDumpFrequency = strtoul(cameraDumpFrequency, nullptr, 0);
        if (gDumpFrequency == 0) gDumpFrequency = 1;
        LOG1("Dump frequency is %d", gDumpFrequency);
    }

    char* cameraDumpPatternEnabled = getenv(PROP_CAMERA_HAL_DUMP_PATTERN_ENABLED);
    if (cameraDumpPatternEnabled) {
        gDumpPatternEnabled = strtoul(cameraDumpPatternEnabled, nullptr, 0);
        LOGI("Dump pattern enabled is %d", gDumpPatternEnabled);
    }

    char* cameraDumpPattern = getenv(PROP_CAMERA_HAL_DUMP_PATTERN);
    if (cameraDumpPattern) {
        gDumpPattern = strtoul(cameraDumpPattern, nullptr, 0);
        LOGI("Dump pattern is 0x%08x", gDumpPattern);
    }

    char* cameraDumpPatternMask = getenv(PROP_CAMERA_HAL_DUMP_PATTERN_MASK);
    if (cameraDumpPatternMask) {
        gDumpPatternMask = strtoul(cameraDumpPatternMask, nullptr, 0);
        LOGI("Dump pattern mask is 0x%08x", gDumpPatternMask);
    }

    char* cameraDumpPatternRange = getenv(PROP_CAMERA_HAL_DUMP_PATTERN_RANGE);
    if (cameraDumpPatternRange) {
        parseRange(cameraDumpPatternRange, &gDumpPatternLineMin, &gDumpPatternLineMax);
        gDumpPatternLineEnabled = true;
        LOG1("Dump pattern range is line %d-%d", gDumpPatternLineMin, gDumpPatternLineMax);
    }

    // the PG dump is implemented in libiacss
    if (gDumpType & DUMP_PSYS_PG) {
        const char* PROP_CAMERA_CSS_DEBUG = "camera_css_debug";
        const char* PROP_CAMERA_CSS_DUMP_PATH = "camera_css_debug_dump_path";

        char newCssDebugEnv[16];
        char* cssDebugEnv = getenv(PROP_CAMERA_CSS_DEBUG);
        int cssDebugType = cssDebugEnv ? strtoul(cssDebugEnv, nullptr, 0) : 0;
        // defined in ia_log.h IA_CSS_LOG_LEVEL_DUMP = 64
        const int IA_CSS_LOG_LEVEL_DUMP = 64;
        snprintf(newCssDebugEnv, sizeof(newCssDebugEnv), "%d",
                 (cssDebugType | IA_CSS_LOG_LEVEL_DUMP));
        // enable dump env in libiacss
        if (setenv(PROP_CAMERA_CSS_DEBUG, newCssDebugEnv, 1)) {
            LOGE("setenv error for %s, current value:%d\n", PROP_CAMERA_CSS_DEBUG, cssDebugType);
        }

        const char* cssDumpPath = getenv(PROP_CAMERA_CSS_DUMP_PATH);
        // set dump path to hal dump path
        if (setenv(PROP_CAMERA_CSS_DUMP_PATH, gDumpPath, 1)) {
            LOGE("setenv error for %s, current path:%s\n", PROP_CAMERA_CSS_DUMP_PATH,
                 cssDumpPath ? cssDumpPath : "null");
        }
    }
}

void CameraDump::setDumpThread(void) {
    if (!gDumpThread) {
        // Default disable AIQDUMP when use dump thread
        setenv("AIQDUMP", "disable", 1);
        gDumpThread = new DumpThread();
        gDumpThread->run("DumpThread", PRIORITY_NORMAL);
    }
}

bool CameraDump::isDumpTypeEnable(int dumpType) {
    return gDumpType & dumpType;
}

bool CameraDump::isDumpFormatEnable(int dumpFormat) {
    return gDumpFormat & dumpFormat;
}

const char* CameraDump::getDumpPath(void) {
    return gDumpPath;
}

void CameraDump::writeData(const void* data, int size, const char* fileName) {
    CheckAndLogError((data == nullptr || size == 0 || fileName == nullptr), VOID_VALUE,
                     "Nothing needs to be dumped");

    FILE* fp = fopen(fileName, "w+");
    CheckAndLogError(fp == nullptr, VOID_VALUE, "open dump file %s failed", fileName);

    LOG1("Write data to file:%s", fileName);
    if ((fwrite(data, size, 1, fp)) != 1)
        LOGW("Error or short count writing %d bytes to %s", size, fileName);
    fclose(fp);
}

static string getNamePrefix(int cameraId, ModuleType_t type, Port port, int sUsage = 0) {
    const char* dumpPath = CameraDump::getDumpPath();
    const char* sensorName = PlatformData::getSensorName(cameraId);
    char prefix[MAX_NAME_LEN] = {'\0'};

    if ((sUsage >= static_cast<int>(ARRAY_SIZE(StreamUsage))) || (sUsage < 0)) {
        sUsage = 0;
    }

    if (icamera::CameraDump::isDumpFormatEnable(DUMP_FORMAT_IQSTUDIO)) {
        snprintf(prefix, (MAX_NAME_LEN - 1), "%s/name#%s_%s", dumpPath, sensorName,
                 StreamUsage[sUsage]);
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

static string getAiqSettingAppendix(int cameraId, int64_t sequence) {
    char settingAppendix[MAX_NAME_LEN] = {'\0'};

    AiqResult* aiqResults =
        const_cast<AiqResult*>(AiqResultStorage::getInstance(cameraId)->getAiqResult(sequence));
    if (aiqResults == nullptr) {
        LOGW("%s: no result for sequence %ld! use the latest instead", __func__, sequence);
        aiqResults =
            const_cast<AiqResult*>(AiqResultStorage::getInstance(cameraId)->getAiqResult());
        CheckAndLogError((aiqResults == nullptr), string(settingAppendix),
                         "Cannot find available aiq result.");
    }

    ia_aiq_exposure_sensor_parameters* sensorExposure =
        aiqResults->mAeResults.exposures[0].sensor_exposure;
    ia_aiq_exposure_parameters* exposure = aiqResults->mAeResults.exposures[0].exposure;

    CheckAndLogError((sensorExposure == nullptr || exposure == nullptr), string(settingAppendix),
                     "Cannot find aiq exposures");

    double ag = sensorExposure->analog_gain_code_global;
    double dg = sensorExposure->digital_gain_global;
    float ispDg = 1.0f;
    const int nSteps = 256;
    const char* sensorName = PlatformData::getSensorName(cameraId);

    LOG2("%s: original sensorExposure AG: %f, DG: %f, exposure: AG: %f, DG: %f", __func__, ag, dg,
         exposure->analog_gain, exposure->digital_gain);

    if (icamera::CameraDump::isDumpFormatEnable(DUMP_FORMAT_IQSTUDIO)) {
        // Convert AG and DG per sensor for IQ Studio input.
        ispDg = sensorExposure->digital_gain_global;

        if (strstr(sensorName, "imx185") != nullptr) {
            LOG2("%s: AG and DG conversion made for %s.", __func__, sensorName);
            if ((double)sensorExposure->analog_gain_code_global * 0.3 > 24) {
                ag = 16.0 * nSteps;
                // real gain should be  pwd(10, (db value / 20))
                dg = nSteps *
                     pow(10, ((double)sensorExposure->analog_gain_code_global * 0.3 - 24) / 20);
            } else {
                ag = nSteps * pow(10, ((double)sensorExposure->analog_gain_code_global * 0.3) / 20);
                dg = 1.0 * nSteps;
            }
            LOG2("%s: converted AG: %f, DG: %f ispDG: %f for %s", __func__, ag, dg, ispDg,
                 sensorName);
        } else if (strstr(sensorName, "imx274") != nullptr) {
            ag = nSteps * exposure->analog_gain;
            dg = nSteps * PlatformData::getSensorDigitalGain(cameraId, exposure->digital_gain);
            ispDg = nSteps * PlatformData::getIspDigitalGain(cameraId, exposure->digital_gain);
            LOG2("%s: converted AG: %f, DG: %f ispDG: %f for %s", __func__, ag, dg, ispDg,
                 sensorName);
        } else if (strstr(sensorName, "imx390") != nullptr) {
            ag = nSteps * pow(10, (double)sensorExposure->analog_gain_code_global * 0.3 / 20);
            dg = 1.0 * nSteps;
            LOG2("%s: converted AG: %f, DG: %f for %s", __func__, ag, dg, sensorName);
        }

        if (aiqResults->mAeResults.num_exposures == 2) {
            if (strstr(sensorName, "imx390") != nullptr) {
                double ag_1 =
                    aiqResults->mAeResults.exposures[1].sensor_exposure[0].analog_gain_code_global;
                LOG2("%s: ag_0: %f, ag_1: %f", __func__, ag, ag_1);

                ag_1 = nSteps * pow(10, (double)ag_1 * 0.3 / 20);
                LOG2("%s: after convert: ag_0: %f, ag_1: %f", __func__, ag, ag_1);

                snprintf(settingAppendix, (MAX_NAME_LEN - 1),
                         "~ag#%.0f,%.0f~dg#%.0f~cmnt#ispdg_%.0f~exp#%d,%d", ag, ag_1, dg, ispDg,
                         exposure->exposure_time_us,
                         aiqResults->mAeResults.exposures[1].exposure->exposure_time_us);
            } else {
                snprintf(settingAppendix, (MAX_NAME_LEN - 1),
                         "~ag#%.0f~dg#%.0f~cmnt#ispdg_%.0f~exp#%d,%d", ag, dg, ispDg,
                         exposure->exposure_time_us,
                         aiqResults->mAeResults.exposures[1].exposure->exposure_time_us);
            }
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
            if (strstr(sensorName, "imx390") != nullptr) {
                double ag_1 =
                    aiqResults->mAeResults.exposures[1].sensor_exposure[0].analog_gain_code_global;
                LOG2("%s: ag_0: %f, ag_1: %f", __func__, ag, ag_1);

                ag_1 = nSteps * pow(10, (double)ag_1 * 0.3 / 20);
                LOG2("%s: after convert: ag_0: %f, ag_1: %f", __func__, ag, ag_1);

                snprintf(settingAppendix, (MAX_NAME_LEN - 1),
                         "_ag#%.0f,%.0f_dg#%.0f_ispdg#%.3f_exp#%d,%d", ag, ag_1, dg, ispDg,
                         exposure->exposure_time_us,
                         aiqResults->mAeResults.exposures[1].exposure->exposure_time_us);
            } else {
                snprintf(settingAppendix, (MAX_NAME_LEN - 1),
                         "_ag#%.0f_dg#%.0f_ispdg#%.3f_exp#%d,%d", ag, dg, ispDg,
                         exposure->exposure_time_us,
                         aiqResults->mAeResults.exposures[1].exposure->exposure_time_us);
            }
        } else {
            snprintf(settingAppendix, (MAX_NAME_LEN - 1), "_ag#%.0f_dg#%.0f_ispdg#%.3f_exp#%d", ag,
                     dg, ispDg, exposure->exposure_time_us);
        }
    }

    return string(settingAppendix);
}

static string formatFrameFileName(const char* prefix, const char* appendix, const char* suffix,
                                  int64_t sequence, int width, int height) {
    char fileName[MAX_NAME_LEN] = {'\0'};

    if (icamera::CameraDump::isDumpFormatEnable(DUMP_FORMAT_IQSTUDIO)) {
        if (strstr(suffix, "GRBG") || strstr(suffix, "RGGB") || strstr(suffix, "GBRG") ||
            strstr(suffix, "BGGR")) {
            snprintf(fileName, (MAX_NAME_LEN - 1),
                     "%s~rev#v1~type#studio%s~msid#4442075~rep#%ld.raw", prefix, appendix,
                     sequence);
        } else {
            snprintf(fileName, (MAX_NAME_LEN - 1),
                     "%s~rev#v1~type#studio%s~msid#4442075~rep#%ld.%s", prefix, appendix, sequence,
                     suffix);
        }
    } else {
        snprintf(fileName, (MAX_NAME_LEN - 1), "%s_frame_%04ld_%dx%d%s.%s", prefix, sequence, width,
                 height, appendix, suffix);
    }
    return string(fileName);
}

static string formatBinFileName(int cameraId, const char* prefix, BinParam_t* binParam) {
    char fileName[MAX_NAME_LEN] = {'\0'};
    string appendix;

    switch (binParam->bType) {
        case BIN_TYPE_GENERAL:
            static int binIndex = 0;
            snprintf(fileName, (MAX_NAME_LEN - 1), "%s_bin_%04ld_%s_%d.bin", prefix,
                     binParam->sequence, binParam->gParam.appendix, binIndex);
            binIndex++;
            break;
        case BIN_TYPE_STATISTIC:
            snprintf(fileName, (MAX_NAME_LEN - 1), "%s_stat_%04ld_grid%dx%d_%s.bin", prefix,
                     binParam->sequence, binParam->sParam.gridWidth, binParam->sParam.gridHeight,
                     binParam->sParam.appendix);
            break;
        case BIN_TYPE_SENSOR_METADATA:
            snprintf(fileName, (MAX_NAME_LEN - 1), "%s_metadata_%04ld_%dx%d_plane%d.%s", prefix,
                     binParam->sequence, binParam->mParam.width, binParam->mParam.height,
                     binParam->mParam.planeIdx,
                     CameraUtils::format2string(binParam->mParam.metaFormat).c_str());
            break;
        case BIN_TYPE_BUFFER:
            appendix = getAiqSettingAppendix(cameraId, binParam->sequence);
            return formatFrameFileName(prefix, appendix.c_str(),
                                       CameraUtils::format2string(binParam->bParam.format).c_str(),
                                       binParam->sequence, binParam->bParam.width,
                                       binParam->bParam.height);

        default:
            LOGW("Unknow binary type:%d", binParam->bType);
            break;
    }

    return string(fileName);
}

int CameraDump::checkPattern(void* data, int bufferSize, int w, int h, int stride) {
    uint32_t val;
    int lineStart = h - 1;
    int lineEnd = h - 1;

    if (gDumpPatternLineEnabled && gDumpPatternLineMin < (uint32_t)h)
        lineStart = gDumpPatternLineMin;

    if (gDumpPatternLineEnabled && gDumpPatternLineMax < (uint32_t)h)
        lineEnd = gDumpPatternLineMax;

    for (; lineStart <= lineEnd; lineStart++)
        for (int col = 0; col < w; col += sizeof(uint32_t)) {
            val = *reinterpret_cast<uint32_t*>((unsigned char*)data + stride * lineStart + col);
            if ((val & gDumpPatternMask) ^ gDumpPattern) return 0;
        }

    return 1;
}

void CameraDump::dumpImage(int cameraId, const shared_ptr<CameraBuffer>& camBuffer,
                           ModuleType_t type, Port port, const char* desc) {
    CheckAndLogError(camBuffer == nullptr, VOID_VALUE, "invalid param");

    if (camBuffer->getSequence() < gDumpSkipNum) return;

    if (gDumpRangeEnabled &&
        (camBuffer->getSequence() < gDumpRangeMin || camBuffer->getSequence() > gDumpRangeMax)) {
        return;
    }

    if (camBuffer->getSequence() % gDumpFrequency != 0) return;

    string prefix = getNamePrefix(cameraId, type, port, camBuffer->getUserBuffer()->s.usage);
    string appendix = getAiqSettingAppendix(cameraId, camBuffer->getSequence());
    if (desc) appendix.append(desc);

    string fileName = formatFrameFileName(
        prefix.c_str(), appendix.c_str(),
        CameraUtils::format2string(camBuffer->getFormat()).c_str(), camBuffer->getSequence(),
        camBuffer->getWidth(), camBuffer->getHeight());

    int fd = camBuffer->getFd();
    int bufferSize = camBuffer->getBufferSize();
    int memoryType = camBuffer->getMemory();

    ScopeMapping mapper(camBuffer);
    void* pBuf = mapper.getUserPtr();

    if (gDumpPatternEnabled) {
        if (!checkPattern(pBuf, bufferSize,
                     camBuffer->getWidth(),
                     camBuffer->getHeight(),
                     camBuffer->getStride()))
            return;
        LOGI("@%s, dump pattern matched frame %d", __func__, camBuffer->getSequence());
    }
    LOG1("@%s, fd:%d, buffersize:%d, buf:%p, memoryType:%d, fileName:%s", __func__, fd, bufferSize,
         pBuf, memoryType, fileName.c_str());
    writeData(pBuf, bufferSize, fileName.c_str());
}

void CameraDump::dumpBinary(int cameraId, const void* data, int size, BinParam_t* binParam) {
    CheckAndLogError(binParam == nullptr, VOID_VALUE, "invalid param");

    if (binParam->sequence < gDumpSkipNum) return;

    if (gDumpRangeEnabled &&
        (binParam->sequence < gDumpRangeMin || binParam->sequence > gDumpRangeMax)) {
        return;
    }

    if (binParam->sequence % gDumpFrequency != 0) return;

    string prefix = getNamePrefix(cameraId, binParam->mType, INVALID_PORT, binParam->sUsage);
    string fileName = formatBinFileName(cameraId, prefix.c_str(), binParam);
    LOG2("@%s, fileName:%s", __func__, fileName.c_str());
    writeData(data, size, fileName.c_str());
}

namespace CameraDump {
DumpThread::DumpThread() {}
DumpThread::~DumpThread() {}

#define FIFO_NAME "/tmp/cameraDump"
#define BUFFER_SIZE 4096
bool DumpThread::threadLoop() {
    int pipe_fd;
    int res;
    int open_mode = O_RDONLY;
    int bytes_read = 0;
    char buffer[BUFFER_SIZE + 1];
    char* equal = nullptr;
    const char* PROP_CAMERA_HAL_DUMP = "cameraDump";
    const char* PROP_CAMERA_HAL_DUMP_FORMAT = "cameraDumpFormat";
    const char* PROP_CAMERA_HAL_DUMP_PATH = "cameraDumpPath";
    const char* PROP_CAMERA_HAL_DUMP_SKIP_NUM = "cameraDumpSkipNum";
    const char* PROP_CAMERA_HAL_DUMP_RANGE = "cameraDumpRange";
    const char* PROP_CAMERA_HAL_DUMP_FREQUENCY = "cameraDumpFrequency";
    const char* PROP_AIQDUMP = "AIQDUMP";

    char fifo_name[BUFFER_SIZE + 1];

    LOGI("DumpThread start");

    memset(fifo_name, '\0', sizeof(fifo_name));
    snprintf(fifo_name, BUFFER_SIZE, "%s_%d", FIFO_NAME, getpid());
    if (access(fifo_name, F_OK) == -1) {
        res = mkfifo(fifo_name, 0777);
        if (res != 0) {
            LOGI("Could not create fifo %s", fifo_name);
        }
    }

    memset(buffer, '\0', sizeof(buffer));

    pipe_fd = open(fifo_name, open_mode);
    LOGI("Process %d opened fd %d", getpid(), pipe_fd);

    if (pipe_fd != -1) {
        do  {
            res = read(pipe_fd, buffer, BUFFER_SIZE);
            bytes_read += res;
        } while (0);
        (void)close(pipe_fd);
    } else {
        return false;
    }

    LOGI("Process %d finished, %s", getpid(), buffer);
    equal = strchr(buffer, '=');
    if (equal) {
        *equal = '\0';
    } else {
        return true;
    }

    LOGI("%s, %d, %s", __func__, __LINE__, buffer);
    if (!strncmp(PROP_CAMERA_HAL_DUMP, buffer, strlen(PROP_CAMERA_HAL_DUMP))
        && strlen(PROP_CAMERA_HAL_DUMP) == strlen(buffer)) {
        setenv(PROP_CAMERA_HAL_DUMP, equal + 1, 1);
    } else if (!strncmp(PROP_CAMERA_HAL_DUMP_FORMAT, buffer,
        strlen(PROP_CAMERA_HAL_DUMP_FORMAT))) {
        setenv(PROP_CAMERA_HAL_DUMP_FORMAT, equal + 1, 1);
    } else if (!strncmp(PROP_CAMERA_HAL_DUMP_PATH, buffer,
        strlen(PROP_CAMERA_HAL_DUMP_PATH))) {
        setenv(PROP_CAMERA_HAL_DUMP_PATH, equal + 1, 1);
    } else if (!strncmp(PROP_CAMERA_HAL_DUMP_SKIP_NUM, buffer,
        strlen(PROP_CAMERA_HAL_DUMP_SKIP_NUM))) {
        setenv(PROP_CAMERA_HAL_DUMP_SKIP_NUM, equal + 1, 1);
    } else if (!strncmp(PROP_CAMERA_HAL_DUMP_RANGE, buffer,
        strlen(PROP_CAMERA_HAL_DUMP_RANGE))) {
        setenv(PROP_CAMERA_HAL_DUMP_RANGE, equal + 1, 1);
    } else if (!strncmp(PROP_CAMERA_HAL_DUMP_FREQUENCY, buffer,
        strlen(PROP_CAMERA_HAL_DUMP_FREQUENCY))) {
        setenv(PROP_CAMERA_HAL_DUMP_FREQUENCY, equal + 1, 1);
    } else if (!strncmp(PROP_AIQDUMP, buffer, strlen(PROP_AIQDUMP))) {
        setenv(PROP_AIQDUMP, equal + 1, 1);
    }
    setDumpLevel();

    return true;
}
}  // namespace CameraDump

}  // namespace icamera
