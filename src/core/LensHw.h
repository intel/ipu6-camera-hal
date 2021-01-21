/*
 * Copyright (C) 2016-2020 Intel Corporation
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

#include <v4l2_device.h>

#include <string>

namespace icamera {

typedef ::cros::V4L2Subdevice V4L2Subdevice;

/**
 * \class LensHw
 * This class adds the methods that are needed
 * to drive the camera lens using v4l2 commands and custom ioctl.
 *
 */
class LensHw {

public:
    LensHw(int cameraId);
    ~LensHw();

    int init();

    const char* getLensName(void);

    int setFocusPosition(int position);
    int setFocusStep(int steps);
    int getFocusPosition(int &position);
    int getFocusStatus(int &status);
    int startAutoFocus(void);
    int stopAutoFocus(void);
    int getAutoFocusStatus(int &status);
    int setAutoFocusRange(int value);
    int getAutoFocusRange(int &value);
    int getLatestPosition(int& lensPosition, unsigned long long& time);
    bool isLensSubdevAvailable() { return (mLensSubdev != nullptr); }

private:
    int mCameraId;
    V4L2Subdevice* mLensSubdev;
    std::string mLensName;
    int mLastLensPosition;
    unsigned long long mLensMovementStartTime; /*!< In microseconds */
};  // class LensHW

} // namespace icamera
