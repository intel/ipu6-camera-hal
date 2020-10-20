/*
 * Copyright (C) 2019-2020 Intel Corporation
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

#define LOG_TAG "PostProcessorCore"

#include "iutils/CameraLog.h"
#include "PostProcessorCore.h"

using std::shared_ptr;

namespace icamera {

PostProcessorCore::PostProcessorCore(int cameraId) :
    mCameraId(cameraId)
{
    LOG1("@%s, cameraId: %d", __func__, mCameraId);
}

PostProcessorCore::~PostProcessorCore()
{
    LOG1("@%s", __func__);
}

bool PostProcessorCore::isPostProcessTypeSupported(PostProcessType type)
{
    return IImageProcessor::isProcessingTypeSupported(type);
}

status_t PostProcessorCore::createProcessor()
{
    LOG2("@%s, cameraId: %d", __func__, mCameraId);

    mProcessorVector.clear();
    for (const auto &order : mProcessorsInfo) {
        shared_ptr<PostProcessorBase> processor = nullptr;
        switch (order.type) {
        case POST_PROCESS_SCALING :
            processor = std::make_shared<ScaleProcess>();
            break;
        case POST_PROCESS_ROTATE :
            processor = std::make_shared<RotateProcess>(order.angle);
            break;
        case POST_PROCESS_CROP :
            processor = std::make_shared<CropProcess>();
            break;
        case POST_PROCESS_CONVERT :
            processor = std::make_shared<ConvertProcess>();
            break;
        case POST_PROCESS_JPEG_ENCODING :
            processor = std::make_shared<JpegProcess>(mCameraId);
            break;
        case POST_PROCESS_NONE:
            break;
        default:
            LOGE("%s, Doesn't support this kind of post-processor");
            return UNKNOWN_ERROR;
        }

        CheckError(!processor, UNKNOWN_ERROR, "%s, Failed to create the post processor: 0x%x", __func__, order.type);
        mProcessorVector.push_back(processor);
    }

    LOG2("%s, the number of post processor unit is %zu", __func__, mProcessorVector.size());
    return OK;
}

status_t PostProcessorCore::allocateBuffers()
{
    LOG2("@%s,mProcessorVector.size: %zu cameraId: %d", __func__, mProcessorVector.size(), mCameraId);

    mInterBuffers.clear();
    for (size_t i = 0; i < mProcessorsInfo.size() - 1; i++) {
        const stream_t &info = mProcessorsInfo[i].outputInfo;
        shared_ptr<camera3::Camera3Buffer> buf = camera3::MemoryUtils::allocateHeapBuffer(info.width, info.height,
                                                                                          info.stride, info.format,
                                                                                          mCameraId, info.size);
        CheckError(!buf, NO_MEMORY, "@%s: Failed to allocate internal buffer: processor: %s",
              __func__, mProcessorVector[i]->getName().c_str());
        mInterBuffers[mProcessorVector[i]] = buf;
    }

    return OK;
}

status_t PostProcessorCore::configure(const std::vector<PostProcessInfo> &processorOrder)
{
    if (processorOrder.empty())
        return OK;

    mProcessorsInfo = processorOrder;
    int ret = createProcessor();
    CheckError(ret != OK, ret, "%s, Failed to create the post processor", __func__);

    ret = allocateBuffers();
    CheckError(ret != OK, ret, "%s, Failed allocate the internal buffers", __func__);

    return OK;
}

status_t PostProcessorCore::doPostProcessing(const shared_ptr<camera3::Camera3Buffer> &inBuf,
                                             const Parameters &parameter,
                                             shared_ptr<camera3::Camera3Buffer> outBuf)
{
    CheckError(!inBuf, UNKNOWN_ERROR, "%s, the inBuf is nullptr", __func__);
    CheckError(!outBuf, UNKNOWN_ERROR, "%s, the outBuf is nullptr", __func__);

    shared_ptr<camera3::Camera3Buffer> input = inBuf;
    shared_ptr<camera3::Camera3Buffer> output = nullptr;
    for (size_t i = 0; i < mProcessorVector.size(); i++) {
        if (i == (mProcessorVector.size() - 1)) {
            output = outBuf;
        } else {
            output = mInterBuffers[mProcessorVector[i]];
        }

        int ret = OK;
        if (mProcessorsInfo[i].type == POST_PROCESS_JPEG_ENCODING)
            ret = mProcessorVector[i]->doPostProcessing(input, parameter, output);
        else
            ret = mProcessorVector[i]->doPostProcessing(input, output);
        CheckError(ret != OK, ret, "%s, Failed to do post processing: %s", __func__, mProcessorVector[i]->getName().c_str());

        input = output;
    }

    return OK;
}
} // namespace icamera
