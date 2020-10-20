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

#define LOG_TAG "IntelDvsServer"

#include "modules/sandboxing/server/IntelDvsServer.h"

#include <utility>

#include "CameraLog.h"
#include "iutils/Utils.h"

namespace icamera {
IntelDvsServer::IntelDvsServer() {
    LOGIPC("@%s", __func__);
}

IntelDvsServer::~IntelDvsServer() {
    LOGIPC("@%s", __func__);

    mIntelDvss.clear();
}

status_t IntelDvsServer::init(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);

    ia_binary_data* dvsDataPtr = nullptr;
    ia_cmc_t* cmc = nullptr;
    bool ret = mIpc.serverUnflattenInit(pData, dataSize, &dvsDataPtr, &cmc);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverUnflattenInit fails", __func__);

    std::unique_ptr<IntelDvs> intelDvs = std::make_unique<IntelDvs>();

    ia_dvs_state* dvs = nullptr;
    ia_err err = intelDvs->init(*dvsDataPtr, cmc, &dvs);
    CheckError(err != ia_err_none, UNKNOWN_ERROR, "@%s, intelDvs->init fails", __func__);

    ret = mIpc.serverFlattenInit(pData, dataSize, dvs);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverflattenInit fails", __func__);

    mIntelDvss[dvs] = std::move(intelDvs);

    return OK;
}

status_t IntelDvsServer::deinit(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);

    ia_dvs_state* dvs = nullptr;
    bool ret = mIpc.serverUnflattenDeinit(pData, dataSize, &dvs);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverUnflattenDeinit fails", __func__);

    IntelDvs* intelDvs = getIntelDvs(dvs);
    CheckError(intelDvs == nullptr, UNKNOWN_ERROR, "@%s, dvs:%p doesn't exist", __func__, dvs);

    intelDvs->deinit(dvs);
    return OK;
}

status_t IntelDvsServer::config(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);

    ia_dvs_state* dvs = nullptr;
    ia_dvs_configuration* config = nullptr;
    float zoomRatio = 0;
    bool ret = mIpc.serverUnflattenConfig(pData, dataSize, &dvs, &config, &zoomRatio);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverUnflattenConfig fails", __func__);

    IntelDvs* intelDvs = getIntelDvs(dvs);
    CheckError(intelDvs == nullptr, UNKNOWN_ERROR, "@%s, dvs:%p doesn't exist", __func__, dvs);

    ia_err err = intelDvs->config(dvs, config, zoomRatio);
    CheckError(err != ia_err_none, UNKNOWN_ERROR, "@%s, config fails", __func__);
    return OK;
}

status_t IntelDvsServer::setNonBlankRatio(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);

    ia_dvs_state* dvs = nullptr;
    float nonBlankingRatio = 0;
    bool ret = mIpc.serverUnflattenNoneBlanckRation(pData, dataSize, &dvs, &nonBlankingRatio);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverUnflattenNoneBlanckRation fails", __func__);

    IntelDvs* intelDvs = getIntelDvs(dvs);
    CheckError(intelDvs == nullptr, UNKNOWN_ERROR, "@%s, dvs:%p doesn't exist", __func__, dvs);

    ia_err err = intelDvs->setNonBlankRatio(dvs, nonBlankingRatio);
    CheckError(err != ia_err_none, UNKNOWN_ERROR, "@%s, setNonBlankRatio fails", __func__);
    return OK;
}

status_t IntelDvsServer::setDigitalZoomMode(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);

    ia_dvs_state* dvs = nullptr;
    ia_dvs_zoom_mode zoomMode = ia_dvs_zoom_mode_center;
    bool ret = mIpc.serverUnflattenDigitalZoomMode(pData, dataSize, &dvs, &zoomMode);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverUnflattenDigitalZoomMode fails", __func__);

    IntelDvs* intelDvs = getIntelDvs(dvs);
    CheckError(intelDvs == nullptr, UNKNOWN_ERROR, "@%s, dvs:%p doesn't exist", __func__, dvs);

    ia_err err = intelDvs->setDigitalZoomMode(dvs, zoomMode);
    CheckError(err != ia_err_none, UNKNOWN_ERROR, "@%s, setDigitalZoomMode fails", __func__);
    return OK;
}

status_t IntelDvsServer::setDigitalZoomRegion(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);

    ia_dvs_state* dvs = nullptr;
    ia_rectangle* zoomRegion = nullptr;
    bool ret = mIpc.serverUnflattenDigitalZoomRegion(pData, dataSize, &dvs, &zoomRegion);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverUnflattenDigitalZoomRegion fails",
               __func__);

    IntelDvs* intelDvs = getIntelDvs(dvs);
    CheckError(intelDvs == nullptr, UNKNOWN_ERROR, "@%s, dvs:%p doesn't exist", __func__, dvs);

    ia_err err = intelDvs->setDigitalZoomRegion(dvs, zoomRegion);
    CheckError(err != ia_err_none, UNKNOWN_ERROR, "@%s, setDigitalZoomRegion fails", __func__);
    return OK;
}

status_t IntelDvsServer::setDigitalZoomCoordinate(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);

    ia_dvs_state* dvs = nullptr;
    ia_coordinate* zoomCoordinate = nullptr;
    bool ret = mIpc.serverUnflattenDigitalZoomCoordinate(pData, dataSize, &dvs, &zoomCoordinate);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverUnflattenDigitalZoomCoordinate fails",
               __func__);

    IntelDvs* intelDvs = getIntelDvs(dvs);
    CheckError(intelDvs == nullptr, UNKNOWN_ERROR, "@%s, dvs:%p doesn't exist", __func__, dvs);

    ia_err err = intelDvs->setDigitalZoomCoordinate(dvs, zoomCoordinate);
    CheckError(err != ia_err_none, UNKNOWN_ERROR, "@%s, setDigitalZoomCoordinate fails", __func__);
    return OK;
}

status_t IntelDvsServer::setDigitalZoomMagnitude(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);

    ia_dvs_state* dvs = nullptr;
    float zoomRatio = 0;
    bool ret = mIpc.serverUnflattenDigitalZoomMagnitude(pData, dataSize, &dvs, &zoomRatio);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverUnflattenDigitalZoomMagnitude fails",
               __func__);

    IntelDvs* intelDvs = getIntelDvs(dvs);
    CheckError(intelDvs == nullptr, UNKNOWN_ERROR, "@%s, dvs:%p doesn't exist", __func__, dvs);

    ia_err err = intelDvs->setDigitalZoomMagnitude(dvs, zoomRatio);
    CheckError(err != ia_err_none, UNKNOWN_ERROR, "@%s, setDigitalZoomMagnitude fails", __func__);
    return OK;
}

status_t IntelDvsServer::freeMorphTable(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);

    ia_dvs_morph_table* morph = nullptr;
    ia_dvs_state* dvs = nullptr;
    bool ret = mIpc.serverUnflattenFreeMorphTable(pData, dataSize, &dvs, &morph);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverUnflattenFreeMorphTable fails", __func__);

    IntelDvs* intelDvs = getIntelDvs(dvs);
    CheckError(intelDvs == nullptr, UNKNOWN_ERROR, "@%s, dvs:%p doesn't exist", __func__, dvs);

    intelDvs->freeMorphTable(dvs, morph);
    return OK;
}

status_t IntelDvsServer::allocateMorphTalbe(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);

    ia_dvs_state* dvs = nullptr;
    bool ret = mIpc.serverUnflattenAllocateMorphTalbe(pData, dataSize, &dvs);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverUnflattenAllocateMorphTalbe fails",
               __func__);

    IntelDvs* intelDvs = getIntelDvs(dvs);
    CheckError(intelDvs == nullptr, UNKNOWN_ERROR, "@%s, dvs:%p doesn't exist", __func__, dvs);

    ia_dvs_morph_table* morph = intelDvs->allocateMorphTalbe(dvs);

    ret = mIpc.serverFlattenAllocateMorphTalbe(pData, dataSize, morph);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverFlattenAllocateMorphTalbe fails", __func__);

    return OK;
}

status_t IntelDvsServer::getMorphTalbe(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);

    ia_dvs_state* dvs = nullptr;
    ia_dvs_morph_table* morph = nullptr;
    bool ret = mIpc.serverUnflattenGetMorphTalbe(pData, dataSize, &dvs, &morph);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverUnflattenGetMorphTalbe fails", __func__);

    IntelDvs* intelDvs = getIntelDvs(dvs);
    CheckError(intelDvs == nullptr, UNKNOWN_ERROR, "@%s, dvs:%p doesn't exist", __func__, dvs);

    int err = intelDvs->getMorphTable(dvs, morph);
    CheckError(err != OK, UNKNOWN_ERROR, "@%s, getMorphTalbe fails", __func__);

    ret = mIpc.serverFlattenGetMorphTalbe(pData, dataSize, morph);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverFlattenGetMorphTalbe fails", __func__);

    return OK;
}

status_t IntelDvsServer::setStatistics(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);

    ia_dvs_state* dvs = nullptr;
    ia_dvs_statistics* statistics = nullptr;
    ia_aiq_ae_results* aeResults = nullptr;
    ia_aiq_af_results* afResults = nullptr;
    ia_aiq_sensor_events* sensorEvents = nullptr;
    uint64_t frameReadoutStart = 0;
    uint64_t frameReadoutEnd = 0;
    bool ret = mIpc.serverUnflattenSetStatistics(pData, dataSize, &dvs, &statistics, &aeResults,
                                                 &afResults, &sensorEvents, &frameReadoutStart,
                                                 &frameReadoutEnd);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverUnflattenSetStatistics fails", __func__);

    IntelDvs* intelDvs = getIntelDvs(dvs);
    CheckError(intelDvs == nullptr, UNKNOWN_ERROR, "@%s, dvs:%p doesn't exist", __func__, dvs);

    ia_err err = intelDvs->setStatistics(dvs, statistics, aeResults, afResults, sensorEvents,
                                         frameReadoutStart, frameReadoutEnd);
    CheckError(err != ia_err_none, UNKNOWN_ERROR, "@%s, execute fails", __func__);
    return OK;
}

status_t IntelDvsServer::execute(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);

    ia_dvs_state* dvs = nullptr;
    uint16_t focusPosition = 0;
    bool ret = mIpc.serverUnflattenExecute(pData, dataSize, &dvs, &focusPosition);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverUnflattenExecute fails", __func__);

    IntelDvs* intelDvs = getIntelDvs(dvs);
    CheckError(intelDvs == nullptr, UNKNOWN_ERROR, "@%s, dvs:%p doesn't exist", __func__, dvs);

    ia_err err = intelDvs->execute(dvs, focusPosition);
    CheckError(err != ia_err_none, UNKNOWN_ERROR, "@%s, execute fails", __func__);
    return OK;
}

status_t IntelDvsServer::getImageTransformation(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);

    ia_dvs_state* dvs = nullptr;
    ia_dvs_image_transformation imageTransformation = {0};
    bool ret = mIpc.serverUnflattenImageTransformation(pData, dataSize, &dvs);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverUnflattenImageTransformation fails",
               __func__);

    IntelDvs* intelDvs = getIntelDvs(dvs);
    CheckError(intelDvs == nullptr, UNKNOWN_ERROR, "@%s, dvs:%p doesn't exist", __func__, dvs);

    ia_err err = intelDvs->getImageTransformation(dvs, &imageTransformation);
    CheckError(err != ia_err_none, UNKNOWN_ERROR, "@%s, getImageTransformation fails", __func__);

    ret = mIpc.serverFlattenImageTransformation(pData, dataSize, &imageTransformation);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverUnflattenImageTransformation fails",
               __func__);
    return OK;
}

IntelDvs* IntelDvsServer::getIntelDvs(ia_dvs_state* dvs) {
    LOGIPC("@%s, dvs:%p", __func__, dvs);

    if (mIntelDvss.find(dvs) == mIntelDvss.end()) {
        LOGE("@%s, dvs:%p doesn't exist", __func__, dvs);
        return nullptr;
    }

    return mIntelDvss[dvs].get();
}
}  // namespace icamera
