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

#include "iutils/Utils.h"

namespace icamera {

#define DEFAULT_JPEG_QUALITY 95
static const unsigned char mJpegMarkerSOI[2] = {0xFF, 0xD8};

struct EncodePackage {
    EncodePackage()
            : inputWidth(0),
              inputHeight(0),
              inputStride(0),
              inputFormat(0),
              inputSize(0),
              inputBufferHandle(nullptr),
              inputData(nullptr),
              outputWidth(0),
              outputHeight(0),
              outputSize(0),
              outputBufferHandle(nullptr),
              outputData(nullptr),
              quality(0),
              encodedDataSize(0),
              exifData(nullptr),
              exifDataSize(0) {}

    /* input buffer info */
    int inputWidth;
    int inputHeight;
    int inputStride;
    int inputFormat;
    unsigned int inputSize;
    void* inputBufferHandle;
    void* inputData;

    /* output buffer info */
    int outputWidth;
    int outputHeight;
    unsigned int outputSize;
    void* outputBufferHandle;
    void* outputData;

    int quality;
    uint32_t encodedDataSize;
    uint8_t* exifData;
    uint32_t exifDataSize;
};

class IJpegEncoder {
 public:
    IJpegEncoder(){};
    virtual ~IJpegEncoder(){};

    static std::unique_ptr<IJpegEncoder> createJpegEncoder();
    virtual bool doJpegEncode(EncodePackage* package) = 0;

 private:
    DISALLOW_COPY_AND_ASSIGN(IJpegEncoder);
};
}  // namespace icamera
