/*
 * Copyright (C) 2018 Intel Corporation.
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

#define LOG_TAG "AiqStatistics"

#include "iutils/Errors.h"
#include "iutils/CameraLog.h"

#include "AiqStatistics.h"

namespace icamera {

AiqStatistics::AiqStatistics()
{
    CLEAR(mRgbsGridArray);
    CLEAR(mAfGridArray);
}

AiqStatistics::~AiqStatistics()
{
    for (unsigned int i = 0; i < MAX_EXPOSURES_NUM; i++) {
        delete [] mRgbsGridArray[i].blocks_ptr;
        mRgbsGridArray[i].blocks_ptr = nullptr;

        delete [] mAfGridArray[i].filter_response_1;
        mAfGridArray[i].filter_response_1 = nullptr;
        delete [] mAfGridArray[i].filter_response_2;
        mAfGridArray[i].filter_response_2 = nullptr;
    }
}

AiqStatistics &AiqStatistics::operator=(const AiqStatistics &other)
{
    mSequence = other.mSequence;
    mTimestamp = other.mTimestamp;
    mTuningMode = other.mTuningMode;

    const ia_aiq_rgbs_grid *rgbs_grid[MAX_EXPOSURES_NUM];
    for (int i = 0; i < other.mExposureNum; i++) {
        rgbs_grid[i] = &other.mRgbsGridArray[i];
    }
    saveRgbsGridData(rgbs_grid, other.mExposureNum);

    saveAfGridData(&other.mAfGridArray[0]);

    return *this;
}

#define GRID_SIZE_UNEQUAL(g1, g2) \
        ((g1)->grid_width != (g2)->grid_width || (g1)->grid_height != (g2)->grid_height)

int AiqStatistics::saveRgbsGridData(const ia_aiq_rgbs_grid* const *rgbsGrids, int exposureNum)
{
    CheckError(rgbsGrids == nullptr, BAD_VALUE, "Rgbs grid is null");

    for (int i = 0; i < exposureNum; i++) {
        int ret = copyRgbsGridData(rgbsGrids[i], &mRgbsGridArray[i]);
        CheckError(ret != OK, ret, "save Rgbs grid %d failed ret %d", i, ret);
    }
    mExposureNum = exposureNum;
    return OK;
}

int AiqStatistics::copyRgbsGridData(const ia_aiq_rgbs_grid *src, ia_aiq_rgbs_grid *dst)
{
    LOG3A("%s", __func__);
    CheckError(src == nullptr || dst == nullptr, BAD_VALUE, "src or dst rgbs grid is null");

    // Release the old memory if size changes.
    if (GRID_SIZE_UNEQUAL(src, dst)) {
        if (dst->blocks_ptr) delete [] dst->blocks_ptr;
        CLEAR(*dst);
    }

    if (src->blocks_ptr == nullptr || src->grid_width == 0 || src->grid_height == 0) {
        return OK;
    }

    size_t gridSize = src->grid_width * src->grid_height;
    if (dst->blocks_ptr == nullptr) {
        dst->blocks_ptr = new rgbs_grid_block[gridSize];
    }

    dst->grid_width = src->grid_width;
    dst->grid_height = src->grid_height;
    MEMCPY_S(dst->blocks_ptr, gridSize * sizeof(rgbs_grid_block),
             src->blocks_ptr, gridSize * sizeof(rgbs_grid_block));

    dst->shading_correction = src->shading_correction;

    LOG3A("%s, grid size=[%dx%d]", __func__, dst->grid_width, dst->grid_height);
    return OK;
}

int AiqStatistics::saveAfGridData(const ia_aiq_af_grid *afGrid)
{
    LOG3A("%s", __func__);
    CheckError(afGrid == nullptr, BAD_VALUE, "AF grid is null");

    // Release the old memory if size changes.
    if (GRID_SIZE_UNEQUAL(afGrid, &mAfGridArray[0])) {
        if (mAfGridArray[0].filter_response_1) delete [] mAfGridArray[0].filter_response_1;
        if (mAfGridArray[0].filter_response_2) delete [] mAfGridArray[0].filter_response_2;
        CLEAR(mAfGridArray);
    }

    if (afGrid->filter_response_1 == nullptr || afGrid->filter_response_2 == nullptr
        || afGrid->grid_width == 0 || afGrid->grid_height == 0) {
        return OK;
    }

    size_t gridSize = afGrid->grid_width * afGrid->grid_height;
    if (mAfGridArray[0].filter_response_1 == nullptr) {
        mAfGridArray[0].filter_response_1 = new int[gridSize];
    }
    if (mAfGridArray[0].filter_response_2 == nullptr) {
        mAfGridArray[0].filter_response_2 = new int[gridSize];
    }

    mAfGridArray[0].grid_width = afGrid->grid_width;
    mAfGridArray[0].grid_height = afGrid->grid_height;
    mAfGridArray[0].block_width = afGrid->block_width;
    mAfGridArray[0].block_height = afGrid->block_height;
    MEMCPY_S(mAfGridArray[0].filter_response_1, gridSize * sizeof(int),
             afGrid->filter_response_1, gridSize * sizeof(int));
    MEMCPY_S(mAfGridArray[0].filter_response_2, gridSize * sizeof(int),
             afGrid->filter_response_2, gridSize * sizeof(int));

    LOG3A("%s, grid size=[%dx%d]", __func__, mAfGridArray[0].grid_width, mAfGridArray[0].grid_height);
    return OK;
}

} /* namespace icamera */

