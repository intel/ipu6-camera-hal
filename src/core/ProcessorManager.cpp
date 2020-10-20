/*
 * Copyright (C) 2017-2020 Intel Corporation.
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

#define LOG_TAG "ProcessorManager"

#include "ProcessorManager.h"
#include "iutils/CameraLog.h"
#include "iutils/Utils.h"

#include "SwImageProcessor.h"
#include "PSysProcessor.h"

namespace icamera {

ProcessorManager::ProcessorManager(int cameraId) :
        mCameraId(cameraId),
        mPsysUsage(PSYS_NOT_USED)
{
    LOG1("@%s, cameraId:%d", __func__, mCameraId);
}

ProcessorManager::~ProcessorManager()
{
    LOG1("@%s, cameraId:%d", __func__, mCameraId);

    deleteProcessors();
}

std::vector<BufferQueue*> ProcessorManager::createProcessors(int inputFmt,
        const std::map<Port, stream_t>& producerConfigs,
        const std::map<int, Port>& streamIdToPortMap,
        stream_config_t *streamList, const Parameters& param, ParameterGenerator* paramGenerator)
{
    LOG1("@%s, mCameraId:%d", __func__, mCameraId);

    ProcessorConfig processorItem;
    processorItem.mInputConfigs = producerConfigs;
    for (const auto& item : streamIdToPortMap) {
        if (streamList->streams[item.first].streamType == CAMERA_STREAM_INPUT) continue;
        processorItem.mOutputConfigs[item.second] = streamList->streams[item.first];
    }

    // Check if PSysProcessor can be used.
    mPsysUsage = PSYS_NORMAL;
    for (int i = 0; i < streamList->num_streams; i++) {
        if (streamList->streams[i].streamType == CAMERA_STREAM_INPUT ||
            streamList->streams[i].usage == CAMERA_STREAM_OPAQUE_RAW) continue;

        if (!PlatformData::usePsys(mCameraId, streamList->streams[i].format)) {
            mPsysUsage = PSYS_NOT_USED;
            break;
        }
    }

    if (mPsysUsage == PSYS_NORMAL) {
        LOG1("Using normal Psys to do image processing.");
        processorItem.mProcessor = new PSysProcessor(mCameraId, paramGenerator);
        mProcessors.push_back(processorItem);
    }

    if (mPsysUsage == PSYS_NOT_USED) {
        LOG1("Using software to do color conversion.");
        processorItem.mProcessor = new SwImageProcessor(mCameraId);
        mProcessors.push_back(processorItem);
    }

    std::vector<BufferQueue*> processors;
    for (auto& p : mProcessors) {
        processors.push_back(p.mProcessor);
    }

    return processors;
}

int ProcessorManager::deleteProcessors()
{
    for (auto& item : mProcessors) {
        delete item.mProcessor;
    }
    mProcessors.clear();

    mPsysUsage = PSYS_NOT_USED;

    return OK;
}

/**
 * Configure processor with input and output streams
 */
int ProcessorManager::configureProcessors(const std::vector<ConfigMode>& configModes,
                                          BufferProducer* producer,
                                          const Parameters& param)
{
    LOG1("@%s, mCameraId:%d", __func__, mCameraId);

    BufferProducer* preProcess =  nullptr;
    for (auto& item : mProcessors) {
        BufferQueue* processor = item.mProcessor;
        processor->setFrameInfo(item.mInputConfigs, item.mOutputConfigs);
        processor->setParameters(param);
        int ret = processor->configure(configModes);
        CheckError(ret < 0, ret, "Configure processor failed with:%d", ret);

        processor->setBufferProducer(preProcess ? preProcess : producer);
        preProcess = processor;
    }

    return OK;
}

} // end of namespace icamera

