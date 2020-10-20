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

#define LOG_TAG "IntelDvs"

#include "modules/algowrapper/IntelDvs.h"

#include "AiqUtils.h"
#include "iutils/CameraLog.h"
#include "iutils/Utils.h"

namespace icamera {
IntelDvs::IntelDvs() {
    LOG1("@%s", __func__);
}

IntelDvs::~IntelDvs() {
    LOG1("@%s", __func__);
}

ia_err IntelDvs::init(const ia_binary_data& aiqTuningBinary, const ia_cmc_t* cmc,
                      ia_dvs_state** dvsHandle) {
    LOG1("@%s", __func__);
    CheckError(!dvsHandle, ia_err_none, "@%s, dvsHandle is nullptr", __func__);

    ia_err err = ia_dvs_init(dvsHandle, &aiqTuningBinary, cmc);
    CheckError(err != ia_err_none, ia_err_general, "@%s, Failed to init dvs lib", __func__);
    return err;
}

void IntelDvs::deinit(ia_dvs_state* dvsHandle) {
    LOG1("@%s", __func__);
    CheckError(!dvsHandle, VOID_VALUE, "@%s, dvsHandle is nullptr", __func__);

    ia_dvs_deinit(dvsHandle);
}

ia_err IntelDvs::config(ia_dvs_state* dvsHandle, ia_dvs_configuration* config, float zoomRatio) {
    LOG1("@%s", __func__);
    CheckError(!dvsHandle, ia_err_general, "@%s, dvsHandle is nullptr", __func__);
    CheckError(!config, ia_err_general, "@%s, config is nullptr", __func__);

    ia_err err = ia_dvs_config(dvsHandle, config, zoomRatio);
    CheckError(err != ia_err_none, err, "@%s, ia_dvs_config fails", __func__);
    return err;
}

ia_err IntelDvs::setNonBlankRatio(ia_dvs_state* dvsHandle, float nonBlankingRatio) {
    LOG1("@%s", __func__);
    CheckError(!dvsHandle, ia_err_general, "@%s, dvsHandle is nullptr", __func__);

    ia_err err = ia_dvs_set_non_blank_ratio(dvsHandle, nonBlankingRatio);
    CheckError(err != ia_err_none, err, "@%s, ia_dvs_set_non_blank_ratio fails", __func__);
    return err;
}

ia_err IntelDvs::setDigitalZoomMode(ia_dvs_state* dvsHandle, ia_dvs_zoom_mode zoomMode) {
    LOG1("@%s", __func__);
    CheckError(!dvsHandle, ia_err_general, "@%s, dvsHandle is nullptr", __func__);

    ia_err err = ia_dvs_set_digital_zoom_mode(dvsHandle, zoomMode);
    CheckError(err != ia_err_none, err, "@%s, ia_dvs_set_digital_zoom_mode fails", __func__);
    return err;
}

ia_err IntelDvs::setDigitalZoomRegion(ia_dvs_state* dvsHandle, ia_rectangle* zoomRegion) {
    LOG1("@%s", __func__);
    CheckError(!dvsHandle, ia_err_general, "@%s, dvsHandle is nullptr", __func__);
    CheckError(!zoomRegion, ia_err_general, "@%s, zoomRegion is nullptr", __func__);

    ia_err err = ia_dvs_set_digital_zoom_region(dvsHandle, zoomRegion);
    CheckError(err != ia_err_none, err, "@%s, ia_dvs_set_digital_zoom_region fails", __func__);
    return err;
}

ia_err IntelDvs::setDigitalZoomCoordinate(ia_dvs_state* dvsHandle, ia_coordinate* zoomCoordinate) {
    LOG1("@%s", __func__);
    CheckError(!dvsHandle, ia_err_general, "@%s, dvsHandle is nullptr", __func__);
    CheckError(!zoomCoordinate, ia_err_general, "@%s, zoomCoordinate is nullptr", __func__);

    ia_err err = ia_dvs_set_digital_zoom_coordinate(dvsHandle, zoomCoordinate);
    CheckError(err != ia_err_none, err, "@%s, ia_dvs_set_digital_zoom_coordinate fails", __func__);
    return err;
}

ia_err IntelDvs::setDigitalZoomMagnitude(ia_dvs_state* dvsHandle, float zoomRatio) {
    LOG1("@%s", __func__);
    CheckError(!dvsHandle, ia_err_general, "@%s, dvsHandle is nullptr", __func__);

    ia_err err = ia_dvs_set_digital_zoom_magnitude(dvsHandle, zoomRatio);
    CheckError(err != ia_err_none, err, "@%s, ia_dvs_set_digital_zoom_magnitude fails", __func__);
    return err;
}

void IntelDvs::freeMorphTable(ia_dvs_state* dvsHandle, ia_dvs_morph_table* morphTable) {
    LOG1("@%s, dvsHandle:%p, morphTable:%p", __func__, dvsHandle, morphTable);
    CheckError(!morphTable, VOID_VALUE, "@%s, morphTable is nullptr", __func__);

    ia_dvs_free_morph_table(morphTable);
}

ia_dvs_morph_table* IntelDvs::allocateMorphTalbe(ia_dvs_state* dvsHandle) {
    LOG1("@%s", __func__);
    CheckError(!dvsHandle, nullptr, "@%s, dvsHandle is nullptr", __func__);

    ia_dvs_morph_table* morphTable = nullptr;
    ia_err err = ia_dvs_allocate_morph_table(dvsHandle, &morphTable);
    CheckError((!morphTable || err != ia_err_none), nullptr,
               "@%s, ia_dvs_allocate_morph_table fails", __func__);
    return morphTable;
}

int IntelDvs::getMorphTable(ia_dvs_state* dvsHandle, ia_dvs_morph_table* morphTable) {
    LOG2("@%s", __func__);
    CheckError(!dvsHandle, UNKNOWN_ERROR, "@%s, dvsHandle is nullptr", __func__);
    CheckError(!morphTable, UNKNOWN_ERROR, "@%s, morphTable is nullptr", __func__);

    ia_err err = ia_dvs_get_morph_table(dvsHandle, morphTable);
    CheckError(err != ia_err_none, UNKNOWN_ERROR, "@%s, ia_dvs_get_morph_table fails, err:%d",
               __func__, err);

    return OK;
}

int IntelDvs::getMorphTable(ia_dvs_state* dvsHandle, ia_dvs_morph_table* morphTable,
                            DvsResult* result) {
    LOG2("@%s", __func__);

    int ret = getMorphTable(dvsHandle, morphTable);
    CheckError(ret != OK, UNKNOWN_ERROR, "@%s, getMorphTable fails", __func__);

    ret = DvsResult::deepCopyDvsResults(*morphTable, &result->mMorphTable);
    CheckError(ret != OK, UNKNOWN_ERROR, "@%s, deepCopyDvsResults fails", __func__);

    return OK;
}

ia_err IntelDvs::setStatistics(ia_dvs_state* dvsHandle, const ia_dvs_statistics* statistics,
                               const ia_aiq_ae_results* aeResults,
                               const ia_aiq_af_results* afResults,
                               const ia_aiq_sensor_events* sensorEvents, uint64_t frameReadoutStart,
                               uint64_t frameReadoutEnd) {
    LOG1("@%s", __func__);
    CheckError(!dvsHandle, ia_err_general, "@%s, dvsHandle is nullptr", __func__);

    ia_err err = ia_dvs_set_statistics(dvsHandle, statistics, aeResults, afResults, sensorEvents,
                                       frameReadoutStart, frameReadoutEnd);
    CheckError(err != ia_err_none, ia_err_general, "@%s, ia_dvs_set_statistics fails", __func__);
    return err;
}

ia_err IntelDvs::execute(ia_dvs_state* dvsHandle, uint16_t focusPosition) {
    LOG1("@%s", __func__);
    CheckError(!dvsHandle, ia_err_general, "@%s, dvsHandle is nullptr", __func__);

    ia_err err = ia_dvs_execute(dvsHandle, focusPosition);
    CheckError(err != ia_err_none, ia_err_general, "@%s, ia_dvs_execute fails", __func__);
    return err;
}

ia_err IntelDvs::getImageTransformation(ia_dvs_state* dvsHandle,
                                        ia_dvs_image_transformation* imageTransformation) {
    LOG1("@%s", __func__);
    CheckError(!dvsHandle, ia_err_general, "@%s, dvsHandle is nullptr", __func__);
    CheckError(!imageTransformation, ia_err_general, "@%s, imageTransformation is nullptr",
               __func__);

    ia_err err = ia_dvs_get_image_transformation(dvsHandle, imageTransformation);
    CheckError(err != ia_err_none, ia_err_general, "@%s, ia_dvs_get_image_transformation fails",
               __func__);
    return err;
}
} /* namespace icamera */
