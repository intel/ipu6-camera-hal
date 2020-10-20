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

#define LOG_TAG "DvsResult"

#include "src/3a/DvsResult.h"

#include "iutils/CameraLog.h"
#include "iutils/Utils.h"
#include "iutils/Errors.h"

namespace icamera {
DvsResult::DvsResult() :
    mSequence(-1) {
    LOG3A("@%s", __func__);

    CLEAR(mTransformation);
    CLEAR(mMorphTable);
    CLEAR(mDvsXcoordsY);
    CLEAR(mDvsYcoordsY);
    CLEAR(mDvsXcoordsUV);
    CLEAR(mDvsYcoordsUV);
    CLEAR(mDvsXcoordsUVFloat);
    CLEAR(mDvsYcoordsUVFloat);
    mMorphTable.xcoords_y = mDvsXcoordsY;
    mMorphTable.ycoords_y = mDvsYcoordsY;
    mMorphTable.xcoords_uv = mDvsXcoordsUV;
    mMorphTable.ycoords_uv = mDvsYcoordsUV;
    mMorphTable.xcoords_uv_float = mDvsXcoordsUVFloat;
    mMorphTable.ycoords_uv_float = mDvsYcoordsUVFloat;
}

DvsResult::~DvsResult() {
    LOG3A("@%s", __func__);
}

int DvsResult::deepCopyDvsResults(const ia_dvs_morph_table& src, ia_dvs_morph_table* dst) {
    LOG3A("%s", __func__);

    CheckError(!dst || !dst->xcoords_y || !dst->ycoords_y
               || !dst->xcoords_uv || !dst->ycoords_uv
               || !dst->xcoords_uv_float || !dst->ycoords_uv_float,
               BAD_VALUE , "Failed to deep copy DVS result- invalid destination");

    CheckError(!src.xcoords_y || !src.ycoords_y
               || !src.xcoords_uv || !src.ycoords_uv
               || !src.xcoords_uv_float || !src.ycoords_uv_float,
               BAD_VALUE , "Failed to deep copy DVS result- invalid source");

    CheckError(src.width_y == 0 || src.height_y == 0 || src.width_uv == 0 || src.height_uv == 0,
               BAD_VALUE , "Failed to deep copy DVS result- invalid source size y[%dx%d] uv[%dx%d]",
               src.width_y, src.height_y, src.width_uv, src.height_uv);

    dst->width_y = src.width_y;
    dst->height_y = src.height_y;
    dst->width_uv = src.width_uv;
    dst->height_uv = src.height_uv;
    dst->morph_table_changed = src.morph_table_changed;
    unsigned int SizeY = dst->width_y  * dst->height_y * sizeof(int32_t);
    unsigned int SizeUV = dst->width_uv * dst->height_uv * sizeof(int32_t);
    MEMCPY_S(dst->xcoords_y, SizeY, src.xcoords_y, SizeY);
    MEMCPY_S(dst->ycoords_y, SizeY, src.ycoords_y, SizeY);
    MEMCPY_S(dst->xcoords_uv, SizeUV, src.xcoords_uv, SizeUV);
    MEMCPY_S(dst->ycoords_uv, SizeUV, src.ycoords_uv, SizeUV);

    SizeUV = dst->width_uv * dst->height_uv * sizeof(float);
    MEMCPY_S(dst->xcoords_uv_float, SizeUV, src.xcoords_uv_float, SizeUV);
    MEMCPY_S(dst->ycoords_uv_float, SizeUV, src.ycoords_uv_float, SizeUV);

    return OK;
}

int DvsResult::deepCopyDvsResults(const ia_dvs_image_transformation& src,
                                  ia_dvs_image_transformation* dst) {
    LOG3A("%s", __func__);

    CheckError(!dst, BAD_VALUE , "Failed to deep copy DVS result- invalid destination");

    dst->num_homography_matrices = src.num_homography_matrices;
    MEMCPY_S(dst->matrices, sizeof(dst->matrices), src.matrices, sizeof(src.matrices));

    return OK;
}

DvsResult &DvsResult::operator=(const DvsResult &other) {
    deepCopyDvsResults(other.mMorphTable, &this->mMorphTable);
    deepCopyDvsResults(other.mTransformation, &this->mTransformation);
    mSequence = other.mSequence;

    return *this;
}
}  // namespace icamera
