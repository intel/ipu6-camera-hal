/*
 * Copyright (C) 2016-2019 Intel Corporation
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

namespace icamera {

// Temporary solution
enum StreamUseCase {
    USE_CASE_COMMON = 0,
    USE_CASE_PREVIEW = 1 << 0,        // For HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED
    USE_CASE_VIDEO = 1 << 1,          // For HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED
    USE_CASE_STILL_CAPTURE = 1 << 2,  // For HAL_PIXEL_FORMAT_BLOB/HAL_PIXEL_FORMAT_YCbCr_420_888
    USE_CASE_RAW = 1 << 3,            // For HAL_PIXEL_FORMAT_RAW16/HAL_PIXEL_FORMAT_RAW_OPAQUE
    USE_CASE_ZSL = 1 << 4,            // For ZSL stream
    USE_CASE_INPUT = 1 << 5,           // For input stream
};

struct streamProps {
    uint32_t width;
    uint32_t height;
    int format;
    int streamId;
    StreamUseCase useCase;
};

class HalStream
{
 public:
    HalStream(struct streamProps &props, void *priv):
        mWidth(props.width),
        mHeight(props.height),
        mFormat(props.format),
        mStreamId(props.streamId),
        mUseCase(props.useCase)
    {
        maxBuffers = 0;
        mPrivate = priv;
    }

    ~HalStream() { }

    uint32_t width() const { return mWidth; }
    uint32_t height() const { return mHeight; }
    int format() const { return mFormat; }
    int streamId() const { return mStreamId; }
    StreamUseCase useCase() const { return mUseCase; }
    void *priv() { return mPrivate; }

 public:
    uint32_t mWidth;
    uint32_t mHeight;
    int mFormat;  // TODO: use v4l2 definition
    int mStreamId;
    StreamUseCase mUseCase;

    int maxBuffers;
    void *mPrivate;
};

} /* namespace icamera */
