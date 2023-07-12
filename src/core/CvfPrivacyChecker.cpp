/*
 * Copyright (C) 2023 Intel Corporation.
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

#define LOG_TAG CvfPrivacyChecker

#include "CvfPrivacyChecker.h"

#include <algorithm>
#include <fstream>
#include <memory>
#include <string>

#include "CameraBuffer.h"
#include "CameraStream.h"
#include "iutils/CameraLog.h"
#include "iutils/Errors.h"
#include "iutils/Thread.h"
#include "PlatformData.h"
#include "V4l2DeviceFactory.h"

namespace icamera {
CvfPrivacyChecker::CvfPrivacyChecker(int _id, CameraStream** _streams, uint32_t _interval)
        : mCameraId(_id),
          mInterval(_interval),
          mPrivacyOn(false),
          mCameraStreams(_streams),
          mPixelArraySubdev(nullptr),
          mThreshold(10U),
          mFrameDelay(5U),
          mFrameCount(0U),
          mPrivacyImageBuf(nullptr) {
    if (PlatformData::getSupportPrivacy(_id) == AE_BASED_PRIVACY_MODE) {
        mThreshold = PlatformData::getPrivacyModeThreshold(_id);
        mFrameDelay = PlatformData::getPrivacyModeFrameDelay(_id);
    }
}

CvfPrivacyChecker::~CvfPrivacyChecker() {
    if (mPrivacyImageBuf) {
        free(mPrivacyImageBuf);
    }
}

int CvfPrivacyChecker::init() {
    std::string subDevName;
    int ret = PlatformData::getDevNameByType(mCameraId, VIDEO_PIXEL_ARRAY, subDevName);
    if (ret == OK) {
        LOG1("%s: ArraySubdev camera id:%d dev name:%s", __func__, mCameraId, subDevName.c_str());
        mPixelArraySubdev = V4l2DeviceFactory::getSubDev(mCameraId, subDevName);
    } else {
        LOG1("%s: Can't get pixel array subdevice. camera id:%d, return: %d", __func__, mCameraId,
             ret);
    }
    return ret;
}

void CvfPrivacyChecker::handleEvent(EventData eventData) {
    LOG2("privacy event: %d", eventData.type);
    switch (eventData.type) {
        case EVENT_3A_READY:
            if (mPrivacyOn) {
                if (eventData.data.run3AReady.maxBin >= mThreshold) {
                    if (mFrameCount > mFrameDelay) {
                        setPrivacy(false);
                        mFrameCount = 0U;
                    } else {
                        mFrameCount++;
                    }
                } else {
                    mFrameCount = 0U;
                }
            } else {
                if (eventData.data.run3AReady.maxBin < mThreshold) {
                    if (mFrameCount > mFrameDelay) {
                        setPrivacy(true);
                        mFrameCount = 0U;
                    } else {
                        mFrameCount++;
                    }
                } else {
                    mFrameCount = 0U;
                }
            }
            break;
        case EVENT_FRAME_AVAILABLE:
            if (mPrivacyOn) {
                setPrivacyImage(eventData.buffer);
            }
        default:
            break;
    }
}

bool CvfPrivacyChecker::threadLoop() {
    usleep(mInterval);
    bool privacy = checkPrivacyStatus();
    LOG2("%s: privacy is %d", __func__, privacy);

    if (privacy && !mPrivacyOn) {
        for (int i = 0; i < MAX_STREAM_NUMBER; ++i) {
            LOGI("%s: mCameraStreams[%d] == %p", __func__, i, mCameraStreams[i]);
            if (mCameraStreams[i]) {
                auto buf = mCameraStreams[i]->getPrivacyBuffer();
                auto port = mCameraStreams[i]->getPort();
                if (buf == nullptr) {
                    LOGI("%s: getPrivacyBuffer returned nullptr", __func__);
                    return true;
                }
                setPrivacyImage(buf);
                mCameraStreams[i]->doFrameAvailable(port, buf);
                setPrivacy(true);
            }
        }
    } else if (!privacy && mPrivacyOn) {
        setPrivacy(false);
    }
    return true;
}

bool CvfPrivacyChecker::checkPrivacyStatus() {
    if (!mPixelArraySubdev) {
        return false;
    }
    int privacy = -1;
    int status = mPixelArraySubdev->GetControl(V4L2_CID_PRIVACY, &privacy);
    CheckAndLogError(status != OK, status, "Couldn't get V4L2_CID_PRIVACY, status:%d", status);
    return (privacy == 1);
}

void CvfPrivacyChecker::setPrivacy(const bool p) {
    mPrivacyOn = p;
    LOG1("%s: change privacy status to %d", __func__, mPrivacyOn);
}

void CvfPrivacyChecker::setPrivacyImage(std::shared_ptr<CameraBuffer>& buf) {
    uint32_t width = buf->getWidth();
    uint32_t height = buf->getHeight();
    uint32_t bufferSize = buf->getBufferSize();

    if (!mPrivacyImageBuf) {
        // if we didn't load privacy image, allocate memory
        mPrivacyImageBuf = malloc(bufferSize);
        if (mPrivacyImageBuf) {
            // if we allocated memory, load image file
            char fileName[256];
            snprintf(fileName, sizeof(fileName), "%s/privacy_image_%s_%u_%u.yuv",
                     PlatformData::getCameraCfgPath().c_str(),
                     PlatformData::getSensorName(mCameraId), width, height);

            std::ifstream file(fileName, std::ios::in | std::ios::binary | std::ios::ate);
            CheckWarningNoReturn(!file.is_open(), "Cannot open privacy image file: %s", fileName);
            if (file.is_open()) {
                uint32_t fileSize = file.tellg();
                file.seekg(0, std::ios::beg);
                file.read(reinterpret_cast<char*>(mPrivacyImageBuf),
                          std::min(fileSize, bufferSize));
            } else {
                // if can't load img, set privacy image as black
                // TODO: handle YUY2 buffer. This can only set NV12 buffer to black.
                uint32_t offset = width * height;
                memset(reinterpret_cast<char*>(mPrivacyImageBuf), 0, offset);
                memset(reinterpret_cast<char*>(mPrivacyImageBuf) + offset, 128, offset / 2);
            }
        } else {
            uint32_t offset = width * height;
            memset(reinterpret_cast<char*>(buf->getBufferAddr()), 0, offset);
            memset(reinterpret_cast<char*>(buf->getBufferAddr()) + offset, 128, offset / 2);
            return;
        }
    }

    MEMCPY_S(reinterpret_cast<char*>(buf->getBufferAddr()), bufferSize, mPrivacyImageBuf,
             bufferSize);
}

}  // namespace icamera
