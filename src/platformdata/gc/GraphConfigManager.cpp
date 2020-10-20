/*
 * Copyright (C) 2015-2020 Intel Corporation
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

#define LOG_TAG "GraphConfigManager"

#include "src/platformdata/gc/GraphConfigManager.h"

#include "iutils/Utils.h"
#include "iutils/CameraLog.h"
#include "PlatformData.h"

using std::vector;
using std::map;

namespace icamera {
GraphConfigManager::GraphConfigManager(int32_t cameraId) :
    mGcConfigured(false),
    mCameraId(cameraId),
    mMcId(-1)
{
}

GraphConfigManager::~GraphConfigManager()
{
    mGraphConfigMap.clear();
    mGcConfigured = false;
    releaseHalStream();
}

void GraphConfigManager::releaseHalStream()
{
    for(auto &halStream : mHalStreamVec) {
        delete halStream;
    }
    mHalStreamVec.clear();
}

/*
 * Get the useCase from the stream and operationMode.
 */
StreamUseCase GraphConfigManager::getUseCaseFromStream(ConfigMode configMode, const stream_t &stream)
{
    if (configMode == CAMERA_STREAM_CONFIGURATION_MODE_STILL_CAPTURE ||
            stream.usage == CAMERA_STREAM_STILL_CAPTURE)
        return USE_CASE_STILL_CAPTURE;

    return USE_CASE_PREVIEW;
}

/**
 * Initialize the state of the GraphConfigManager after parsing the stream
 * configuration.
 * Perform the first level query to find a subset of settings that fulfill the
 * constrains from the stream configuration.
 *
 * \param[in] streamList: all the streams info.
 */
status_t GraphConfigManager::configStreams(const stream_config_t *streamList)
{
    HAL_TRACE_CALL(CAMERA_DEBUG_LOG_LEVEL1);
    CheckError(!streamList, BAD_VALUE, "%s: Null streamList configured", __func__);

    vector <ConfigMode> configModes;
    int ret = PlatformData::getConfigModesByOperationMode(mCameraId, streamList->operation_mode, configModes);
    CheckError(ret != OK, ret, "%s, get ConfigMode failed %d", __func__, ret);

    // Convert the stream_t to HalStream
    // Use the stream list with descending order to find graph settings.
    releaseHalStream();
    for (int i = 0; i < streamList->num_streams; i++) {
        // Don't handle input stream or opaque RAW stream when configure graph configuration.
        if (streamList->streams[i].streamType == CAMERA_STREAM_INPUT ||
            streamList->streams[i].usage == CAMERA_STREAM_OPAQUE_RAW) continue;

        bool stored = false;
        StreamUseCase useCase = getUseCaseFromStream(configModes[0], streamList->streams[i]);
        streamProps props = {
            static_cast<uint32_t>(streamList->streams[i].width),
            static_cast<uint32_t>(streamList->streams[i].height),
            streamList->streams[i].format,
            streamList->streams[i].id,
            useCase,
        };
        HalStream* halStream = new HalStream(props, static_cast<void*>(&streamList->streams[i]));
        CheckError(!halStream, UNKNOWN_ERROR, "Failed to create hal stream");

        for (size_t j = 0; j < mHalStreamVec.size(); j++) {
            if (halStream->width() * halStream->height() > mHalStreamVec[j]->width() * mHalStreamVec[j]->height()) {
                stored = true;
                mHalStreamVec.insert((mHalStreamVec.begin() + j), halStream);
                break;
            }
        }
        if (!stored)
            mHalStreamVec.push_back(halStream);
    }

    //debug
    dumpStreamConfig();
    mGraphConfigMap.clear();
    mMcId = -1;

    for (auto mode : configModes) {
        LOG1("Mapping the operationMode %d to ConfigMode %d", streamList->operation_mode, mode);

        std::shared_ptr<GraphConfig> graphConfig = std::make_shared<GraphConfig>(mCameraId, mode);
        ret = graphConfig->configStreams(mHalStreamVec);
        CheckWarning(ret != OK, ret, "%s, Failed to configure graph: real ConfigMode %x", __func__, mode);

        int id = graphConfig->getSelectedMcId();
        CheckError((id != -1 && mMcId != -1 && mMcId != id), UNKNOWN_ERROR,
                    "Not support two different MC ID at same time:(%d/%d)", mMcId, id);
        mMcId = id;
        LOGG("%s: Add graph setting for op_mode %d", __func__, mode);
        mGraphConfigMap[mode] = graphConfig;
    }

    mGcConfigured = true;
    return OK;
}

std::shared_ptr<IGraphConfig> GraphConfigManager::getGraphConfig(ConfigMode configMode)
{
    for (auto& gc : mGraphConfigMap) {
        if (gc.first == configMode) {
            LOGG("%s: found graph config for mode %d", __func__, configMode);
            return gc.second;
        }
    }

    return nullptr;
}

void GraphConfigManager::dumpStreamConfig()
{
    for (size_t i = 0; i < mHalStreamVec.size(); i++) {
        LOG1("stream[%zu] %dx%d, fmt %s", i,
             mHalStreamVec[i]->width(), mHalStreamVec[i]->height(),
             CameraUtils::pixelCode2String(mHalStreamVec[i]->format()));
    }
}

map<int, IGraphConfigManager*> IGraphConfigManager::sInstances;
Mutex IGraphConfigManager::sLock;

IGraphConfigManager* IGraphConfigManager::getInstance(int cameraId)
{
    AutoMutex lock(sLock);
    if (sInstances.find(cameraId) != sInstances.end()) {
        return sInstances[cameraId];
    }

    sInstances[cameraId] = new GraphConfigManager(cameraId);
    return sInstances[cameraId];
}

void IGraphConfigManager::releaseInstance(int cameraId)
{
    AutoMutex lock(sLock);
    if (sInstances.find(cameraId) != sInstances.end()) {
        IGraphConfigManager* gcManager = sInstances[cameraId];
        sInstances.erase(cameraId);
        delete gcManager;
    }
}
}  // icamera
