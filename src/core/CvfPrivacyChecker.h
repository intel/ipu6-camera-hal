/*
 * Copyright (C) 2022 Intel Corporation.
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
#include <string>

#include "iutils/Errors.h"
#include "iutils/CameraLog.h"
#include "iutils/Thread.h"

#include "CameraStream.h"
#include "CameraBuffer.h"
#include "PlatformData.h"
#include "V4l2DeviceFactory.h"

namespace icamera {

class CvfPrivacyChecker : public Thread {
    const int mCameraId;
    const uint32_t mInterval;  // us
    int mPrivacy;
    CameraStream** mCameraStreams;
    V4L2Subdevice* mPixelArraySubdev;

 public:
    CvfPrivacyChecker(int _id, CameraStream** _streams,
                      uint32_t _interval = 67000) :
                        mCameraId(_id),
                        mInterval(_interval),
                        mPrivacy(-1),
                        mCameraStreams(_streams),
                        mPixelArraySubdev(nullptr) {}

    int init() {
        std::string subDevName;
        int ret = PlatformData::getDevNameByType(mCameraId, VIDEO_PIXEL_ARRAY,
                                                 subDevName);
        if (ret == OK) {
            LOG1("%s: ArraySubdev camera id:%d dev name:%s",
                 __PRETTY_FUNCTION__, mCameraId, subDevName.c_str());
            mPixelArraySubdev = V4l2DeviceFactory::getSubDev(mCameraId, subDevName);
        } else {
            LOG1("%s: Can't get pixel array subdevice. camera id:%d, return: %d",
                 __PRETTY_FUNCTION__, mCameraId, ret);
        }
        return ret;
    }

    /**
     * \brief Check CVF privacy status and process
     */
    bool threadLoop() {
        usleep(mInterval);
        int privacy = checkPrivacyStatus();
        LOG2("%s: privacy is %d", __PRETTY_FUNCTION__, privacy);

        if (privacy == 1 && mPrivacy < 1) {
            for (int i = 0; i < MAX_STREAM_NUMBER; ++i) {
                LOGI("%s: mCameraStreams[%d] == %p", __PRETTY_FUNCTION__, i,
                     mCameraStreams[i]);
                if (mCameraStreams[i]) {
                    auto buf = mCameraStreams[i]->getPrivacyBuffer();
                    auto port = mCameraStreams[i]->getPort();
                    if (buf == nullptr) {
                        LOGI("%s: getPrivacyBuffer returned nullptr",
                             __PRETTY_FUNCTION__);
                        return true;
                    }
                    setPrivacyImage(buf);
                    mCameraStreams[i]->doFrameAvailable(port, buf);
                    setPrivacy(1);
                }
            }
        } else if (privacy == 0 && mPrivacy != 0) {
            setPrivacy(0);
        }
        return true;
    }

 private:
    int checkPrivacyStatus() {
        if (!mPixelArraySubdev) {
            return -1;
        }
        int privacy = -1;
        int status = mPixelArraySubdev->GetControl(V4L2_CID_PRIVACY, &privacy);
        CheckAndLogError(status != OK, status,
                         "Couldn't get V4L2_CID_PRIVACY, status:%d", status);
        return privacy;
    }

    /**
     * \brief Update mPrivacy status
     */
    void setPrivacy(const int p) {
        mPrivacy = p;
        LOG1("%s: change privacy status to %d", __func__, mPrivacy);
    }

    /**
     * \brief Set the CameraBuffer to privacy image
     */
    void setPrivacyImage(std::shared_ptr<CameraBuffer>& buf) {
        // TODO: handle YUY2 buffer. This can only set NV12 buffer to black.
        uint32_t width = buf->getWidth();
        uint32_t height = buf->getHeight();
        uint32_t offset = width * height;

        memset(buf->getBufferAddr(), 0, offset);
        memset(reinterpret_cast<char*>(buf->getBufferAddr()) + offset, 128,
               offset / 2);
        LOG1("%s: set %p size %u to black", __func__, buf->getBufferAddr(),
             buf->getBufferSize());
    }

    DISALLOW_COPY_AND_ASSIGN(CvfPrivacyChecker);
};

}  // namespace icamera
