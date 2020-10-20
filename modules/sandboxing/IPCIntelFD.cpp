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

#define LOG_TAG "IPC_FACE_DETECTION"

#include "modules/sandboxing/IPCIntelFD.h"

#include "CameraLog.h"
#include "iutils/Errors.h"
#include "iutils/Utils.h"

namespace icamera {
IPCIntelFD::IPCIntelFD() {
    LOG1("@%s", __func__);
}

IPCIntelFD::~IPCIntelFD() {
    LOG1("@%s", __func__);
}

bool IPCIntelFD::clientFlattenInit(unsigned int max_face_num, FaceDetectionInitParams* params) {
    LOG1("@%s, params:%p", __func__, params);
    CheckError(params == nullptr, false, "@%s, params is nullptr", __func__);

    params->max_face_num = max_face_num;

    return true;
}

bool IPCIntelFD::serverUnflattenRun(const FaceDetectionRunParams& inParams, void* imageData,
                                    pvl_image* image) {
    LOG1("@%s, image:%p", __func__, image);
    CheckError(image == nullptr, false, "@%s, iamge is nullptr", __func__);

    image->size = inParams.size;
    image->width = inParams.width;
    image->height = inParams.height;
    image->format = inParams.format;
    image->stride = inParams.stride;
    image->rotation = inParams.rotation;

    if (imageData) {
        image->data = const_cast<uint8_t*>(static_cast<uint8_t*>(imageData));
    } else {
        image->data = const_cast<uint8_t*>(inParams.data);
    }

    return true;
}
} /* namespace icamera */
