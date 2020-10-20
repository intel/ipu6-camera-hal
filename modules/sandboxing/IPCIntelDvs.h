/*
 * Copyright (C) 2019-2020 Intel Corporation.
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

#include <ia_dvs.h>
#include <ia_dvs_types.h>
#include <ia_isp_bxt.h>

#include "DvsResult.h"
#include "modules/sandboxing/IPCCommon.h"
#include "modules/sandboxing/IPCIntelAiq.h"

namespace icamera {
#define MAX_DVS_DATA_SIZE MAX_IA_BINARY_DATA_SIZE

struct DvsInitParams {
    ia_binary_data base;
    unsigned char data[MAX_DVS_DATA_SIZE];
    uintptr_t cmcHandle;

    // the below is returned
    uintptr_t dvsHandle;
};

struct DvsDeinitParams {
    uintptr_t dvsHandle;
};

struct DvsConfigParams {
    uintptr_t dvsHandle;
    ia_dvs_configuration configData;
    float zoomRatio;
};

struct DvsNoneBlankRatioParams {
    uintptr_t dvsHandle;
    float nonBlankingRatio;
};

struct DvsDigitalZoomMode {
    uintptr_t dvsHandle;
    ia_dvs_zoom_mode zoomMode;
};

struct DvsDigitalZoomRegion {
    uintptr_t dvsHandle;
    ia_rectangle zoomRegion;
};

struct DvsDigitalZoomCoordinate {
    uintptr_t dvsHandle;
    ia_coordinate zoomCoordinate;
};

struct DvsDigitalZoomMagnitude {
    uintptr_t dvsHandle;
    float zoomRatio;
};

struct DvsFreeMorphParams {
    uintptr_t dvsHandle;
    uintptr_t morphHandle;
};

struct DvsMorphParams {
    uintptr_t dvsHandle;
    uintptr_t morphHandle;  // save remote morph table pointer

    ia_dvs_morph_table morphTable;  // used by client
    uint32_t xcoordsY[MAX_DVS_COORDS_Y_SIZE];
    uint32_t ycoordsY[MAX_DVS_COORDS_Y_SIZE];
    uint32_t xcoordsUV[MAX_DVS_COORDS_UV_SIZE];
    uint32_t ycoordsUV[MAX_DVS_COORDS_UV_SIZE];
    float xcoordsUVFloat[MAX_DVS_COORDS_UV_SIZE];
    float ycoordsUVFloat[MAX_DVS_COORDS_UV_SIZE];
};

struct DvsExecute {
    uintptr_t dvsHandle;
    uint16_t focusPosition;
};

struct DvsImageTransformation {
    uintptr_t dvsHandle;
    ia_dvs_image_transformation imageTransformation;
};

#define MAX_MOTION_SENSOR_COUNT_DVS 100
struct DvsSensorEvents {
    ia_aiq_sensor_data accelerometer_events[MAX_MOTION_SENSOR_COUNT_DVS];
    ia_aiq_sensor_data gravity_events[MAX_MOTION_SENSOR_COUNT_DVS];
    ia_aiq_sensor_data gyroscope_events[MAX_MOTION_SENSOR_COUNT_DVS];
    ia_aiq_ambient_light_events ambient_light_events[MAX_MOTION_SENSOR_COUNT_DVS];
    ia_aiq_dmd_sensor_events dmd_events[MAX_MOTION_SENSOR_COUNT_DVS];
};

#define MV_ENTRIE_COUNT 5000
struct DvsStatistcs {
    uintptr_t dvsHandle;

    bool statisticsFlag;
    ia_dvs_statistics statistics;
    ia_dvs_motion_vector motion_vectors[MV_ENTRIE_COUNT];

    bool aeResultsFlag;
    ae_run_params_results aeResultsBase;

    bool afResultsFlag;
    ia_aiq_af_results afResultsBase;

    bool sensorEventsFlag;
    ia_aiq_sensor_events sensorEventsBase;
    DvsSensorEvents sensorEvents;

    uint64_t frameReadoutStart;
    uint64_t frameReadoutEnd;
};

class IPCIntelDvs {
 public:
    IPCIntelDvs();
    ~IPCIntelDvs();

    // for DvsInitParams
    bool clientFlattenInit(void* pData, unsigned int size, const ia_binary_data* dvsDataPtr,
                           const ia_cmc_t* cmc);
    bool serverUnflattenInit(void* pData, int size, ia_binary_data** dvsDataPtr, ia_cmc_t** cmc);
    bool serverFlattenInit(void* pData, unsigned int size, ia_dvs_state* dvs);
    bool clientUnflattenInit(const void* pData, unsigned int size, ia_dvs_state** dvs);

    // for DvsDeinitParams
    bool clientFlattenDeinit(void* pData, unsigned int size, ia_dvs_state* dvs);
    bool serverUnflattenDeinit(const void* pData, unsigned int size, ia_dvs_state** dvs);

    // for DvsConfigParams
    bool clientFlattenConfig(void* pData, unsigned int size, ia_dvs_state* dvs,
                             const ia_dvs_configuration* config, float zoomRatio);
    bool serverUnflattenConfig(const void* pData, unsigned int size, ia_dvs_state** dvs,
                               ia_dvs_configuration** config, float* zoomRatio);

    // for DvsNoneBlankRatioParams
    bool clientFlattenNoneBlanckRation(void* pData, unsigned int size, ia_dvs_state* dvs,
                                       float nonBlankingRatio);
    bool serverUnflattenNoneBlanckRation(const void* pData, unsigned int size, ia_dvs_state** dvs,
                                         float* nonBlankingRatio);

    // for DvsDigitalZoomMode
    bool clientFlattenDigitalZoomMode(void* pData, unsigned int size, ia_dvs_state* dvs,
                                      ia_dvs_zoom_mode zoomMode);
    bool serverUnflattenDigitalZoomMode(const void* pData, unsigned int size, ia_dvs_state** dvs,
                                        ia_dvs_zoom_mode* zoomMode);

    // for DvsDigitalZoomRegion
    bool clientFlattenDigitalZoomRegion(void* pData, unsigned int size, ia_dvs_state* dvs,
                                        ia_rectangle* zoomRegion);
    bool serverUnflattenDigitalZoomRegion(const void* pData, unsigned int size, ia_dvs_state** dvs,
                                          ia_rectangle** zoomRegion);

    // for DvsDigitalZoomCoordinate
    bool clientFlattenDigitalZoomCoordinate(void* pData, unsigned int size, ia_dvs_state* dvs,
                                            ia_coordinate* zoomCoordinate);
    bool serverUnflattenDigitalZoomCoordinate(const void* pData, unsigned int size,
                                              ia_dvs_state** dvs, ia_coordinate** zoomCoordinate);

    // for DvsDigitalZoomMagnitude
    bool clientFlattenDigitalZoomMagnitude(void* pData, unsigned int size, ia_dvs_state* dvs,
                                           float zoomRatio);
    bool serverUnflattenDigitalZoomMagnitude(const void* pData, unsigned int size,
                                             ia_dvs_state** dvs, float* zoomRatio);

    // for DvsFreeMorphParams
    bool clientFlattenFreeMorphTable(void* pData, unsigned int size, ia_dvs_state* dvs,
                                     ia_dvs_morph_table* morphTable);
    bool serverUnflattenFreeMorphTable(const void* pData, unsigned int size, ia_dvs_state** dvs,
                                       ia_dvs_morph_table** morphTable);

    // for DvsMorphParams
    bool clientFlattenAllocateMorphTable(void* pData, unsigned int size, ia_dvs_state* dvs);
    bool serverUnflattenAllocateMorphTalbe(const void* pData, unsigned int size,
                                           ia_dvs_state** dvs);
    bool serverFlattenAllocateMorphTalbe(void* pData, unsigned int size,
                                         ia_dvs_morph_table* morphTable);
    bool clientUnflattenAllocateMorphTalbe(const void* pData, unsigned int size,
                                           ia_dvs_morph_table** morphTable);
    bool clientFlattenGetMorphTable(void* pData, unsigned int size, ia_dvs_state* dvs,
                                    ia_dvs_morph_table* morphTable);
    bool serverUnflattenGetMorphTalbe(const void* pData, unsigned int size, ia_dvs_state** dvs,
                                      ia_dvs_morph_table** morphTable);
    bool serverFlattenGetMorphTalbe(void* pData, unsigned int size, ia_dvs_morph_table* morphTable);
    bool clientUnflattenGetMorphTalbe(void* pData, unsigned int size,
                                      ia_dvs_morph_table** morphTable);
    bool unflattenMorphTalbe(DvsMorphParams* params);
    bool flattenMorphTable(const ia_dvs_morph_table* morphTable, DvsMorphParams* params);

    // for DvsStatistcs
    bool clientFlattenSetStatistics(void* pData, unsigned int size, ia_dvs_state* dvs,
                                    const ia_dvs_statistics* statistics,
                                    const ia_aiq_ae_results* aeResults,
                                    const ia_aiq_af_results* afResults,
                                    const ia_aiq_sensor_events* sensorEvents,
                                    uint64_t frameReadoutStart, uint64_t frameReadoutEnd);
    bool serverUnflattenSetStatistics(void* pData, unsigned int size, ia_dvs_state** dvs,
                                      ia_dvs_statistics** statistics, ia_aiq_ae_results** aeResults,
                                      ia_aiq_af_results** afResults,
                                      ia_aiq_sensor_events** sensorEvents,
                                      uint64_t* frameReadoutStart, uint64_t* frameReadoutEnd);

    // for DvsExecute
    bool clientFlattenExecute(void* pData, unsigned int size, ia_dvs_state* dvs,
                              uint16_t focusPosition);
    bool serverUnflattenExecute(const void* pData, unsigned int size, ia_dvs_state** dvs,
                                uint16_t* focusPosition);

    // for DvsImageTransformation
    bool clientFlattenImageTransformation(void* pData, unsigned int size, ia_dvs_state* dvs);
    bool serverUnflattenImageTransformation(void* pData, unsigned int size, ia_dvs_state** dvs);
    bool serverFlattenImageTransformation(void* pData, unsigned int size,
                                          ia_dvs_image_transformation* imageTransformation);
    bool clientUnflattenImageTransformation(const void* pData, unsigned int size,
                                            ia_dvs_image_transformation** imageTransformation);

 private:
    IPCIntelAiq mIpcAiq;
};
}  // namespace icamera
