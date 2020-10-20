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

#define LOG_TAG "IntelFaceDetection"
#include "modules/algowrapper/IntelFaceDetection.h"

#include <string.h>

#include <algorithm>

#include "AiqUtils.h"
#include "PlatformData.h"
#include "iutils/CameraLog.h"

namespace icamera {
IntelFaceDetection::IntelFaceDetection() : mFDHandle(nullptr), mMaxFacesNum(0) {
    LOG1("@%s", __func__);
}

IntelFaceDetection::~IntelFaceDetection() {
    LOG1("@%s", __func__);
}

status_t IntelFaceDetection::init(FaceDetectionInitParams* pData, int dataSize) {
    LOG1("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(!pData, UNKNOWN_ERROR, "pData is nullptr");
    CheckError(dataSize < static_cast<int>(sizeof(FaceDetectionInitParams)), UNKNOWN_ERROR,
               "buffer is small");

    mMaxFacesNum = std::min(pData->max_face_num, static_cast<unsigned int>(MAX_FACES_DETECTABLE));
    LOG2("@%s, mMaxFacesNum:%d", __func__, mMaxFacesNum);

    pvl_err faceRet = pvl_face_detection_create(nullptr, &mFDHandle);
    CheckError(faceRet != pvl_success, UNKNOWN_ERROR,
               "@%s, Failed to pvl_face_detection_create,faceRet:%d", __func__, faceRet);

    pvl_face_detection_parameters params = {};
    pvl_face_detection_default_parameters(normal, &params);
    params.max_num_faces = mMaxFacesNum;
    faceRet = pvl_face_detection_set_parameters(mFDHandle, &params);
    CheckError(faceRet != pvl_success, UNKNOWN_ERROR,
               "@%s, Failed to pvl_face_detection_set_paramters,faceRet:%d", __func__, faceRet);

    LOG2("@%s, max_num_faces:%d, rip_range:%d, num_rollover_frames:%d", __func__,
         params.max_num_faces, params.rip_range, params.num_rollover_frames);

    return OK;
}

status_t IntelFaceDetection::deinit() {
    LOG1("@%s", __func__);

    if (mFDHandle) {
        pvl_face_detection_destroy(mFDHandle);
        mFDHandle = nullptr;
    }
    return OK;
}

void IntelFaceDetection::convertCoordinate(int faceId, int width, int height, const pvl_rect& src,
                                           pvl_rect* dst) {
    CheckError(!dst, VOID_VALUE, "dst is nullptr");

    const camera_coordinate_system_t iaCoordinate = {IA_COORDINATE_LEFT, IA_COORDINATE_TOP,
                                                     IA_COORDINATE_RIGHT, IA_COORDINATE_BOTTOM};
    const camera_coordinate_system_t faceCoordinate = {0, 0, width, height};

    camera_coordinate_t topLeft =
        AiqUtils::convertCoordinateSystem(faceCoordinate, iaCoordinate, {src.left, src.top});
    camera_coordinate_t bottomRight =
        AiqUtils::convertCoordinateSystem(faceCoordinate, iaCoordinate, {src.right, src.bottom});

    *dst = {topLeft.x, topLeft.y, bottomRight.x, bottomRight.y};
    LOG2("@%s, face:%d, dst left:%d, top:%d, right:%d, bottom:%d", __func__, faceId, dst->left,
         dst->top, dst->right, dst->bottom);
}

FaceDetectionRunParams* IntelFaceDetection::prepareRunBuffer(unsigned int index) {
    LOG1("@%s", __func__);
    CheckError(index >= MAX_STORE_FACE_DATA_BUF_NUM, nullptr, "@%s, index is error %d", __func__,
               index);
    CheckError(!mFDHandle, nullptr, "mFDHandle is nullptr");

    return &mMemRunBufs[index];
}

status_t IntelFaceDetection::run(pvl_image* pImage, FaceDetectionResult* fdResults) {
    LOG1("@%s, pImage:%p", __func__, pImage);
    CheckError(!pImage, UNKNOWN_ERROR, "pData is nullptr");
    CheckError(!mFDHandle, UNKNOWN_ERROR, "mFDHandle is nullptr");

    int32_t fdRet =
        pvl_face_detection_run_in_preview(mFDHandle, pImage, fdResults->faceResults, mMaxFacesNum);
    fdResults->faceNum = (fdRet > 0) ? fdRet : 0;
    LOG1("@%s, fdRet:%d, detected face number:%d, w:%d, h:%d", __func__, fdRet, fdResults->faceNum,
         pImage->width, pImage->height);
    for (int i = 0; i < fdResults->faceNum; i++) {
        LOG2("@%s, face:%d rect, left:%d, top:%d, right:%d, bottom:%d", __func__, i,
             fdResults->faceResults[i].rect.left, fdResults->faceResults[i].rect.top,
             fdResults->faceResults[i].rect.right, fdResults->faceResults[i].rect.bottom);
        LOG2("@%s, confidence:%d, rip_angle:%d, rop_angle:%d, tracking_id:%d", __func__,
             fdResults->faceResults[i].confidence, fdResults->faceResults[i].rip_angle,
             fdResults->faceResults[i].rop_angle, fdResults->faceResults[i].tracking_id);
    }

    for (int i = 0; i < fdResults->faceNum; i++) {
        convertCoordinate(i, pImage->width, pImage->height, fdResults->faceResults[i].rect,
                          &fdResults->faceResults[i].rect);
    }

    return OK;
}

status_t IntelFaceDetection::run(FaceDetectionRunParams* fdRunParams, int dataSize, void* addr) {
    LOG1("@%s, fdRunParams:%p, dataSize:%d, addr:%p", __func__, fdRunParams, dataSize, addr);
    CheckError(!fdRunParams, UNKNOWN_ERROR, "pData is nullptr");
    CheckError(dataSize < static_cast<int>(sizeof(FaceDetectionRunParams)), UNKNOWN_ERROR,
               "buffer is small");
    CheckError(!mFDHandle, UNKNOWN_ERROR, "mFDHandle is nullptr");

    pvl_image image;
    image.size = fdRunParams->size;
    image.width = fdRunParams->width;
    image.height = fdRunParams->height;
    image.format = fdRunParams->format;
    image.stride = fdRunParams->stride;
    image.rotation = fdRunParams->rotation;
    if (addr) {
        image.data = const_cast<uint8_t*>(static_cast<uint8_t*>(addr));
    } else {
        image.data = const_cast<uint8_t*>(fdRunParams->data);
    }

    return run(&image, &fdRunParams->results);
}
}  // namespace icamera
