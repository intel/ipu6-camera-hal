/*
 * Copyright (C) 2020 Intel Corporation.
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

/*
 *
 * Filename: LiveTuning.h
 *
 * ------------------------------------------------------------------------------
 * REVISION HISTORY
 *******************************************************************************
 *     Version        0.10       Initialize Live Tuning HAL API
 *******************************************************************************
 *
 */

#pragma once

#include "ICamera.h"
#include "Parameters.h"

#include <stdlib.h>  // For including definition of NULL
#include <vector>

#include "cros-camera/camera_algorithm_bridge.h"

namespace icamera {

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief
 *   Setup IPC environment to init Livetune.
 *
 * \note
 *   Call the API once when live tuning camera engine constructed.
 *
 * \param[in/out]
 *   cros::CameraMojoChannelManager* mojoManager: pointer of CameraMojoChannelManager.
 *
 * \return
 *   0 means the api run succeed.
 * \return
 *   non-0 means the api run failed.
 *
 */
int setupIPCEnvForLiveTune(cros::CameraMojoChannelManager* mojoManager);

/**
 * \brief
 *   Tear Down IPC environment to deinit Livetune.
 *
 * \note
 *   Call the API once when live tuning camera engine deconstructed.
 *
 * \return
 *   0 means the api run succeed.
 * \return
 *   non-0 means the api run failed.
 *
 */
int tearDownIPCEnvForLiveTune();

/**
 * \brief
 *   Get the supported raw info from isys.
 *
 * \note
 *   Call the API after camera_device_config_streams.
 *
 * \param[in]
 *   int cameraId: ID of the camera, [0, MAX_CAMERA_NUMBER - 1]
 *
 * \param[out]
 *   int* width: width of raw.
 *
 * \param[out]
 *   int* height: height of raw.
 *
 * \param[out]
 *   int* format: format of raw.
 *
 * \param[out]
 *   int* mcId: id of media ctrl configuration.
 *
 * \return
 *   0 succeed to get the isys supported raw info.
 * \return
 *   non-0 error code, failed to get the isys supported raw info.
 *
 */
int getSupportedRawInfo(int cameraId, int* width, int* height, int* format, int* mcId);

/**
 * \brief
 *   Get Makernote data.
 *
 * \note
 *   It can be called after capture done
 *
 * \param[in]
 *   int cameraId: ID of the camera, [0, MAX_CAMERA_NUMBER - 1]
 *
 * \param[in]
 *   int64_t sequence: the frame buffer sequence
 *
 * \param[in]
 *   uint64_t timestamp: the frame buffer timestamp
 *
 * \param[in/out]
 *   icamera::Parameters* param: input parameters
 *
 * \return
 *   0 succeed to get Makernote Data
 * \return
 *   non-0 error code, failed to get Makernote Data
 *
 */
int acquireMakernoteData(int cameraId, int64_t sequence, uint64_t timestamp,
                         icamera::Parameters* param);

#ifdef __cplusplus
}
#endif

}  // namespace icamera
