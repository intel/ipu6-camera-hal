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

#define LOG_TAG "ClientIntelDvs"

#include "modules/sandboxing/client/IntelDvs.h"

#include <string>

#include "CameraLog.h"
#include "iutils/Utils.h"

namespace icamera {
IntelDvs::IntelDvs() : mInitialized(false) {
    LOGIPC("@%s", __func__);

    uintptr_t personal = reinterpret_cast<uintptr_t>(this);
    std::string initName = "/dvsInit" + std::to_string(personal) + SHM_NAME;
    std::string deinitName = "/dvsDeinit" + std::to_string(personal) + SHM_NAME;
    std::string configName = "/dvsConfig" + std::to_string(personal) + SHM_NAME;
    std::string setNonBlankRatioName = "/dvsSetNonBlankRatio" + std::to_string(personal) + SHM_NAME;
    std::string setDigitalZoomModeName =
        "/dvsSetDigitalZoomModeName" + std::to_string(personal) + SHM_NAME;
    std::string setDigitalZoomRegionName =
        "/dvsSetDigitalZoomRegionName" + std::to_string(personal) + SHM_NAME;
    std::string setDigitalZoomCoordinateName =
        "/dvsSetDigitalZoomCoordinateName" + std::to_string(personal) + SHM_NAME;
    std::string setDigitalZoomMagnitudeName =
        "/dvsSetDigitalZoomMagnitudeName" + std::to_string(personal) + SHM_NAME;
    std::string freeMorphTableName = "/dvsFreeMorphTableName" + std::to_string(personal) + SHM_NAME;
    std::string allocateMorphTalbeName =
        "/dvsAllocateMorphTalbeName" + std::to_string(personal) + SHM_NAME;
    std::string getMorphTableName = "/dvsGetMorphTableName" + std::to_string(personal) + SHM_NAME;
    std::string setStatisticsName = "/dvsSetStatisticsName" + std::to_string(personal) + SHM_NAME;
    std::string executeName = "/dvsExecuteName" + std::to_string(personal) + SHM_NAME;
    std::string getImageTransformationName =
        "/dvsGetImageTransformationName" + std::to_string(personal) + SHM_NAME;

    mMems = {
        {initName.c_str(), sizeof(DvsInitParams), &mMemInit, false},
        {deinitName.c_str(), sizeof(DvsDeinitParams), &mMemDeinit, false},
        {configName.c_str(), sizeof(DvsConfigParams), &mMemConfig, false},
        {setNonBlankRatioName.c_str(), sizeof(DvsNoneBlankRatioParams), &mMemNonBlankRatio, false},
        {setDigitalZoomModeName.c_str(), sizeof(DvsDigitalZoomMode), &mMemZoomMode, false},
        {setDigitalZoomRegionName.c_str(), sizeof(DvsDigitalZoomRegion), &mMemZoomRegion, false},
        {setDigitalZoomCoordinateName.c_str(), sizeof(DvsDigitalZoomCoordinate),
         &mMemZoomCoordinate, false},
        {setDigitalZoomMagnitudeName.c_str(), sizeof(DvsDigitalZoomMagnitude), &mMemZoomMagnitude,
         false},
        {freeMorphTableName.c_str(), sizeof(DvsFreeMorphParams), &mMemFreeMorph, false},
        {allocateMorphTalbeName.c_str(), sizeof(DvsMorphParams), &mMemAllocateMorph, false},
        {getMorphTableName.c_str(), sizeof(DvsMorphParams), &mMemGetMorphTable, false},
        {setStatisticsName.c_str(), sizeof(DvsStatistcs), &mMemStatistics, false},
        {executeName.c_str(), sizeof(DvsExecute), &mMemExecute, false},
        {getImageTransformationName.c_str(), sizeof(DvsImageTransformation), &mMemImageTransfor,
         false},
    };

    bool success = mCommon.allocateAllShmMems(&mMems);
    if (!success) {
        mCommon.releaseAllShmMems(mMems);
        return;
    }

    LOGIPC("@%s, done", __func__);
    mInitialized = true;
}

IntelDvs::~IntelDvs() {
    LOGIPC("@%s", __func__);
    mCommon.releaseAllShmMems(mMems);
}

ia_err IntelDvs::init(const ia_binary_data& aiqTuningBinary, const ia_cmc_t* cmc,
                      ia_dvs_state** dvsHandle) {
    LOGIPC("@%s", __func__);
    CheckError(mInitialized == false, ia_err_none, "@%s, mInitialized is false", __func__);
    CheckError(aiqTuningBinary.data == nullptr, ia_err_none, "@%s, aiqTuningBinary.data is nullptr",
               __func__);
    CheckError(aiqTuningBinary.size == 0, ia_err_none, "@%s, aiqTuningBinary.size is 0", __func__);
    CheckError(dvsHandle == nullptr, ia_err_none, "@%s, dvsHandle is nullptr", __func__);

    bool ret = mIpc.clientFlattenInit(mMemInit.mAddr, mMemInit.mSize, &aiqTuningBinary, cmc);
    CheckError(ret == false, ia_err_none, "@%s, clientFlattenInit fails", __func__);

    ret = mCommon.requestSync(IPC_DVS_INIT, mMemInit.mHandle);
    CheckError(ret == false, ia_err_none, "@%s, requestSync fails", __func__);

    ret = mIpc.clientUnflattenInit(mMemInit.mAddr, mMemInit.mSize, dvsHandle);
    CheckError(ret == false, ia_err_none, "@%s, clientUnflattenInit fails", __func__);
    return ia_err_none;
}

void IntelDvs::deinit(ia_dvs_state* dvsHandle) {
    LOGIPC("@%s, dvsHandle:%p", __func__, dvsHandle);
    CheckError(mInitialized == false, VOID_VALUE, "@%s, mInitialized is false", __func__);
    CheckError(dvsHandle == nullptr, VOID_VALUE, "@%s, dvsHandle is nullptr", __func__);

    bool ret = mIpc.clientFlattenDeinit(mMemDeinit.mAddr, mMemDeinit.mSize, dvsHandle);
    CheckError(ret == false, VOID_VALUE, "@%s, clientFlattenDeinit fails", __func__);

    ret = mCommon.requestSync(IPC_DVS_DEINIT, mMemDeinit.mHandle);
    CheckError(ret == false, VOID_VALUE, "@%s, requestSync fails", __func__);
}

ia_err IntelDvs::config(ia_dvs_state* dvsHandle, ia_dvs_configuration* config, float zoomRatio) {
    LOGIPC("@%s, dvsHandle:%p, config:%p, zoomRatio:%f", __func__, dvsHandle, config, zoomRatio);
    CheckError(mInitialized == false, ia_err_general, "@%s, mInitialized is false", __func__);
    CheckError(dvsHandle == nullptr, ia_err_general, "@%s, dvsHandle is nullptr", __func__);
    CheckError(config == nullptr, ia_err_general, "@%s, config is nullptr", __func__);

    bool ret =
        mIpc.clientFlattenConfig(mMemConfig.mAddr, mMemConfig.mSize, dvsHandle, config, zoomRatio);
    CheckError(ret == false, ia_err_general, "@%s, clientFlattenConfig fails", __func__);

    ret = mCommon.requestSync(IPC_DVS_CONFIG, mMemConfig.mHandle);
    CheckError(ret == false, ia_err_general, "@%s, requestSync fails", __func__);

    return ia_err_none;
}

ia_err IntelDvs::setNonBlankRatio(ia_dvs_state* dvsHandle, float nonBlankingRatio) {
    LOGIPC("@%s, dvsHandle:%p, nonBlankingRatio:%f", __func__, dvsHandle, nonBlankingRatio);
    CheckError(mInitialized == false, ia_err_general, "@%s, mInitialized is false", __func__);
    CheckError(dvsHandle == nullptr, ia_err_general, "@%s, dvsHandle is nullptr", __func__);

    bool ret = mIpc.clientFlattenNoneBlanckRation(mMemNonBlankRatio.mAddr, mMemNonBlankRatio.mSize,
                                                  dvsHandle, nonBlankingRatio);
    CheckError(ret == false, ia_err_general, "@%s, clientFlattenNoneBlanckRation fails", __func__);

    ret = mCommon.requestSync(IPC_DVS_SET_NONE_BLANK_RATION, mMemNonBlankRatio.mHandle);
    CheckError(ret == false, ia_err_general, "@%s, requestSync fails", __func__);

    return ia_err_none;
}

ia_err IntelDvs::setDigitalZoomMode(ia_dvs_state* dvsHandle, ia_dvs_zoom_mode zoomMode) {
    LOGIPC("@%s, dvsHandle:%p, zoomMode:%f", __func__, dvsHandle, zoomMode);
    CheckError(mInitialized == false, ia_err_general, "@%s, mInitialized is false", __func__);
    CheckError(dvsHandle == nullptr, ia_err_general, "@%s, dvsHandle is nullptr", __func__);

    bool ret = mIpc.clientFlattenDigitalZoomMode(mMemZoomMode.mAddr, mMemZoomMode.mSize, dvsHandle,
                                                 zoomMode);
    CheckError(ret == false, ia_err_general, "@%s, clientFlattenDigitalZoomMode fails", __func__);

    ret = mCommon.requestSync(IPC_DVS_SET_DIGITAL_ZOOM_MODE, mMemZoomMode.mHandle);
    CheckError(ret == false, ia_err_general, "@%s, requestSync fails", __func__);

    return ia_err_none;
}

ia_err IntelDvs::setDigitalZoomRegion(ia_dvs_state* dvsHandle, ia_rectangle* zoomRegion) {
    LOGIPC("@%s, dvsHandle:%p, zoomRegion:%p", __func__, dvsHandle, zoomRegion);
    CheckError(mInitialized == false, ia_err_general, "@%s, mInitialized is false", __func__);
    CheckError(dvsHandle == nullptr, ia_err_general, "@%s, dvsHandle is nullptr", __func__);
    CheckError(zoomRegion == nullptr, ia_err_general, "@%s, zoomRegion is nullptr", __func__);

    bool ret = mIpc.clientFlattenDigitalZoomRegion(mMemZoomRegion.mAddr, mMemZoomRegion.mSize,
                                                   dvsHandle, zoomRegion);
    CheckError(ret == false, ia_err_general, "@%s, clientFlattenDigitalZoomRegion fails", __func__);

    ret = mCommon.requestSync(IPC_DVS_SET_DIGITAL_ZOOM_REGION, mMemZoomRegion.mHandle);
    CheckError(ret == false, ia_err_general, "@%s, requestSync fails", __func__);

    return ia_err_none;
}

ia_err IntelDvs::setDigitalZoomCoordinate(ia_dvs_state* dvsHandle, ia_coordinate* zoomCoordinate) {
    LOGIPC("@%s, dvsHandle:%p, zoomCoordinate:%p", __func__, dvsHandle, zoomCoordinate);
    CheckError(mInitialized == false, ia_err_general, "@%s, mInitialized is false", __func__);
    CheckError(dvsHandle == nullptr, ia_err_general, "@%s, dvsHandle is nullptr", __func__);
    CheckError(zoomCoordinate == nullptr, ia_err_general, "@%s, zoomCoordinate is nullptr",
               __func__);

    bool ret = mIpc.clientFlattenDigitalZoomCoordinate(
        mMemZoomCoordinate.mAddr, mMemZoomCoordinate.mSize, dvsHandle, zoomCoordinate);
    CheckError(ret == false, ia_err_general, "@%s, clientFlattenDigitalZoomCoordinate fails",
               __func__);

    ret = mCommon.requestSync(IPC_DVS_SET_DIGITAL_ZOOM_COORDINATE, mMemZoomCoordinate.mHandle);
    CheckError(ret == false, ia_err_general, "@%s, requestSync fails", __func__);

    return ia_err_none;
}

ia_err IntelDvs::setDigitalZoomMagnitude(ia_dvs_state* dvsHandle, float zoomRatio) {
    LOGIPC("@%s, dvsHandle:%p, zoomRatio:%f", __func__, dvsHandle, zoomRatio);
    CheckError(mInitialized == false, ia_err_general, "@%s, mInitialized is false", __func__);
    CheckError(dvsHandle == nullptr, ia_err_general, "@%s, dvsHandle is nullptr", __func__);

    bool ret = mIpc.clientFlattenDigitalZoomMagnitude(
        mMemZoomMagnitude.mAddr, mMemZoomMagnitude.mSize, dvsHandle, zoomRatio);
    CheckError(ret == false, ia_err_general, "@%s, clientFlattenDigitalZoomMagnitude fails",
               __func__);

    ret = mCommon.requestSync(IPC_DVS_SET_DIGITAL_ZOOM_MAGNITUDE, mMemZoomMagnitude.mHandle);
    CheckError(ret == false, ia_err_general, "@%s, requestSync fails", __func__);

    return ia_err_none;
}

void IntelDvs::freeMorphTable(ia_dvs_state* dvsHandle, ia_dvs_morph_table* morphTable) {
    LOGIPC("@%s, dvsHandle%p, morphTable:%p", __func__, dvsHandle, morphTable);
    CheckError(mInitialized == false, VOID_VALUE, "@%s, mInitialized is false", __func__);
    CheckError(dvsHandle == nullptr, VOID_VALUE, "@%s, dvsHandle is nullptr", __func__);
    CheckError(morphTable == nullptr, VOID_VALUE, "@%s, morphTable is nullptr", __func__);

    bool ret = mIpc.clientFlattenFreeMorphTable(mMemFreeMorph.mAddr, mMemFreeMorph.mSize, dvsHandle,
                                                morphTable);
    CheckError(ret == false, VOID_VALUE, "@%s, clientFlattenFreeMorphTable fails", __func__);

    ret = mCommon.requestSync(IPC_DVS_FREE_MORPH_TABLE, mMemFreeMorph.mHandle);
    CheckError(ret == false, VOID_VALUE, "@%s, requestSync fails", __func__);
}

ia_dvs_morph_table* IntelDvs::allocateMorphTalbe(ia_dvs_state* dvsHandle) {
    LOGIPC("@%s, dvsHandle:%p", __func__, dvsHandle);
    CheckError(mInitialized == false, nullptr, "@%s, mInitialized is false", __func__);
    CheckError(dvsHandle == nullptr, nullptr, "@%s, dvsHandle is nullptr", __func__);

    bool ret = mIpc.clientFlattenAllocateMorphTable(mMemAllocateMorph.mAddr,
                                                    mMemAllocateMorph.mSize, dvsHandle);
    CheckError(ret == false, nullptr, "@%s, clientFlattenAllocateMorphTable fails", __func__);

    ret = mCommon.requestSync(IPC_DVS_ALLOCATE_MORPH_TABLE, mMemAllocateMorph.mHandle);
    CheckError(ret == false, nullptr, "@%s, requestSync fails", __func__);

    ia_dvs_morph_table* morphTable = nullptr;
    ret = mIpc.clientUnflattenAllocateMorphTalbe(mMemAllocateMorph.mAddr, mMemAllocateMorph.mSize,
                                                 &morphTable);
    CheckError(ret == false, nullptr, "@%s, clientUnflattenAllocateMorphTalbe fails", __func__);

    return morphTable;
}

int IntelDvs::getMorphTable(ia_dvs_state* dvsHandle, ia_dvs_morph_table* morphTable,
                            DvsResult* result) {
    LOGIPC("@%s, dvsHandle:%p, morphTable:%p", __func__, dvsHandle, morphTable);
    CheckError(mInitialized == false, UNKNOWN_ERROR, "@%s, mInitialized is false", __func__);
    CheckError(dvsHandle == nullptr, UNKNOWN_ERROR, "@%s, dvsHandle is nullptr", __func__);
    CheckError(morphTable == nullptr, UNKNOWN_ERROR, "@%s, morphTable is nullptr", __func__);
    CheckError(result == nullptr, UNKNOWN_ERROR, "@%s, result is nullptr", __func__);

    bool ret = mIpc.clientFlattenGetMorphTable(mMemGetMorphTable.mAddr, mMemGetMorphTable.mSize,
                                               dvsHandle, morphTable);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, clientFlattenGetMorphTable fails", __func__);

    ret = mCommon.requestSync(IPC_DVS_GET_MORPH_TABLE, mMemGetMorphTable.mHandle);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, requestSync fails", __func__);

    ia_dvs_morph_table* morphTableTmp = nullptr;
    ret = mIpc.clientUnflattenGetMorphTalbe(mMemGetMorphTable.mAddr, mMemGetMorphTable.mSize,
                                            &morphTableTmp);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, clientUnflattenGetMorphTalbe fails", __func__);

    int err = DvsResult::deepCopyDvsResults(*morphTableTmp, &result->mMorphTable);
    CheckError(err != OK, UNKNOWN_ERROR, "@%s, deepCopyDvsResults fails", __func__);

    return OK;
}

ia_err IntelDvs::setStatistics(ia_dvs_state* dvsHandle, const ia_dvs_statistics* statistics,
                               const ia_aiq_ae_results* aeResults,
                               const ia_aiq_af_results* afResults,
                               const ia_aiq_sensor_events* sensorEvents, uint64_t frameReadoutStart,
                               uint64_t frameReadoutEnd) {
    LOGIPC("@%s, dvsHandle:%p", __func__, dvsHandle);
    CheckError(mInitialized == false, ia_err_general, "@%s, mInitialized is false", __func__);
    CheckError(dvsHandle == nullptr, ia_err_general, "@%s, dvsHandle is nullptr", __func__);

    bool ret = mIpc.clientFlattenSetStatistics(mMemStatistics.mAddr, mMemStatistics.mSize,
                                               dvsHandle, statistics, aeResults, afResults,
                                               sensorEvents, frameReadoutStart, frameReadoutEnd);
    CheckError(ret == false, ia_err_general, "@%s, clientFlattenSetStatistics fails", __func__);

    ret = mCommon.requestSync(IPC_DVS_SET_STATISTICS, mMemStatistics.mHandle);
    CheckError(ret == false, ia_err_general, "@%s, requestSync fails", __func__);

    return ia_err_none;
}

ia_err IntelDvs::execute(ia_dvs_state* dvsHandle, uint16_t focusPosition) {
    LOGIPC("@%s, dvsHandle:%p", __func__, dvsHandle);
    CheckError(mInitialized == false, ia_err_general, "@%s, mInitialized is false", __func__);
    CheckError(dvsHandle == nullptr, ia_err_general, "@%s, dvsHandle is nullptr", __func__);

    bool ret =
        mIpc.clientFlattenExecute(mMemExecute.mAddr, mMemExecute.mSize, dvsHandle, focusPosition);
    CheckError(ret == false, ia_err_general, "@%s, clientFlattenGetMorphTable fails", __func__);

    ret = mCommon.requestSync(IPC_DVS_EXECUTE, mMemExecute.mHandle);
    CheckError(ret == false, ia_err_general, "@%s, requestSync fails", __func__);

    return ia_err_none;
}

ia_err IntelDvs::getImageTransformation(ia_dvs_state* dvsHandle,
                                        ia_dvs_image_transformation* imageTransformation) {
    LOGIPC("@%s, dvsHandle:%p", __func__, dvsHandle);
    CheckError(mInitialized == false, ia_err_general, "@%s, mInitialized is false", __func__);
    CheckError(dvsHandle == nullptr, ia_err_general, "@%s, dvsHandle is nullptr", __func__);
    CheckError(imageTransformation == nullptr, ia_err_general,
               "@%s, imageTransformation is nullptr", __func__);

    bool ret = mIpc.clientFlattenImageTransformation(mMemImageTransfor.mAddr,
                                                     mMemImageTransfor.mSize, dvsHandle);
    CheckError(ret == false, ia_err_general, "@%s, clientFlattenImageTransformation fails",
               __func__);

    ret = mCommon.requestSync(IPC_DVS_GET_IMAGE_TRANSFORMATION, mMemImageTransfor.mHandle);
    CheckError(ret == false, ia_err_general, "@%s, requestSync fails", __func__);

    ia_dvs_image_transformation* info = nullptr;
    ret = mIpc.clientUnflattenImageTransformation(mMemImageTransfor.mAddr, mMemImageTransfor.mSize,
                                                  &info);
    CheckError(ret == false, ia_err_general, "@%s, clientUnflattenImageTransformation fails",
               __func__);

    MEMCPY_S(imageTransformation, sizeof(ia_dvs_image_transformation), info,
             sizeof(ia_dvs_image_transformation));
    return ia_err_none;
}
}  // namespace icamera
