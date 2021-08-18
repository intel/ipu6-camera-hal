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

#pragma once

#include <vector>
#include <map>

#include "iutils/Utils.h"
#include "iutils/Errors.h"
#include "PostProcessorBase.h"
#include "ProcessType.h"

namespace icamera {

struct PostProcessInfo {
    stream_t inputInfo;
    stream_t outputInfo;
    PostProcessType type;
    int angle;
    PostProcessInfo() : type(POST_PROCESS_NONE),angle(0) { CLEAR(inputInfo); CLEAR(outputInfo); }
};

/**
 * \class PostProcessorCore
 *
 * This class is used to encode JPEG and rotate image.
 *
 */
class PostProcessorCore {

public:
    PostProcessorCore(int cameraId);
    virtual ~PostProcessorCore();

    bool isPostProcessTypeSupported(PostProcessType type);
    status_t configure(const std::vector<PostProcessInfo> &processorOrder);
    status_t doPostProcessing(const std::shared_ptr<camera3::Camera3Buffer> &mainBuf,
                              const Parameters &parameter,
                              std::shared_ptr<camera3::Camera3Buffer> outBuf);
private:
    status_t createProcessor();
    status_t allocateBuffers();

private:
    DISALLOW_COPY_AND_ASSIGN(PostProcessorCore);

private:
    int mCameraId;
    std::map<std::shared_ptr<PostProcessorBase>, std::shared_ptr<camera3::Camera3Buffer>>
            mInterBuffers;
    std::vector<PostProcessInfo> mProcessorsInfo;
    std::vector<std::shared_ptr<PostProcessorBase>> mProcessorVector;
};
} // namespace icamera
