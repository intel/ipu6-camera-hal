/*
 * Copyright (C) 2022-2023 Intel Corporation.
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

#include "CameraBuffer.h"
#include "CameraStream.h"
#include "iutils/Thread.h"
#include "V4l2DeviceFactory.h"

namespace icamera {

class CvfPrivacyChecker : public Thread, public EventListener {
 private:
    const int mCameraId;
    const uint32_t mInterval;  // us
    bool mPrivacyOn;
    CameraStream** mCameraStreams;
    V4L2Subdevice* mPixelArraySubdev;
    uint32_t mThreshold;   // Threshold for camera enter AE based privacy mode
    uint32_t mFrameDelay;  // Frame delay count before camera enter AE based privacy mode
    uint32_t mFrameCount;
    void* mPrivacyImageBuf;

 public:
    CvfPrivacyChecker(int _id, CameraStream** _streams, uint32_t _interval = 67000);
    virtual ~CvfPrivacyChecker();

    /**
     * \brief Init the mPixelArraySubdev
     */
    int init();
    void handleEvent(EventData eventData) override;

    /**
     * \brief Check CVF privacy status and process
     */
    bool threadLoop() override;

 private:
    bool checkPrivacyStatus();

    /**
     * \brief Update mPrivacyOn status
     */
    void setPrivacy(const bool p);

    /**
     * \brief Set the CameraBuffer to privacy image
     */
    void setPrivacyImage(std::shared_ptr<CameraBuffer>& buf);

    DISALLOW_COPY_AND_ASSIGN(CvfPrivacyChecker);
};

}  // namespace icamera
