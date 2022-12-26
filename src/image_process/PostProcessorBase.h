/*
 * Copyright (C) 2019 Intel Corporation.
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
#include "iutils/Errors.h"
#include "iutils/Utils.h"
#include "Parameters.h"
#include "Camera3Buffer.h"
#include "IImageProcessor.h"
#include "EXIFMetaData.h"
#include "IJpegEncoder.h"
#include "JpegMaker.h"

namespace icamera {

class PostProcessorBase {
public:
    PostProcessorBase(std::string processName);
    virtual ~PostProcessorBase();

    std::string getName() { return mName; }
    virtual status_t doPostProcessing(const std::shared_ptr<camera3::Camera3Buffer> &inBuf,
                                      std::shared_ptr<camera3::Camera3Buffer> &outBuf) { return OK; }

    virtual status_t doPostProcessing(const std::shared_ptr<camera3::Camera3Buffer> &inBuf,
                                      const Parameters &parameter,
                                      std::shared_ptr<camera3::Camera3Buffer> &outBuf) { return OK; }
private:
    DISALLOW_COPY_AND_ASSIGN(PostProcessorBase);

protected:
    std::string mName;
    std::unique_ptr<IImageProcessor> mProcessor;
};

class ScaleProcess : public PostProcessorBase {
public:
    ScaleProcess();
    ~ScaleProcess() {};

    virtual status_t doPostProcessing(const std::shared_ptr<camera3::Camera3Buffer> &inBuf,
                                      std::shared_ptr<camera3::Camera3Buffer> &outBuf);
};

class RotateProcess : public PostProcessorBase {
public:
    RotateProcess(int angle);
    ~RotateProcess() {};

    virtual status_t doPostProcessing(const std::shared_ptr<camera3::Camera3Buffer> &inBuf,
                                      std::shared_ptr<camera3::Camera3Buffer> &outBuf);
private:
    int mAngle;
};

class CropProcess : public PostProcessorBase {
public:
    CropProcess();
    ~CropProcess() {};

    virtual status_t doPostProcessing(const std::shared_ptr<camera3::Camera3Buffer> &inBuf,
                                      std::shared_ptr<camera3::Camera3Buffer> &outBuf);
};

class ConvertProcess : public PostProcessorBase {
public:
    ConvertProcess();
    ~ConvertProcess() {};

    virtual status_t doPostProcessing(const std::shared_ptr<camera3::Camera3Buffer> &inBuf,
                                      std::shared_ptr<camera3::Camera3Buffer> &outBuf);
};

class JpegProcess : public PostProcessorBase {
public:
    JpegProcess(int cameraId);
    ~JpegProcess();

    virtual status_t doPostProcessing(const std::shared_ptr<camera3::Camera3Buffer> &inBuf,
                                      const Parameters &parameter,
                                      std::shared_ptr<camera3::Camera3Buffer> &outBuf);
private:
    void attachJpegBlob(const EncodePackage &package);
    std::shared_ptr<camera3::Camera3Buffer>
    cropAndDownscaleThumbnail(int thumbWidth, int thumbHeight,
                              const std::shared_ptr<camera3::Camera3Buffer> &inBuf);
    void fillEncodeInfo(const std::shared_ptr<camera3::Camera3Buffer> &inBuf,
                        const std::shared_ptr<camera3::Camera3Buffer> &outBuf,
                        EncodePackage &package);

private:
    int mCameraId;

    std::shared_ptr<camera3::Camera3Buffer> mCropBuffer;
    std::shared_ptr<camera3::Camera3Buffer> mScaleBuffer;
    std::shared_ptr<camera3::Camera3Buffer> mThumbOutput;

    std::unique_ptr<JpegMaker> mJpegMaker;
    std::unique_ptr<IJpegEncoder> mJpegEncoder;
    std::unique_ptr<unsigned char[]> mExifData;
};

} // namespace icamera
