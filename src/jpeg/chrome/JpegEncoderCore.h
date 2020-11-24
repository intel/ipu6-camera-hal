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

#include <memory>
#include <mutex>

#include "IJpegEncoder.h"
#include "camera/include/cros-camera/jpeg_compressor.h"
#include "iutils/Utils.h"

namespace icamera {

class JpegEncoderCore : public IJpegEncoder {
 public:
    JpegEncoderCore();
    ~JpegEncoderCore();

    virtual bool doJpegEncode(EncodePackage* pa);

 private:
    DISALLOW_COPY_AND_ASSIGN(JpegEncoderCore);

    std::unique_ptr<cros::JpegCompressor> mJpegCompressor;
};
}  // namespace icamera
