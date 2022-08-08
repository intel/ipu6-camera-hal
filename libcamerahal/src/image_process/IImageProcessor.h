/*
 * Copyright (C) 2019 Intel Corporation
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

#include "iutils/Utils.h"
#include "iutils/Errors.h"
#include "Camera3Buffer.h"
#include "ProcessType.h"

namespace icamera {

class IImageProcessor {
public:
    IImageProcessor() {};
    virtual ~IImageProcessor() {};

    static std::unique_ptr<IImageProcessor> createImageProcessor();
    static bool isProcessingTypeSupported(PostProcessType type);

    virtual status_t cropFrame(const std::shared_ptr<camera3::Camera3Buffer> &input,
                               std::shared_ptr<camera3::Camera3Buffer> &output) = 0;
    virtual status_t scaleFrame(const std::shared_ptr<camera3::Camera3Buffer> &input,
                                std::shared_ptr<camera3::Camera3Buffer> &output) = 0;
    virtual status_t rotateFrame(const std::shared_ptr<camera3::Camera3Buffer> &input,
                                 std::shared_ptr<camera3::Camera3Buffer> &output,
                                 int angle, std::vector<uint8_t> &rotateBuf) = 0;
    virtual status_t convertFrame(const std::shared_ptr<camera3::Camera3Buffer> &input,
                                  std::shared_ptr<camera3::Camera3Buffer> &output) = 0;
private:
    DISALLOW_COPY_AND_ASSIGN(IImageProcessor);
};
} // namespace icamera
