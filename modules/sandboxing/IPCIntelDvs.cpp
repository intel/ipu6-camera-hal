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

#define LOG_TAG "IPC_INTEL_DVS"

#include "modules/sandboxing/IPCIntelDvs.h"

#include <ia_types.h>

#include "CameraLog.h"
#include "iutils/Errors.h"
#include "iutils/Utils.h"

namespace icamera {
IPCIntelDvs::IPCIntelDvs() {
    LOGIPC("@%s", __func__);
}

IPCIntelDvs::~IPCIntelDvs() {
    LOGIPC("@%s", __func__);
}

bool IPCIntelDvs::clientFlattenInit(void* pData, unsigned int size,
                                    const ia_binary_data* dvsDataPtr, const ia_cmc_t* cmc) {
    LOGIPC("@%s, pData:%p, size:%d, dvsDataPtr:%p, cmc:%p", __func__, pData, size, dvsDataPtr, cmc);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(!dvsDataPtr, false, "@%s, dvsDataPtr is nullptr", __func__);
    CheckError(!cmc, false, "@%s, cmc is nullptr", __func__);
    CheckError(size < sizeof(DvsInitParams), false, "@%s, buffer is small", __func__);
    CheckError(dvsDataPtr->size > MAX_DVS_DATA_SIZE, false, "@%s, data:%d is too small", __func__,
               dvsDataPtr->size);

    DvsInitParams* params = static_cast<DvsInitParams*>(pData);
    params->base.size = dvsDataPtr->size;
    params->base.data = params->data;
    MEMCPY_S(params->data, MAX_DVS_DATA_SIZE, dvsDataPtr->data, dvsDataPtr->size);

    params->cmcHandle = reinterpret_cast<uintptr_t>(cmc);
    return true;
}

bool IPCIntelDvs::serverUnflattenInit(void* pData, int size, ia_binary_data** dvsDataPtr,
                                      ia_cmc_t** cmc) {
    LOGIPC("@%s, pData:%p, size:%d, dvsDataPtr:%p, cmc:%p", __func__, pData, size, dvsDataPtr, cmc);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(size < sizeof(DvsInitParams), false, "@%s, buffer is small", __func__);
    CheckError(!dvsDataPtr, false, "@%s, dvsDataPtr is nullptr", __func__);
    CheckError(!cmc, false, "@%s, cmc is nullptr", __func__);

    DvsInitParams* params = reinterpret_cast<DvsInitParams*>(pData);
    params->base.data = params->data;

    *dvsDataPtr = &params->base;
    *cmc = reinterpret_cast<ia_cmc_t*>(params->cmcHandle);
    return true;
}

bool IPCIntelDvs::serverFlattenInit(void* pData, unsigned int size, ia_dvs_state* dvs) {
    LOGIPC("@%s, pData:%p, size:%d, dvs:%p", __func__, pData, size, dvs);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(size < sizeof(DvsInitParams), false, "@%s, buffer is small", __func__);

    DvsInitParams* params = static_cast<DvsInitParams*>(pData);
    params->dvsHandle = reinterpret_cast<uintptr_t>(dvs);
    return true;
}

bool IPCIntelDvs::clientUnflattenInit(const void* pData, unsigned int size, ia_dvs_state** dvs) {
    LOGIPC("@%s, pData:%p, size:%d, dvs:%p", __func__, pData, size, dvs);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(size < sizeof(DvsInitParams), false, "@%s, buffer is small", __func__);
    CheckError(!dvs, false, "@%s, dvs is nullptr", __func__);

    const DvsInitParams* params = static_cast<const DvsInitParams*>(pData);
    *dvs = reinterpret_cast<ia_dvs_state*>(params->dvsHandle);

    return true;
}

bool IPCIntelDvs::clientFlattenDeinit(void* pData, unsigned int size, ia_dvs_state* dvs) {
    LOGIPC("@%s, pData:%p, size:%d, dvs:%p", __func__, pData, size, dvs);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(size < sizeof(DvsDeinitParams), false, "@%s, buffer is small", __func__);
    CheckError(!dvs, false, "@%s, dvs is nullptr", __func__);

    DvsDeinitParams* params = static_cast<DvsDeinitParams*>(pData);
    params->dvsHandle = reinterpret_cast<uintptr_t>(dvs);

    return true;
}

bool IPCIntelDvs::serverUnflattenDeinit(const void* pData, unsigned int size, ia_dvs_state** dvs) {
    LOGIPC("@%s, pData:%p, size:%d, dvs:%p", __func__, pData, size, dvs);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(size < sizeof(DvsDeinitParams), false, "@%s, buffer is small", __func__);
    CheckError(!dvs, false, "@%s, dvs is nullptr", __func__);

    const DvsDeinitParams* params = static_cast<const DvsDeinitParams*>(pData);
    *dvs = reinterpret_cast<ia_dvs_state*>(params->dvsHandle);

    return true;
}

bool IPCIntelDvs::clientFlattenConfig(void* pData, unsigned int size, ia_dvs_state* dvs,
                                      const ia_dvs_configuration* config, float zoomRatio) {
    LOGIPC("@%s, pData:%p, size:%d, dvs:%p, config:%p, zoomRatio:%f", __func__, pData, size, dvs,
           config, zoomRatio);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(size < sizeof(DvsConfigParams), false, "@%s, buffer is small", __func__);
    CheckError(!dvs, false, "@%s, dvs is nullptr", __func__);
    CheckError(!config, false, "@%s, config is nullptr", __func__);

    DvsConfigParams* params = static_cast<DvsConfigParams*>(pData);
    params->dvsHandle = reinterpret_cast<uintptr_t>(dvs);
    MEMCPY_S(&params->configData, sizeof(ia_dvs_configuration), config,
             sizeof(ia_dvs_configuration));
    params->zoomRatio = zoomRatio;

    return true;
}

bool IPCIntelDvs::serverUnflattenConfig(const void* pData, unsigned int size, ia_dvs_state** dvs,
                                        ia_dvs_configuration** config, float* zoomRatio) {
    LOGIPC("@%s, pData:%p, size:%d, dvs:%p, config:%p, zoomRatio:%f", __func__, pData, size, dvs,
           config, zoomRatio);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(size < sizeof(DvsConfigParams), false, "@%s, buffer is small", __func__);
    CheckError(!dvs, false, "@%s, dvs is nullptr", __func__);
    CheckError(!config, false, "@%s, config is nullptr", __func__);
    CheckError(!zoomRatio, false, "@%s, zoomRatio is nullptr", __func__);

    DvsConfigParams* params =
        const_cast<DvsConfigParams*>(static_cast<const DvsConfigParams*>(pData));
    *dvs = reinterpret_cast<ia_dvs_state*>(params->dvsHandle);
    *config = &params->configData;
    *zoomRatio = params->zoomRatio;

    return true;
}

bool IPCIntelDvs::clientFlattenNoneBlanckRation(void* pData, unsigned int size, ia_dvs_state* dvs,
                                                float nonBlankingRatio) {
    LOGIPC("@%s, pData:%p, size:%d, dvs:%p, nonBlankingRatio:%f", __func__, pData, size, dvs,
           nonBlankingRatio);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(size < sizeof(DvsNoneBlankRatioParams), false, "@%s, buffer is small", __func__);
    CheckError(!dvs, false, "@%s, dvs is nullptr", __func__);

    DvsNoneBlankRatioParams* params = static_cast<DvsNoneBlankRatioParams*>(pData);
    params->dvsHandle = reinterpret_cast<uintptr_t>(dvs);
    params->nonBlankingRatio = nonBlankingRatio;

    return true;
}

bool IPCIntelDvs::serverUnflattenNoneBlanckRation(const void* pData, unsigned int size,
                                                  ia_dvs_state** dvs, float* nonBlankingRatio) {
    LOGIPC("@%s, pData:%p, size:%d, dvs:%p, nonBlankingRatio:%f", __func__, pData, size, dvs,
           nonBlankingRatio);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(size < sizeof(DvsNoneBlankRatioParams), false, "@%s, buffer is small", __func__);
    CheckError(!dvs, false, "@%s, dvs is nullptr", __func__);
    CheckError(!nonBlankingRatio, false, "@%s, nonBlankingRatio is nullptr", __func__);

    const DvsNoneBlankRatioParams* params = static_cast<const DvsNoneBlankRatioParams*>(pData);
    *dvs = reinterpret_cast<ia_dvs_state*>(params->dvsHandle);
    *nonBlankingRatio = params->nonBlankingRatio;

    return true;
}

bool IPCIntelDvs::clientFlattenDigitalZoomMode(void* pData, unsigned int size, ia_dvs_state* dvs,
                                               ia_dvs_zoom_mode zoomMode) {
    LOGIPC("@%s, pData:%p, size:%d, dvs:%p, zoomMode:%d", __func__, pData, size, dvs, zoomMode);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(size < sizeof(DvsDigitalZoomMode), false, "@%s, buffer is small", __func__);
    CheckError(!dvs, false, "@%s, dvs is nullptr", __func__);

    DvsDigitalZoomMode* params = static_cast<DvsDigitalZoomMode*>(pData);
    params->dvsHandle = reinterpret_cast<uintptr_t>(dvs);
    params->zoomMode = zoomMode;

    return true;
}

bool IPCIntelDvs::serverUnflattenDigitalZoomMode(const void* pData, unsigned int size,
                                                 ia_dvs_state** dvs, ia_dvs_zoom_mode* zoomMode) {
    LOGIPC("@%s, pData:%p, size:%d, dvs:%p, zoomMode:%d", __func__, pData, size, dvs, zoomMode);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(size < sizeof(DvsDigitalZoomMode), false, "@%s, buffer is small", __func__);
    CheckError(!dvs, false, "@%s, dvs is nullptr", __func__);
    CheckError(!zoomMode, false, "@%s, zoomMode is nullptr", __func__);

    const DvsDigitalZoomMode* params = static_cast<const DvsDigitalZoomMode*>(pData);
    *dvs = reinterpret_cast<ia_dvs_state*>(params->dvsHandle);
    *zoomMode = params->zoomMode;

    return true;
}

bool IPCIntelDvs::clientFlattenDigitalZoomRegion(void* pData, unsigned int size, ia_dvs_state* dvs,
                                                 ia_rectangle* zoomRegion) {
    LOGIPC("@%s, pData:%p, size:%d, dvs:%p, zoomRegion:%p", __func__, pData, size, dvs, zoomRegion);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(size < sizeof(DvsDigitalZoomRegion), false, "@%s, buffer is small", __func__);
    CheckError(!dvs, false, "@%s, dvs is nullptr", __func__);
    CheckError(!zoomRegion, false, "@%s, zoomRegion is nullptr", __func__);

    DvsDigitalZoomRegion* params = static_cast<DvsDigitalZoomRegion*>(pData);
    params->dvsHandle = reinterpret_cast<uintptr_t>(dvs);
    params->zoomRegion = *zoomRegion;

    return true;
}

bool IPCIntelDvs::serverUnflattenDigitalZoomRegion(const void* pData, unsigned int size,
                                                   ia_dvs_state** dvs, ia_rectangle** zoomRegion) {
    LOGIPC("@%s, pData:%p, size:%d, dvs:%p, zoomRegion:%p", __func__, pData, size, dvs, zoomRegion);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(size < sizeof(DvsDigitalZoomRegion), false, "@%s, buffer is small", __func__);
    CheckError(!dvs, false, "@%s, dvs is nullptr", __func__);
    CheckError(!zoomRegion, false, "@%s, zoomRegion is nullptr", __func__);

    DvsDigitalZoomRegion* params =
        const_cast<DvsDigitalZoomRegion*>(static_cast<const DvsDigitalZoomRegion*>(pData));
    *dvs = reinterpret_cast<ia_dvs_state*>(params->dvsHandle);
    *zoomRegion = &params->zoomRegion;
    return true;
}

bool IPCIntelDvs::clientFlattenDigitalZoomCoordinate(void* pData, unsigned int size,
                                                     ia_dvs_state* dvs,
                                                     ia_coordinate* zoomCoordinate) {
    LOGIPC("@%s, pData:%p, size:%d, dvs:%p, zoomCoordinate:%p", __func__, pData, size, dvs,
           zoomCoordinate);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(size < sizeof(DvsDigitalZoomCoordinate), false, "@%s, buffer is small", __func__);
    CheckError(!dvs, false, "@%s, dvs is nullptr", __func__);
    CheckError(!zoomCoordinate, false, "@%s, zoomCoordinate is nullptr", __func__);

    DvsDigitalZoomCoordinate* params = static_cast<DvsDigitalZoomCoordinate*>(pData);
    params->dvsHandle = reinterpret_cast<uintptr_t>(dvs);
    params->zoomCoordinate = *zoomCoordinate;

    return true;
}

bool IPCIntelDvs::serverUnflattenDigitalZoomCoordinate(const void* pData, unsigned int size,
                                                       ia_dvs_state** dvs,
                                                       ia_coordinate** zoomCoordinate) {
    LOGIPC("@%s, pData:%p, size:%d, dvs:%p, zoomCoordinate:%p", __func__, pData, size, dvs,
           zoomCoordinate);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(size < sizeof(DvsDigitalZoomCoordinate), false, "@%s, buffer is small", __func__);
    CheckError(!dvs, false, "@%s, dvs is nullptr", __func__);
    CheckError(!zoomCoordinate, false, "@%s, zoomCoordinate is nullptr", __func__);

    DvsDigitalZoomCoordinate* params =
        const_cast<DvsDigitalZoomCoordinate*>(static_cast<const DvsDigitalZoomCoordinate*>(pData));
    *dvs = reinterpret_cast<ia_dvs_state*>(params->dvsHandle);
    *zoomCoordinate = &params->zoomCoordinate;

    return true;
}

bool IPCIntelDvs::clientFlattenDigitalZoomMagnitude(void* pData, unsigned int size,
                                                    ia_dvs_state* dvs, float zoomRatio) {
    LOGIPC("@%s, pData:%p, size:%d, dvs:%p, zoomRatio:%f", __func__, pData, size, dvs, zoomRatio);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(size < sizeof(DvsDigitalZoomMagnitude), false, "@%s, buffer is small", __func__);
    CheckError(!dvs, false, "@%s, dvs is nullptr", __func__);

    DvsDigitalZoomMagnitude* params = static_cast<DvsDigitalZoomMagnitude*>(pData);
    params->dvsHandle = reinterpret_cast<uintptr_t>(dvs);
    params->zoomRatio = zoomRatio;

    return true;
}

bool IPCIntelDvs::serverUnflattenDigitalZoomMagnitude(const void* pData, unsigned int size,
                                                      ia_dvs_state** dvs, float* zoomRatio) {
    LOGIPC("@%s, pData:%p, size:%d, dvs:%p, zoomRatio:%f", __func__, pData, size, dvs, zoomRatio);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(size < sizeof(DvsDigitalZoomMagnitude), false, "@%s, buffer is small", __func__);
    CheckError(!dvs, false, "@%s, dvs is nullptr", __func__);
    CheckError(!zoomRatio, false, "@%s, zoomRatio is nullptr", __func__);

    const DvsDigitalZoomMagnitude* params = static_cast<const DvsDigitalZoomMagnitude*>(pData);
    *dvs = reinterpret_cast<ia_dvs_state*>(params->dvsHandle);
    *zoomRatio = params->zoomRatio;

    return true;
}

bool IPCIntelDvs::clientFlattenFreeMorphTable(void* pData, unsigned int size, ia_dvs_state* dvs,
                                              ia_dvs_morph_table* morphTable) {
    LOGIPC("@%s, pData:%p, size:%d, morphTable:%p", __func__, pData, size, morphTable);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(size < sizeof(DvsFreeMorphParams), false, "@%s, buffer is small", __func__);
    CheckError(!dvs, false, "@%s, dvs is nullptr", __func__);
    CheckError(!morphTable, false, "@%s, morphTable is nullptr", __func__);

    DvsFreeMorphParams* params = static_cast<DvsFreeMorphParams*>(pData);
    params->dvsHandle = reinterpret_cast<uintptr_t>(dvs);
    params->morphHandle = reinterpret_cast<uintptr_t>(morphTable);

    return true;
}

bool IPCIntelDvs::serverUnflattenFreeMorphTable(const void* pData, unsigned int size,
                                                ia_dvs_state** dvs,
                                                ia_dvs_morph_table** morphTable) {
    LOGIPC("@%s, pData:%p, size:%d, morphTable:%p", __func__, pData, size, morphTable);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(size < sizeof(DvsFreeMorphParams), false, "@%s, buffer is small", __func__);
    CheckError(!morphTable, false, "@%s, morphTable is nullptr", __func__);

    const DvsFreeMorphParams* params = static_cast<const DvsFreeMorphParams*>(pData);
    *dvs = reinterpret_cast<ia_dvs_state*>(params->dvsHandle);
    *morphTable = reinterpret_cast<ia_dvs_morph_table*>(params->morphHandle);

    return true;
}

bool IPCIntelDvs::clientFlattenAllocateMorphTable(void* pData, unsigned int size,
                                                  ia_dvs_state* dvs) {
    LOGIPC("@%s, pData:%p, size:%d, dvs:%p", __func__, pData, size, dvs);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(size < sizeof(DvsMorphParams), false, "@%s, buffer is small", __func__);
    CheckError(!dvs, false, "@%s, dvs is nullptr", __func__);

    DvsMorphParams* params = static_cast<DvsMorphParams*>(pData);
    params->dvsHandle = reinterpret_cast<uintptr_t>(dvs);

    return true;
}

bool IPCIntelDvs::serverUnflattenAllocateMorphTalbe(const void* pData, unsigned int size,
                                                    ia_dvs_state** dvs) {
    LOGIPC("@%s, pData:%p, size:%d, dvs:%p", __func__, pData, size, dvs);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(size < sizeof(DvsMorphParams), false, "@%s, buffer is small", __func__);
    CheckError(!dvs, false, "@%s, dvs is nullptr", __func__);

    const DvsMorphParams* params = static_cast<const DvsMorphParams*>(pData);
    *dvs = reinterpret_cast<ia_dvs_state*>(params->dvsHandle);

    return true;
}

bool IPCIntelDvs::serverFlattenAllocateMorphTalbe(void* pData, unsigned int size,
                                                  ia_dvs_morph_table* morphTable) {
    LOGIPC("@%s, pData:%p, size:%d, morphRemote:%p", __func__, pData, size, morphTable);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(size < sizeof(DvsMorphParams), false, "@%s, buffer is small", __func__);
    CheckError(!morphTable, false, "@%s, morphTable is nullptr", __func__);

    DvsMorphParams* params = static_cast<DvsMorphParams*>(pData);
    params->morphHandle = reinterpret_cast<uintptr_t>(morphTable);

    return true;
}

bool IPCIntelDvs::clientUnflattenAllocateMorphTalbe(const void* pData, unsigned int size,
                                                    ia_dvs_morph_table** morphTable) {
    LOGIPC("@%s, pData:%p, size:%d", __func__, pData, size);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(size < sizeof(DvsMorphParams), false, "@%s, buffer is small", __func__);
    CheckError(!morphTable, false, "@%s, morphLocal is nullptr", __func__);

    const DvsMorphParams* params = static_cast<const DvsMorphParams*>(pData);
    *morphTable = reinterpret_cast<ia_dvs_morph_table*>(params->morphHandle);
    return true;
}

bool IPCIntelDvs::clientFlattenGetMorphTable(void* pData, unsigned int size, ia_dvs_state* dvs,
                                             ia_dvs_morph_table* morphTable) {
    LOGIPC("@%s, pData:%p, size:%d, morphTable:%p", __func__, pData, size, morphTable);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(size < sizeof(DvsMorphParams), false, "@%s, buffer is small", __func__);
    CheckError(!dvs, false, "@%s, dvs is nullptr", __func__);
    CheckError(!morphTable, false, "@%s, morphTable is nullptr", __func__);

    DvsMorphParams* params = static_cast<DvsMorphParams*>(pData);
    params->dvsHandle = reinterpret_cast<uintptr_t>(dvs);
    params->morphHandle = reinterpret_cast<uintptr_t>(morphTable);

    return true;
}

bool IPCIntelDvs::serverUnflattenGetMorphTalbe(const void* pData, unsigned int size,
                                               ia_dvs_state** dvs,
                                               ia_dvs_morph_table** morphTable) {
    LOGIPC("@%s, pData:%p, size:%d, dvs:%p, morphTable:%p", __func__, pData, size, dvs, morphTable);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(size < sizeof(DvsMorphParams), false, "@%s, buffer is small", __func__);
    CheckError(!dvs, false, "@%s, dvs is nullptr", __func__);
    CheckError(!morphTable, false, "@%s, morphTable is nullptr", __func__);

    const DvsMorphParams* params = static_cast<const DvsMorphParams*>(pData);
    *dvs = reinterpret_cast<ia_dvs_state*>(params->dvsHandle);
    *morphTable = reinterpret_cast<ia_dvs_morph_table*>(params->morphHandle);

    return true;
}

bool IPCIntelDvs::flattenMorphTable(const ia_dvs_morph_table* morphTable, DvsMorphParams* params) {
    LOGIPC("@%s", __func__);
    CheckError(!morphTable, false, "@%s, morphTable is nullptr", __func__);
    CheckError(!params, false, "@%s, params is nullptr", __func__);
    CheckError(!morphTable->xcoords_y || !morphTable->ycoords_y, false, "@%s, y coords is nullptr",
               __func__);
    CheckError(!morphTable->xcoords_uv || !morphTable->ycoords_uv, false,
               "@%s, uv coords is nullptr", __func__);
    CheckError(!morphTable->xcoords_uv_float || !morphTable->ycoords_uv_float, false,
               "@%s, uv coords float is nullptr", __func__);

    unsigned int SizeY = morphTable->width_y * morphTable->height_y * sizeof(int32_t);
    unsigned int SizeUV = morphTable->width_uv * morphTable->height_uv * sizeof(int32_t);
    unsigned int SizeYLocal = MAX_DVS_COORDS_Y_SIZE * sizeof(int32_t);
    unsigned int SizeUVLocal = MAX_DVS_COORDS_UV_SIZE * sizeof(int32_t);
    CheckError(SizeY > SizeYLocal, false, "@%s, coords_y data is small", __func__);
    CheckError(SizeUV > SizeUVLocal, false, "@%s, coords_uv data is small", __func__);
    MEMCPY_S(params->xcoordsY, SizeYLocal, morphTable->xcoords_y, SizeY);
    MEMCPY_S(params->ycoordsY, SizeYLocal, morphTable->ycoords_y, SizeY);
    MEMCPY_S(params->xcoordsUV, SizeUVLocal, morphTable->xcoords_uv, SizeUV);
    MEMCPY_S(params->ycoordsUV, SizeUVLocal, morphTable->ycoords_uv, SizeUV);

    SizeUV = morphTable->width_uv * morphTable->height_uv * sizeof(float);
    SizeUVLocal = MAX_DVS_COORDS_UV_SIZE * sizeof(float);
    CheckError(SizeUV > SizeUVLocal, false, "@%s, coords_uv data of float type is small", __func__);
    MEMCPY_S(params->xcoordsUVFloat, SizeUVLocal, morphTable->xcoords_uv_float, SizeUV);
    MEMCPY_S(params->ycoordsUVFloat, SizeUVLocal, morphTable->ycoords_uv_float, SizeUV);

    return true;
}

bool IPCIntelDvs::serverFlattenGetMorphTalbe(void* pData, unsigned int size,
                                             ia_dvs_morph_table* morphTable) {
    LOGIPC("@%s, pData:%p, size:%d, morphTable:%p", __func__, pData, size, morphTable);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(size < sizeof(DvsMorphParams), false, "@%s, buffer is small", __func__);
    CheckError(!morphTable, false, "@%s, morphTable is nullptr", __func__);

    DvsMorphParams* params = static_cast<DvsMorphParams*>(pData);
    params->morphTable = *morphTable;

    return flattenMorphTable(morphTable, params);
}

bool IPCIntelDvs::unflattenMorphTalbe(DvsMorphParams* params) {
    LOGIPC("@%s", __func__);
    CheckError(!params, false, "@%s, params is nullptr", __func__);

    params->morphTable.xcoords_y = params->xcoordsY;
    params->morphTable.ycoords_y = params->ycoordsY;
    params->morphTable.xcoords_uv = params->xcoordsUV;
    params->morphTable.ycoords_uv = params->ycoordsUV;
    params->morphTable.xcoords_uv_float = params->xcoordsUVFloat;
    params->morphTable.ycoords_uv_float = params->ycoordsUVFloat;

    return true;
}

bool IPCIntelDvs::clientUnflattenGetMorphTalbe(void* pData, unsigned int size,
                                               ia_dvs_morph_table** morphTable) {
    LOGIPC("@%s, pData:%p, size:%d", __func__, pData, size);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(size < sizeof(DvsMorphParams), false, "@%s, buffer is small", __func__);
    CheckError(!morphTable, false, "@%s, morphLocal is nullptr", __func__);

    DvsMorphParams* params = static_cast<DvsMorphParams*>(pData);
    bool ret = unflattenMorphTalbe(params);
    CheckError(!ret, false, "@%s, unflattenMorphTalbe fails", __func__);
    *morphTable = &params->morphTable;

    return true;
}

bool IPCIntelDvs::clientFlattenSetStatistics(void* pData, unsigned int size, ia_dvs_state* dvs,
                                             const ia_dvs_statistics* statistics,
                                             const ia_aiq_ae_results* aeResults,
                                             const ia_aiq_af_results* afResults,
                                             const ia_aiq_sensor_events* sensorEvents,
                                             uint64_t frameReadoutStart, uint64_t frameReadoutEnd) {
    LOGIPC("@%s, pData:%p, size:%d", __func__, pData, size);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(size < sizeof(DvsStatistcs), false, "@%s, buffer is small", __func__);
    CheckError(!dvs, false, "@%s, dvs is nullptr", __func__);

    DvsStatistcs* params = static_cast<DvsStatistcs*>(pData);
    CLEAR(*params);
    params->dvsHandle = reinterpret_cast<uintptr_t>(dvs);

    if (statistics) {
        params->statisticsFlag = true;
        CheckError(MV_ENTRIE_COUNT < statistics->vector_count, false, "statistics buffer is small");
        params->statistics.vector_count = statistics->vector_count;
        MEMCPY_S(params->motion_vectors, MV_ENTRIE_COUNT * sizeof(ia_dvs_motion_vector),
                 statistics->motion_vectors,
                 statistics->vector_count * sizeof(ia_dvs_motion_vector));
    }

    if (aeResults) {
        params->aeResultsFlag = true;
        bool ret = mIpcAiq.flattenAeResults(*aeResults, &params->aeResultsBase);
        CheckError(!ret, false, "failed to copy ae result");
    }

    if (afResults) {
        params->afResultsFlag = true;
        params->afResultsBase = *afResults;
    }

    if (sensorEvents) {
        params->sensorEventsFlag = true;
        size_t sensorDataLen = sizeof(ia_aiq_sensor_data);
        size_t sensorEventLen = sensorDataLen * MAX_MOTION_SENSOR_COUNT_DVS;
        CheckError(MAX_MOTION_SENSOR_COUNT_DVS < sensorEvents->num_accelerometer_events, false,
                   "accelerometer_events buffer is small");
        CheckError(MAX_MOTION_SENSOR_COUNT_DVS < sensorEvents->num_gravity_events, false,
                   "gravity_events buffer is small");
        CheckError(MAX_MOTION_SENSOR_COUNT_DVS < sensorEvents->num_gyroscope_events, false,
                   "gyroscope_events buffer is small");
        CheckError(MAX_MOTION_SENSOR_COUNT_DVS < sensorEvents->num_ambient_light_events, false,
                   "ambient_light_events buffer is small");
        CheckError(MAX_MOTION_SENSOR_COUNT_DVS < sensorEvents->num_dmd_events, false,
                   "dmd_events buffer is small");
        MEMCPY_S(params->sensorEvents.accelerometer_events, sensorEventLen,
                 sensorEvents->accelerometer_events,
                 sensorEvents->num_accelerometer_events * sensorDataLen);
        MEMCPY_S(params->sensorEvents.gravity_events, sensorEventLen, sensorEvents->gravity_events,
                 sensorEvents->num_gravity_events * sensorDataLen);
        MEMCPY_S(params->sensorEvents.gyroscope_events, sensorEventLen,
                 sensorEvents->gyroscope_events,
                 sensorEvents->num_gyroscope_events * sensorDataLen);
        MEMCPY_S(params->sensorEvents.ambient_light_events,
                 sizeof(params->sensorEvents.ambient_light_events),
                 sensorEvents->ambient_light_events,
                 sensorEvents->num_ambient_light_events * sizeof(ia_aiq_ambient_light_events));
        MEMCPY_S(params->sensorEvents.dmd_events, sizeof(params->sensorEvents.dmd_events),
                 sensorEvents->dmd_events,
                 sensorEvents->num_dmd_events * sizeof(ia_aiq_dmd_sensor_events));
        params->sensorEventsBase = *sensorEvents;
    }
    params->frameReadoutStart = frameReadoutStart;
    params->frameReadoutEnd = frameReadoutEnd;

    return true;
}

bool IPCIntelDvs::serverUnflattenSetStatistics(
    void* pData, unsigned int size, ia_dvs_state** dvs, ia_dvs_statistics** statistics,
    ia_aiq_ae_results** aeResults, ia_aiq_af_results** afResults,
    ia_aiq_sensor_events** sensorEvents, uint64_t* frameReadoutStart, uint64_t* frameReadoutEnd) {
    LOGIPC("@%s, pData:%p, size:%d", __func__, pData, size);

    DvsStatistcs* params = static_cast<DvsStatistcs*>(pData);
    *dvs = reinterpret_cast<ia_dvs_state*>(params->dvsHandle);

    *statistics = nullptr;
    if (params->statisticsFlag) {
        params->statistics.motion_vectors = params->motion_vectors;
        *statistics = &params->statistics;
    }

    *aeResults = nullptr;
    if (params->aeResultsFlag) {
        bool ret = mIpcAiq.unflattenAeResults(&params->aeResultsBase);
        CheckError(!ret, false, "@%s, unflattenAeResults fails", __func__);
        *aeResults = &params->aeResultsBase.base;
    }

    *afResults = nullptr;
    if (params->afResultsFlag) {
        *afResults = &params->afResultsBase;
    }

    *sensorEvents = nullptr;
    if (params->sensorEventsFlag) {
        if (params->sensorEventsBase.num_accelerometer_events != 0) {
            params->sensorEventsBase.accelerometer_events =
                params->sensorEvents.accelerometer_events;
        }
        if (params->sensorEventsBase.num_gravity_events != 0) {
            params->sensorEventsBase.gravity_events = params->sensorEvents.gravity_events;
        }
        if (params->sensorEventsBase.num_gyroscope_events != 0) {
            params->sensorEventsBase.gyroscope_events = params->sensorEvents.gyroscope_events;
        }
        if (params->sensorEventsBase.num_ambient_light_events != 0) {
            params->sensorEventsBase.ambient_light_events =
                params->sensorEvents.ambient_light_events;
        }
        if (params->sensorEventsBase.num_dmd_events != 0) {
            params->sensorEventsBase.dmd_events = params->sensorEvents.dmd_events;
        }
        *sensorEvents = &params->sensorEventsBase;
    }

    *frameReadoutStart = params->frameReadoutStart;
    *frameReadoutEnd = params->frameReadoutEnd;

    return true;
}

bool IPCIntelDvs::clientFlattenExecute(void* pData, unsigned int size, ia_dvs_state* dvs,
                                       uint16_t focusPosition) {
    LOGIPC("@%s, pData:%p, size:%d, dvs:%p, focusPosition:%d", __func__, pData, size, dvs,
           focusPosition);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(size < sizeof(DvsExecute), false, "@%s, buffer is small", __func__);
    CheckError(!dvs, false, "@%s, dvs is nullptr", __func__);

    DvsExecute* params = static_cast<DvsExecute*>(pData);
    params->dvsHandle = reinterpret_cast<uintptr_t>(dvs);
    params->focusPosition = focusPosition;

    return true;
}

bool IPCIntelDvs::serverUnflattenExecute(const void* pData, unsigned int size, ia_dvs_state** dvs,
                                         uint16_t* focusPosition) {
    LOGIPC("@%s, pData:%p, size:%d, dvs:%p, focusPosition:%p", __func__, pData, size, dvs,
           focusPosition);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(size < sizeof(DvsExecute), false, "@%s, buffer is small", __func__);
    CheckError(!dvs, false, "@%s, dvs is nullptr", __func__);
    CheckError(!focusPosition, false, "@%s, focusPosition is nullptr", __func__);

    const DvsExecute* params = static_cast<const DvsExecute*>(pData);
    *dvs = reinterpret_cast<ia_dvs_state*>(params->dvsHandle);
    *focusPosition = params->focusPosition;

    return true;
}

bool IPCIntelDvs::clientFlattenImageTransformation(void* pData, unsigned int size,
                                                   ia_dvs_state* dvs) {
    LOGIPC("@%s, pData:%p, size:%d, dvs:%p", __func__, pData, size, dvs);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(size < sizeof(DvsImageTransformation), false, "@%s, buffer is small", __func__);
    CheckError(!dvs, false, "@%s, dvs is nullptr", __func__);

    DvsImageTransformation* params = static_cast<DvsImageTransformation*>(pData);
    params->dvsHandle = reinterpret_cast<uintptr_t>(dvs);

    return true;
}

bool IPCIntelDvs::serverUnflattenImageTransformation(void* pData, unsigned int size,
                                                     ia_dvs_state** dvs) {
    LOGIPC("@%s, pData:%p, size:%d, dvs:%p", __func__, pData, size, dvs);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(size < sizeof(DvsImageTransformation), false, "@%s, buffer is small", __func__);
    CheckError(!dvs, false, "@%s, dvs is nullptr", __func__);

    DvsImageTransformation* params = static_cast<DvsImageTransformation*>(pData);
    *dvs = reinterpret_cast<ia_dvs_state*>(params->dvsHandle);

    return true;
}

bool IPCIntelDvs::serverFlattenImageTransformation(
    void* pData, unsigned int size, ia_dvs_image_transformation* imageTransformation) {
    LOGIPC("@%s, pData:%p, size:%d", __func__, pData, size);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(size < sizeof(DvsImageTransformation), false, "@%s, buffer is small", __func__);
    CheckError(!imageTransformation, false, "@%s, imageTransformation is nullptr", __func__);

    DvsImageTransformation* params = static_cast<DvsImageTransformation*>(pData);
    params->imageTransformation = *imageTransformation;

    return true;
}

bool IPCIntelDvs::clientUnflattenImageTransformation(
    const void* pData, unsigned int size, ia_dvs_image_transformation** imageTransformation) {
    LOGIPC("@%s, pData:%p, size:%d", __func__, pData, size);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(size < sizeof(DvsImageTransformation), false, "@%s, buffer is small", __func__);
    CheckError(!imageTransformation, false, "@%s, imageTransformation is nullptr", __func__);

    DvsImageTransformation* params =
        const_cast<DvsImageTransformation*>(static_cast<const DvsImageTransformation*>(pData));
    *imageTransformation = &params->imageTransformation;

    return true;
}

}  // namespace icamera
