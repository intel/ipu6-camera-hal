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

#include <ia_dvs_types.h>

namespace icamera {
static const int MAX_DVS_COORDS_Y_SIZE = 33 * 69;
static const int MAX_DVS_COORDS_UV_SIZE = 33 * 69;

class DvsResult {
 public:
    DvsResult();
    ~DvsResult();

    DvsResult& operator=(const DvsResult& other);

    ia_dvs_morph_table mMorphTable;
    ia_dvs_image_transformation mTransformation;
    int64_t mSequence;

    static int deepCopyDvsResults(const ia_dvs_morph_table& src, ia_dvs_morph_table* dst);
    static int deepCopyDvsResults(const ia_dvs_image_transformation& src,
                                  ia_dvs_image_transformation* dst);

 private:
    uint32_t mDvsXcoordsY[MAX_DVS_COORDS_Y_SIZE];
    uint32_t mDvsYcoordsY[MAX_DVS_COORDS_Y_SIZE];
    uint32_t mDvsXcoordsUV[MAX_DVS_COORDS_UV_SIZE];
    uint32_t mDvsYcoordsUV[MAX_DVS_COORDS_UV_SIZE];
    float mDvsXcoordsUVFloat[MAX_DVS_COORDS_UV_SIZE];
    float mDvsYcoordsUVFloat[MAX_DVS_COORDS_UV_SIZE];
};
}  // namespace icamera
